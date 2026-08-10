#!/usr/bin/env python3

import math
import unittest

from tools.signed_area_geometry import (
    SignedArea,
    complete_square_offset,
    conserved,
    discriminant,
    equivalent_square_side,
    isosceles_area,
    isosceles_height,
    quadratic_roots_real,
    s2,
    s2_inverse,
    s30,
    s30_inverse,
    shear_area_scale,
    total_signed_area,
    transfer_area,
)


class SignedAreaGeometryTests(unittest.TestCase):
    def test_transfer_preserves_total(self):
        before = (7.0, 5.0)
        after = transfer_area(*before, 1.25)
        self.assertTrue(conserved(sum(before), sum(after)))

    def test_signed_area(self):
        value = total_signed_area([SignedArea(9.0, +1), SignedArea(4.0, -1)])
        self.assertTrue(conserved(value, 5.0))

    def test_equilateral_special_case(self):
        h = isosceles_height(1.0, 1.0)
        self.assertTrue(conserved(h, math.sqrt(3.0) / 2.0))
        self.assertTrue(conserved(isosceles_area(1.0, 1.0), math.sqrt(3.0) / 4.0))

    def test_s30_forward_inverse(self):
        x = 7.25
        self.assertTrue(conserved(s30_inverse(s30(x)), x))
        self.assertTrue(conserved(s30(s30(x)), 0.75 * x))

    def test_s2_forward_inverse(self):
        x = 7.25
        self.assertTrue(conserved(s2_inverse(s2(x)), x))

    def test_shear_preserves_area(self):
        for k in (-1e6, -2.0, 0.0, 3.0, 1e6):
            self.assertTrue(conserved(shear_area_scale(k), 1.0))

    def test_equivalent_square(self):
        self.assertTrue(conserved(equivalent_square_side(81.0), 9.0))

    def test_complete_square_offset(self):
        self.assertTrue(conserved(complete_square_offset(2.0, 8.0), 4.0))

    def test_quadratic_roots(self):
        roots = sorted(quadratic_roots_real(1.0, -5.0, 6.0))
        self.assertTrue(conserved(roots[0], 2.0))
        self.assertTrue(conserved(roots[1], 3.0))
        self.assertTrue(conserved(discriminant(1.0, -5.0, 6.0), 1.0))

    def test_invalid_geometry_is_rejected(self):
        with self.assertRaises(ValueError):
            isosceles_height(1.0, 3.0)

    def test_negative_unsigned_square_area_rejected(self):
        with self.assertRaises(ValueError):
            equivalent_square_side(-1.0)


if __name__ == "__main__":
    unittest.main()
