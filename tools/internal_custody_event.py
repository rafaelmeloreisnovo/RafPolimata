#!/usr/bin/env python3
"""
Internal Custody Event Writer

Deterministic, tamper-evident append-only ledger for RafPolimata transformations.

Chain relation:
    E_n = SHA256(canonical(E_n without event_id) || E_(n-1).event_id)

The writer serializes parent selection + append under a POSIX advisory lock and
updates a sidecar manifest atomically. The manifest anchors event_count and
head_event_id so deletion of the terminal event is detectable unless both
ledger and manifest are altered together.
"""

from __future__ import annotations

import argparse
import fcntl
import hashlib
import json
import os
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

GENESIS_PARENT_ID = "GENESIS_RAFPOLIMATA_CUSTODY_LEDGER_INCEPTION_20260815"
EVENT_VERSION = "1.1.0"
TOKEN_VAZIO = "TOKEN_VAZIO"


def sha256_file(path: Path | str) -> str:
    """Hash a file in bounded-memory streaming mode."""
    sha = hashlib.sha256()
    try:
        with open(path, "rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                sha.update(chunk)
        return sha.hexdigest()
    except (OSError, IOError):
        return TOKEN_VAZIO


def sha256_data(data: str | bytes) -> str:
    if isinstance(data, str):
        data = data.encode("utf-8")
    return hashlib.sha256(data).hexdigest()


def git_commit_sha(repo_path: Path | str = ".") -> str:
    try:
        result = subprocess.run(
            ["git", "-C", str(repo_path), "rev-parse", "HEAD"],
            capture_output=True,
            text=True,
            timeout=5,
            check=False,
        )
        if result.returncode == 0:
            value = result.stdout.strip()
            if value:
                return value
    except (subprocess.TimeoutExpired, FileNotFoundError, OSError):
        pass
    return TOKEN_VAZIO


def git_blob_sha(repo_path: Path | str, rel_path: str) -> str:
    try:
        result = subprocess.run(
            ["git", "-C", str(repo_path), "rev-parse", f"HEAD:{rel_path}"],
            capture_output=True,
            text=True,
            timeout=5,
            check=False,
        )
        if result.returncode == 0:
            value = result.stdout.strip()
            if value:
                return value
    except (subprocess.TimeoutExpired, FileNotFoundError, OSError):
        pass
    return TOKEN_VAZIO


def _first_version_line(command: list[str]) -> str:
    try:
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            timeout=5,
            check=False,
        )
        if result.returncode == 0:
            output = result.stdout or result.stderr
            line = output.splitlines()[0].strip() if output else ""
            return line or TOKEN_VAZIO
    except (subprocess.TimeoutExpired, FileNotFoundError, OSError):
        pass
    return TOKEN_VAZIO


def get_toolchain_info() -> dict[str, str]:
    compiler = TOKEN_VAZIO
    for candidate in ("gcc", "clang", "cc"):
        compiler = _first_version_line([candidate, "--version"])
        if compiler != TOKEN_VAZIO:
            break
    linker = TOKEN_VAZIO
    for candidate in ("ld", "lld"):
        linker = _first_version_line([candidate, "--version"])
        if linker != TOKEN_VAZIO:
            break
    return {
        "compiler": compiler,
        "linker": linker,
        "platform": sys.platform or TOKEN_VAZIO,
    }


def canonical_json_bytes(data: dict[str, Any]) -> bytes:
    """Canonical UTF-8 JSON: sorted keys, compact separators, exactly one LF."""
    payload = {key: value for key, value in data.items() if key != "event_id"}
    text = json.dumps(
        payload,
        separators=(",", ":"),
        sort_keys=True,
        ensure_ascii=False,
        allow_nan=False,
    )
    return (text + "\n").encode("utf-8")


def calculate_event_id(event_data: dict[str, Any], parent_event_id: str) -> str:
    return hashlib.sha256(
        canonical_json_bytes(event_data) + parent_event_id.encode("utf-8")
    ).hexdigest()


def manifest_path_for(ledger_path: Path | str) -> Path:
    path = Path(ledger_path)
    return path.with_name(path.name + ".manifest.json")


def _parse_events_from_text(text: str) -> list[dict[str, Any]]:
    if not text:
        return []
    if not text.endswith("\n"):
        raise ValueError("ledger is truncated: missing final newline")
    events: list[dict[str, Any]] = []
    for line_number, line in enumerate(text.splitlines(), 1):
        if not line:
            raise ValueError(f"ledger contains blank line at {line_number}")
        value = json.loads(line)
        if not isinstance(value, dict):
            raise ValueError(f"ledger line {line_number} is not an object")
        events.append(value)
    return events


def _atomic_write_json(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_name(f".{path.name}.{os.getpid()}.tmp")
    data = (
        json.dumps(payload, separators=(",", ":"), sort_keys=True, ensure_ascii=False)
        + "\n"
    ).encode("utf-8")
    with open(temp, "wb") as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())
    os.replace(temp, path)


class CustodyLedger:
    def __init__(self, ledger_path: Path | str):
        self.ledger_path = Path(ledger_path)
        self.manifest_path = manifest_path_for(self.ledger_path)
        self.lock_path = self.ledger_path.with_name(self.ledger_path.name + ".lock")

    def load_last_event(self) -> dict[str, Any] | None:
        if not self.ledger_path.exists():
            return None
        try:
            events = _parse_events_from_text(
                self.ledger_path.read_text(encoding="utf-8")
            )
            return events[-1] if events else None
        except (OSError, UnicodeDecodeError, json.JSONDecodeError, ValueError):
            return None

    def append_event(self, event: dict[str, Any]) -> dict[str, Any] | None:
        self.ledger_path.parent.mkdir(parents=True, exist_ok=True)
        try:
            with open(self.lock_path, "a+", encoding="utf-8") as lock_stream:
                fcntl.flock(lock_stream.fileno(), fcntl.LOCK_EX)
                try:
                    if self.ledger_path.exists():
                        text = self.ledger_path.read_text(encoding="utf-8")
                    else:
                        text = ""
                    events = _parse_events_from_text(text)
                    parent_event_id = (
                        events[-1]["event_id"] if events else GENESIS_PARENT_ID
                    )

                    candidate = dict(event)
                    candidate["event_version"] = candidate.get(
                        "event_version", EVENT_VERSION
                    )
                    candidate["parent_event_id"] = parent_event_id
                    candidate["timestamp_utc"] = candidate.get(
                        "timestamp_utc",
                        datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
                    )
                    candidate["event_id"] = calculate_event_id(
                        candidate, parent_event_id
                    )

                    line = (
                        json.dumps(
                            candidate,
                            separators=(",", ":"),
                            sort_keys=True,
                            ensure_ascii=False,
                            allow_nan=False,
                        )
                        + "\n"
                    )
                    with open(self.ledger_path, "a", encoding="utf-8") as ledger:
                        ledger.write(line)
                        ledger.flush()
                        os.fsync(ledger.fileno())

                    manifest = {
                        "manifest_version": "1.0.0",
                        "ledger": self.ledger_path.name,
                        "event_count": len(events) + 1,
                        "head_event_id": candidate["event_id"],
                        "ledger_sha256": sha256_file(self.ledger_path),
                    }
                    _atomic_write_json(self.manifest_path, manifest)
                    return candidate
                finally:
                    fcntl.flock(lock_stream.fileno(), fcntl.LOCK_UN)
        except (
            OSError,
            UnicodeDecodeError,
            json.JSONDecodeError,
            ValueError,
            TypeError,
        ) as exc:
            print(f"ERROR: custody append failed: {exc}", file=sys.stderr)
            return None


def record_event(
    ledger_path: Path | str,
    repository: str,
    path: str,
    symbol: str,
    result: str,
    input_file: Path | str | None = None,
    output_file: Path | str | None = None,
    stdout_file: Path | str | None = None,
    stderr_file: Path | str | None = None,
    exit_code: int = 0,
    parameters: dict[str, Any] | None = None,
    seed: str = TOKEN_VAZIO,
    repo_root: Path | str = ".",
    timestamp_utc: str | None = None,
) -> dict[str, Any] | None:
    if result not in {"PASS", "FAIL", TOKEN_VAZIO}:
        raise ValueError("result must be PASS, FAIL, or TOKEN_VAZIO")

    root = Path(repo_root)
    artifact = Path(path)
    if artifact.is_absolute():
        try:
            rel_path = artifact.relative_to(root)
        except ValueError:
            rel_path = None
    else:
        rel_path = artifact

    event: dict[str, Any] = {
        "blob_sha": git_blob_sha(root, str(rel_path)) if rel_path else TOKEN_VAZIO,
        "commit_sha": git_commit_sha(root),
        "environment": {
            "PWD": os.environ.get("PWD", TOKEN_VAZIO),
            "USER": os.environ.get("USER", TOKEN_VAZIO),
        },
        "exit_code": int(exit_code),
        "input_sha256": sha256_file(input_file) if input_file else TOKEN_VAZIO,
        "output_sha256": sha256_file(output_file) if output_file else TOKEN_VAZIO,
        "parameters": parameters if parameters is not None else {"state": TOKEN_VAZIO},
        "path": path,
        "repository": repository,
        "result": result,
        "seed": seed or TOKEN_VAZIO,
        "stderr_hash": sha256_file(stderr_file) if stderr_file else TOKEN_VAZIO,
        "stdout_hash": sha256_file(stdout_file) if stdout_file else TOKEN_VAZIO,
        "symbol": symbol,
        "toolchain": get_toolchain_info(),
    }
    if timestamp_utc is not None:
        event["timestamp_utc"] = timestamp_utc
    return CustodyLedger(ledger_path).append_event(event)


def _load_parameters(raw: str | None) -> dict[str, Any] | None:
    if raw is None:
        return None
    value = json.loads(raw)
    if not isinstance(value, dict):
        raise ValueError("--parameters-json must decode to an object")
    return value


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Record custody event to ledger")
    parser.add_argument("--ledger", required=True)
    parser.add_argument("--repository", required=True)
    parser.add_argument("--path", required=True)
    parser.add_argument("--symbol", required=True)
    parser.add_argument("--result", required=True, choices=["PASS", "FAIL", TOKEN_VAZIO])
    parser.add_argument("--exit-code", type=int, default=0)
    parser.add_argument("--input")
    parser.add_argument("--output")
    parser.add_argument("--stdout-file")
    parser.add_argument("--stderr-file")
    parser.add_argument("--seed", default=TOKEN_VAZIO)
    parser.add_argument("--parameters-json")
    parser.add_argument("--repo-root", default=".")
    args = parser.parse_args(argv)
    try:
        parameters = _load_parameters(args.parameters_json)
        result = record_event(
            ledger_path=args.ledger,
            repository=args.repository,
            path=args.path,
            symbol=args.symbol,
            result=args.result,
            exit_code=args.exit_code,
            input_file=args.input,
            output_file=args.output,
            stdout_file=args.stdout_file,
            stderr_file=args.stderr_file,
            seed=args.seed,
            parameters=parameters,
            repo_root=args.repo_root,
        )
    except (ValueError, json.JSONDecodeError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    if result is None:
        return 1
    print(json.dumps(result, indent=2, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
