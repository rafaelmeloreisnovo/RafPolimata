#!/usr/bin/env python3
import copy
import importlib.util
import json
import pathlib
import tempfile
import unittest

HERE = pathlib.Path(__file__).resolve().parent
SPEC = HERE / "orquestrador-omega-excellence-v1.json"
MODULE_SPEC = importlib.util.spec_from_file_location("validator", HERE / "validate_orquestrador_omega.py")
VALIDATOR = importlib.util.module_from_spec(MODULE_SPEC)
MODULE_SPEC.loader.exec_module(VALIDATOR)


class TestOmegaOrchestrator(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.base = json.loads(SPEC.read_text(encoding="utf-8"))

    def validate_data(self, data):
        with tempfile.NamedTemporaryFile("w", encoding="utf-8", delete=False) as handle:
            json.dump(data, handle)
            path = handle.name
        return VALIDATOR.validate(path)

    def test_valid_contract(self):
        self.assertEqual(self.validate_data(self.base), [])

    def test_need_count_is_fail_closed(self):
        data = copy.deepcopy(self.base)
        data["needs"].pop()
        self.assertIn("needs=11", self.validate_data(data))

    def test_vector_count_is_fail_closed(self):
        data = copy.deepcopy(self.base)
        data["vectors"].pop()
        self.assertIn("vectors=11", self.validate_data(data))

    def test_matrix_count_is_fail_closed(self):
        data = copy.deepcopy(self.base)
        data["matrix_contract"]["expected_cells"] = 143
        self.assertIn("matrix_coverage", self.validate_data(data))

    def test_duplicate_need_is_rejected(self):
        data = copy.deepcopy(self.base)
        data["needs"][1]["id"] = data["needs"][0]["id"]
        self.assertIn("need_ids", self.validate_data(data))

    def test_claim_promotion_is_rejected(self):
        data = copy.deepcopy(self.base)
        data["global_state"]["claim_allowed"] = True
        self.assertIn("claim_gate", self.validate_data(data))

    def test_automatic_authority_is_rejected(self):
        data = copy.deepcopy(self.base)
        data["cell_defaults"]["authority"] = "AUTO"
        self.assertIn("authority", self.validate_data(data))

    def test_compensatory_excellence_is_rejected(self):
        data = copy.deepcopy(self.base)
        data["matrix_contract"]["non_compensatory"] = False
        self.assertIn("non_compensatory", self.validate_data(data))


if __name__ == "__main__":
    unittest.main(verbosity=2)
