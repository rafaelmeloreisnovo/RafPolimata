#!/usr/bin/env python3
"""
Internal Custody Ledger Verifier

Independent verification of tamper-evident ledger.
Does not trust the writer; verifies all constraints independently.
"""

from __future__ import annotations

import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any, Optional

# Genesis parent_event_id for first event
GENESIS_PARENT_ID = "GENESIS_RAFPOLIMATA_CUSTODY_LEDGER_INCEPTION_20260815"


def canonical_json_bytes(data: dict[str, Any]) -> bytes:
    """Recreate canonical JSON representation without event_id."""
    data_copy = {k: v for k, v in data.items() if k != "event_id"}
    json_str = json.dumps(data_copy, separators=(",", ":"), sort_keys=True, ensure_ascii=False)
    if not json_str.endswith("\n"):
        json_str += "\n"
    return json_str.encode("utf-8")


def verify_event_id(event: dict[str, Any], parent_event_id: str) -> bool:
    """Verify that event_id = SHA256(canonical_bytes || parent_event_id)."""
    canonical = canonical_json_bytes(event)
    combined = canonical + parent_event_id.encode("utf-8")
    expected_id = hashlib.sha256(combined).hexdigest()
    actual_id = event.get("event_id", "")
    return expected_id == actual_id


class LedgerVerifier:
    """Verifies internal custody ledger."""

    def __init__(self, ledger_path: Path | str, strict: bool = True):
        self.ledger_path = Path(ledger_path)
        self.strict = strict
        self.errors: list[str] = []
        self.warnings: list[str] = []

    def add_error(self, msg: str) -> None:
        self.errors.append(msg)

    def add_warning(self, msg: str) -> None:
        self.warnings.append(msg)

    def verify_schema(self, event: dict[str, Any], line_num: int) -> bool:
        """Verify event matches schema."""
        required = [
            "event_version",
            "event_id",
            "parent_event_id",
            "repository",
            "commit_sha",
            "path",
            "symbol",
            "exit_code",
            "timestamp_utc",
            "result",
        ]

        for field in required:
            if field not in event:
                self.add_error(f"Line {line_num}: missing required field '{field}'")
                return False

        # Validate event_version format
        version = event.get("event_version", "")
        if not re.match(r"^\d+\.\d+\.\d+$", str(version)):
            self.add_warning(f"Line {line_num}: event_version '{version}' doesn't match X.Y.Z format")

        # Validate event_id format
        event_id = event.get("event_id", "")
        if not re.match(r"^[a-f0-9]{64}$", str(event_id)):
            self.add_error(f"Line {line_num}: event_id '{event_id}' is not valid SHA256 hex")
            return False

        # Validate parent_event_id format
        parent_id = event.get("parent_event_id", "")
        if not (re.match(r"^[a-f0-9]{64}$", str(parent_id)) or parent_id == GENESIS_PARENT_ID):
            self.add_error(f"Line {line_num}: parent_event_id '{parent_id}' is invalid")
            return False

        # Validate result
        result = event.get("result", "")
        if result not in ("PASS", "FAIL", "TOKEN_VAZIO"):
            self.add_error(f"Line {line_num}: result '{result}' is not PASS/FAIL/TOKEN_VAZIO")
            return False

        # Validate exit_code is integer
        try:
            int(event.get("exit_code", 0))
        except (ValueError, TypeError):
            self.add_error(f"Line {line_num}: exit_code is not an integer")
            return False

        # Validate timestamp format (ISO 8601)
        timestamp = event.get("timestamp_utc", "")
        if not re.match(r"^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?Z?$", str(timestamp)):
            self.add_warning(f"Line {line_num}: timestamp_utc '{timestamp}' doesn't match ISO 8601")

        return True

    def verify_chain(self, events: list[dict[str, Any]]) -> bool:
        """Verify hash chain integrity."""
        if not events:
            return True

        # First event must have genesis parent
        first_event = events[0]
        if first_event.get("parent_event_id") != GENESIS_PARENT_ID:
            self.add_error(f"Line 1: first event parent_event_id must be {GENESIS_PARENT_ID}")
            return False

        # Verify first event's event_id
        if not verify_event_id(first_event, GENESIS_PARENT_ID):
            self.add_error("Line 1: first event_id verification failed")
            return False

        # Verify remaining events
        for i in range(1, len(events)):
            event = events[i]
            prev_event = events[i - 1]
            line_num = i + 1

            expected_parent = prev_event.get("event_id")
            actual_parent = event.get("parent_event_id")

            if expected_parent != actual_parent:
                self.add_error(
                    f"Line {line_num}: parent_event_id mismatch: expected {expected_parent}, "
                    f"got {actual_parent}"
                )
                return False

            if not verify_event_id(event, actual_parent):
                self.add_error(f"Line {line_num}: event_id verification failed")
                return False

        return True

    def verify_ledger(self) -> bool:
        """Verify entire ledger for tampering, broken chains, etc."""
        if not self.ledger_path.exists():
            self.add_error(f"Ledger file not found: {self.ledger_path}")
            return False

        try:
            content = self.ledger_path.read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError) as e:
            self.add_error(f"Failed to read ledger: {e}")
            return False

        if not content.strip():
            # Empty ledger is valid
            return True

        events: list[dict[str, Any]] = []
        lines = content.strip().split("\n")

        # Parse all events
        for i, line in enumerate(lines, 1):
            if not line.strip():
                continue
            try:
                event = json.loads(line)
                if not isinstance(event, dict):
                    self.add_error(f"Line {i}: not a JSON object")
                    return False
                events.append(event)
            except json.JSONDecodeError as e:
                self.add_error(f"Line {i}: JSON parse error: {e}")
                return False

        # Verify each event's schema
        for i, event in enumerate(events, 1):
            if not self.verify_schema(event, i):
                if self.strict:
                    return False

        # Verify hash chain
        if not self.verify_chain(events):
            return False

        return True

    def report(self) -> str:
        """Generate verification report."""
        lines = []

        if self.errors:
            lines.append("ERRORS:")
            for error in self.errors:
                lines.append(f"  {error}")

        if self.warnings:
            lines.append("WARNINGS:")
            for warning in self.warnings:
                lines.append(f"  {warning}")

        if not self.errors and not self.warnings:
            lines.append("✓ Ledger verification PASSED")

        return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    import argparse

    parser = argparse.ArgumentParser(description="Verify internal custody ledger")
    parser.add_argument("ledger", help="Path to JSONL ledger file")
    parser.add_argument("--strict", action="store_true", help="Fail on any warning")

    args = parser.parse_args(argv)

    verifier = LedgerVerifier(args.ledger, strict=args.strict)
    valid = verifier.verify_ledger()

    print(verifier.report())

    return 0 if valid else 1


if __name__ == "__main__":
    sys.exit(main())
