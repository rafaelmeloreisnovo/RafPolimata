import copy
import importlib.util
import json
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "validate_authorial_omega_assurance.py"
CONFIG = ROOT / "configs" / "authorial-omega-assurance.v1.json"

spec = importlib.util.spec_from_file_location("omega_assurance", SCRIPT)
M = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(M)

class AuthorialOmegaAssuranceV1Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = json.loads(CONFIG.read_text(encoding="utf-8"))

    def test_current_config(self):
        self.assertTrue(M.validate(copy.deepcopy(self.data)))

    def test_reject_federation_pin_drift(self):
        bad = copy.deepcopy(self.data)
        bad["source_bindings"]["federation"]["commit"] = "0" * 40
        with self.assertRaises(M.ValidationError):
            M.validate(bad)

    def test_reject_executor_pin_drift(self):
        bad = copy.deepcopy(self.data)
        bad["source_bindings"]["executor"]["blob_sha1"] = "f" * 40
        with self.assertRaises(M.ValidationError):
            M.validate(bad)

    def test_reject_whole_fork_authorship(self):
        bad = copy.deepcopy(self.data)
        bad["authorial_policy"]["whole_fork_authorship_allowed"] = True
        with self.assertRaises(M.ValidationError):
            M.validate(bad)

    def test_reject_vectra_code_import(self):
        bad = copy.deepcopy(self.data)
        bad["model_boundaries"]["Vectra"]["code_import_allowed"] = True
        with self.assertRaises(M.ValidationError):
            M.validate(bad)

    def test_reject_runtime_promotion_without_receipt(self):
        bad = copy.deepcopy(self.data)
        for gate in bad["assurance_gates"]:
            if gate["id"] == "A06_PHYSICAL_RUNTIME":
                gate["state"] = "PASS"
        with self.assertRaises(M.ValidationError):
            M.validate(bad)

    def test_reject_executor_evidence_invention(self):
        bad = copy.deepcopy(self.data)
        for gate in bad["assurance_gates"]:
            if gate["id"] == "A05_EXECUTOR_EXACT_HEAD":
                gate["state"] = "PASS"
        with self.assertRaises(M.ValidationError):
            M.validate(bad)

if __name__ == "__main__":
    unittest.main()
