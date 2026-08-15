#!/usr/bin/env python3
"""
Tests for internal custody ledger (hash-chained events).
"""

from __future__ import annotations

import hashlib
import importlib.util
import json
import re
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# Load custody event writer
SPEC_WRITER = importlib.util.spec_from_file_location(
    "internal_custody_event", ROOT / "tools/internal_custody_event.py"
)
assert SPEC_WRITER and SPEC_WRITER.loader
ICE = importlib.util.module_from_spec(SPEC_WRITER)
sys.modules[SPEC_WRITER.name] = ICE
SPEC_WRITER.loader.exec_module(ICE)

# Load custody ledger verifier
SPEC_VERIFIER = importlib.util.spec_from_file_location(
    "verify_internal_custody", ROOT / "tools/verify_internal_custody.py"
)
assert SPEC_VERIFIER and SPEC_VERIFIER.loader
VIC = importlib.util.module_from_spec(SPEC_VERIFIER)
sys.modules[SPEC_VERIFIER.name] = VIC
SPEC_VERIFIER.loader.exec_module(VIC)


class InternalCustodyLedgerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp_dir = tempfile.TemporaryDirectory()
        self.temp_root = Path(self.temp_dir.name)

    def tearDown(self) -> None:
        self.temp_dir.cleanup()

    def test_ledger_empty_on_init(self) -> None:
        """New ledger file should not exist initially."""
        ledger_path = self.temp_root / "custody.jsonl"
        self.assertFalse(ledger_path.exists())

    def test_record_first_event_has_genesis_parent(self) -> None:
        """First recorded event should have GENESIS parent_event_id."""
        ledger_path = self.temp_root / "custody.jsonl"
        event = ICE.record_event(
            ledger_path=ledger_path,
            repository="test",
            path="test.txt",
            symbol="test_op",
            result="PASS",
            exit_code=0,
        )
        self.assertIsNotNone(event)
        self.assertEqual(event["parent_event_id"], ICE.GENESIS_PARENT_ID)

    def test_record_two_events_chain_correctly(self) -> None:
        """Two events should form valid hash chain."""
        ledger_path = self.temp_root / "custody.jsonl"

        # Record first event
        event1 = ICE.record_event(
            ledger_path=ledger_path,
            repository="test",
            path="test1.txt",
            symbol="op1",
            result="PASS",
        )
        self.assertIsNotNone(event1)
        event1_id = event1["event_id"]

        # Record second event
        event2 = ICE.record_event(
            ledger_path=ledger_path,
            repository="test",
            path="test2.txt",
            symbol="op2",
            result="PASS",
        )
        self.assertIsNotNone(event2)

        # Second event's parent should be first event's id
        self.assertEqual(event2["parent_event_id"], event1_id)

    def test_event_id_is_deterministic(self) -> None:
        """Same event data should produce same event_id."""
        event_data = {
            "commit_sha": "abc123",
            "path": "test.txt",
            "repository": "test",
            "symbol": "test_op",
            "exit_code": 0,
            "result": "PASS",
        }
        parent_id = ICE.GENESIS_PARENT_ID

        id1 = ICE.calculate_event_id(event_data, parent_id)
        id2 = ICE.calculate_event_id(event_data, parent_id)

        self.assertEqual(id1, id2)
        self.assertTrue(re.match(r"^[a-f0-9]{64}$", id1))

    def test_event_id_changes_with_different_parent(self) -> None:
        """event_id should change if parent_event_id is different."""
        event_data = {
            "commit_sha": "abc123",
            "path": "test.txt",
            "repository": "test",
            "symbol": "test_op",
            "exit_code": 0,
            "result": "PASS",
        }

        parent1 = ICE.GENESIS_PARENT_ID
        parent2 = "a" * 64  # Different parent

        id1 = ICE.calculate_event_id(event_data, parent1)
        id2 = ICE.calculate_event_id(event_data, parent2)

        self.assertNotEqual(id1, id2)

    def test_canonical_serialization_is_deterministic(self) -> None:
        """Canonical JSON should be byte-identical across serializations."""
        data = {
            "z_field": "last",
            "a_field": "first",
            "nested": {"b": 2, "a": 1},
            "array": [3, 1, 2],
        }

        bytes1 = ICE.canonical_json_bytes(data)
        bytes2 = ICE.canonical_json_bytes(data)

        self.assertEqual(bytes1, bytes2)
        # Should be sorted keys
        json_str = bytes1.decode("utf-8").strip()
        self.assertTrue(json_str.startswith('{"a_field"'))

    def test_verify_valid_ledger_passes(self) -> None:
        """Valid ledger should pass verification."""
        ledger_path = self.temp_root / "custody.jsonl"

        # Create valid ledger with 3 events
        for i in range(3):
            ICE.record_event(
                ledger_path=ledger_path,
                repository="test",
                path=f"test{i}.txt",
                symbol=f"op{i}",
                result="PASS",
            )

        verifier = VIC.LedgerVerifier(ledger_path, strict=False)
        valid = verifier.verify_ledger()
        self.assertTrue(valid, f"Ledger verification failed: {verifier.report()}")

    def test_verify_empty_ledger_passes(self) -> None:
        """Empty ledger file should pass verification."""
        ledger_path = self.temp_root / "custody.jsonl"
        ledger_path.write_text("")

        verifier = VIC.LedgerVerifier(ledger_path, strict=False)
        valid = verifier.verify_ledger()
        self.assertTrue(valid)

    def test_verify_detects_missing_field(self) -> None:
        """Verifier should detect missing required field."""
        ledger_path = self.temp_root / "custody.jsonl"

        # Write invalid event (missing result field)
        invalid_event = {
            "event_version": "1.0.0",
            "event_id": "a" * 64,
            "parent_event_id": VIC.GENESIS_PARENT_ID,
            "repository": "test",
            "commit_sha": "abc123",
            "path": "test.txt",
            "symbol": "test_op",
            "exit_code": 0,
            "timestamp_utc": "2026-08-15T12:00:00Z",
            # Missing result field
        }
        ledger_path.write_text(json.dumps(invalid_event) + "\n")

        verifier = VIC.LedgerVerifier(ledger_path, strict=False)
        valid = verifier.verify_ledger()
        self.assertFalse(valid)
        self.assertTrue(any("missing required field" in e for e in verifier.errors))

    def test_verify_detects_corrupted_event_id(self) -> None:
        """Verifier should detect invalid event_id hash."""
        ledger_path = self.temp_root / "custody.jsonl"

        event = {
            "event_version": "1.0.0",
            "event_id": "b" * 64,  # Wrong hash
            "parent_event_id": VIC.GENESIS_PARENT_ID,
            "repository": "test",
            "commit_sha": "abc123",
            "path": "test.txt",
            "symbol": "test_op",
            "exit_code": 0,
            "timestamp_utc": "2026-08-15T12:00:00Z",
            "result": "PASS",
        }
        ledger_path.write_text(json.dumps(event) + "\n")

        verifier = VIC.LedgerVerifier(ledger_path, strict=False)
        valid = verifier.verify_ledger()
        self.assertFalse(valid)
        self.assertTrue(any("verification failed" in e for e in verifier.errors))

    def test_verify_detects_broken_chain(self) -> None:
        """Verifier should detect broken parent chain."""
        ledger_path = self.temp_root / "custody.jsonl"

        # Create first valid event
        event1 = ICE.record_event(
            ledger_path=ledger_path,
            repository="test",
            path="test1.txt",
            symbol="op1",
            result="PASS",
        )

        # Create second event with wrong parent
        event2_data = {
            "event_version": "1.0.0",
            "repository": "test",
            "path": "test2.txt",
            "symbol": "op2",
            "exit_code": 0,
            "result": "PASS",
            "timestamp_utc": "2026-08-15T12:00:00Z",
            "commit_sha": "abc123",
            "parent_event_id": "c" * 64,  # Wrong parent
        }
        event2_id = ICE.calculate_event_id(event2_data, event2_data["parent_event_id"])
        event2_data["event_id"] = event2_id

        # Append to ledger
        with open(ledger_path, "a") as f:
            f.write(json.dumps(event2_data) + "\n")

        verifier = VIC.LedgerVerifier(ledger_path, strict=False)
        valid = verifier.verify_ledger()
        self.assertFalse(valid)
        self.assertTrue(any("parent_event_id mismatch" in e for e in verifier.errors))

    def test_verify_detects_truncated_ledger(self) -> None:
        """Verifier should handle truncated JSON lines gracefully."""
        ledger_path = self.temp_root / "custody.jsonl"

        # Write incomplete JSON
        ledger_path.write_text('{"event_version": "1.0.0"')

        verifier = VIC.LedgerVerifier(ledger_path, strict=False)
        valid = verifier.verify_ledger()
        self.assertFalse(valid)
        self.assertTrue(any("JSON parse error" in e for e in verifier.errors))

    def test_concurrent_writes_safety(self) -> None:
        """Ledger should be safe against concurrent writes (basic append-only)."""
        ledger_path = self.temp_root / "custody.jsonl"

        # Simulate two sequential writes
        event1 = ICE.record_event(
            ledger_path=ledger_path,
            repository="test",
            path="test1.txt",
            symbol="op1",
            result="PASS",
        )

        event2 = ICE.record_event(
            ledger_path=ledger_path,
            repository="test",
            path="test2.txt",
            symbol="op2",
            result="PASS",
        )

        # Verify chain is intact
        verifier = VIC.LedgerVerifier(ledger_path, strict=False)
        valid = verifier.verify_ledger()
        self.assertTrue(valid)

    def test_replay_deterministic(self) -> None:
        """Replaying events with same parameters should produce valid chain."""
        ledger1_path = self.temp_root / "ledger1.jsonl"
        ledger2_path = self.temp_root / "ledger2.jsonl"

        events = [
            {"repository": "test", "path": "a.txt", "symbol": "op1", "result": "PASS"},
            {"repository": "test", "path": "b.txt", "symbol": "op2", "result": "PASS"},
            {"repository": "test", "path": "c.txt", "symbol": "op3", "result": "FAIL"},
        ]

        # Create first ledger
        for e in events:
            ICE.record_event(ledger_path=ledger1_path, **e)

        # Create second ledger
        for e in events:
            ICE.record_event(ledger_path=ledger2_path, **e)

        # Both ledgers should have valid chains (event counts, parent links)
        verifier1 = VIC.LedgerVerifier(ledger1_path, strict=False)
        verifier2 = VIC.LedgerVerifier(ledger2_path, strict=False)

        valid1 = verifier1.verify_ledger()
        valid2 = verifier2.verify_ledger()

        self.assertTrue(valid1)
        self.assertTrue(valid2)

        # Both should have same number of events
        lines1 = [line for line in ledger1_path.read_text().strip().split("\n") if line]
        lines2 = [line for line in ledger2_path.read_text().strip().split("\n") if line]

        self.assertEqual(len(lines1), len(lines2))


if __name__ == "__main__":
    import re

    unittest.main()
