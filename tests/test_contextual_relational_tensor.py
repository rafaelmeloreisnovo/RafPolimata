import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "tensor", ROOT / "scripts/contextual_relational_tensor.py"
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC and SPEC.loader
SPEC.loader.exec_module(MODULE)


class ContextualRelationalTensorTest(unittest.TestCase):
    def case(self):
        return json.loads(
            (ROOT / "examples/wine-formula-relational-tensor.v1.json")
            .read_text(encoding="utf-8")
        )

    def test_case_abstains_with_blocking_gaps(self):
        result = MODULE.evaluate_case(self.case())
        self.assertTrue(result["abstain"])
        self.assertIsNone(result["selected_path"])
        self.assertFalse(result["claim_allowed"])

    def test_structural_path_ranks_above_lexical_oracle(self):
        result = MODULE.evaluate_case(self.case())
        self.assertEqual(
            "path:climate-phenology-yield-quality-market-price",
            result["ranked_paths"][0]["path_id"],
        )

    def test_lexical_match_alone_is_not_eligible(self):
        result = MODULE.evaluate_case(self.case())
        lexical = next(
            item for item in result["ranked_paths"]
            if item["path_id"] == "path:lexical-oracle"
        )
        self.assertFalse(lexical["eligible"])

    def test_observed_support_can_open_candidate_for_review(self):
        case = self.case()
        case["blocking_gaps"] = []
        path = case["paths"][2]
        path["features"].update(
            source_support=0.8,
            provenance=0.9,
            gap_penalty=0.1,
            unit_completeness=0.8,
        )
        result = MODULE.evaluate_case(case)
        self.assertFalse(result["abstain"])
        self.assertEqual(path["path_id"], result["selected_path"])
        self.assertFalse(result["claim_allowed"])

    def test_out_of_range_feature_is_rejected(self):
        case = self.case()
        case["paths"][0]["features"]["lexical_match"] = 1.5
        with self.assertRaises(MODULE.TensorError):
            MODULE.evaluate_case(case)


if __name__ == "__main__":
    unittest.main()
