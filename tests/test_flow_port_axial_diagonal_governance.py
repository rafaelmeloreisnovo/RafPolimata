#!/usr/bin/env python3
import importlib.util
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VALIDATOR = ROOT / "scripts" / "validate_flow_port_axial_diagonal_governance.py"
CONFIG = ROOT / "configs" / "flow-port-axial-diagonal.json"

SPEC = importlib.util.spec_from_file_location("validator", VALIDATOR)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("cannot load validator")
module = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = module
SPEC.loader.exec_module(module)


class GovernanceTests(unittest.TestCase):
    def test_governance(self):
        config = json.loads(CONFIG.read_text(encoding="utf-8"))
        report = module.validate(config)
        self.assertEqual(report["status"], "PASS")
        self.assertEqual(report["checks_passed"], 16)
        self.assertTrue(report["claim_allowed"])
        self.assertEqual(report["operator_state"], "PASS")
        self.assertEqual(report["empirical_fluid_interpretation"], "NOT_APPLICABLE")
        self.assertTrue(all(report["checks"].values()))


if __name__ == "__main__":
    unittest.main()
