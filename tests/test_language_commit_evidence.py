#!/usr/bin/env python3
from __future__ import annotations

import copy
import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "language_commit_evidence", ROOT / "scripts/language_commit_evidence.py"
)
assert SPEC and SPEC.loader
EVIDENCE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = EVIDENCE
SPEC.loader.exec_module(EVIDENCE)


class LanguageCommitEvidenceTests(unittest.TestCase):
    def setUp(self) -> None:
        self.fixture = ROOT / "data/language/fixtures/language-matrix-fixture.v1.json"
        self.digest = ROOT / "data/language/fixtures/language-matrix-fixture.v1.sha256"
        self.ledger = ROOT / "data/language/language-matrix-commit-ledger.v1.json"

    def test_frozen_fixture_hash_matches_ledger(self) -> None:
        digest = EVIDENCE.validate_fixture_hash(self.fixture, self.digest)
        ledger = EVIDENCE.read_json(self.ledger)
        self.assertEqual(digest, ledger["fixture"]["sha256"])

    def test_tampered_fixture_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            fixture = root / self.fixture.name
            digest = root / self.digest.name
            fixture.write_bytes(self.fixture.read_bytes() + b"\n")
            digest.write_bytes(self.digest.read_bytes())
            with self.assertRaises(AssertionError):
                EVIDENCE.validate_fixture_hash(fixture, digest)

    def test_commit_ledger_is_unique_and_bounded(self) -> None:
        ledger = EVIDENCE.read_json(self.ledger)
        digest = EVIDENCE.validate_fixture_hash(self.fixture, self.digest)
        EVIDENCE.validate_ledger(ledger, digest)
        self.assertEqual(len(ledger["commits"]), 7)
        self.assertEqual(ledger["branch_relation"]["behind_by"], 0)

    def test_duplicate_commit_is_rejected(self) -> None:
        ledger = EVIDENCE.read_json(self.ledger)
        digest = EVIDENCE.validate_fixture_hash(self.fixture, self.digest)
        duplicate = copy.deepcopy(ledger["commits"][0])
        duplicate["sequence"] = len(ledger["commits"]) + 1
        ledger["commits"].append(duplicate)
        with self.assertRaises(AssertionError):
            EVIDENCE.validate_ledger(ledger, digest)

    def test_open_evidence_remains_token_vazio(self) -> None:
        ledger = EVIDENCE.read_json(self.ledger)
        self.assertTrue(
            ledger["evidence_scope"]["parent_chain_linearity"].startswith("TOKEN_VAZIO")
        )
        self.assertEqual(
            ledger["evidence_scope"]["runtime_reexecution"],
            "TOKEN_VAZIO_CURRENT_CHECKOUT",
        )


if __name__ == "__main__":
    unittest.main()
