#!/usr/bin/env python3
"""Executable reference subset for RAFAELIA formula registry 01-20.

This module intentionally implements only mathematically well-scoped corrections.
It does not promote physical, biological or ethical claims.  All functions are
stdlib-only and deterministic so they can be exercised in CI and Termux.
"""
from __future__ import annotations

import cmath
import math
from collections.abc import Sequence


def sigmoid(x: float) -> float:
    """Numerically stable logistic function."""
    if x >= 0.0:
        z = math.exp(-x)
        return 1.0 / (1.0 + z)
    z = math.exp(x)
    return z / (1.0 + z)


def _square_matrix(matrix: Sequence[Sequence[float]], n: int) -> list[list[float]]:
    rows = [list(map(float, row)) for row in matrix]
    if len(rows) != n or any(len(row) != n for row in rows):
        raise ValueError(f"expected {n}x{n} matrix")
    return rows


def t7_port_hamiltonian_drift(
    j: Sequence[Sequence[float]],
    g: Sequence[Sequence[float]],
    grad_h: Sequence[float],
    *,
    antisym_tol: float = 1e-12,
) -> list[float]:
    """F01 corrected reference: (J-G) grad(H) in seven dimensions.

    The routine checks the type-level invariant J^T=-J.  It does not attempt a
    general PSD proof for G; that remains a model-specific proof obligation.
    """
    n = 7
    jj = _square_matrix(j, n)
    gg = _square_matrix(g, n)
    grad = list(map(float, grad_h))
    if len(grad) != n:
        raise ValueError("expected seven-dimensional gradient")
    for r in range(n):
        for c in range(n):
            if abs(jj[r][c] + jj[c][r]) > antisym_tol:
                raise ValueError("J must be antisymmetric")
    return [
        sum((jj[r][c] - gg[r][c]) * grad[c] for c in range(n))
        for r in range(n)
    ]


def g_inv_series_coefficients(order: int) -> dict[int, float]:
    """F03: coefficients of sum (-1)^k/k! D^(2k-1), k=1..order.

    The formal spectral identity requires an explicit scale/domain before it is
    interpreted dimensionally.  This helper exposes the exact truncated series.
    """
    if order < 1:
        raise ValueError("order must be >= 1")
    return {2 * k - 1: ((-1.0) ** k) / math.factorial(k) for k in range(1, order + 1)}


def harmonic_number(n: int) -> float:
    if n < 1:
        raise ValueError("n must be >= 1")
    return math.fsum(1.0 / k for k in range(1, n + 1))


def psi42_amplitudes(phases: Sequence[float] | None = None) -> list[complex]:
    """F07 corrected reference: normalized 42-component amplitude vector.

    It models 42 basis indices only.  It does not claim 42 dynamical attractors.
    """
    n = 42
    theta = [0.0] * n if phases is None else list(map(float, phases))
    if len(theta) != n:
        raise ValueError("expected exactly 42 phases")
    norm = math.sqrt(harmonic_number(n))
    return [cmath.exp(1j * theta[k - 1]) / math.sqrt(k) / norm for k in range(1, n + 1)]


def memory_aux_exact_step(h_constant: float, m0: float, dt: float, lam: float) -> float:
    """F08: exact step for M_t = H - lambda*M when H is constant on the step."""
    if dt < 0.0:
        raise ValueError("dt must be non-negative")
    if lam < 0.0:
        raise ValueError("lambda must be non-negative")
    h = float(h_constant)
    m = float(m0)
    if lam == 0.0:
        return m + dt * h
    decay = math.exp(-lam * dt)
    return decay * m + (1.0 - decay) * h / lam


def fibonacci_rafael_corrected() -> float:
    """F09 corrected convergent invariant.

    alpha* = 1/sqrt(5) * sum(phi^-i cos(2*pi*i/7), i>=0).
    """
    phi = (1.0 + math.sqrt(5.0)) / 2.0
    r = 1.0 / phi
    theta = 2.0 * math.pi / 7.0
    numerator = 1.0 - r * math.cos(theta)
    denominator = 1.0 - 2.0 * r * math.cos(theta) + r * r
    return numerator / denominator / math.sqrt(5.0)


def fibonacci_rafael_corrected_partial(terms: int) -> float:
    if terms < 1:
        raise ValueError("terms must be >= 1")
    phi = (1.0 + math.sqrt(5.0)) / 2.0
    return math.fsum(
        (phi ** (-i)) * math.cos(2.0 * math.pi * i / 7.0) / math.sqrt(5.0)
        for i in range(terms)
    )


def torus_fourier_character(m: int, n: int, x: float, y: float) -> complex:
    """F12 corrected T^2 periodic character exp(-2*pi*i*(m*x+n*y))."""
    return cmath.exp(-2j * math.pi * (int(m) * float(x) + int(n) * float(y)))


def fragmentation_probability(contributions: Sequence[float]) -> float:
    """F16 corrected probability: sigmoid(sum of latent contributions)."""
    return sigmoid(math.fsum(map(float, contributions)))


def ethical_hyperplane(
    metrics: Sequence[float],
    weights: Sequence[float],
    theta: float,
    *,
    temperature: float = 1.0,
) -> tuple[int, float, float]:
    """F20 corrected separation of decision, confidence and raw margin.

    This is a generic classifier primitive, not a proof that the supplied metrics
    are ethically valid.  Metric validity remains an external governance gate.
    """
    mm = list(map(float, metrics))
    ww = list(map(float, weights))
    if len(mm) != 7 or len(ww) != 7:
        raise ValueError("expected seven metrics and seven weights")
    if temperature <= 0.0:
        raise ValueError("temperature must be > 0")
    z = math.fsum(w * x for w, x in zip(ww, mm)) - float(theta)
    decision = 1 if z > 0.0 else (-1 if z < 0.0 else 0)
    confidence = sigmoid(abs(z) / temperature)
    return decision, confidence, z
