"""P0: malformed operational evidence must fail closed before any promotion."""
import copy
import importlib.util
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "validate_chatgpt_interaction_bundle_v1.py"
SPEC = importlib.util.spec_from_file_location("chatgpt_evidence_validator", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class ReceiptShapeFalsifiers(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.bundle = MODULE.load(MODULE.DEFAULT_BUNDLE)
        cls.contract = MODULE.load(MODULE.DEFAULT_CONTRACT)

    def test_baseline_is_valid_before_mutation(self):
        self.assertEqual(MODULE.validate(self.bundle, self.contract), [])

    def test_malformed_top_level_fails_without_exception(self):
        for entry in (None, [], "receipt", 0):
            with self.subTest(entry=entry):
                self.assertTrue(MODULE.validate(entry, self.contract))

    def test_untrusted_contract_fails(self):
        for change in (
            {"required_interaction_count": "3"},
            {"required_gates": "not an array"},
            {"required_urgency_ids": [None]},
        ):
            mutated = copy.deepcopy(self.contract)
            mutated.update(change)
            with self.subTest(change=change):
                self.assertTrue(MODULE.validate(self.bundle, mutated))

    def test_nested_malformed_data_fails_without_exception(self):
        replacements = {
            "interactions": ["I1", "I2", "I3"],
            "urgency": ["URG-P0"],
            "gates": [None],
            "typed_gaps": [7],
            "convergence": [],
        }
        for key, value in replacements.items():
            mutated = copy.deepcopy(self.bundle)
            mutated[key] = value
            with self.subTest(field=key):
                self.assertTrue(MODULE.validate(mutated, self.contract))

    def test_unhashable_operator_identifiers_fail(self):
        mutated = copy.deepcopy(self.bundle)
        mutated["convergence"]["priority_p0_operator_ids"] = [["OP1"]]
        self.assertTrue(MODULE.validate(mutated, self.contract))

    def test_unhashable_urgency_and_typed_gap_ids_fail(self):
        broken_urgency = copy.deepcopy(self.bundle)
        broken_urgency["urgency"][0]["id"] = ["unhashable"]
        self.assertTrue(MODULE.validate(broken_urgency, self.contract))
        broken_gap = copy.deepcopy(self.bundle)
        broken_gap["typed_gaps"] = [{"id": 77, "type": "UNKNOWN",
                                      "closure": "review"}]
        self.assertTrue(MODULE.validate(broken_gap, self.contract))

    def test_absence_distinct_from_valid_empty_observation(self):
        mutated = copy.deepcopy(self.bundle)
        mutated["interactions"][0]["prompt_text"] = None
        self.assertTrue(MODULE.validate(mutated, self.contract))


if __name__ == "__main__":
    unittest.main()
