"""Reference-only checks for ΔG-MANIFOLD V1.

These functions validate bounded mathematical identities. They do not validate
a biomedical or clinical hypothesis.
"""
from __future__ import annotations
import math

R = 8.31446261815324
F = 96485.33212

def nernst_potential(c_out: float, c_in: float, z: int, T: float = 310.15) -> float:
    if c_out <= 0 or c_in <= 0:
        raise ValueError("concentrations must be positive")
    if z == 0:
        raise ValueError("ionic valence z must be nonzero")
    return (R*T/(z*F))*math.log(c_out/c_in)

def electrochemical_delta_mu(c_out: float, c_in: float, z: int, delta_v: float, T: float = 310.15) -> float:
    if c_out <= 0 or c_in <= 0:
        raise ValueError("concentrations must be positive")
    return R*T*math.log(c_out/c_in) - z*F*delta_v

def solvent_flux(Lp: float, delta_p: float, sigma: float, delta_pi: float) -> float:
    return Lp * (delta_p - sigma*delta_pi)

def shannon_entropy(probabilities) -> float:
    ps = list(probabilities)
    if not ps:
        raise ValueError("empty distribution")
    if any(p < 0 for p in ps):
        raise ValueError("negative probability")
    if not math.isclose(sum(ps), 1.0, rel_tol=0.0, abs_tol=1e-12):
        raise ValueError("probabilities must sum to 1")
    return -sum(p*math.log(p) for p in ps if p > 0)

def entropy_production(fluxes, forces) -> float:
    j, x = list(fluxes), list(forces)
    if len(j) != len(x):
        raise ValueError("dimension mismatch")
    return sum(a*b for a,b in zip(j,x))

def diagonal_onsager_flux(coefficients, forces):
    L, x = list(coefficients), list(forces)
    if len(L) != len(x):
        raise ValueError("dimension mismatch")
    if any(l < 0 for l in L):
        raise ValueError("nonnegative diagonal coefficients required")
    return [l*v for l,v in zip(L,x)]

def distinct_entropy_record(s_thermo_j_per_k: float, h_shannon_nat: float) -> dict:
    return {
        "S_thermo_J_per_K": float(s_thermo_j_per_k),
        "H_Shannon_nat": float(h_shannon_nat),
        "equal_by_definition": False,
    }
