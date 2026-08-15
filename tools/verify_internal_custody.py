#!/usr/bin/env python3
"""Independent verifier for the RafPolimata internal custody ledger."""
from __future__ import annotations
import argparse
import hashlib
import json
import re
from pathlib import Path
from typing import Any

GENESIS_PARENT_ID = "GENESIS_RAFPOLIMATA_CUSTODY_LEDGER_INCEPTION_20260815"
TOKEN_VAZIO = "TOKEN_VAZIO"
REQUIRED_FIELDS = (
    "event_version", "event_id", "parent_event_id", "repository", "commit_sha",
    "path", "blob_sha", "symbol", "toolchain", "parameters", "seed",
    "environment", "input_sha256", "output_sha256", "stdout_hash", "stderr_hash",
    "exit_code", "timestamp_utc", "result",
)

def sha256_file(path: Path | str) -> str:
    sha = hashlib.sha256()
    try:
        with open(path, "rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                sha.update(chunk)
        return sha.hexdigest()
    except OSError:
        return TOKEN_VAZIO

def canonical_json_bytes(data: dict[str, Any]) -> bytes:
    payload = {k: v for k, v in data.items() if k != "event_id"}
    text = json.dumps(payload, separators=(",", ":"), sort_keys=True, ensure_ascii=False, allow_nan=False)
    return (text + "\n").encode("utf-8")

def verify_event_id(event: dict[str, Any], parent_event_id: str) -> bool:
    expected = hashlib.sha256(canonical_json_bytes(event) + parent_event_id.encode("utf-8")).hexdigest()
    return event.get("event_id") == expected

def manifest_path_for(ledger_path: Path | str) -> Path:
    path = Path(ledger_path)
    return path.with_name(path.name + ".manifest.json")

class LedgerVerifier:
    def __init__(self, ledger_path: Path | str, strict: bool = True, manifest_path: Path | str | None = None, artifact_root: Path | str | None = None):
        self.ledger_path = Path(ledger_path)
        self.strict = strict
        self.manifest_path = Path(manifest_path) if manifest_path is not None else manifest_path_for(self.ledger_path)
        self.artifact_root = Path(artifact_root) if artifact_root else None
        self.errors: list[str] = []
        self.warnings: list[str] = []
        self.events: list[dict[str, Any]] = []

    def add_error(self, message: str) -> None: self.errors.append(message)
    def add_warning(self, message: str) -> None: self.warnings.append(message)

    def verify_schema(self, event: dict[str, Any], line_num: int) -> bool:
        ok = True
        for field in REQUIRED_FIELDS:
            if field not in event:
                self.add_error(f"Line {line_num}: missing required field '{field}'")
                ok = False
        if not ok: return False
        extra = sorted(set(event) - set(REQUIRED_FIELDS))
        if extra:
            self.add_error(f"Line {line_num}: unknown fields: {extra}")
            ok = False
        patterns = {
            "event_version": r"^\d+\.\d+\.\d+$",
            "event_id": r"^[a-f0-9]{64}$",
            "commit_sha": r"^(?:[a-f0-9]{40}|TOKEN_VAZIO)$",
            "blob_sha": r"^(?:[a-f0-9]{40}|TOKEN_VAZIO)$",
            "input_sha256": r"^(?:[a-f0-9]{64}|TOKEN_VAZIO)$",
            "output_sha256": r"^(?:[a-f0-9]{64}|TOKEN_VAZIO)$",
            "stdout_hash": r"^(?:[a-f0-9]{64}|TOKEN_VAZIO)$",
            "stderr_hash": r"^(?:[a-f0-9]{64}|TOKEN_VAZIO)$",
            "timestamp_utc": r"^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$",
        }
        for field, pattern in patterns.items():
            if not re.fullmatch(pattern, str(event.get(field, ""))):
                self.add_error(f"Line {line_num}: invalid {field}")
                ok = False
        parent = str(event["parent_event_id"])
        if parent != GENESIS_PARENT_ID and not re.fullmatch(r"[a-f0-9]{64}", parent):
            self.add_error(f"Line {line_num}: invalid parent_event_id"); ok = False
        if event["result"] not in {"PASS", "FAIL", TOKEN_VAZIO}:
            self.add_error(f"Line {line_num}: invalid result"); ok = False
        if type(event["exit_code"]) is not int:
            self.add_error(f"Line {line_num}: exit_code must be integer"); ok = False
        if not isinstance(event["toolchain"], dict): self.add_error(f"Line {line_num}: toolchain must be object"); ok = False
        if not isinstance(event["parameters"], dict): self.add_error(f"Line {line_num}: parameters must be object"); ok = False
        if not isinstance(event["environment"], dict): self.add_error(f"Line {line_num}: environment must be object"); ok = False
        return ok

    def verify_chain(self) -> bool:
        if not self.events: return True
        if self.events[0]["parent_event_id"] != GENESIS_PARENT_ID:
            self.add_error("Line 1: first event does not use genesis parent"); return False
        for index, event in enumerate(self.events):
            expected_parent = GENESIS_PARENT_ID if index == 0 else self.events[index - 1]["event_id"]
            if event["parent_event_id"] != expected_parent:
                self.add_error(f"Line {index + 1}: parent_event_id mismatch"); return False
            if not verify_event_id(event, expected_parent):
                self.add_error(f"Line {index + 1}: event_id verification failed"); return False
        return True

    def verify_manifest(self) -> bool:
        if not self.manifest_path.exists():
            self.add_error(f"Manifest file not found: {self.manifest_path}"); return False
        try: manifest = json.loads(self.manifest_path.read_text(encoding="utf-8"))
        except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
            self.add_error(f"Manifest parse error: {exc}"); return False
        expected_count = len(self.events)
        expected_head = self.events[-1]["event_id"] if self.events else TOKEN_VAZIO
        if manifest.get("event_count") != expected_count:
            self.add_error(f"Manifest event_count mismatch: expected {expected_count}, got {manifest.get('event_count')}")
        if expected_count and manifest.get("head_event_id") != expected_head: self.add_error("Manifest head_event_id mismatch")
        if manifest.get("ledger_sha256") != sha256_file(self.ledger_path): self.add_error("Manifest ledger_sha256 mismatch")
        return not self.errors

    def verify_artifacts(self) -> bool:
        if self.artifact_root is None: return True
        root = self.artifact_root.resolve()
        for line_num, event in enumerate(self.events, 1):
            candidate = (root / event["path"]).resolve()
            try: candidate.relative_to(root)
            except ValueError:
                self.add_error(f"Line {line_num}: artifact path escapes root"); continue
            if not candidate.is_file(): continue
            expected = event["output_sha256"] if event["output_sha256"] != TOKEN_VAZIO else event["input_sha256"]
            if expected == TOKEN_VAZIO: continue
            if sha256_file(candidate) != expected: self.add_error(f"Line {line_num}: artifact SHA-256 mismatch")
        return not self.errors

    def verify_ledger(self) -> bool:
        if not self.ledger_path.exists(): self.add_error(f"Ledger file not found: {self.ledger_path}"); return False
        try: content = self.ledger_path.read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError) as exc: self.add_error(f"Failed to read ledger: {exc}"); return False
        if not content:
            self.events = []; return self.verify_manifest()
        if not content.endswith("\n"): self.add_error("Ledger is truncated: missing final newline"); return False
        for line_num, line in enumerate(content.splitlines(), 1):
            if not line: self.add_error(f"Line {line_num}: blank line is not allowed"); return False
            try: event = json.loads(line)
            except json.JSONDecodeError as exc: self.add_error(f"Line {line_num}: JSON parse error: {exc}"); return False
            if not isinstance(event, dict): self.add_error(f"Line {line_num}: not a JSON object"); return False
            self.events.append(event)
        for line_num, event in enumerate(self.events, 1):
            if not self.verify_schema(event, line_num): return False
        return self.verify_chain() and self.verify_manifest() and self.verify_artifacts() and not self.errors

    def report(self) -> str:
        if not self.errors and not self.warnings: return "PASS: internal custody ledger verified"
        lines: list[str] = []
        if self.errors: lines += ["ERRORS:"] + [f"  {x}" for x in self.errors]
        if self.warnings: lines += ["WARNINGS:"] + [f"  {x}" for x in self.warnings]
        return "\n".join(lines)

def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Verify internal custody ledger")
    parser.add_argument("ledger"); parser.add_argument("--manifest"); parser.add_argument("--artifact-root"); parser.add_argument("--strict", action="store_true"); parser.add_argument("--report")
    args = parser.parse_args(argv)
    verifier = LedgerVerifier(args.ledger, strict=args.strict, manifest_path=args.manifest, artifact_root=args.artifact_root)
    valid = verifier.verify_ledger(); report = verifier.report(); print(report)
    if args.report:
        Path(args.report).parent.mkdir(parents=True, exist_ok=True); Path(args.report).write_text(report + "\n", encoding="utf-8")
    return 0 if valid else 1

if __name__ == "__main__": raise SystemExit(main())
