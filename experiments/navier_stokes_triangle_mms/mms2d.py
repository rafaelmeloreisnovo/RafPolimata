#!/usr/bin/env python3
"""PRM-08: deterministic manufactured incompressible flow on triangles.

This module does not solve the three-dimensional Navier–Stokes regularity
problem. It supplies a small, exact, reproducible manufactured-solution gate
for two-dimensional triangular discretization experiments.

The steady equation used is

    (u · ∇)u = -∇p + nu Δu + f,

with a polynomial streamfunction that makes ∇·u = 0 exactly.
"""
from __future__ import annotations

from dataclasses import asdict, dataclass
from math import cos, fsum, pi, sin, sqrt
from typing import Iterable, Sequence

Point = tuple[float, float]
Triangle = tuple[Point, Point, Point]
EPS = 1e-12


@dataclass(frozen=True)
class TriangleMMSReport:
    vertices: Triangle
    signed_area: float
    edge_fluxes: tuple[float, float, float]
    flux_balance: float
    centroid: Point
    velocity_at_centroid: Point
    pressure_at_centroid: float
    residual_at_centroid: Point
    viscosity: float

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


def centroid(vertices: Iterable[Sequence[float]]) -> Point:
    tri = normalize_triangle(vertices)
    return (
        fsum(p[0] for p in tri) / 3.0,
        fsum(p[1] for p in tri) / 3.0,
    )


def streamfunction(x: float, y: float) -> float:
    """Polynomial streamfunction ψ=x²y-xy²."""
    return x * x * y - x * y * y


def velocity(x: float, y: float) -> Point:
    """u=(∂ψ/∂y,-∂ψ/∂x)."""
    return (x * x - 2.0 * x * y, y * y - 2.0 * x * y)


def pressure(x: float, y: float) -> float:
    return x + y


def pressure_gradient(_: float, __: float) -> Point:
    return (1.0, 1.0)


def divergence(x: float, y: float) -> float:
    du_dx = 2.0 * x - 2.0 * y
    dv_dy = 2.0 * y - 2.0 * x
    return du_dx + dv_dy


def laplacian_velocity(_: float, __: float) -> Point:
    return (2.0, 2.0)


def convective_acceleration(x: float, y: float) -> Point:
    u, v = velocity(x, y)
    du_dx = 2.0 * x - 2.0 * y
    du_dy = -2.0 * x
    dv_dx = -2.0 * y
    dv_dy = 2.0 * y - 2.0 * x
    return (
        u * du_dx + v * du_dy,
        u * dv_dx + v * dv_dy,
    )


def body_force(x: float, y: float, viscosity: float) -> Point:
    """Return f making the manufactured field an exact steady solution."""
    if viscosity < 0.0:
        raise ValueError("viscosity must be non-negative")
    conv_x, conv_y = convective_acceleration(x, y)
    grad_x, grad_y = pressure_gradient(x, y)
    lap_x, lap_y = laplacian_velocity(x, y)
    return (
        conv_x + grad_x - viscosity * lap_x,
        conv_y + grad_y - viscosity * lap_y,
    )


def momentum_residual(x: float, y: float, viscosity: float) -> Point:
    """Return LHS-RHS for the steady incompressible momentum equation."""
    conv_x, conv_y = convective_acceleration(x, y)
    grad_x, grad_y = pressure_gradient(x, y)
    lap_x, lap_y = laplacian_velocity(x, y)
    force_x, force_y = body_force(x, y, viscosity)
    rhs_x = -grad_x + viscosity * lap_x + force_x
    rhs_y = -grad_y + viscosity * lap_y + force_y
    return (conv_x - rhs_x, conv_y - rhs_y)


def edge_flux_from_streamfunction(start: Sequence[float], end: Sequence[float]) -> float:
    """Exact oriented edge flux ∫u·n ds = ψ(end)-ψ(start).

    For a consistently oriented polygon, these edge contributions telescope.
    """
    x0, y0 = _point(start)
    x1, y1 = _point(end)
    return streamfunction(x1, y1) - streamfunction(x0, y0)


def triangle_edge_fluxes(vertices: Iterable[Sequence[float]]) -> tuple[float, float, float]:
    a, b, c = normalize_triangle(vertices)
    return (
        edge_flux_from_streamfunction(a, b),
        edge_flux_from_streamfunction(b, c),
        edge_flux_from_streamfunction(c, a),
    )


def triangle_report(
    vertices: Iterable[Sequence[float]], viscosity: float = 0.1
) -> TriangleMMSReport:
    if viscosity < 0.0:
        raise ValueError("viscosity must be non-negative")
    tri = normalize_triangle(vertices)
    center = centroid(tri)
    fluxes = triangle_edge_fluxes(tri)
    return TriangleMMSReport(
        vertices=tri,
        signed_area=signed_area(tri),
        edge_fluxes=fluxes,
        flux_balance=fsum(fluxes),
        centroid=center,
        velocity_at_centroid=velocity(*center),
        pressure_at_centroid=pressure(*center),
        residual_at_centroid=momentum_residual(*center, viscosity),
        viscosity=viscosity,
    )


def canonical_triangle_families() -> dict[str, Triangle]:
    half = 5.0 * pi / 180.0
    return {
        "right_45_45_90": ((0.0, 0.0), (1.0, 0.0), (0.0, 1.0)),
        "right_30_60_90": ((0.0, 0.0), (sqrt(3.0), 0.0), (0.0, 1.0)),
        "equilateral": ((0.0, 0.0), (1.0, 0.0), (0.5, sqrt(3.0) / 2.0)),
        "isosceles_10_85_85": (
            (-sin(half), 0.0),
            (sin(half), 0.0),
            (0.0, cos(half)),
        ),
    }


def canonical_study(viscosity: float = 0.1) -> dict:
    reports = {
        name: triangle_report(vertices, viscosity).to_dict()
        for name, vertices in canonical_triangle_families().items()
    }
    max_flux_error = max(abs(item["flux_balance"]) for item in reports.values())
    max_residual = max(
        max(abs(component) for component in item["residual_at_centroid"])
        for item in reports.values()
    )
    return {
        "schema": "raf.prm08.triangle-mms.v1",
        "claim_allowed": False,
        "scope": "2D manufactured incompressible triangular conservation gate",
        "equation": "(u·grad)u = -grad(p) + nu*laplacian(u) + f",
        "reports": reports,
        "summary": {
            "triangle_families": len(reports),
            "max_flux_balance_error": max_flux_error,
            "max_momentum_residual": max_residual,
        },
        "boundaries": {
            "navier_stokes_3d_regularity_solution": "NOT_CLAIMED",
            "numerical_solver_convergence": "TOKEN_VAZIO",
            "independent_replication": "TOKEN_VAZIO",
        },
    }


if __name__ == "__main__":
    import json

    print(json.dumps(canonical_study(), indent=2, sort_keys=True))
