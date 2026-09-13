import math
import unittest
import reference as r

class TestDGManifoldReference(unittest.TestCase):
    def test_nernst_equilibrium_zero_delta_mu(self):
        e = r.nernst_potential(140.0, 14.0, 1)
        dm = r.electrochemical_delta_mu(140.0, 14.0, 1, e)
        self.assertAlmostEqual(dm, 0.0, places=9)

    def test_osmotic_hydrostatic_reversal(self):
        self.assertGreater(r.solvent_flux(1.0, 12.0, 1.0, 5.0), 0.0)
        self.assertLess(r.solvent_flux(1.0, 4.0, 1.0, 5.0), 0.0)

    def test_shannon_bounds(self):
        h = r.shannon_entropy([0.25, 0.25, 0.25, 0.25])
        self.assertGreaterEqual(h, 0.0)
        self.assertAlmostEqual(h, math.log(4.0), places=12)

    def test_nonnegative_entropy_production_diagonal_onsager(self):
        forces = [2.0, -3.0, 0.5]
        fluxes = r.diagonal_onsager_flux([1.5, 2.0, 4.0], forces)
        self.assertGreaterEqual(r.entropy_production(fluxes, forces), 0.0)

    def test_shannon_not_thermodynamic_identity(self):
        rec = r.distinct_entropy_record(2.0, 2.0)
        self.assertFalse(rec["equal_by_definition"])

if __name__ == "__main__":
    unittest.main()
