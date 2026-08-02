import importlib.util
import pathlib
import sys
import unittest

MODULE_PATH = pathlib.Path(__file__).parents[1] / "experiments" / "yang_mills_u1_triangle" / "u1_triangle.py"
spec = importlib.util.spec_from_file_location("u1_triangle", MODULE_PATH)
u1 = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = u1
assert spec.loader is not None
spec.loader.exec_module(u1)


class U1TriangleTests(unittest.TestCase):
    def setUp(self):
        self.triangle = ((-0.2, 0.1), (1.3, -0.1), (0.4, 1.2))
        self.field = 0.7

    def test_links_have_unit_modulus(self):
        for link in u1.triangle_links(self.triangle, self.field):
            self.assertAlmostEqual(abs(link), 1.0, places=14)

    def test_constant_field_holonomy_equals_flux_area(self):
        observed = u1.triangle_holonomy(self.triangle, self.field)
        expected = u1.expected_constant_field_holonomy(self.triangle, self.field)
        self.assertLessEqual(abs(observed - expected), 1e-14)

    def test_gauge_transformation_preserves_holonomy(self):
        links = u1.triangle_links(self.triangle, self.field)
        before = u1.holonomy(links)
        after = u1.holonomy(u1.gauge_transform_triangle(links, (0.2, -0.9, 1.7)))
        self.assertLessEqual(abs(before - after), 1e-14)

    def test_reversed_orientation_conjugates_holonomy(self):
        reversed_triangle = (self.triangle[0], self.triangle[2], self.triangle[1])
        forward = u1.triangle_holonomy(self.triangle, self.field)
        reverse = u1.triangle_holonomy(reversed_triangle, self.field)
        self.assertLessEqual(abs(reverse - forward.conjugate()), 1e-14)

    def test_refinement_product_matches_whole_face(self):
        whole = u1.triangle_holonomy(self.triangle, self.field)
        refined = u1.refinement_holonomy_product(self.triangle, self.field)
        self.assertLessEqual(abs(whole - refined), 1e-14)

    def test_zero_field_is_identity(self):
        observed = u1.triangle_holonomy(self.triangle, 0.0)
        self.assertLessEqual(abs(observed - (1.0 + 0.0j)), 1e-14)
        self.assertAlmostEqual(u1.wilson_action(observed), 0.0, places=14)

    def test_wilson_action_is_nonnegative(self):
        observed = u1.triangle_holonomy(self.triangle, self.field)
        self.assertGreaterEqual(u1.wilson_action(observed), 0.0)

    def test_non_unit_link_rejected(self):
        with self.assertRaises(ValueError):
            u1.holonomy((2.0 + 0.0j, 1.0 + 0.0j, 1.0 + 0.0j))

    def test_degenerate_triangle_rejected(self):
        with self.assertRaises(ValueError):
            u1.triangle_report(((0.0, 0.0), (1.0, 1.0), (2.0, 2.0)), self.field)

    def test_canonical_study_is_fail_closed(self):
        study = u1.canonical_study(self.field)
        self.assertFalse(study["claim_allowed"])
        self.assertEqual(study["boundaries"]["mass_gap"], "NOT_CLAIMED")
        self.assertEqual(study["boundaries"]["continuum_limit"], "TOKEN_VAZIO")
        self.assertLessEqual(study["invariants"]["gauge_holonomy_error"], 1e-14)
        self.assertLessEqual(study["invariants"]["refinement_holonomy_error"], 1e-14)
        self.assertLessEqual(study["invariants"]["max_constant_field_error"], 1e-14)


if __name__ == "__main__":
    unittest.main()
