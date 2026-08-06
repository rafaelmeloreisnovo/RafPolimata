#!/usr/bin/env python3
"""Bounded reference implementations for RAFAELIA operators 15, 16, 18, 19 and 24.

These functions implement normalized mathematical operators, not the submitted
metaphorical names. Existence of code is not runtime or scientific proof.
"""
from __future__ import annotations

import argparse
import cmath
import json
import math
from dataclasses import asdict, dataclass
from typing import Callable


class OperatorDomainError(ValueError):
    """Raised when an operator is evaluated outside its declared domain."""


@dataclass(frozen=True)
class OperatorResult:
    operator_id: str
    value_real: float
    value_imag: float = 0.0
    claim_allowed: bool = False
    state: str = "VERIFIED_LIMITED_LOCAL"

    @classmethod
    def from_value(cls, operator_id: str, value: float | complex) -> "OperatorResult":
        z = complex(value)
        return cls(operator_id=operator_id, value_real=float(z.real), value_imag=float(z.imag))


def _positive_step(step: float) -> float:
    if not math.isfinite(step) or step <= 0.0:
        raise OperatorDomainError("step must be finite and > 0")
    return float(step)


def operator_15_second_derivative(
    function: Callable[[float], float | complex],
    tau: float,
    step: float,
) -> OperatorResult:
    """Central second derivative: [f(t+h)-2f(t)+f(t-h)]/h²."""
    h = _positive_step(step)
    value = (function(tau + h) - 2.0 * function(tau) + function(tau - h)) / (h * h)
    return OperatorResult.from_value("COG-015", value)


def operator_16_backward_second_difference(
    current: float | complex,
    previous: float | complex,
    previous_previous: float | complex,
    step: float,
) -> OperatorResult:
    """Backward second difference on a uniform grid."""
    h = _positive_step(step)
    value = (current - 2.0 * previous + previous_previous) / (h * h)
    return OperatorResult.from_value("COG-016", value)


def operator_18_implicit_derivative(
    partial_phi: float | complex,
    partial_psi: float | complex,
    *,
    singular_tolerance: float = 1e-15,
) -> OperatorResult:
    """Implicit-function derivative -F_phi/F_psi."""
    denominator = complex(partial_psi)
    if abs(denominator) <= singular_tolerance:
        raise OperatorDomainError("partial_psi is singular or below tolerance")
    return OperatorResult.from_value("COG-018", -complex(partial_phi) / denominator)


def operator_19_complex_product_derivative(
    amplitude_second: float | complex,
    amplitude_first: float | complex,
    phase: float,
    phase_first: float,
    denominator: float | complex,
    *,
    singular_tolerance: float = 1e-15,
) -> OperatorResult:
    """Derivative of (amplitude_first * exp(i*phase))/denominator.

    The denominator is treated as constant in tau. A varying denominator belongs
    to a different operator and must include quotient-rule terms.
    """
    divisor = complex(denominator)
    if abs(divisor) <= singular_tolerance:
        raise OperatorDomainError("denominator is singular or below tolerance")
    phase_factor = cmath.exp(1j * phase)
    value = (
        complex(amplitude_second) * phase_factor
        + 1j * complex(amplitude_first) * phase_first * phase_factor
    ) / divisor
    return OperatorResult.from_value("COG-019", value)


def operator_24_second_log_derivative(
    phi: float,
    phi_first: float,
    phi_second: float,
    *,
    positive_tolerance: float = 0.0,
) -> OperatorResult:
    """Second derivative of ln(phi) for real phi > 0."""
    if not all(math.isfinite(x) for x in (phi, phi_first, phi_second)):
        raise OperatorDomainError("phi and derivatives must be finite")
    if phi <= positive_tolerance:
        raise OperatorDomainError("phi must be strictly positive")
    value = (phi * phi_second - phi_first * phi_first) / (phi * phi)
    return OperatorResult.from_value("COG-024", value)


def self_test() -> dict:
    """Run analytic fixtures without third-party dependencies."""
    h = 1e-4
    quadratic = operator_15_second_derivative(lambda t: 3.0 * t * t + 2.0 * t + 1.0, 0.7, h)
    backward = operator_16_backward_second_difference(4.0, 1.0, 0.0, 1.0)
    implicit = operator_18_implicit_derivative(1.0, 4.0)

    tau = 0.3
    complex_value = operator_19_complex_product_derivative(
        amplitude_second=2.0,
        amplitude_first=2.0 * tau,
        phase=tau,
        phase_first=1.0,
        denominator=2.0,
    )
    expected_complex = cmath.exp(1j * tau) * (1.0 + 1j * tau)

    phi = math.exp(0.4)
    log_second = operator_24_second_log_derivative(phi, phi, phi)

    checks = {
        "COG-015": abs(quadratic.value_real - 6.0) < 1e-6,
        "COG-016": abs(backward.value_real - 2.0) < 1e-12,
        "COG-018": abs(implicit.value_real + 0.25) < 1e-12,
        "COG-019": abs(complex(complex_value.value_real, complex_value.value_imag) - expected_complex) < 1e-12,
        "COG-024": abs(log_second.value_real) < 1e-12,
    }
    return {
        "schema": "rafaelia.cognitive-operator-reference-report/v1",
        "claim_allowed": False,
        "state": "PASS_LOCAL_ANALYTIC_FIXTURES" if all(checks.values()) else "FAIL_LOCAL_ANALYTIC_FIXTURES",
        "checks": checks,
        "results": [
            asdict(quadratic),
            asdict(backward),
            asdict(implicit),
            asdict(complex_value),
            asdict(log_second),
        ],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output")
    args = parser.parse_args(argv)
    report = self_test()
    text = json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    if args.output:
        from pathlib import Path
        output = Path(args.output)
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text, encoding="utf-8")
    print(text, end="")
    return 0 if report["state"].startswith("PASS") else 1


if __name__ == "__main__":
    raise SystemExit(main())
