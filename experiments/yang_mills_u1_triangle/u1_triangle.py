#!/usr/bin/env python3
"""PRM-09: deterministic U(1) gauge transport on triangular complexes.

The implementation is a classical discrete U(1) model. It does not construct
four-dimensional quantum Yang–Mills theory and does not claim a mass gap.
"""
from __future__ import annotations

from dataclasses import asdict, dataclass
from math import cos, isclose, sin
from typing import Iterable, Sequence

Point = tuple[float, float]
Triangle = tuple[Point, Point, Point]
EPS = 1e-12


@dataclass(frozen=True)
class U1TriangleReport:
    vertices: Triangle
    magnetic_field: float
    signed_area: float
    links: tuple[tuple[float, float], tuple[float, float], tuple[float, float]]
    holonomy: tuple[float, float]
    expected_holonomy: tuple[float, float]
    holonomy_error: float
    wilson_action: float

    def to_dict(self) -> dict:
        return asdict(self)


def _point(p: Sequence[float]) -> Point:
    if len(p) != 2:
        raise ValueError("points must be two-dimensional")
    return (float(p[0]), float(p[1]))


def normalize_triangle(vertices: Iterable[Sequence[float]]) -> Triangle:
    pts = tuple(_point(p) for p in vertices)
    if len(pts) != 3:
        raise ValueError("exactly three vertices are required")
    tri = (pts[0], pts[1], pts[2])
    if abs(signed_area(tri)) <= EPS:
        raise ValueError("degenerate triangle")
    return tri


def signed_area(vertices: Iterable[Sequence[float]]) -> float:
    pts = tuple(_point(p) for p in vertices)
    if len(pts) != 3:
        raise ValueError("exactly three vertices are required")
    (x0, y0), (x1, y1), (x2, y2) = pts
    return 0.5 * ((x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0))


def u1_phase(angle: float) -> complex:
    return complex(cos(angle), sin(angle))


def _assert_unit(value: complex, name: str = "U(1) element") -> None:
    if not isclose(abs(value), 1.0, rel_tol=1e-12, abs_tol=1e-12):
        raise ValueError(f"{name} must have unit modulus")


def link_symmetric_gauge(
    magnetic_field: float, start: Sequence[float], end: Sequence[float]
) -> complex:
    """Exact straight-edge link for A=(-By/2,Bx/2)."""
    x0, y0 = _point(start)
    x1, y1 = _point(end)
    angle = 0.5 * magnetic_field * (x0 * y1 - y0 * x1)
    return u1_phase(angle)


def reverse_link(link: complex) -> complex:
    _assert_unit(link, "link")
    return link.conjugate()


def triangle_links(
    vertices: Iterable[Sequence[float]], magnetic_field: float
) -> tuple[complex, complex, complex]:
    a, b, c = normalize_triangle(vertices)
    return (
        link_symmetric_gauge(magnetic_field, a, b),
        link_symmetric_gauge(magnetic_field, b, c),
        link_symmetric_gauge(magnetic_field, c, a),
    )


def holonomy(links: Sequence[complex]) -> complex:
    if len(links) != 3:
        raise ValueError("triangular holonomy requires three oriented links")
    result = 1.0 + 0.0j
    for index, link in enumerate(links):
        _assert_unit(link, f"link[{index}]")
        result *= link
    _assert_unit(result, "holonomy")
    return result


def triangle_holonomy(
    vertices: Iterable[Sequence[float]], magnetic_field: float
) -> complex:
    return holonomy(triangle_links(vertices, magnetic_field))


def expected_constant_field_holonomy(
    vertices: Iterable[Sequence[float]], magnetic_field: float
) -> complex:
    return u1_phase(magnetic_field * signed_area(vertices))


def gauge_transform_link(link: complex, start_gauge: complex, end_gauge: complex) -> complex:
    _assert_unit(link, "link")
    _assert_unit(start_gauge, "start gauge")
    _assert_unit(end_gauge, "end gauge")
    transformed = start_gauge * link * end_gauge.conjugate()
    _assert_unit(transformed, "transformed link")
    return transformed


def gauge_transform_triangle(
    links: Sequence[complex], vertex_phases: Sequence[float]
) -> tuple[complex, complex, complex]:
    if len(links) != 3 or len(vertex_phases) != 3:
        raise ValueError("three links and three vertex phases are required")
    ga, gb, gc = (u1_phase(float(angle)) for angle in vertex_phases)
    uab, ubc, uca = links
    return (
        gauge_transform_link(uab, ga, gb),
        gauge_transform_link(ubc, gb, gc),
        gauge_transform_link(uca, gc, ga),
    )


def wilson_action(face_holonomy: complex, weight: float = 1.0) -> float:
    _assert_unit(face_holonomy, "face holonomy")
    if weight < 0.0:
        raise ValueError("weight must be non-negative")
    return weight * (1.0 - face_holonomy.real)


def complex_pair(value: complex) -> tuple[float, float]:
    return (float(value.real), float(value.imag))


def triangle_report(
    vertices: Iterable[Sequence[float]], magnetic_field: float = 0.7
) -> U1TriangleReport:
    tri = normalize_triangle(vertices)
    links = triangle_links(tri, magnetic_field)
    observed = holonomy(links)
    expected = expected_constant_field_holonomy(tri, magnetic_field)
    return U1TriangleReport(
        vertices=tri,
        magnetic_field=float(magnetic_field),
        signed_area=signed_area(tri),
        links=tuple(complex_pair(link) for link in links),
        holonomy=complex_pair(observed),
        expected_holonomy=complex_pair(expected),
        holonomy_error=abs(observed - expected),
        wilson_action=wilson_action(observed),
    )


def split_on_bc(vertices: Iterable[Sequence[float]]) -> tuple[Triangle, Triangle]:
    """Split ABC at the midpoint M of BC into ABM and AMC."""
    a, b, c = normalize_triangle(vertices)
    midpoint = ((b[0] + c[0]) / 2.0, (b[1] + c[1]) / 2.0)
    return ((a, b, midpoint), (a, midpoint, c))


def refinement_holonomy_product(
    vertices: Iterable[Sequence[float]], magnetic_field: float
) -> complex:
    left, right = split_on_bc(vertices)
    return triangle_holonomy(left, magnetic_field) * triangle_holonomy(
        right, magnetic_field
    )


def canonical_study(magnetic_field: float = 0.7) -> dict:
    families = {
        "right_unit": ((0.0, 0.0), (1.0, 0.0), (0.0, 1.0)),
        "skew": ((-0.2, 0.1), (1.3, -0.1), (0.4, 1.2)),
        "reversed_orientation": ((0.0, 0.0), (0.0, 1.0), (1.0, 0.0)),
    }
    reports = {
        name: triangle_report(vertices, magnetic_field).to_dict()
        for name, vertices in families.items()
    }

    gauge_probe_tri = families["skew"]
    links = triangle_links(gauge_probe_tri, magnetic_field)
    before = holonomy(links)
    after = holonomy(gauge_transform_triangle(links, (0.2, -0.9, 1.7)))
    whole = triangle_holonomy(gauge_probe_tri, magnetic_field)
    refined = refinement_holonomy_product(gauge_probe_tri, magnetic_field)

    return {
        "schema": "raf.prm09.u1-triangle.v1",
        "claim_allowed": False,
        "scope": "classical 2D U(1) triangular holonomy gate",
        "reports": reports,
        "invariants": {
            "gauge_holonomy_error": abs(before - after),
            "refinement_holonomy_error": abs(whole - refined),
            "max_constant_field_error": max(
                item["holonomy_error"] for item in reports.values()
            ),
        },
        "boundaries": {
            "yang_mills_quantum_4d_construction": "NOT_CLAIMED",
            "mass_gap": "NOT_CLAIMED",
            "continuum_limit": "TOKEN_VAZIO",
            "independent_replication": "TOKEN_VAZIO",
        },
    }


if __name__ == "__main__":
    import json

    print(json.dumps(canonical_study(), indent=2, sort_keys=True))
