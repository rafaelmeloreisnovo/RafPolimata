#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "research/recurrence_matrix_reference/reference.py"
SPEC = importlib.util.spec_from_file_location("recurrence_matrix_reference", MODULE_PATH)
assert SPEC and SPEC.loader
REF = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = REF
SPEC.loader.exec_module(REF)


class RecurrenceMatrixReferenceTests(unittest.TestCase):
    def test_fibonacci_golden_vectors(self) -> None:
        expected = [0,1,1,2,3,5,8,13,21,34,55,89,144]
        self.assertEqual([REF.fibonacci_matrix(n) for n in range(len(expected))], expected)

    def test_tribonacci_golden_vectors(self) -> None:
        expected = [0,0,1,1,2,4,7,13,24,44,81,149,274]
        self.assertEqual([REF.tribonacci_matrix(n) for n in range(len(expected))], expected)

    def test_matrix_power_identity(self) -> None:
        self.assertEqual(REF.matrix_pow(REF.FIBONACCI_COMPANION, 0), ((1,0),(0,1)))
        self.assertEqual(
            REF.matrix_pow(REF.TRIBONACCI_COMPANION, 0),
            ((1,0,0),(0,1,0),(0,0,1)),
        )

    def test_trinity633_vectors(self) -> None:
        vectors = [
            ((1,1,1),1),
            ((2,1,1),64),
            ((1,2,1),8),
            ((1,1,2),8),
            ((2,3,5),216000),
        ]
        for args, expected in vectors:
            self.assertEqual(REF.trinity633(*args), expected)

    def test_semantics_are_co_published_not_fused(self) -> None:
        packet = REF.reference_packet(10,2,3,5)
        self.assertEqual(packet["relation"], "CO_PUBLISHED_NOT_FUSED")
        self.assertEqual(packet["trinity633"]["matrix_semantics"], "NOT_APPLICABLE")

    def test_repository_vectors_match_code(self) -> None:
        vectors = json.loads(
            (ROOT / "data/formulas/recurrence-matrix-vectors.v1.json").read_text(encoding="utf-8")
        )
        self.assertFalse(vectors["claim_allowed"])
        self.assertEqual(
            [REF.fibonacci_matrix(n) for n in range(13)],
            vectors["fibonacci"]["values_n_0_12"],
        )
        self.assertEqual(
            [REF.tribonacci_matrix(n) for n in range(13)],
            vectors["tribonacci"]["values_n_0_12"],
        )
        for vector in vectors["trinity633"]["vectors"]:
            self.assertEqual(REF.trinity633(*vector["input"]), vector["output"])


if __name__ == "__main__":
    unittest.main()
