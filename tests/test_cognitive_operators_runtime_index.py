#!/usr/bin/env python3
from __future__ import annotations
import copy
import json
import unittest
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from validate_cognitive_operators_runtime_index import validate

FIXTURE = Path(__file__).resolve().parents[1] / "research" / "cognitive_operators_71_126" / "operators.index.v1.json"


class RuntimeIndexTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.data = json.loads(FIXTURE.read_text(encoding="utf-8"))

    def test_registry_passes(self) -> None:
        self.assertEqual(validate(self.data), [])

    def test_duplicate_id_fails(self) -> None:
        data = copy.deepcopy(self.data)
        data["operators"][1]["id"] = 71
        self.assertIn("ids", validate(data))

    def test_claim_promotion_fails(self) -> None:
        data = copy.deepcopy(self.data)
        data["claim_allowed"] = True
        self.assertIn("claim_allowed", validate(data))

    def test_invalid_binding_fails(self) -> None:
        data = copy.deepcopy(self.data)
        data["operators"][0]["bindings"] = ["TOKEN_INVENTADO"]
        self.assertTrue(any(x.endswith("_bindings") for x in validate(data)))

    def test_wrong_class_count_fails(self) -> None:
        data = copy.deepcopy(self.data)
        data["operators"][0]["class"] = "MODEL_ANALOGY"
        self.assertIn("class_counts", validate(data))


if __name__ == "__main__":
    unittest.main()
