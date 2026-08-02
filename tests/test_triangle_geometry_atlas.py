import importlib.util
import math
import pathlib
import sys
import unittest

MODULE_PATH = pathlib.Path(__file__).parents[1] / "experiments" / "triangle_geometry_atlas" / "triangle_atlas.py"
spec = importlib.util.spec_from_file_location("triangle_atlas", MODULE_PATH)
triangle_atlas = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = triangle_atlas
assert spec.loader is not None
spec.loader.exec_module(triangle_atlas)


class TriangleAtlasTests(unittest.TestCase):
    def test_equilateral_height_and_area(self):
        t = triangle_atlas.equilateral(2.0)
        self.assertAlmostEqual(t.altitudes[0], math.sqrt(3.0), places=12)
        self.assertAlmostEqual(t.area, math.sqrt(3.0), places=12)
        self.assertIn("equilateral", t.triangle_class)

    def test_equilateral_gram(self):
        t = triangle_atlas.equilateral(1.0)
        self.assertAlmostEqual(t.gram[0][0], 1.0, places=12)
        self.assertAlmostEqual(t.gram[1][1], 1.0, places=12)
        self.assertAlmostEqual(t.gram[0][1], 0.5, places=12)

    def test_right_triangle_pythagoras(self):
        t = triangle_atlas.right_from_hypotenuse(1.0, 10.0)
        a, b, c = sorted(t.sides)
        self.assertAlmostEqual(a * a + b * b, c * c, places=12)
        self.assertIn("right", t.triangle_class)

    def test_right_10_degree_area(self):
        t = triangle_atlas.right_from_hypotenuse(1.0, 10.0)
        self.assertAlmostEqual(t.area, 0.25 * math.sin(math.radians(20.0)), places=12)

    def test_isosceles_10_degree_base(self):
        t = triangle_atlas.isosceles_vertex_angle(1.0, 10.0)
        base = min(t.sides)
        self.assertAlmostEqual(base, 2.0 * math.sin(math.radians(5.0)), places=12)
        self.assertIn("isosceles", t.triangle_class)

    def test_isosceles_10_degree_height(self):
        t = triangle_atlas.isosceles_vertex_angle(1.0, 10.0)
        self.assertAlmostEqual(max(t.altitudes), math.cos(math.radians(5.0)), places=12)

    def test_cos10_cubic_identity(self):
        x = math.cos(math.radians(10.0))
        self.assertAlmostEqual(8.0 * x**3 - 6.0 * x, math.sqrt(3.0), places=12)

    def test_spiral_radius_contracts(self):
        p = (1.0, 0.0)
        q = triangle_atlas.spiral_step(p)
        self.assertAlmostEqual(math.hypot(*q), math.sqrt(3.0) / 2.0, places=12)

    def test_six_spiral_steps_radius(self):
        p = (1.0, 0.0)
        for _ in range(6):
            p = triangle_atlas.spiral_step(p)
        self.assertAlmostEqual(math.hypot(*p), (math.sqrt(3.0) / 2.0) ** 6, places=12)

    def test_atlas_blocks_open_problem_overclaims(self):
        atlas = triangle_atlas.canonical_atlas()
        self.assertFalse(atlas["claim_allowed"])
        self.assertEqual(atlas["boundaries"]["navier_stokes_solution_claim"], "PROHIBITED")
        self.assertEqual(atlas["boundaries"]["yang_mills_mass_gap_claim"], "PROHIBITED")


if __name__ == "__main__":
    unittest.main()
