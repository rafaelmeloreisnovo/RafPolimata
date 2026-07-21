#!/usr/bin/env python3
import importlib.util
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "scripts" / "validate_hex_matrix_projection_governance.py"
CONFIG_PATH = ROOT / "configs" / "hex-matrix-projection.json"
SPEC = importlib.util.spec_from_file_location("hex_governance", MODULE_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"cannot load {MODULE_PATH}")
module = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = module
SPEC.loader.exec_module(module)


class HexProjectionGovernanceTests(unittest.TestCase):
    def test_canonical_configuration_passes(self):
        report = module.validate(module.load_config(CONFIG_PATH))
        self.assertEqual(report["status"], "PASS")
        self.assertEqual(report["checks_failed"], 0)
        self.assertFalse(report["claim_allowed"])
        self.assertEqual(report["exact_geometry_state"], "PASS")
        self.assertEqual(report["physical_state"], "TOKEN_VAZIO")

    def test_physical_claim_promotion_fails(self):
        data = module.load_config(CONFIG_PATH)
        for claim in data["claims"]:
            if claim["class"] == "H":
                claim["state"] = "PASS"
                break
        report = module.validate(data)
        self.assertEqual(report["status"], "FAIL")
        self.assertFalse(report["checks"]["physical_claims_fail_closed"])

    def test_tensor_cardinality_tampering_fails(self):
        data = module.load_config(CONFIG_PATH)
        data["matrices"]["relation_tensor"]["records"] = 839
        report = module.validate(data)
        self.assertEqual(report["status"], "FAIL")
        self.assertFalse(report["checks"]["tensor_records"])


if __name__ == "__main__":
    unittest.main()
