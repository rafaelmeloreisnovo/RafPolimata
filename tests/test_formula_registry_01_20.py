from __future__ import annotations

import importlib.util
import json
import math
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "research" / "formula_registry_01_20" / "reference.py"
SPEC = importlib.util.spec_from_file_location("formula_reference", MODULE_PATH)
assert SPEC and SPEC.loader
ref = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = ref
SPEC.loader.exec_module(ref)


class FormulaRegistryTests(unittest.TestCase):
    def test_registry_has_20_non_promoted_records(self) -> None:
        path = ROOT / "research" / "formula_registry_01_20" / "registry.v1.json"
        doc = json.loads(path.read_text(encoding="utf-8"))
        self.assertFalse(doc["claim_allowed"])
        self.assertEqual([r["id"] for r in doc["records"]], [f"F{i:02d}" for i in range(1, 21)])
        self.assertTrue(all(r["claim_allowed"] is False for r in doc["records"]))

    def test_f01_seven_dimensional_port_hamiltonian_type(self) -> None:
        j = [[0.0] * 7 for _ in range(7)]
        j[0][1] = 2.0
        j[1][0] = -2.0
        g = [[0.0] * 7 for _ in range(7)]
        g[0][0] = 1.0
        grad = [1.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        out = ref.t7_port_hamiltonian_drift(j, g, grad)
        self.assertEqual(len(out), 7)
        self.assertAlmostEqual(out[0], 5.0)
        self.assertAlmostEqual(out[1], -2.0)
        bad = [row[:] for row in j]
        bad[1][0] = 2.0
        with self.assertRaises(ValueError):
            ref.t7_port_hamiltonian_drift(bad, g, grad)

    def test_f03_truncated_operator_coefficients(self) -> None:
        coeff = ref.g_inv_series_coefficients(4)
        self.assertEqual(set(coeff), {1, 3, 5, 7})
        self.assertAlmostEqual(coeff[1], -1.0)
        self.assertAlmostEqual(coeff[3], 0.5)
        self.assertAlmostEqual(coeff[5], -1.0 / 6.0)
        self.assertAlmostEqual(coeff[7], 1.0 / 24.0)

    def test_f07_psi42_is_normalized(self) -> None:
        amps = ref.psi42_amplitudes()
        self.assertEqual(len(amps), 42)
        norm2 = math.fsum(abs(a) ** 2 for a in amps)
        self.assertAlmostEqual(norm2, 1.0, places=14)

    def test_f08_memory_aux_exact_step(self) -> None:
        result = ref.memory_aux_exact_step(2.0, 0.0, 1.0, 1.0)
        self.assertAlmostEqual(result, 2.0 * (1.0 - math.exp(-1.0)), places=14)
        self.assertAlmostEqual(ref.memory_aux_exact_step(2.0, 3.0, 0.5, 0.0), 4.0)

    def test_f09_corrected_value_and_convergence(self) -> None:
        expected = 0.44968042643383016
        self.assertAlmostEqual(ref.fibonacci_rafael_corrected(), expected, places=14)
        self.assertAlmostEqual(ref.fibonacci_rafael_corrected_partial(256), expected, places=13)

    def test_f12_character_is_periodic(self) -> None:
        a = ref.torus_fourier_character(3, -2, 0.17, 0.31)
        b = ref.torus_fourier_character(3, -2, 1.17, -0.69)
        self.assertAlmostEqual(abs(a - b), 0.0, places=13)

    def test_f16_probability_is_bounded(self) -> None:
        for values in ([1000.0], [-1000.0], [2.0, -1.0, 0.5]):
            p = ref.fragmentation_probability(values)
            self.assertGreaterEqual(p, 0.0)
            self.assertLessEqual(p, 1.0)

    def test_f20_decision_confidence_separation(self) -> None:
        decision, confidence, margin = ref.ethical_hyperplane(
            [1.0] * 7, [0.2] * 7, 1.0, temperature=0.5
        )
        self.assertEqual(decision, 1)
        self.assertAlmostEqual(margin, 0.4)
        self.assertGreater(confidence, 0.5)
        self.assertLess(confidence, 1.0)


if __name__ == "__main__":
    unittest.main()
