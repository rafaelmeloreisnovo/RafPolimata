"""Falsifiers for the bounded geometric/semantic research fixture.

Copyright (c) 2026 Rafael Melo Reis. No brain/LLM scientific claim.
"""
import json
import math
import unittest
from fractions import Fraction

from scripts import cognitive_spiral_cube_v1 as model


class CognitiveSpiralCubeTests(unittest.TestCase):
    def test_void_is_not_zero_or_character_zero(self):
        self.assertEqual(model.typed_numeric_sum((model.VOID,) * 3),
                         {"state": "TOKEN_VAZIO", "value": None})
        self.assertEqual(model.typed_numeric_sum((0, 0, 0)),
                         {"state": "NUMERIC", "value": 0})
        self.assertEqual(model.typed_numeric_sum((0, None)),
                         {"state": "TOKEN_VAZIO", "value": None})
        with self.assertRaises(model.DomainError):
            model.typed_numeric_sum(("0",))
        with self.assertRaises(model.DomainError):
            model.typed_numeric_sum((True, 1))
        self.assertEqual(model.typed_numeric_sum((1, 1)), {"state": "NUMERIC", "value": 2})

    def test_fibonacci_chars_are_distinct_from_numeric_prefix(self):
        canonical = model.fibonacci_as_characters("01123")
        self.assertEqual(canonical["numeric_digits"], [0, 1, 1, 2, 3])
        self.assertEqual(canonical["codepoints"], [48, 49, 49, 50, 51])
        self.assertEqual(canonical["state"], "CANONICAL_FIB_PREFIX")
        padded = model.fibonacci_as_characters("0001123")
        self.assertEqual(padded["numeric_digits"], [0, 0, 0, 1, 1, 2, 3])
        self.assertNotEqual(padded["state"], canonical["state"])
        self.assertEqual(model.fibonacci_as_characters("123")["numeric_digits"], [1, 2, 3])
        with self.assertRaises(model.DomainError):
            model.fibonacci_as_characters("11123")
        self.assertEqual(model.fibonacci_up_to(8), (0, 1, 1, 2, 3, 5, 8, 13))
        with self.assertRaises(model.DomainError):
            model.fibonacci_up_to(1001)

    def test_grid_is_bijective_and_bounded(self):
        result = {model.index_to_coordinate(i) for i in range(1000)}
        self.assertEqual(len(result), 1000)
        for i in range(1000):
            xyz = model.index_to_coordinate(i)
            self.assertEqual(model.coordinate_to_index(*xyz), i)
        for wrong in (-1, 1000, 1.5, True):
            with self.assertRaises(model.DomainError):
                model.index_to_coordinate(wrong)
        with self.assertRaises(model.DomainError):
            model.coordinate_to_index(10, 0, 0)

    def test_moduli_primality_and_exact_arithmetic(self):
        self.assertEqual(model.MODULI, (7, 3, 35, 10, 13, 70, 14, 50))
        self.assertTrue(all(model.prime_exact(p) for p in (3, 7, 13)))
        self.assertTrue(all(not model.prime_exact(p) for p in (0, 1, 10, 14, 35, 50, 70)))
        self.assertEqual(Fraction(999, 936), Fraction(111, 104))
        self.assertEqual(Fraction(777, 555), Fraction(7, 5))
        self.assertEqual(Fraction(140, 144), Fraction(35, 36))
        self.assertEqual(288, 2 * 12 ** 2)
        self.assertEqual(288000, 2 * 144000)

    def test_loglog_domain_cannot_coerce_void_to_zero(self):
        self.assertAlmostEqual(model.strict_loglog(math.e), 0, places=12)
        for x in (0, -1, 1, float("nan"), float("inf"), False):
            with self.assertRaises(model.DomainError):
                model.strict_loglog(x)
        items = model.ratios_and_domains()
        self.assertEqual([d["exact"] for d in items], ["111/104", "7/5", "35/36"])
        self.assertEqual([d["loglog"]["state"] for d in items],
                         ["DEFINED", "DEFINED", "UNDEFINED_DOMAIN"])

    def test_three_directions_and_radial_positive(self):
        # Same residue/modulus, 3 azimuths differing by exactly 120 degrees:
        # the horizontal components cancel even at a nonzero elevation.
        three = [model.unit_direction(144, arm, 1, 7) for arm in range(3)]
        for direction in three:
            self.assertAlmostEqual(sum(x*x for x in direction), 1.0, places=12)
        self.assertAlmostEqual(sum(x[0] for x in three), 0, places=12)
        self.assertAlmostEqual(sum(x[1] for x in three), 0, places=12)
        fib = model.fibonacci_up_to(1000)
        for i in range(1000):
            r = model.radial_fibonacci(i, fib)
            self.assertGreater(r, 0)
            self.assertLessEqual(r, 1)
            self.assertTrue(math.isfinite(r))
        self.assertNotEqual(math.sqrt(math.pi)/5, math.sqrt(math.pi/5))

    def test_full_grid_vector_bound_and_modular_roots(self):
        fib = model.fibonacci_up_to(1000)
        for i in range(1000):
            cell = model.voxel(i, fib)
            self.assertEqual(model.coordinate_to_index(*cell["coordinate"]), i)
            self.assertEqual(len(cell["directions"]), 3)
            self.assertEqual(len(cell["modular_roots"]), 3)
            for item in cell["modular_roots"]:
                self.assertGreaterEqual(item["remainder"], 0)
                self.assertLess(item["remainder"], item["modulus"])
                self.assertAlmostEqual(item["sqrt_remainder"]**2,
                                       item["remainder"], places=9)
            weights_bound = cell["radius"] * sum(model.ARM_COEFFICIENTS)
            self.assertLessEqual(cell["magnitude"], weights_bound + 2e-12)
        with self.assertRaises(model.DomainError):
            model.voxel(1000, fib)
        with self.assertRaises(model.DomainError):
            model.voxel(0, (0, 1))

    def test_entropy_is_normalized_only_for_discrete_symbols(self):
        self.assertAlmostEqual(model.normalized_entropy((5, 0)), 0)
        self.assertAlmostEqual(model.normalized_entropy((5, 5)), 1)
        for bad in ((), (0, 0), (1,), (1, -1)):
            with self.assertRaises(model.DomainError):
                model.normalized_entropy(bad)

    def test_complete_report_reproducible_without_external_runtime(self):
        report = model.calculate()
        self.assertEqual(report["schema"], "rafaelia.cognitive-spiral-cube/v1")
        self.assertEqual(report["cell_count"], 1000)
        self.assertEqual(report["claim_allowed"], False)
        self.assertEqual(report["external_corpus_evaluation"], "NOT_RUN")
        self.assertEqual(report["physical_android_execution"], "TOKEN_VAZIO")
        h = report["entropy"]
        self.assertEqual(sum(h["histogram"]), 1000)
        self.assertTrue(0 <= h["normalized"] <= 1)
        stats = report["overlap_magnitude_min_median_max"]
        self.assertLessEqual(stats["min"], stats["median"])
        self.assertLessEqual(stats["median"], stats["max"])
        self.assertEqual(len(report["anchor_voxels"]), 16)
        self.assertEqual(json.dumps(report, sort_keys=True),
                         json.dumps(model.calculate(), sort_keys=True))


if __name__ == "__main__":
    unittest.main()
