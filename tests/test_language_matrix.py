#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import json
import math
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "language_matrix", ROOT / "scripts/language_matrix.py"
)
assert SPEC and SPEC.loader
LM = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = LM
SPEC.loader.exec_module(LM)


class LanguageMatrixTests(unittest.TestCase):
    def test_unknown_is_not_zero(self) -> None:
        axis = {
            "id": "weights",
            "score": None,
            "status": "TOKEN_VAZIO",
            "evidence": [],
            "token_vazio": "TOKEN_VAZIO_CALIBRATION",
            "next_gate": "calibrate",
        }
        LM.validate_axis(axis)
        axis["score"] = 0.0
        with self.assertRaises(AssertionError):
            LM.validate_axis(axis)

    def test_promotion_is_one_tenth_and_requires_next_gate(self) -> None:
        self.assertEqual(LM.promote(None, ["definition"]), 0.1)
        self.assertEqual(LM.promote(0.1, ["contract"]), 0.2)
        with self.assertRaises(AssertionError):
            LM.promote(0.2, ["local_test"])

    def test_direct_inverse_indirect_and_reciprocal(self) -> None:
        matrix = [[0, 1], [2, 0]]
        self.assertEqual(LM.direct_matrix(matrix), [[0.0, 1.0], [2.0, 0.0]])
        self.assertEqual(LM.inverse_relational(matrix), [[0.0, 2.0], [1.0, 0.0]])
        self.assertEqual(LM.indirect_two_step(matrix), [[2.0, 0.0], [0.0, 2.0]])
        self.assertEqual(LM.reciprocal_matrix(matrix), [[0.0, 1.0], [1.0, 0.0]])

    def test_logarithmic_roundtrip(self) -> None:
        matrix = [[0.0, 1.0], [3.0, 10.0]]
        restored = LM.inverse_logarithmic_matrix(LM.logarithmic_matrix(matrix))
        for expected_row, actual_row in zip(matrix, restored):
            for expected, actual in zip(expected_row, actual_row):
                self.assertTrue(math.isclose(expected, actual, rel_tol=1e-12, abs_tol=1e-12))
        with self.assertRaises(AssertionError):
            LM.logarithmic_matrix([[0.0, -1.0], [1.0, 0.0]])

    def test_fibonacci_windows_and_inverse(self) -> None:
        self.assertEqual(LM.fibonacci_windows(13), [1, 2, 3, 5, 8, 13])
        self.assertEqual(LM.fibonacci_inverse_index(13), 7)
        self.assertIsNone(LM.fibonacci_inverse_index(4))

    def test_dyadic_partition_covers_without_gaps(self) -> None:
        levels = LM.dyadic_partition(7)
        for level in levels:
            flattened = []
            for start, end in level:
                flattened.extend(range(start, end))
            self.assertEqual(flattened, list(range(7)))
        self.assertTrue(all(end - start == 1 for start, end in levels[-1]))

    def test_unicode_baselines_preserve_multiscript_input(self) -> None:
        text = "λόγος בראשית ܒܪܫܝܬ TOKEN_VAZIO"
        self.assertEqual(
            LM.tokenize_whitespace(text),
            ["λόγος", "בראשית", "ܒܪܫܝܬ", "TOKEN_VAZIO"],
        )
        self.assertIn("λ", LM.tokenize_codepoints(text))
        self.assertIn("ב", LM.tokenize_codepoints(text))
        self.assertIn("ܒ", LM.tokenize_codepoints(text))

    def test_repository_state_is_valid(self) -> None:
        state = json.loads(
            (ROOT / "data/language/language-matrix-state.v1.json").read_text(encoding="utf-8")
        )
        LM.validate_state(state)
        self.assertFalse(state["claim_allowed"])
        empty = {axis["id"] for axis in state["axes"] if axis["score"] is None}
        self.assertIn("weights", empty)
        self.assertIn("time", empty)
        self.assertIn("accuracy", empty)


if __name__ == "__main__":
    unittest.main()
