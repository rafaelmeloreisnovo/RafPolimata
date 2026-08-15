#!/usr/bin/env python3
"""
Internal Custody Event Writer

Implements deterministic, tamper-evident ledger for RafPolimata transformations.
Records each event with hash chaining: E_n = SHA256(canonical(E_n) || E_(n-1).event_id)
"""

from __future__ import annotations

import hashlib
import json
import os
import re
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Optional


# Genesis parent_event_id for first event
GENESIS_PARENT_ID = "GENESIS_RAFPOLIMATA_CUSTODY_LEDGER_INCEPTION_20260815"


def sha256_file(path: Path | str) -> str:
    """Calculate SHA256 hash of file with streaming to handle large files."""
    sha = hashlib.sha256()
    try:
        with open(path, "rb") as f:
            while True:
                chunk = f.read(65536)
                if not chunk:
                    break
                sha.update(chunk)
        return sha.hexdigest()
    except (OSError, IOError):
        return "TOKEN_VAZIO"


def sha256_data(data: str | bytes) -> str:
    """Calculate SHA256 hash of string/bytes data."""
    if isinstance(data, str):
        data = data.encode("utf-8")
    return hashlib.sha256(data).hexdigest()


def git_commit_sha(repo_path: Path | str = ".") -> str:
    """Get current git commit SHA."""
    try:
        result = subprocess.run(
            ["git", "-C", str(repo_path), "rev-parse", "HEAD"],
            capture_output=True,
            text=True,
            timeout=5,
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except (subprocess.TimeoutExpired, FileNotFoundError):
        pass
    return "TOKEN_VAZIO"


def git_blob_sha(repo_path: Path | str, rel_path: str) -> str:
    """Get git blob SHA for a file in the repository."""
    try:
        result = subprocess.run(
            ["git", "-C", str(repo_path), "hash-object", rel_path],
            capture_output=True,
            text=True,
            timeout=5,
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except (subprocess.TimeoutExpired, FileNotFoundError):
        pass
    return "TOKEN_VAZIO"


def get_toolchain_info() -> dict[str, str]:
    """Capture toolchain versions."""
    toolchain = {}

    # Try to get compiler version
    for compiler in ["gcc", "clang", "cc"]:
        try:
            result = subprocess.run(
                [compiler, "--version"],
                capture_output=True,
                text=True,
                timeout=5,
            )
            if result.returncode == 0:
                first_line = result.stdout.split("\n")[0]
                toolchain["compiler"] = first_line
                break
        except (subprocess.TimeoutExpired, FileNotFoundError):
            pass

    # Try to get linker version
    for linker in ["ld", "lld"]:
        try:
            result = subprocess.run(
                [linker, "--version"],
                capture_output=True,
                text=True,
                timeout=5,
            )
            if result.returncode == 0:
                first_line = result.stdout.split("\n")[0]
                toolchain["linker"] = first_line
                break
        except (subprocess.TimeoutExpired, FileNotFoundError):
            pass

    # Capture platform
    toolchain["platform"] = sys.platform

    return toolchain


def canonical_json_bytes(data: dict[str, Any]) -> bytes:
    """
    Serialize dict to canonical JSON bytes:
    - UTF-8 encoding
    - Lexically sorted keys
    - No extraneous whitespace
    - Stable number representation
    - Explicit null/TOKEN_VAZIO handling
    - Final newline
    """
    # Remove event_id if present (it's calculated separately)
    data_copy = {k: v for k, v in data.items() if k != "event_id"}

    # Serialize with sorted keys, no spaces
    json_str = json.dumps(data_copy, separators=(",", ":"), sort_keys=True, ensure_ascii=False)

    # Add final newline (required for determinism)
    if not json_str.endswith("\n"):
        json_str += "\n"

    return json_str.encode("utf-8")


def calculate_event_id(event_data: dict[str, Any], parent_event_id: str) -> str:
    """
    Calculate event_id = SHA256(canonical_bytes || parent_event_id)
    """
    canonical = canonical_json_bytes(event_data)
    combined = canonical + parent_event_id.encode("utf-8")
    return hashlib.sha256(combined).hexdigest()


class CustodyLedger:
    """Manages internal custody event ledger."""

    def __init__(self, ledger_path: Path | str):
        self.ledger_path = Path(ledger_path)

    def load_last_event(self) -> dict[str, Any] | None:
        """Load the last event from ledger, or None if ledger is empty."""
        if not self.ledger_path.exists():
            return None

        try:
            lines = self.ledger_path.read_text(encoding="utf-8").strip().split("\n")
            if not lines or not lines[-1].strip():
                return None
            return json.loads(lines[-1])
        except (json.JSONDecodeError, OSError, ValueError):
            return None

    def append_event(self, event: dict[str, Any]) -> dict[str, Any] | None:
        """
        Append an event to the ledger with hash chaining.
        Returns the event with event_id set, or None on error.
        """
        self.ledger_path.parent.mkdir(parents=True, exist_ok=True)

        # Get parent event_id
        last_event = self.load_last_event()
        if last_event is None:
            parent_event_id = GENESIS_PARENT_ID
        else:
            parent_event_id = last_event.get("event_id", "TOKEN_VAZIO")

        # Set required fields if not present
        event.setdefault("event_version", "1.0.0")
        event.setdefault("parent_event_id", parent_event_id)
        # Use ISO 8601 UTC format with Z suffix
        now_utc = datetime.now(timezone.utc)
        iso_timestamp = now_utc.strftime("%Y-%m-%dT%H:%M:%SZ")
        event.setdefault("timestamp_utc", iso_timestamp)

        # Calculate event_id
        event_id = calculate_event_id(event, parent_event_id)
        event["event_id"] = event_id

        # Write event as JSON line (atomic append)
        try:
            with open(self.ledger_path, "a", encoding="utf-8") as f:
                f.write(json.dumps(event, separators=(",", ":"), sort_keys=True, ensure_ascii=False))
                f.write("\n")
            return event
        except OSError as e:
            print(f"ERROR: Failed to write custody event: {e}", file=sys.stderr)
            return None


def record_event(
    ledger_path: Path | str,
    repository: str,
    path: str,
    symbol: str,
    result: str,
    input_file: Path | str | None = None,
    output_file: Path | str | None = None,
    exit_code: int = 0,
    parameters: dict[str, Any] | None = None,
    repo_root: Path | str = ".",
) -> dict[str, Any] | None:
    """
    Record a custody event in the ledger.

    Args:
        ledger_path: Path to JSONL ledger file
        repository: Repository name
        path: Path of artifact being transformed
        symbol: Identifier for the operation
        result: PASS | FAIL | TOKEN_VAZIO
        input_file: Optional path to input file (will hash)
        output_file: Optional path to output file (will hash)
        exit_code: Exit/result code
        parameters: Optional dict of operation parameters
        repo_root: Repository root for git operations

    Returns:
        The recorded event dict with event_id, or None on error
    """
    ledger = CustodyLedger(ledger_path)

    event: dict[str, Any] = {
        "commit_sha": git_commit_sha(repo_root),
        "environment": {
            "USER": os.environ.get("USER", "TOKEN_VAZIO"),
            "PWD": os.environ.get("PWD", "TOKEN_VAZIO"),
        },
        "exit_code": exit_code,
        "input_sha256": sha256_file(input_file) if input_file else "TOKEN_VAZIO",
        "output_sha256": sha256_file(output_file) if output_file else "TOKEN_VAZIO",
        "parameters": parameters or {},
        "path": path,
        "repository": repository,
        "result": result,
        "seed": "TOKEN_VAZIO",
        "stderr_hash": "TOKEN_VAZIO",
        "stdout_hash": "TOKEN_VAZIO",
        "symbol": symbol,
        "toolchain": get_toolchain_info(),
    }

    # Try to get blob SHA if file is in repo
    if Path(repo_root).is_dir():
        try:
            rel_path = Path(path).relative_to(repo_root)
            event["blob_sha"] = git_blob_sha(repo_root, str(rel_path))
        except (ValueError, OSError):
            event["blob_sha"] = "TOKEN_VAZIO"
    else:
        event["blob_sha"] = "TOKEN_VAZIO"

    return ledger.append_event(event)


if __name__ == "__main__":
    # Example usage
    import argparse

    parser = argparse.ArgumentParser(description="Record custody event to ledger")
    parser.add_argument("--ledger", required=True, help="Path to JSONL ledger file")
    parser.add_argument("--repository", required=True, help="Repository name")
    parser.add_argument("--path", required=True, help="Path of artifact")
    parser.add_argument("--symbol", required=True, help="Operation symbol")
    parser.add_argument("--result", required=True, choices=["PASS", "FAIL", "TOKEN_VAZIO"])
    parser.add_argument("--exit-code", type=int, default=0)
    parser.add_argument("--input", help="Input file path")
    parser.add_argument("--output", help="Output file path")
    parser.add_argument("--repo-root", default=".", help="Repository root")

    args = parser.parse_args()

    result = record_event(
        ledger_path=args.ledger,
        repository=args.repository,
        path=args.path,
        symbol=args.symbol,
        result=args.result,
        exit_code=args.exit_code,
        input_file=args.input,
        output_file=args.output,
        repo_root=args.repo_root,
    )

    if result:
        print(json.dumps(result, indent=2, ensure_ascii=False))
        sys.exit(0)
    else:
        sys.exit(1)
