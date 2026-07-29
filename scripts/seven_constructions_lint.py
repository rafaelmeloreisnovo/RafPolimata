#!/usr/bin/env python3
"""Deterministic conservative checks for S7F-Ω V1.

This script proves only the encoded mathematical sanity checks. It does not
prove novelty, usefulness, patentability, or physical validity.
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any

SINGULAR_RADIUS = math.sqrt(math.e - 1.0)


def poincare_distance_to_origin(radius: float) -> float:
    if not 0.0 <= radius < 1.0:
        raise ValueError("radius must satisfy 0 <= r < 1")
    return 2.0 * math.atanh(radius)


def reciprocal_loglog(radius: float, eta: float = 1.0) -> float:
    if radius <= 0.0:
        raise ValueError("radius must be positive")
    denominator = math.log(math.log1p(radius * radius))
    if math.isclose(denominator, 0.0, rel_tol=0.0, abs_tol=1e-12):
        raise ZeroDivisionError("reciprocal log-log singularity")
    return eta / denominator


def load_contract(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as handle:
        data = json.load(handle)
    if data.get("claim_allowed") is not False:
        raise ValueError("claim_allowed must remain false")
    records = data.get("constructions")
    if not isinstance(records, list) or len(records) != 7:
        raise ValueError("exactly seven constructions are required")
    ids = [record.get("id") for record in records]
    expected = [f"S7F-{index:02d}" for index in range(1, 8)]
    if ids != expected:
        raise ValueError(f"construction IDs must be ordered as {expected}")
    return data


def audit(path: Path) -> dict[str, Any]:
    contract = load_contract(path)
    finite_origin_distances = {
        str(radius): poincare_distance_to_origin(radius)
        for radius in (0.0, 0.25, 0.5, 0.9, 0.999)
    }
    sign_inside_ball = reciprocal_loglog(0.5) < 0.0 and reciprocal_loglog(0.999) < 0.0
    singularity_detected = False
    try:
        reciprocal_loglog(SINGULAR_RADIUS)
    except ZeroDivisionError:
        singularity_detected = True

    states = {record["id"]: record["state"] for record in contract["constructions"]}
    action_incomplete = states["S7F-07"] == "ACTION_INCOMPLETE_REQUIRES_VARIATIONAL_COUPLING"
    void_geometry_corrected = "invalid_claim" in contract["constructions"][3]
    blenddigs_underdefined = states["S7F-05"] == "AUTHORIAL_NOTATION_UNDERDEFINED"

    checks = {
        "contract_has_seven_ordered_records": True,
        "claim_gate_closed": True,
        "poincare_origin_distance_finite_inside_ball": all(math.isfinite(v) for v in finite_origin_distances.values()),
        "reciprocal_loglog_negative_on_unit_ball_samples": sign_inside_ball,
        "reciprocal_loglog_singularity_detected": singularity_detected,
        "typed_void_infinite_origin_claim_rejected": void_geometry_corrected,
        "blenddigs_definition_gap_preserved": blenddigs_underdefined,
        "action_attention_coupling_gap_preserved": action_incomplete,
    }
    return {
        "state": "LOCAL_PASS" if all(checks.values()) else "LOCAL_FAIL",
        "claim_allowed": False,
        "checks": checks,
        "poincare_origin_distances": finite_origin_distances,
        "reciprocal_loglog_singularity_radius": SINGULAR_RADIUS,
        "limitations": [
            "does_not_prove_novelty",
            "does_not_prove_physical_validity",
            "does_not_prove_patentability",
            "does_not_replace_peer_review",
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("contract", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    result = audit(args.contract)
    text = json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    print(text, end="")
    return 0 if result["state"] == "LOCAL_PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
