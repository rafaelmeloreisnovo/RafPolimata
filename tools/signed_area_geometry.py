#!/usr/bin/env python3
"""RafPolimata evidence kernel: signed-area reversible geometry.

Stdlib-only. Validates explicit identities and conservation laws.
No claim of mathematical novelty is made by this module.
"""

from __future__ import annotations

from dataclasses import dataclass
from math import isclose, sqrt
from typing import Iterable

EPS = 1e-12


@dataclass(frozen=True)
class SignedArea:
    magnitude: float
    sign: int = 1

    def value(self) -> float:
        if self.sign not in (-1, 1):
            raise ValueError("sign must be -1 or +1")
        if self.magnitude < 0:
            raise ValueError("area magnitude must be non-negative")
        return self.sign * self.magnitude


def total_signed_area(parts: Iterable[SignedArea]) -> float:
    return sum(p.value() for p in parts)


def transfer_area(a1: float, a2: float, delta: float) -> tuple[float, float]:
    """Move delta from region 1 to region 2; the total is invariant."""
    return a1 - delta, a2 + delta


def isosceles_height(equal_side: float, base: float) -> float:
    if equal_side <= 0 or base <= 0:
        raise ValueError("lengths must be positive")
    radicand = equal_side * equal_side - (base * base) / 4.0
    if radicand < -EPS:
        raise ValueError("invalid isosceles geometry")
    return sqrt(max(0.0, radicand))


def isosceles_area(equal_side: float, base: float) -> float:
    return 0.5 * base * isosceles_height(equal_side, base)


def equivalent_square_side(area: float) -> float:
    if area < 0:
        raise ValueError("unsigned square area must be non-negative")
    return sqrt(area)


def s30(length: float) -> float:
    """Equilateral-height / 30° projection factor sqrt(3)/2."""
    return length * sqrt(3.0) / 2.0


def s30_inverse(length: float) -> float:
    return length * 2.0 / sqrt(3.0)


def s2(length: float) -> float:
    """Square-diagonal factor sqrt(2)."""
    return length * sqrt(2.0)


def s2_inverse(length: float) -> float:
    return length / sqrt(2.0)


def determinant_2x2(m00: float, m01: float, m10: float, m11: float) -> float:
    return m00 * m11 - m01 * m10


def linear_area_scale(m00: float, m01: float, m10: float, m11: float) -> float:
    return abs(determinant_2x2(m00, m01, m10, m11))


def shear_area_scale(k: float) -> float:
    return linear_area_scale(1.0, k, 0.0, 1.0)


def complete_square_offset(a: float, b: float) -> float:
    """Area term (b/2a)^2 used after normalizing ax²+bx+c by a."""
    if a == 0:
        raise ValueError("a must be non-zero")
    t = b / (2.0 * a)
    return t * t


def discriminant(a: float, b: float, c: float) -> float:
    return b * b - 4.0 * a * c


def quadratic_roots_real(a: float, b: float, c: float) -> tuple[float, float]:
    if a == 0:
        raise ValueError("a must be non-zero")
    disc = discriminant(a, b, c)
    if disc < 0:
        raise ValueError("real-roots kernel requires discriminant >= 0")
    r = sqrt(disc)
    return ((-b + r) / (2.0 * a), (-b - r) / (2.0 * a))


def conserved(before: float, after: float, *, eps: float = EPS) -> bool:
    return isclose(before, after, rel_tol=eps, abs_tol=eps)


def self_check() -> dict[str, bool]:
    length = 1.0
    a1, a2 = 7.0, 5.0
    b1, b2 = transfer_area(a1, a2, 1.25)

    checks = {
        "transfer_conserves_area": conserved(a1 + a2, b1 + b2),
        "s30_inverse": conserved(length, s30_inverse(s30(length))),
        "s2_inverse": conserved(length, s2_inverse(s2(length))),
        "s30_squared_is_3_over_4": conserved(s30(s30(length)), 0.75 * length),
        "equilateral_height": conserved(isosceles_height(1.0, 1.0), sqrt(3.0) / 2.0),
        "shear_preserves_area": conserved(shear_area_scale(42.0), 1.0),
        "signed_area_cancels": conserved(
            total_signed_area([SignedArea(3.0, +1), SignedArea(1.0, -1)]), 2.0
        ),
        "complete_square_offset": conserved(complete_square_offset(1.0, 2.0), 1.0),
    }
    return checks


if __name__ == "__main__":
    checks = self_check()
    for name, ok in checks.items():
        print(f"{name}: {'PASS' if ok else 'FAIL'}")
    raise SystemExit(0 if all(checks.values()) else 1)
