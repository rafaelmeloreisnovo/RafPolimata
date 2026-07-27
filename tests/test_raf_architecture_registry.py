from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "raf_architecture_registry.py"
REGISTRY = ROOT / "compiler" / "architectures.v2.json"
SPEC = importlib.util.spec_from_file_location("raf_architecture_registry", SCRIPT)
assert SPEC and SPEC.loader
module = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = module
SPEC.loader.exec_module(module)


class ArchitectureRegistryTests(unittest.TestCase):
    def registry(self):
        return json.loads(REGISTRY.read_text(encoding="utf-8"))

    def test_exactly_seven_active_architectures(self):
        items = module.validate(self.registry())
        self.assertEqual([item["id"] for item in items], list(module.EXPECTED_IDS))

    def test_i386_is_retired_and_not_active(self):
        registry = self.registry()
        self.assertNotIn("i386", [item["id"] for item in registry["architectures"]])
        self.assertIn("i386", [item["id"] for item in registry["retired"]])

    def test_i386_in_active_matrix_fails_closed(self):
        registry = self.registry()
        registry["architectures"][0]["id"] = "i386"
        with self.assertRaises(module.RegistryError):
            module.validate(registry)

    def test_execution_cannot_be_promoted_without_receipt(self):
        registry = copy.deepcopy(self.registry())
        registry["architectures"][0]["execution"] = "PASS"
        with self.assertRaises(module.RegistryError):
            module.validate(registry)

    def test_audit_keeps_claim_false(self):
        receipt = module.audit(REGISTRY)
        self.assertEqual(receipt["state"], "PASS_SEVEN_ARCHITECTURE_POLICY")
        self.assertFalse(receipt["claim_allowed"])
        self.assertEqual(receipt["retired_i386_active_occurrences"], 0)


if __name__ == "__main__":
    unittest.main()
