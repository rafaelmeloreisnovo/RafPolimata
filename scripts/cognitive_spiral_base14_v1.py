#!/usr/bin/env python3
"""Base-14 numeral and optional 14^3 geometry adapter for cognitive spiral V1.

Copyright (c) 2026 Rafael Melo Reis. License: see root LICENSE_DECISION_RECORD.
Research fixture only; hosted stdlib reference, no synaptic/LLM internal access.
Crucial: changing digits != changing grid cardinality.
Governance anchor: CLOSURE_L11; physical/runtime absence also maps to CLOSURE_L2.
"""
from __future__ import annotations

import argparse
import json
import math
from statistics import median

from scripts import cognitive_spiral_cube_v1 as base10

RADIX = 14
CELL_COUNT = RADIX ** 3
DIGITS = "0123456789ABCD"
FIB_WINDOW = 10  # Hold old radial dynamics constant for A/B comparison.
MATERIALIZED_ANCHORS = (0, 1, 2, 3, 7, 10, 13, 14, 35, 50, 70,
                        144, 288, 555, 777, 936, 999, 1000, 2743)


def encode14(number: int) -> str:
    """Canonical uppercase base-14, with zero and no leading zeros."""
    if isinstance(number, bool) or not isinstance(number, int) or number < 0:
        raise base10.DomainError("radix14 input must be nonnegative integer")
    if number == 0:
        return "0"
    out = []
    while number:
        number, digit = divmod(number, RADIX)
        out.append(DIGITS[digit])
    return "".join(reversed(out))


def decode14(token: str) -> int:
    """Strict parse. No prefixes, whitespace, signs or ambiguity about 0/A."""
    if not isinstance(token, str) or not token or token != token.upper():
        raise base10.DomainError("invalid radix14 token")
    if len(token) > 1 and token[0] == "0":
        raise base10.DomainError("noncanonical leading zero")
    result = 0
    for ch in token:
        digit = DIGITS.find(ch)
        if digit < 0:
            raise base10.DomainError("character outside radix14 alphabet")
        result = RADIX * result + digit
    return result


def coordinate14_to_index(x: int, y: int, z: int) -> int:
    if any(not isinstance(v, int) or isinstance(v, bool) or not 0 <= v < RADIX
           for v in (x, y, z)):
        raise base10.DomainError("base14 coordinate outside [0,13]^3")
    return x + RADIX*y + RADIX*RADIX*z


def index14_to_coordinate(index: int) -> tuple[int, int, int]:
    if isinstance(index, bool) or not isinstance(index, int) or not 0 <= index < CELL_COUNT:
        raise base10.DomainError("base14 index outside [0,2743]")
    return index % RADIX, (index // RADIX) % RADIX, index // (RADIX * RADIX)


def fibonacci_bounded(count: int) -> tuple[int, ...]:
    if isinstance(count, bool) or not isinstance(count, int) or not 1 <= count <= CELL_COUNT:
        raise base10.DomainError("base14 Fibonacci length outside declared cube")
    a, b = 0, 1
    result = []
    for _ in range(count):
        result.append(a)
        a, b = b, a + b
    return tuple(result)


def direction14(index: int, arm: int, remainder: int, modulus: int) -> tuple[float, float, float]:
    if not isinstance(arm, int) or isinstance(arm, bool) or not 0 <= arm < 3:
        raise base10.DomainError("arm out of range")
    if not isinstance(modulus, int) or isinstance(modulus, bool) or modulus < 2:
        raise base10.DomainError("invalid modulus")
    if not isinstance(remainder, int) or isinstance(remainder, bool) or not 0 <= remainder < modulus:
        raise base10.DomainError("invalid residue")
    _, _, z = index14_to_coordinate(index)
    angle = 2.0 * math.pi * (remainder / modulus + arm / 3.0)
    elevation = (z - (RADIX-1)/2.0) * math.pi / (2.0 * (RADIX-1))
    return (math.cos(elevation) * math.cos(angle),
            math.cos(elevation) * math.sin(angle),
            math.sin(elevation))


def voxel14(index: int, fib: tuple[int, ...]) -> dict[str, object]:
    if len(fib) != CELL_COUNT:
        raise base10.DomainError("base14 voxel requires 2744-term exact Fibonacci list")
    coord = index14_to_coordinate(index)
    # F_(index mod 10) is preserved from the previous 10^3 reference.
    radial = base10.SPIRAL_BASE ** (base10.RADIAL_FACTOR * fib[index % FIB_WINDOW])
    combined = [0.0, 0.0, 0.0]
    vector_rows = []
    modular_rows = []
    for arm in range(3):
        modulus = base10.MODULI[(index + arm) % len(base10.MODULI)]
        remainder = fib[index] % modulus
        amplitude = radial * base10.ARM_COEFFICIENTS[arm]
        direction = direction14(index, arm, remainder, modulus)
        values = tuple(amplitude * d for d in direction)
        vector_rows.append([round(d, 12) for d in values])
        for j in range(3):
            combined[j] += values[j]
        modular_rows.append({
            "modulus_dec": modulus, "modulus_base14": encode14(modulus),
            "remainder_dec": remainder, "remainder_base14": encode14(remainder),
            "sqrt_remainder": round(math.sqrt(remainder), 12)
        })
    magnitude = math.sqrt(sum(v*v for v in combined))
    return {
        "index_dec": index, "index_base14": encode14(index),
        "coordinate_dec": list(coord), "coordinate_base14": [DIGITS[c] for c in coord],
        "radius": round(radial, 12),
        "directions": vector_rows, "modular_roots": modular_rows,
        "overlap": [round(x, 12) for x in combined],
        "magnitude": round(magnitude, 12),
    }


def _observations14(fib: tuple[int, ...]) -> tuple[list[float], list[dict[str, object]], list[int]]:
    if len(fib) != CELL_COUNT:
        raise base10.DomainError("base14 layer input needs 2744 values")
    hist = [0] * RADIX
    all_magnitudes = []
    observations = []
    weight_total = sum(base10.ARM_COEFFICIENTS)
    for z in range(RADIX):
        layer_hist = [0] * RADIX
        layer_c = 0.0
        for index in range(z * RADIX * RADIX, (z+1) * RADIX * RADIX):
            digit = fib[index] % RADIX
            layer_hist[digit] += 1
            hist[digit] += 1
            cell = voxel14(index, fib)
            all_magnitudes.append(cell["magnitude"])
            # Remove common radial factor in geometric alignment proxy.
            unit_vec = [0.0] * 3
            for arm in range(3):
                modulus = base10.MODULI[(index + arm) % len(base10.MODULI)]
                d = direction14(index, arm, fib[index] % modulus, modulus)
                for axis in range(3):
                    unit_vec[axis] += base10.ARM_COEFFICIENTS[arm] * d[axis]
            layer_c += math.sqrt(sum(v*v for v in unit_vec)) / weight_total
        observations.append({
            "layer_z": z, "coherence_observed": layer_c / (RADIX*RADIX),
            "entropy_observed": base10.normalized_entropy(tuple(layer_hist)),
        })
    return all_magnitudes, observations, hist


def calculate14() -> dict[str, object]:
    fib = fibonacci_bounded(CELL_COUNT)
    magnitudes, observed, histogram = _observations14(fib)
    forward = base10.adaptive_layer_trace(observed)
    backwards = base10.adaptive_layer_trace(list(reversed(observed)))
    entropy = base10.normalized_entropy(tuple(histogram))
    return {
        "schema": "rafaelia.cognitive-spiral-cube-radix14/v1",
        "state": "DETERMINISTIC_RESEARCH_FIXTURE",
        "claim_allowed": False,
        "source_authority": "RafPolimata cognitive-spiral-cube/v1; additive radix14 profile",
        "physical_android_execution": "TOKEN_VAZIO",
        "neural_validation": "NOT_RUN",
        "radix": RADIX, "digits": DIGITS,
        "side_dec": RADIX, "side_base14": encode14(RADIX),
        "cells_dec": CELL_COUNT, "cells_base14": encode14(CELL_COUNT),
        "legacy_cells_dec": base10.CELL_COUNT,
        "legacy_cells_in_base14": encode14(base10.CELL_COUNT),
        "linearization": "k=x+14*y+196*z",
        "geometry_changed": True,
        "radial_fibonacci_window": FIB_WINDOW,
        "moduli_decimal": list(base10.MODULI),
        "moduli_base14": [encode14(m) for m in base10.MODULI],
        "examples": [
            {"decimal": n, "base14": encode14(n)} for n in
            (0, 1, 3, 7, 10, 13, 14, 35, 50, 70, 140, 144, 288,
             555, 777, 936, 999, 1000, 2743, 2744, 144000, 288000)
        ],
        "entropy": {
            "name": "normalized_Shannon_entropy_of_F_index_mod_14",
            "histogram": histogram, "normalized": round(entropy, 12),
            "organization_proxy_1_minus_H": round(1.0-entropy, 12),
        },
        "overlap_magnitude_min_median_max": {
            "min": round(min(magnitudes), 12),
            "median": round(median(magnitudes), 12),
            "max": round(max(magnitudes), 12),
        },
        "adaptive_layer_ema": {
            "alpha": 0.25, "ordering": "z ascending, spatial not time",
            "forward": forward, "reverse_order_final": backwards[-1],
            "permutation_delta_C": round(forward[-1]["C_ema"] - backwards[-1]["C_ema"], 12),
            "permutation_delta_H": round(forward[-1]["H_ema"] - backwards[-1]["H_ema"], 12),
        },
        "anchors": [voxel14(i, fib) for i in MATERIALIZED_ANCHORS],
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Radix-14 numeral + 14^3 geometry research profile")
    parser.add_argument("--output", help="JSON output file, default stdout")
    args = parser.parse_args()
    report = json.dumps(calculate14(), indent=2, sort_keys=True, ensure_ascii=False) + "\n"
    if args.output:
        from pathlib import Path
        file = Path(args.output)
        file.parent.mkdir(parents=True, exist_ok=True)
        file.write_text(report, encoding="utf-8")
    else:
        print(report, end="")


if __name__ == "__main__":
    main()
