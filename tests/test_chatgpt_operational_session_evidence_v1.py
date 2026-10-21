import copy
import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts/validate_chatgpt_operational_evidence_v1.py"
spec = importlib.util.spec_from_file_location("chatgpt_evidence_validator", SCRIPT)
mod = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(mod)

CONTRACT = json.loads((ROOT / "data/contracts/chatgpt_operational_session_evidence.v1.json").read_text(encoding="utf-8"))
RECEIPT = json.loads((ROOT / "data/evidence/chatgpt/2026-10-20_operational_session_evidence.v1.json").read_text(encoding="utf-8"))


class ChatGPTOperationalEvidenceTests(unittest.TestCase):
    def test_current_receipt_passes(self):
        self.assertEqual(mod.validate_receipt(RECEIPT, CONTRACT), [])

    def test_training_use_is_rejected(self):
        receipt = copy.deepcopy(RECEIPT)
        receipt["source"]["training_use"] = True
        errors = mod.validate_receipt(receipt, CONTRACT)
        self.assertIn("training_use must be false", errors)

    def test_dataset_use_is_rejected(self):
        receipt = copy.deepcopy(RECEIPT)
        receipt["source"]["dataset_use"] = True
        errors = mod.validate_receipt(receipt, CONTRACT)
        self.assertIn("dataset_use must be false", errors)

    def test_auto_correction_without_reproducer_fails_closed(self):
        receipt = copy.deepcopy(RECEIPT)
        receipt["automatic_correction"] = {
            "state": "AUTO_CORRECT_ELIGIBLE",
            "eligible": True,
            "envelope": {}
        }
        receipt["evidence"]["state"] = "AUTO_CORRECT_ELIGIBLE"
        errors = mod.validate_receipt(receipt, CONTRACT)
        self.assertTrue(any("eligible automatic correction missing:" in error for error in errors))

    def test_observation_does_not_self_promote(self):
        receipt = copy.deepcopy(RECEIPT)
        receipt["claims"]["allowed"] = True
        errors = mod.validate_receipt(receipt, CONTRACT)
        self.assertIn("claims.allowed must remain false at evidence-intake stage", errors)


if __name__ == "__main__":
    unittest.main()
