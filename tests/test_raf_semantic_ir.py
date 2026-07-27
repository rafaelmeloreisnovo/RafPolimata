from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "raf_semantic_ir.py"
FIXTURE = ROOT / "tests" / "fixtures" / "semantic_equivalence_mix32.v1.json"
SPEC = importlib.util.spec_from_file_location("raf_semantic_ir", SCRIPT)
assert SPEC and SPEC.loader
module = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = module
SPEC.loader.exec_module(module)


class SemanticIrTests(unittest.TestCase):
    def contract(self):
        return json.loads(FIXTURE.read_text(encoding="utf-8"))

    def test_ten_frontends_lower_to_one_kernel(self):
        ir = module.compile_source_set(self.contract())
        self.assertEqual(ir["semantic_equivalence"], "PASS")
        self.assertEqual(len(ir["languages"]), 10)
        self.assertTrue(ir["kernel_id"].startswith("rafk2-"))
        self.assertEqual({v["state"] for v in ir["vectors"]}, {"PASS"})

    def test_formatting_and_commutative_order_normalize(self):
        contract = self.contract()
        contract["sources"] = {
            "c": "unsigned f(unsigned a,unsigned b,unsigned mask){return a+b;}",
            "py": "def f(a,b,mask):\n return b + a",
        }
        contract["kernel"] = "f"
        contract["vectors"] = [{"inputs": {"a": 1, "b": 2, "mask": 0}, "expected": 3}]
        ir = module.compile_source_set(contract)
        self.assertEqual(ir["semantic_equivalence"], "PASS")

    def test_semantic_mismatch_fails_closed(self):
        contract = self.contract()
        contract["sources"]["js"] = "function f(a,b,mask){ return a - b; }"
        with self.assertRaises(module.SemanticError):
            module.compile_source_set(contract)

    def test_function_call_is_rejected(self):
        contract = self.contract()
        contract["sources"] = {
            "c": "unsigned f(unsigned a,unsigned b,unsigned mask){return helper(a);}",
            "py": "def f(a,b,mask):\n return helper(a)",
        }
        contract["kernel"] = "f"
        with self.assertRaises(module.SemanticError):
            module.compile_source_set(contract)

    def test_unsigned_wrap_is_explicit(self):
        ir = module.compile_source_set(self.contract())
        wrap = next(item for item in ir["vectors"] if item["id"] == "wrap")
        self.assertEqual(wrap["observed"], 4294967287)

    def test_c_emitter_is_deterministic_and_freestanding(self):
        ir = module.compile_source_set(self.contract())
        first = module.emit_c(ir)
        second = module.emit_c(ir)
        self.assertEqual(first, second)
        self.assertIn(ir["kernel_id"], first)
        self.assertNotIn("malloc", first)
        self.assertIn("_Static_assert", first)

    def test_architecture_emission_is_separated_from_semantics(self):
        ir = module.compile_source_set(self.contract())
        self.assertEqual(ir["architecture_policy"], "compiler/architectures.v2.json")
        self.assertEqual(ir["retired_architectures"], ["i386", "ia32", "x86-32"])
        self.assertFalse(hasattr(module, "x3264_harness"))

    def test_contract_requires_claim_false(self):
        contract = self.contract()
        contract["claim_allowed"] = True
        with self.assertRaises(module.SemanticError):
            module.compile_source_set(contract)


if __name__ == "__main__":
    unittest.main()
