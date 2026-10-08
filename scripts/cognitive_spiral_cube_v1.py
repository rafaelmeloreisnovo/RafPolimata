#!/usr/bin/env python3
"""Bounded, auditable 10^3 semantic/spiral lattice — *research fixture* only.

Copyright (c) 2026 Rafael Melo Reis.
Root licensing undecided: see docs/LICENSE_DECISION_RECORD.md.
No neurophysiology or intrinsic LLM-state access is implied.
Uses Python's standard library; NOT freestanding, and NOT an APK runtime.
Governance anchor: CLOSURE_L11; physical/runtime absence also maps to CLOSURE_L2.
"""
from __future__ import annotations

import argparse
import json
import math
from fractions import Fraction
from statistics import median

SIDE = 10
CELL_COUNT = SIDE ** 3
MODULI = (7, 3, 35, 10, 13, 70, 14, 50)
SEQUENCES = {
    "123": (1, 2, 3),               # textual projection; NOT whole Fibonacci prefix
    "01123": (0, 1, 1, 2, 3),      # canonical F0..F4
    "0001123": (0, 0, 0, 1, 1, 2, 3),  # two extra leading zero *characters*
}
VOID = object()
PHI = (1.0 + math.sqrt(5.0)) / 2.0
SPIRAL_BASE = math.sqrt(3.0) / 2.0
RADIAL_FACTOR = math.pi * PHI
# Explicit interpretations of sqrt(pi)/5, sqrt(pi)/12 and sqrt(5)/9.
# sqrt(pi/5) and sqrt(pi/12) are DIFFERENT expressions.
ARM_COEFFICIENTS = (math.sqrt(math.pi) / 5.0,
                    math.sqrt(math.pi) / 12.0, math.sqrt(5.0) / 9.0)


class DomainError(ValueError):
    """Invalid mathematical domain; a domain failure is never mapped to zero."""


def fibonacci_up_to(count: int) -> tuple[int, ...]:
    if not isinstance(count, int) or isinstance(count, bool) or not 1 <= count <= CELL_COUNT:
        raise DomainError("fibonacci count out of bound")
    values = [0, 1]
    for _ in range(2, count):
        values.append(values[-1] + values[-2])
    return tuple(values[:count])


def coordinate_to_index(x: int, y: int, z: int) -> int:
    if any(not isinstance(a, int) or isinstance(a, bool) or not 0 <= a < SIDE
           for a in (x, y, z)):
        raise DomainError("voxel outside [0,9]^3")
    return x + SIDE * y + SIDE * SIDE * z


def index_to_coordinate(index: int) -> tuple[int, int, int]:
    if not isinstance(index, int) or isinstance(index, bool) or not 0 <= index < CELL_COUNT:
        raise DomainError("voxel index outside [0,999]")
    return index % SIDE, (index // SIDE) % SIDE, index // (SIDE * SIDE)


def typed_numeric_sum(values: tuple[object, ...]) -> dict[str, object]:
    """Distinguishes empty/void observations, numeric 0, and digit-character '0'."""
    if not values or any(value is VOID or value is None for value in values):
        return {"state": "TOKEN_VAZIO", "value": None}
    if any(isinstance(value, bool) or not isinstance(value, (int, float))
           or not math.isfinite(value) for value in values):
        raise DomainError("non-numeric item is not an additive observation")
    return {"state": "NUMERIC", "value": sum(values)}


def strict_loglog(x: float) -> float:
    if isinstance(x, bool) or not isinstance(x, (int, float)) or not math.isfinite(x) or x <= 1:
        raise DomainError("log(log(x)) requires real finite x > 1")
    return math.log(math.log(x))


def prime_exact(value: int) -> bool:
    if isinstance(value, bool) or not isinstance(value, int):
        raise DomainError("prime test requires integer")
    if value < 2:
        return False
    divisor = 2
    while divisor * divisor <= value:
        if value % divisor == 0:
            return False
        divisor += 1
    return True


def fibonacci_as_characters(text: str) -> dict[str, object]:
    if text not in SEQUENCES:
        raise DomainError("unknown or ambiguous char() sequence")
    digits = SEQUENCES[text]
    return {
        "chars": text,
        "codepoints": [ord(c) for c in text],
        "numeric_digits": list(digits),
        "state": "CANONICAL_FIB_PREFIX" if text == "01123" else
                 "TEXTUAL_PROJECTION_OR_ZERO_PADDING",
    }


def normalized_entropy(histogram: tuple[int, ...]) -> float:
    total = sum(histogram)
    if total == 0 or len(histogram) < 2 or any(x < 0 for x in histogram):
        raise DomainError("entropy requires nonempty count histogram")
    return -sum((n / total) * math.log2(n / total) for n in histogram if n) / math.log2(len(histogram))


def radial_fibonacci(index: int, fib: tuple[int, ...]) -> float:
    # Bounded ten-state recurrence window avoids enormous exponents/underflow.
    # This is a *chosen model*, not a discovered identity.
    return SPIRAL_BASE ** (RADIAL_FACTOR * fib[index % 10])


def unit_direction(index: int, arm: int, residue: int, modulus: int) -> tuple[float, float, float]:
    if not 0 <= arm < 3 or modulus < 2 or not 0 <= residue < modulus:
        raise DomainError("invalid angle/residue")
    x, y, z = index_to_coordinate(index)
    azimuth = 2.0 * math.pi * (residue / modulus + arm / 3.0)
    # Geographic latitude-like coordinate is a DESIGN CHOICE, not measured GPS.
    elevation = ((z - (SIDE - 1) / 2.0) / (SIDE - 1)) * (math.pi / 2.0)
    return (math.cos(elevation) * math.cos(azimuth),
            math.cos(elevation) * math.sin(azimuth),
            math.sin(elevation))


def voxel(index: int, fib: tuple[int, ...]) -> dict[str, object]:
    if len(fib) != CELL_COUNT:
        raise DomainError("expected exact 1000-term recurrence")
    xyz = index_to_coordinate(index)
    radial = radial_fibonacci(index, fib)
    vectors = []
    mods = []
    for arm in range(3):
        modulus = MODULI[(index + arm) % len(MODULI)]
        residue = fib[index] % modulus
        direction = unit_direction(index, arm, residue, modulus)
        amplitude = radial * ARM_COEFFICIENTS[arm]
        vectors.append(tuple(amplitude * v for v in direction))
        mods.append({"modulus": modulus, "remainder": residue,
                     "sqrt_remainder": round(math.sqrt(residue), 12)})
    combined = tuple(sum(v[d] for v in vectors) for d in range(3))
    magnitude = math.sqrt(sum(c * c for c in combined))
    return {
        "index": index, "coordinate": list(xyz), "radius": round(radial, 12),
        "directions": [list(round(v, 12) for v in vector) for vector in vectors],
        "modular_roots": mods,
        "overlap": [round(v, 12) for v in combined],
        "magnitude": round(magnitude, 12),
    }


def _round(value: float) -> float:
    return round(value, 12)


def ratios_and_domains() -> list[dict[str, object]]:
    cases = [(999, 936), (777, 555), (140, 144)]
    out = []
    for num, den in cases:
        ratio = Fraction(num, den)
        try:
            loglog = {"state": "DEFINED", "value": _round(strict_loglog(float(ratio)))}
        except DomainError:
            loglog = {"state": "UNDEFINED_DOMAIN", "value": None}
        out.append({"original": f"{num}/{den}", "exact": str(ratio),
                    "loglog": loglog})
    return out


def update_observed_state(coherence: float, entropy: float,
                          input_coherence: float, input_entropy: float,
                          alpha: float = 0.25) -> tuple[float, float, float]:
    """External EMA state, NOT brain/plasticity/LLM model-weight training."""
    values = (coherence, entropy, input_coherence, input_entropy, alpha)
    if any(isinstance(x, bool) or not isinstance(x, (int, float))
           or not math.isfinite(x) or not 0.0 <= x <= 1.0 for x in values):
        raise DomainError("EMA requires finite [0,1] scalar states")
    next_c = (1.0 - alpha) * coherence + alpha * input_coherence
    next_h = (1.0 - alpha) * entropy + alpha * input_entropy
    return next_c, next_h, (1.0 - next_h) * next_c


def layer_observations(fib: tuple[int, ...]) -> list[dict[str, float | int]]:
    """10 spatial z-layers; scan-order dependent by design, not temporal data."""
    if len(fib) != CELL_COUNT:
        raise DomainError("layer observations need exact recurrence")
    result = []
    weight_total = sum(ARM_COEFFICIENTS)
    for z in range(SIDE):
        histogram = [0] * SIDE
        coherence_total = 0.0
        for k in range(z * SIDE * SIDE, (z + 1) * SIDE * SIDE):
            histogram[fib[k] % SIDE] += 1
            components = [0.0, 0.0, 0.0]
            for arm in range(3):
                m = MODULI[(k + arm) % len(MODULI)]
                direction = unit_direction(k, arm, fib[k] % m, m)
                for axis in range(3):
                    components[axis] += ARM_COEFFICIENTS[arm] * direction[axis]
            coherence_total += math.sqrt(sum(v * v for v in components)) / weight_total
        result.append({
            "layer_z": z,
            "coherence_observed": coherence_total / (SIDE * SIDE),
            "entropy_observed": normalized_entropy(tuple(histogram)),
        })
    return result


def adaptive_layer_trace(observations: list[dict[str, float | int]]) -> list[dict[str, object]]:
    """An explicit ten-step spatial EMA; input order is intentionally observable."""
    c, h = 0.0, 1.0
    trace = []
    for obs in observations:
        c, h, score = update_observed_state(c, h,
                                            float(obs["coherence_observed"]),
                                            float(obs["entropy_observed"]))
        trace.append({
            "layer_z": obs["layer_z"],
            "C_observed": _round(float(obs["coherence_observed"])),
            "H_observed": _round(float(obs["entropy_observed"])),
            "C_ema": _round(c),
            "H_ema": _round(h),
            "organization_coherence_proxy": _round(score),
        })
    return trace


def calculate() -> dict[str, object]:
    fib = fibonacci_up_to(CELL_COUNT)
    magnitudes = []
    hist = [0] * SIDE
    for index in range(CELL_COUNT):
        cell = voxel(index, fib)
        magnitudes.append(cell["magnitude"])
        hist[fib[index] % SIDE] += 1
    h = normalized_entropy(tuple(hist))
    anchors = (0, 1, 2, 3, 7, 10, 13, 35, 50, 70, 144, 288, 555, 777, 936, 999)
    observations = layer_observations(fib)
    forward = adaptive_layer_trace(observations)
    backward = adaptive_layer_trace(list(reversed(observations)))
    return {
        "schema": "rafaelia.cognitive-spiral-cube/v1",
        "state": "DETERMINISTIC_RESEARCH_FIXTURE",
        "claim_allowed": False,
        "external_corpus_evaluation": "NOT_RUN",
        "brain_or_llm_internal_state": "NOT_MEASURED",
        "physical_android_execution": "TOKEN_VAZIO",
        "side": SIDE, "cell_count": CELL_COUNT,
        "linearization": "index=x+10*y+100*z",
        "moduli": list(MODULI),
        "three_arm_coefficients": [_round(x) for x in ARM_COEFFICIENTS],
        "spiral_base": _round(SPIRAL_BASE),
        "spiral_radial_exponent_factor_pi_phi": _round(RADIAL_FACTOR),
        "characters": [fibonacci_as_characters(t) for t in SEQUENCES],
        "void_example": typed_numeric_sum((VOID, VOID, VOID)),
        "numeric_zero_example": typed_numeric_sum((0, 0, 0)),
        "ratios": ratios_and_domains(),
        "identity_288_equals_2x12_squared": 288 == 2 * (12 ** 2),
        "scaled_144000": 144_000, "scaled_288000": 288_000,
        "prime_seed_classification": [
            {"n": n, "prime": prime_exact(n)} for n in MODULI
        ],
        "entropy": {
            "name": "normalized_Shannon_entropy_of_F_index_mod_10",
            "histogram": hist,
            "normalized": _round(h),
            "organization_proxy_1_minus_H": _round(1 - h),
            "scope": "modular_symbol_distribution_only",
        },
        "overlap_magnitude_min_median_max": {
            "min": _round(min(magnitudes)),
            "median": _round(median(magnitudes)),
            "max": _round(max(magnitudes)),
        },
        "adaptive_layer_ema": {
            "alpha": 0.25,
            "initial_C": 0.0, "initial_H": 1.0,
            "ordering": "z ascending, spatial not time",
            "forward": forward,
            "reverse_order_final": backward[-1],
            "permutation_delta_C": _round(forward[-1]["C_ema"] - backward[-1]["C_ema"]),
            "permutation_delta_H": _round(forward[-1]["H_ema"] - backward[-1]["H_ema"]),
            "scope": "external directional scan/proxy, not synaptic plasticity",
        },
        "anchor_voxels": [voxel(n, fib) for n in anchors],
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="10x10x10 geometric/semantic exploratory fixture")
    parser.add_argument("--output", help="JSON output path; otherwise stdout")
    args = parser.parse_args()
    report = json.dumps(calculate(), sort_keys=True, ensure_ascii=False, indent=2) + "\n"
    if args.output:
        from pathlib import Path
        p = Path(args.output)
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(report, encoding="utf-8")
    else:
        print(report, end="")


if __name__ == "__main__":
    main()
