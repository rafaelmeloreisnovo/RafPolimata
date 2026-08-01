#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
from pathlib import Path
import unittest

HERE = Path(__file__).resolve().parent
SPEC = importlib.util.spec_from_file_location("compiler", HERE / "compile_domain_cell_ir.py")
compiler = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(compiler)


def fixture():
    payload = {
        "domain": {"primary_language": "pt-BR"},
        "seed": {"source_disclosure": "NO_RAW_PRIVATE_TEXT", "claim_allowed": False, "summary": "not exported"},
        "mirrors": {"human": {"role": "FINAL"}, "ai": {"role": "ADVISORY_ONLY"}},
        "modules": [
            {"id": "music.core", "kind": "DOMAIN", "required_user_knowledge": []},
            {"id": "support.math", "kind": "SUPPORT", "required_user_knowledge": [], "must_translate_to_domain_language": True}
        ],
        "governance": {"ai_can_approve": False, "claim_allowed": False},
        "privacy_security": {"raw_private_text_committed": False, "secrets_allowed": False},
        "workflow_proof": {"required_receipt_fields": ["intent_id", "object_id", "outputs", "tests", "rollback", "F_ok", "F_gap", "F_next"]}
    }
    return {
        "schema": "rafaelia.living-book.domain-cell/v1",
        "cell_id": "LBC-MUSIC-TEST-0001",
        "payload": payload,
        "integrity": {"digests": compiler.digests(payload)}
    }


class CompilerTests(unittest.TestCase):
    def test_valid_non_executable_ir(self):
        ir = compiler.compile_ir(fixture(), "support.math", "PROPOSE_ANALYSIS", "INT-001")
        self.assertEqual("COMPILED_NON_EXECUTABLE_IR", ir["ir"]["state"])
        self.assertFalse(ir["ir"]["policy_gates"]["execution_allowed"])
        self.assertFalse(ir["ir"]["policy_gates"]["claim_allowed"])

    def test_execute_action_rejected(self):
        with self.assertRaisesRegex(ValueError, "action not allowed"):
            compiler.compile_ir(fixture(), "support.math", "EXECUTE", "INT-002")

    def test_digest_tamper_rejected(self):
        cell = fixture()
        cell["payload"]["domain"]["primary_language"] = "en"
        with self.assertRaisesRegex(ValueError, "digest mismatch"):
            compiler.compile_ir(cell, "support.math", "PROPOSE_ANALYSIS", "INT-003")

    def test_private_text_rejected(self):
        cell = fixture()
        cell["payload"]["privacy_security"]["raw_private_text_committed"] = True
        cell["integrity"]["digests"] = compiler.digests(cell["payload"])
        with self.assertRaisesRegex(ValueError, "raw private text"):
            compiler.compile_ir(cell, "support.math", "PROPOSE_ANALYSIS", "INT-004")

    def test_ai_approval_rejected(self):
        cell = fixture()
        cell["payload"]["governance"]["ai_can_approve"] = True
        cell["integrity"]["digests"] = compiler.digests(cell["payload"])
        with self.assertRaisesRegex(ValueError, "AI approval"):
            compiler.compile_ir(cell, "support.math", "PROPOSE_ANALYSIS", "INT-005")

    def test_technical_prerequisite_rejected(self):
        cell = fixture()
        cell["payload"]["modules"][1]["required_user_knowledge"] = ["calculus"]
        cell["integrity"]["digests"] = compiler.digests(cell["payload"])
        with self.assertRaisesRegex(ValueError, "technical user knowledge"):
            compiler.compile_ir(cell, "support.math", "PROPOSE_ANALYSIS", "INT-006")

    def test_seed_summary_not_copied_to_ir(self):
        ir = compiler.compile_ir(fixture(), "support.math", "PROPOSE_TRANSLATION", "INT-007")
        self.assertNotIn("not exported", str(ir))
        self.assertFalse(ir["ir"]["output_contract"]["raw_seed_text_in_output"])

    def test_forbidden_capabilities_are_explicit(self):
        ir = compiler.compile_ir(fixture(), "music.core", "INDEX_ONLY", "INT-008")
        self.assertIn("publish", ir["ir"]["forbidden_capabilities"])
        self.assertIn("execute_untrusted", ir["ir"]["forbidden_capabilities"])
        self.assertEqual(ir["integrity"]["digests"], compiler.digests(ir["ir"]))


if __name__ == "__main__":
    unittest.main()
