#!/usr/bin/env python3
"""Tests for bounded cognitive operator reference implementations."""
from __future__ import annotations

import cmath
import importlib.util
import json
import math
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULE = ROOT / "scripts" / "cognitive_operator_reference.py"
SPEC = importlib.util.spec_from_file_location("cognitive_operator_reference", MODULE)
assert SPEC and SPEC.loader
reference = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = reference
SPEC.loader.exec_module(reference)


class CognitiveOperatorReferenceTests(unittest.TestCase):
    def test_operator_15_quadratic(self) -> None:
        result = reference.operator_15_second_derivative(lambda x: 5.0*x*x - 2.0*x + 7.0, 0.3, 1e-4)
        self.assertAlmostEqual(result.value_real, 10.0, places=5)
        self.assertFalse(result.claim_allowed)

    def test_operator_16_backward_difference(self) -> None:
        result = reference.operator_16_backward_second_difference(9.0, 4.0, 1.0, 1.0)
        self.assertEqual(result.value_real, 2.0)

    def test_operator_18_regular_and_singular(self) -> None:
        result = reference.operator_18_implicit_derivative(3.0, -6.0)
        self.assertEqual(result.value_real, 0.5)
        with self.assertRaises(reference.OperatorDomainError):
            reference.operator_18_implicit_derivative(1.0, 0.0)

    def test_operator_19_matches_analytic_derivative(self) -> None:
        tau = 0.37
        result = reference.operator_19_complex_product_derivative(
            amplitude_second=2.0,
            amplitude_first=2.0*tau,
            phase=tau,
            phase_first=1.0,
            denominator=2.0,
        )
        observed = complex(result.value_real, result.value_imag)
        expected = cmath.exp(1j*tau) * (1.0 + 1j*tau)
        self.assertAlmostEqual(observed.real, expected.real, places=12)
        self.assertAlmostEqual(observed.imag, expected.imag, places=12)

    def test_operator_19_rejects_zero_denominator(self) -> None:
        with self.assertRaises(reference.OperatorDomainError):
            reference.operator_19_complex_product_derivative(1.0, 1.0, 0.0, 0.0, 0.0)

    def test_operator_24_exponential_is_zero(self) -> None:
        phi = math.exp(0.8)
        result = reference.operator_24_second_log_derivative(phi, phi, phi)
        self.assertAlmostEqual(result.value_real, 0.0, places=12)

    def test_operator_24_rejects_non_positive_domain(self) -> None:
        with self.assertRaises(reference.OperatorDomainError):
            reference.operator_24_second_log_derivative(0.0, 1.0, 1.0)

    def test_steps_must_be_positive(self) -> None:
        with self.assertRaises(reference.OperatorDomainError):
            reference.operator_15_second_derivative(lambda x: x*x, 0.0, 0.0)
        with self.assertRaises(reference.OperatorDomainError):
            reference.operator_16_backward_second_difference(1.0, 1.0, 1.0, -1.0)

    def test_self_test_passes_all_fixtures(self) -> None:
        report = reference.self_test()
        self.assertEqual(report["state"], "PASS_LOCAL_ANALYTIC_FIXTURES")
        self.assertTrue(all(report["checks"].values()))
        self.assertFalse(report["claim_allowed"])

    def test_cli_writes_report(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "report.json"
            rc = reference.main(["--output", str(output)])
            self.assertEqual(rc, 0)
            report = json.loads(output.read_text(encoding="utf-8"))
            self.assertEqual(report["state"], "PASS_LOCAL_ANALYTIC_FIXTURES")


if __name__ == "__main__":
    unittest.main()
