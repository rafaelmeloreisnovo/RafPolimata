#!/usr/bin/env python3
"""Deterministic triangle geometry atlas using only the Python standard library."""
from __future__ import annotations

from dataclasses import asdict, dataclass
from math import acos, cos, degrees, hypot, isclose, pi, sin, sqrt
from typing import Iterable, Sequence

EPS = 1e-12


@dataclass(frozen=True)
class TriangleState:
    vertices: tuple[tuple[float, float], tuple[float, float], tuple[float, float]]
    sides: tuple[float, float, float]  # a=|BC|, b=|CA|, c=|AB|
    angles_deg: tuple[float, float, float]
    gram: tuple[tuple[float, float], tuple[float, float]]
    area: float
    altitudes: tuple[float, float, float]
    triangle_class: tuple[str, ...]

    def to_dict(self) -> dict:
        return asdict(self)


def _sub(p: Sequence[float], q: Sequence[float]) -> tuple[float, float]:
    return (float(p[0]) - float(q[0]), float(p[1]) - float(q[1]))


def _dot(u: Sequence[float], v: Sequence[float]) -> float:
    return float(u[0]) * float(v[0]) + float(u[1]) * float(v[1])


def gram_from_vertices(
    a: Sequence[float], b: Sequence[float], c: Sequence[float]
) -> tuple[tuple[float, float], tuple[float, float]]:
    u = _sub(b, a)
    v = _sub(c, a)
    return ((_dot(u, u), _dot(u, v)), (_dot(v, u), _dot(v, v)))


def sides_from_vertices(
    a: Sequence[float], b: Sequence[float], c: Sequence[float]
) -> tuple[float, float, float]:
    side_a = hypot(c[0] - b[0], c[1] - b[1])
    side_b = hypot(a[0] - c[0], a[1] - c[1])
    side_c = hypot(b[0] - a[0], b[1] - a[1])
    return (side_a, side_b, side_c)


def area_from_gram(gram: Sequence[Sequence[float]]) -> float:
    det = gram[0][0] * gram[1][1] - gram[0][1] * gram[1][0]
    if det < -EPS:
        raise ValueError("Gram determinant cannot be negative for Euclidean vectors")
    return 0.5 * sqrt(max(0.0, det))


def _angle_from_sides(opposite: float, adjacent_1: float, adjacent_2: float) -> float:
    denom = 2.0 * adjacent_1 * adjacent_2
    if denom <= EPS:
        raise ValueError("Degenerate triangle")
    value = (adjacent_1**2 + adjacent_2**2 - opposite**2) / denom
    value = min(1.0, max(-1.0, value))
    return degrees(acos(value))


def classify_triangle(sides: Sequence[float], angles_deg: Sequence[float]) -> tuple[str, ...]:
    a, b, c = sides
    labels: list[str] = []
    eq_ab = isclose(a, b, rel_tol=1e-10, abs_tol=1e-10)
    eq_bc = isclose(b, c, rel_tol=1e-10, abs_tol=1e-10)
    eq_ca = isclose(c, a, rel_tol=1e-10, abs_tol=1e-10)
    if eq_ab and eq_bc:
        labels.append("equilateral")
    elif eq_ab or eq_bc or eq_ca:
        labels.append("isosceles")
    else:
        labels.append("scalene")

    max_angle = max(angles_deg)
    if isclose(max_angle, 90.0, rel_tol=1e-10, abs_tol=1e-9):
        labels.append("right")
    elif max_angle > 90.0:
        labels.append("obtuse")
    else:
        labels.append("acute")
    return tuple(labels)


def triangle_state(vertices: Iterable[Sequence[float]]) -> TriangleState:
    pts = tuple((float(p[0]), float(p[1])) for p in vertices)
    if len(pts) != 3:
        raise ValueError("Exactly three vertices are required")
    a_pt, b_pt, c_pt = pts
    gram = gram_from_vertices(a_pt, b_pt, c_pt)
    sides = sides_from_vertices(a_pt, b_pt, c_pt)
    area = area_from_gram(gram)
    if area <= EPS or min(sides) <= EPS:
        raise ValueError("Degenerate triangle")
    a, b, c = sides
    angles = (
        _angle_from_sides(a, b, c),
        _angle_from_sides(b, c, a),
        _angle_from_sides(c, a, b),
    )
    altitudes = (2.0 * area / a, 2.0 * area / b, 2.0 * area / c)
    return TriangleState(
        vertices=pts,
        sides=sides,
        angles_deg=angles,
        gram=gram,
        area=area,
        altitudes=altitudes,
        triangle_class=classify_triangle(sides, angles),
    )


def equilateral(side: float = 1.0) -> TriangleState:
    if side <= 0.0:
        raise ValueError("side must be positive")
    return triangle_state(((0.0, 0.0), (side, 0.0), (side / 2.0, side * sqrt(3.0) / 2.0)))


def isosceles_vertex_angle(equal_side: float, angle_deg: float) -> TriangleState:
    if equal_side <= 0.0 or not 0.0 < angle_deg < 180.0:
        raise ValueError("invalid side or angle")
    half = angle_deg * pi / 360.0
    half_base = equal_side * sin(half)
    height = equal_side * cos(half)
    return triangle_state(((-half_base, 0.0), (half_base, 0.0), (0.0, height)))


def right_from_hypotenuse(hypotenuse: float, angle_deg: float) -> TriangleState:
    if hypotenuse <= 0.0 or not 0.0 < angle_deg < 90.0:
        raise ValueError("invalid hypotenuse or acute angle")
    theta = angle_deg * pi / 180.0
    x = hypotenuse * cos(theta)
    y = hypotenuse * sin(theta)
    return triangle_state(((0.0, 0.0), (x, 0.0), (0.0, y)))


def spiral_step(
    point: Sequence[float], angle_deg: float = 60.0, scale: float = sqrt(3.0) / 2.0
) -> tuple[float, float]:
    theta = angle_deg * pi / 180.0
    x, y = float(point[0]), float(point[1])
    return (
        scale * (x * cos(theta) - y * sin(theta)),
        scale * (x * sin(theta) + y * cos(theta)),
    )


def canonical_atlas() -> dict:
    eq = equilateral(1.0)
    iso10 = isosceles_vertex_angle(1.0, 10.0)
    right10 = right_from_hypotenuse(1.0, 10.0)
    right_iso = triangle_state(((0.0, 0.0), (1.0, 0.0), (0.0, 1.0)))
    spiral = [(1.0, 0.0)]
    for _ in range(6):
        spiral.append(spiral_step(spiral[-1]))
    return {
        "schema": "raf.triangle-geometry-atlas.v1",
        "claim_allowed": False,
        "triangles": {
            "equilateral_unit": eq.to_dict(),
            "isosceles_vertex_10deg": iso10.to_dict(),
            "right_hypotenuse_1_angle_10deg": right10.to_dict(),
            "right_isosceles_unit_legs": right_iso.to_dict(),
        },
        "sqrt3_over_2": sqrt(3.0) / 2.0,
        "spiral_scale_60deg": spiral,
        "boundaries": {
            "navier_stokes_solution_claim": "PROHIBITED",
            "yang_mills_mass_gap_claim": "PROHIBITED",
            "peer_review_scope": "GEOMETRY_AND_NUMERICAL_MODELING_ONLY",
        },
    }


if __name__ == "__main__":
    import json

    print(json.dumps(canonical_atlas(), indent=2, sort_keys=True))
