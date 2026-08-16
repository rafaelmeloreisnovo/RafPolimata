#!/usr/bin/env python3
"""Run one hosted gate and append its execution receipt to the custody ledger.

CLOSURE_L0 / CLOSURE_L1 integration adapter: the wrapped command remains the
source of truth for its exit status. This adapter captures stdout/stderr,
records hashes/parameters/environment through ``internal_custody_event.py``,
and returns the original command status. It never turns a failing gate green.

This is intentionally hosted Python. It is not linked into freestanding C or
Android targets.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

from internal_custody_event import TOKEN_VAZIO, record_event


def _load_object(raw: str | None) -> dict[str, Any]:
    if raw is None:
        return {}
    value = json.loads(raw)
    if not isinstance(value, dict):
        raise ValueError("--parameters-json must decode to an object")
    return value


def _emit_capture(path: Path, stream: Any) -> None:
    try:
        stream.buffer.write(path.read_bytes())
        stream.buffer.flush()
    except (AttributeError, OSError):
        try:
            stream.write(path.read_text(encoding="utf-8", errors="replace"))
            stream.flush()
        except OSError:
            pass


def run(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Execute a command and append a tamper-evident custody event"
    )
    parser.add_argument("--ledger", required=True)
    parser.add_argument("--repository", required=True)
    parser.add_argument("--path", required=True)
    parser.add_argument("--symbol", required=True)
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--cwd")
    parser.add_argument("--input")
    parser.add_argument("--output")
    parser.add_argument("--stdin-file")
    parser.add_argument("--seed", default=TOKEN_VAZIO)
    parser.add_argument("--parameters-json")
    parser.add_argument(
        "--token-vazio-marker",
        action="append",
        default=[],
        help="If command exits 0 but capture contains this marker, record TOKEN_VAZIO",
    )
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args(argv)

    command = list(args.command)
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        print("ERROR: missing wrapped command", file=sys.stderr)
        return 2

    repo_root = Path(args.repo_root).resolve()
    cwd = Path(args.cwd).resolve() if args.cwd else repo_root
    ledger = Path(args.ledger)
    if not ledger.is_absolute():
        ledger = repo_root / ledger

    try:
        parameters = _load_object(args.parameters_json)
    except (ValueError, json.JSONDecodeError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2

    parameters = dict(parameters)
    parameters["command"] = command
    parameters["cwd"] = str(cwd)
    parameters["adapter"] = "tools/run_with_internal_custody.py"

    capture_dir = ledger.parent / "captures"
    capture_dir.mkdir(parents=True, exist_ok=True)
    safe_symbol = "".join(ch if ch.isalnum() or ch in "._-" else "_" for ch in args.symbol)
    stdout_path = capture_dir / f"{safe_symbol}.stdout"
    stderr_path = capture_dir / f"{safe_symbol}.stderr"

    input_stream = None
    command_rc = 127
    command_missing = False
    try:
        if args.stdin_file:
            input_stream = open(args.stdin_file, "rb")
        with open(stdout_path, "wb") as stdout_stream, open(stderr_path, "wb") as stderr_stream:
            try:
                completed = subprocess.run(
                    command,
                    cwd=cwd,
                    stdin=input_stream,
                    stdout=stdout_stream,
                    stderr=stderr_stream,
                    check=False,
                )
                command_rc = int(completed.returncode)
            except FileNotFoundError as exc:
                command_missing = True
                stderr_stream.write(f"command_not_found: {exc}\n".encode("utf-8", "replace"))
                command_rc = 127
            except OSError as exc:
                stderr_stream.write(f"command_os_error: {exc}\n".encode("utf-8", "replace"))
                command_rc = 126
    finally:
        if input_stream is not None:
            input_stream.close()

    if command_missing:
        result = TOKEN_VAZIO
    elif command_rc != 0:
        result = "FAIL"
    else:
        capture = stdout_path.read_bytes() + b"\n" + stderr_path.read_bytes()
        markers = [marker.encode("utf-8") for marker in args.token_vazio_marker]
        result = TOKEN_VAZIO if any(marker in capture for marker in markers) else "PASS"

    event = record_event(
        ledger_path=ledger,
        repository=args.repository,
        path=args.path,
        symbol=args.symbol,
        result=result,
        input_file=args.input,
        output_file=args.output,
        stdout_file=stdout_path,
        stderr_file=stderr_path,
        exit_code=command_rc,
        parameters=parameters,
        seed=args.seed,
        repo_root=repo_root,
    )

    _emit_capture(stdout_path, sys.stdout)
    _emit_capture(stderr_path, sys.stderr)

    if event is None:
        print("ERROR: wrapped command executed but custody event could not be persisted", file=sys.stderr)
        return 125

    print(
        json.dumps(
            {
                "custody_event_id": event["event_id"],
                "symbol": args.symbol,
                "result": event["result"],
                "exit_code": command_rc,
            },
            sort_keys=True,
        )
    )
    return command_rc


if __name__ == "__main__":
    raise SystemExit(run())
