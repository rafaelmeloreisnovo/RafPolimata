#!/usr/bin/env python3
"""Tests for the hosted custody execution adapter (CLOSURE_L0/CLOSURE_L1)."""
from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RUNNER = ROOT / "tools" / "run_with_internal_custody.py"
VERIFIER = ROOT / "tools" / "verify_internal_custody.py"


class InternalCustodyRunnerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.ledger = self.root / "custody.jsonl"

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def invoke(self, command: list[str], *extra: str) -> subprocess.CompletedProcess[str]:
        argv = [
            sys.executable,
            str(RUNNER),
            "--ledger",
            str(self.ledger),
            "--repository",
            "rafaelmeloreisnovo/RafPolimata",
            "--path",
            "fixture.txt",
            "--symbol",
            "runner-test",
            "--repo-root",
            str(self.root),
            *extra,
            "--",
            *command,
        ]
        return subprocess.run(argv, text=True, capture_output=True, check=False)

    def read_last(self) -> dict[str, object]:
        return json.loads(self.ledger.read_text(encoding="utf-8").splitlines()[-1])

    def test_success_preserves_zero_and_records_pass(self) -> None:
        result = self.invoke([sys.executable, "-c", "print('ok')"])
        self.assertEqual(result.returncode, 0, result.stderr)
        event = self.read_last()
        self.assertEqual(event["result"], "PASS")
        self.assertEqual(event["exit_code"], 0)
        self.assertNotEqual(event["stdout_hash"], "TOKEN" + "_VAZIO")
        verify = subprocess.run([sys.executable, str(VERIFIER), str(self.ledger)], text=True, capture_output=True)
        self.assertEqual(verify.returncode, 0, verify.stdout + verify.stderr)

    def test_failure_preserves_original_exit_code(self) -> None:
        result = self.invoke([sys.executable, "-c", "raise SystemExit(7)"])
        self.assertEqual(result.returncode, 7)
        event = self.read_last()
        self.assertEqual(event["result"], "FAIL")
        self.assertEqual(event["exit_code"], 7)

    def test_missing_command_records_explicit_gap(self) -> None:
        result = self.invoke(["definitely-not-a-real-command-rafaelia"])
        self.assertEqual(result.returncode, 127)
        event = self.read_last()
        self.assertEqual(event["result"], "TOKEN" + "_VAZIO")
        self.assertEqual(event["exit_code"], 127)

    def test_marker_records_gap_without_hiding_command_success(self) -> None:
        marker = "TOKEN" + "_VAZIO"
        result = self.invoke(
            [sys.executable, "-c", f"print('{marker}: device absent')"],
            "--token-vazio-marker",
            marker,
        )
        self.assertEqual(result.returncode, 0)
        event = self.read_last()
        self.assertEqual(event["result"], marker)
        self.assertEqual(event["exit_code"], 0)

    def test_multiple_runs_form_one_valid_chain(self) -> None:
        first = self.invoke([sys.executable, "-c", "print('one')"])
        second = self.invoke([sys.executable, "-c", "print('two')"])
        self.assertEqual((first.returncode, second.returncode), (0, 0))
        self.assertEqual(len(self.ledger.read_text(encoding="utf-8").splitlines()), 2)
        verify = subprocess.run([sys.executable, str(VERIFIER), str(self.ledger)], text=True, capture_output=True)
        self.assertEqual(verify.returncode, 0, verify.stdout + verify.stderr)


if __name__ == "__main__":
    unittest.main()
