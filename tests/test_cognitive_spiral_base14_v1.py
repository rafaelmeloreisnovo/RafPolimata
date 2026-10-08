"""Radix-14 falsifiers: representation, geometry, 3 arms, 14³ census.

Copyright (c) 2026 Rafael Melo Reis. No neural or LLM-internals claim.
"""
import json
import math
import unittest
from scripts import cognitive_spiral_base14_v1 as model
from scripts import cognitive_spiral_cube_v1 as legacy


class Radix14SpiralTests(unittest.TestCase):
    def test_round_cube_and_legacy_remain_distinct(self):
        self.assertEqual(model.RADIX, 14)
        self.assertEqual(model.CELL_COUNT, 2744)
        self.assertEqual(model.encode14(2744), "1000")
        self.assertEqual(model.decode14("1000"), 2744)
        self.assertEqual(model.encode14(legacy.CELL_COUNT), "516")
        self.assertEqual(model.decode14("516"), 1000)
        self.assertEqual(legacy.CELL_COUNT, 1000)

    def test_canonical_digits_and_moduli(self):
        self.assertEqual(model.DIGITS, "0123456789ABCD")
        examples = {0: "0", 7: "7", 10: "A", 13: "D", 14: "10",
                    35: "27", 50: "38", 70: "50", 140: "A0",
                    144: "A4", 288: "168", 999: "515", 1000: "516"}
        for decimal, base14 in examples.items():
            self.assertEqual(model.encode14(decimal), base14)
            self.assertEqual(model.decode14(base14), decimal)
        self.assertEqual([model.encode14(m) for m in legacy.MODULI],
                         ["7", "3", "27", "A", "D", "50", "10", "38"])
        self.assertEqual(legacy.MODULI, (7, 3, 35, 10, 13, 70, 14, 50))

    def test_codec_roundtrip_exhaustive_14_cube(self):
        for i in range(model.CELL_COUNT):
            self.assertEqual(model.decode14(model.encode14(i)), i)
            self.assertEqual(model.coordinate14_to_index(*model.index14_to_coordinate(i)), i)
        self.assertEqual(model.index14_to_coordinate(2743), (13, 13, 13))
        self.assertEqual(model.coordinate14_to_index(13, 13, 13), 2743)

    def test_codec_rejects_ambiguous_and_noncanonical_tokens(self):
        for token in ("", "00", "014", "a", "G", "-1", "+A", "14_14", " 1", "1 ", None, 1):
            with self.assertRaises(legacy.DomainError):
                model.decode14(token)
        for value in (True, -1, 0.5, "14"):
            with self.assertRaises(legacy.DomainError):
                model.encode14(value)
        for point in ((14, 0, 0), (13, 14, 13), (-1, 0, 0), (1, 1, True)):
            with self.assertRaises(legacy.DomainError):
                model.coordinate14_to_index(*point)
        for i in (True, -1, 2744, 1.1):
            with self.assertRaises(legacy.DomainError):
                model.index14_to_coordinate(i)

    def test_fibonacci_canonical_and_bounds(self):
        self.assertEqual(model.fibonacci_bounded(9), (0, 1, 1, 2, 3, 5, 8, 13, 21))
        self.assertEqual(len(model.fibonacci_bounded(model.CELL_COUNT)), 2744)
        for invalid in (0, -1, 2745, True, 1.0):
            with self.assertRaises(legacy.DomainError):
                model.fibonacci_bounded(invalid)
        self.assertEqual(legacy.fibonacci_up_to(5), (0, 1, 1, 2, 3))

    def test_same_angle_three_arms_cancel_in_xy(self):
        triple = [model.direction14(144, arm, 1, 7) for arm in range(3)]
        for vec in triple:
            self.assertAlmostEqual(math.sqrt(sum(x*x for x in vec)), 1.0, places=12)
        self.assertAlmostEqual(sum(v[0] for v in triple), 0.0, places=12)
        self.assertAlmostEqual(sum(v[1] for v in triple), 0.0, places=12)

    def test_all_2744_voxels_and_modular_bounds(self):
        fib = model.fibonacci_bounded(model.CELL_COUNT)
        for i in range(model.CELL_COUNT):
            cell = model.voxel14(i, fib)
            self.assertEqual(cell["index_base14"], model.encode14(i))
            self.assertEqual([model.DIGITS.find(d) for d in cell["coordinate_base14"]],
                             cell["coordinate_dec"])
            self.assertEqual(len(cell["modular_roots"]), 3)
            self.assertEqual(len(cell["directions"]), 3)
            self.assertTrue(0 <= cell["radius"] <= 1)
            self.assertGreaterEqual(cell["magnitude"], 0)
            self.assertLessEqual(cell["magnitude"],
                                 cell["radius"] * sum(legacy.ARM_COEFFICIENTS) + 2e-12)
            for m in cell["modular_roots"]:
                self.assertEqual(model.decode14(m["modulus_base14"]), m["modulus_dec"])
                self.assertEqual(model.decode14(m["remainder_base14"]), m["remainder_dec"])
                self.assertTrue(0 <= m["remainder_dec"] < m["modulus_dec"])
        with self.assertRaises(legacy.DomainError):
            model.voxel14(2744, fib)
        with self.assertRaises(legacy.DomainError):
            model.voxel14(0, (0, 1))

    def test_census_entropy_ema_permutation_and_typed_claim(self):
        doc = model.calculate14()
        self.assertEqual(doc["schema"], "rafaelia.cognitive-spiral-cube-radix14/v1")
        self.assertEqual((doc["side_dec"], doc["side_base14"]), (14, "10"))
        self.assertEqual((doc["cells_dec"], doc["cells_base14"]), (2744, "1000"))
        self.assertEqual(doc["legacy_cells_in_base14"], "516")
        self.assertTrue(doc["geometry_changed"])
        self.assertFalse(doc["claim_allowed"])
        self.assertEqual(doc["physical_android_execution"], "TOKEN_VAZIO")
        self.assertEqual(doc["neural_validation"], "NOT_RUN")
        self.assertEqual(len(doc["anchors"]), len(model.MATERIALIZED_ANCHORS))
        self.assertEqual(sum(doc["entropy"]["histogram"]), model.CELL_COUNT)
        self.assertEqual(len(doc["entropy"]["histogram"]), 14)
        self.assertTrue(0 <= doc["entropy"]["normalized"] <= 1)
        ema = doc["adaptive_layer_ema"]
        self.assertEqual(ema["alpha"], 0.25)
        self.assertEqual(len(ema["forward"]), 14)
        self.assertNotEqual(
            (ema["forward"][-1]["C_ema"], ema["forward"][-1]["H_ema"]),
            (ema["reverse_order_final"]["C_ema"], ema["reverse_order_final"]["H_ema"]))
        self.assertEqual(json.dumps(doc, sort_keys=True),
                         json.dumps(model.calculate14(), sort_keys=True))


if __name__ == "__main__":
    unittest.main()
