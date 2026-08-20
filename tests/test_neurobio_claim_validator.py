import json
import tempfile
import unittest
from pathlib import Path

from tools.neurobio_claim_validator import validate_claim, validate_jsonl

class NeurobioClaimValidatorTests(unittest.TestCase):
    def base(self):
        return {
            "claim_id": "NCB-T",
            "status": "EVIDENCE_FOUND_GATE_OPEN",
            "priority": "P0",
            "claim_allowed": False,
            "claim": "Distinct biological endpoints require distinct evidence.",
            "falsifier": "A direct test contradicts the claim.",
            "next_gate": "Run a preregistered endpoint-specific gate.",
        }

    def test_base_valid(self):
        self.assertEqual(validate_claim(self.base()), [])

    def test_claim_allowed_cannot_be_true(self):
        c = self.base()
        c["claim_allowed"] = True
        self.assertIn("claim_allowed_must_be_false", validate_claim(c))

    def test_emf_requires_dosimetry_gate(self):
        c = self.base()
        c["claim"] = "An electromagnetic EMF exposure changes a biological endpoint."
        self.assertIn("emf_missing_dosimetry_gate", validate_claim(c))
        c["next_gate"] = "Run blinded calibrated dosimetry with sham control."
        self.assertNotIn("emf_missing_dosimetry_gate", validate_claim(c))

    def test_upe_pass_blocked_without_source_attribution(self):
        c = self.base()
        c["claim"] = "UPE photon signal communicates neural information."
        c["status"] = "PASS"
        c["next_gate"] = "Repeat measurement."
        errors = validate_claim(c)
        self.assertIn("upe_promotion_without_source_attribution", errors)
        self.assertIn("pass_not_authorized_by_static_validator", errors)

    def test_jsonl(self):
        c = self.base()
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "claims.jsonl"
            p.write_text(json.dumps(c) + "\n", encoding="utf-8")
            count, issues = validate_jsonl(p)
            self.assertEqual(count, 1)
            self.assertEqual(issues, [])

if __name__ == "__main__":
    unittest.main()
