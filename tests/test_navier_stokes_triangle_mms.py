import importlib.util
import math
import pathlib
import sys
import unittest

MODULE_PATH = pathlib.Path(__file__).parents[1] / "experiments" / "navier_stokes_triangle_mms" / "mms2d.py"
spec = importlib.util.spec_from_file_location("mms2d", MODULE_PATH)
mms = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = mms
assert spec.loader is not None
spec.loader.exec_module(mms)


class TriangleMMSTests(unittest.TestCase):
    def test_divergence_is_zero(self):
        for point in ((0.0, 0.0), (0.2, 0.3), (-1.5, 2.0), (4.0, -3.0)):
            self.assertAlmostEqual(mms.divergence(*point), 0.0, places=14)

    def test_momentum_residual_is_zero(self):
        for viscosity in (0.0, 0.1, 1.0):
            for point in ((0.0, 0.0), (0.2, 0.3), (-0.7, 1.1)):
                rx, ry = mms.momentum_residual(*point, viscosity)
                self.assertAlmostEqual(rx, 0.0, places=14)
                self.assertAlmostEqual(ry, 0.0, places=14)

    def test_edge_fluxes_telescope(self):
        for tri in mms.canonical_triangle_families().values():
            self.assertAlmostEqual(math.fsum(mms.triangle_edge_fluxes(tri)), 0.0, places=14)

    def test_orientation_reversal_preserves_zero_balance(self):
        tri = ((0.0, 0.0), (1.2, 0.1), (0.3, 0.9))
        reversed_tri = (tri[0], tri[2], tri[1])
        self.assertAlmostEqual(mms.triangle_report(tri).flux_balance, 0.0, places=14)
        self.assertAlmostEqual(mms.triangle_report(reversed_tri).flux_balance, 0.0, places=14)
        self.assertAlmostEqual(mms.signed_area(tri), -mms.signed_area(reversed_tri), places=14)

    def test_canonical_study_is_fail_closed(self):
        study = mms.canonical_study()
        self.assertFalse(study["claim_allowed"])
        self.assertEqual(study["boundaries"]["navier_stokes_3d_regularity_solution"], "NOT_CLAIMED")
        self.assertEqual(study["boundaries"]["numerical_solver_convergence"], "TOKEN_VAZIO")
        self.assertLessEqual(study["summary"]["max_flux_balance_error"], 1e-14)
        self.assertLessEqual(study["summary"]["max_momentum_residual"], 1e-14)

    def test_degenerate_triangle_rejected(self):
        with self.assertRaises(ValueError):
            mms.triangle_report(((0.0, 0.0), (1.0, 1.0), (2.0, 2.0)))

    def test_negative_viscosity_rejected(self):
        with self.assertRaises(ValueError):
            mms.body_force(0.2, 0.3, -0.1)
        with self.assertRaises(ValueError):
            mms.triangle_report(((0.0, 0.0), (1.0, 0.0), (0.0, 1.0)), -0.1)


if __name__ == "__main__":
    unittest.main()
