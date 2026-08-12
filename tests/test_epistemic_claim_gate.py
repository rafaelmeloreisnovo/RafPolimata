import importlib.util
import unittest
from pathlib import Path

MODULE_PATH = Path(__file__).resolve().parents[1] / "scripts" / "epistemic_claim_gate.py"
SPEC = importlib.util.spec_from_file_location("epistemic_claim_gate", MODULE_PATH)
assert SPEC and SPEC.loader
GATE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GATE)


def base_record():
    return {
        "claim_id": "CLAIM-TEST-001",
        "proposition": "The bounded test result exceeds the frozen threshold.",
        "claim_kind": "HYPOTHESIS",
        "state": "EVIDENCE_LINKED",
        "domain": "test-fixture",
        "assumptions": ["fixture only"],
        "h0": "result <= 0.5",
        "h1": "result > 0.5",
        "falsifier": "result <= 0.5 on the frozen protocol",
        "protocol": {
            "version": "v1",
            "frozen": True,
            "procedure": "run deterministic fixture"
        },
        "metric": "result",
        "threshold": 0.5,
        "data_provenance": [
            {"source": "tests/fixture.json", "sha256": "0" * 64}
        ],
        "code_provenance": [
            {"repo": "RafPolimata", "commit": "deadbeef"}
        ],
        "environment": {"runtime": "python-stdlib"},
        "evidence": [{"artifact": "receipt.json", "sha256": "1" * 64}],
        "negative_evidence": [{"test": "negative-control", "result": "PASS"}],
        "replication": {"level": "REPRODUCED_CLEAN_ENV", "independent": False},
        "limitations": ["synthetic fixture"],
        "token_vazio": [],
        "claim_allowed": False,
        "intended_scope": "INTERNAL"
    }


class EpistemicClaimGateTests(unittest.TestCase):
    def test_complete_internal_hypothesis_can_compute_allowed(self):
        record = base_record()
        computed, errors, blockers = GATE.evaluate(record)
        self.assertTrue(computed)
        self.assertEqual(errors, [])
        self.assertEqual(blockers, [])

    def test_critical_token_vazio_blocks_promotion(self):
        record = base_record()
        record["token_vazio"] = [{
            "missing": "independent dataset",
            "critical": True,
            "next_verifiable_action": "obtain and hash independent dataset"
        }]
        computed, errors, blockers = GATE.evaluate(record)
        self.assertFalse(computed)
        self.assertEqual(errors, [])
        self.assertTrue(any("TOKEN_VAZIO" in item for item in blockers))

    def test_public_scientific_requires_independent_replication(self):
        record = base_record()
        record["intended_scope"] = "PUBLIC_SCIENTIFIC"
        computed, _, blockers = GATE.evaluate(record)
        self.assertFalse(computed)
        self.assertTrue(any("independent replication" in item for item in blockers))

    def test_parable_never_promotes_empirical_claim(self):
        record = base_record()
        record["claim_kind"] = "PARABLE"
        computed, _, blockers = GATE.evaluate(record)
        self.assertFalse(computed)
        self.assertTrue(any("PARABLE" in item for item in blockers))

    def test_contradiction_blocks(self):
        record = base_record()
        record["state"] = "CONTRADICTION"
        computed, _, blockers = GATE.evaluate(record)
        self.assertFalse(computed)
        self.assertIn("state=CONTRADICTION", blockers)


if __name__ == "__main__":
    unittest.main()
