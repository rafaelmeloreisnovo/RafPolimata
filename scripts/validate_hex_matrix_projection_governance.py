#!/usr/bin/env python3
"""Validate RafPolimata governance for the hex matrix projection bridge."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any

EXPECTED_SCHEMA = "rafpolimata.geometry.hex_projection.v1"
EXPECTED_DIRECTIONS = [[1, 0], [0, 1], [-1, 1], [-1, 0], [0, -1], [1, -1]]


def load_config(path: Path) -> dict[str, Any]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(data, dict):
        raise ValueError("configuration root must be an object")
    return data


def validate(data: dict[str, Any]) -> dict[str, Any]:
    projection = data["projection"]
    matrices = data["matrices"]
    basis = projection["basis"]
    metric = projection["metric"]

    h00, h01 = map(float, basis[0])
    h10, h11 = map(float, basis[1])
    determinant = h00 * h11 - h01 * h10
    expected_height = math.sqrt(3.0) / 2.0
    computed_metric = [
        [h00 * h00 + h10 * h10, h00 * h01 + h10 * h11],
        [h01 * h00 + h11 * h10, h01 * h01 + h11 * h11],
    ]

    claims = data["claims"]
    claim_ids = [claim["id"] for claim in claims]
    physical_claims = [claim for claim in claims if claim["class"] == "H"]

    checks = {
        "schema": data.get("schema") == EXPECTED_SCHEMA,
        "fail_closed_global_claim": data.get("claim_allowed") is False,
        "index_order": projection.get("index_order") == ["column", "row"],
        "basis_shape": len(basis) == 2 and all(len(row) == 2 for row in basis),
        "determinant": math.isclose(determinant, expected_height, abs_tol=1e-15),
        "declared_determinant": math.isclose(float(projection["determinant"]), determinant, abs_tol=1e-15),
        "metric": all(
            math.isclose(float(metric[i][j]), computed_metric[i][j], abs_tol=1e-15)
            for i in range(2)
            for j in range(2)
        ),
        "neighbors": projection.get("neighbor_directions") == EXPECTED_DIRECTIONS,
        "matrix_A": matrices["A"]["shape"] == [8, 5] and matrices["A"]["states"] == 40,
        "matrix_B": matrices["B"]["shape"] == [7, 3] and matrices["B"]["states"] == 21,
        "tensor_shape": matrices["relation_tensor"]["shape"] == [8, 5, 7, 3, 2],
        "tensor_records": matrices["relation_tensor"]["records"] == 840,
        "unique_claim_ids": len(claim_ids) == len(set(claim_ids)),
        "physical_claims_fail_closed": all(
            claim["state"] in {"TOKEN_VAZIO", "PENDING", "PROHIBITED"}
            for claim in physical_claims
        ),
        "runtime_routed_to_chipquantum": data["implementation"]["repository"] == "rafaelmeloreisnovo/ChipQuantum",
        "paper_routed_to_rll": data["canonical_paper"]["repository"] == "instituto-Rafael/relativity-living-light",
    }

    passed = sum(bool(value) for value in checks.values())
    exact_keys = (
        "basis_shape",
        "determinant",
        "declared_determinant",
        "metric",
        "neighbors",
        "matrix_A",
        "matrix_B",
        "tensor_shape",
        "tensor_records",
    )
    return {
        "schema": EXPECTED_SCHEMA,
        "status": "PASS" if passed == len(checks) else "FAIL",
        "checks_total": len(checks),
        "checks_passed": passed,
        "checks_failed": len(checks) - passed,
        "checks": checks,
        "claim_allowed": False,
        "exact_geometry_state": "PASS" if all(checks[key] for key in exact_keys) else "FAIL",
        "physical_state": "TOKEN_VAZIO",
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--config",
        default="configs/hex-matrix-projection.json",
    )
    parser.add_argument("--output")
    args = parser.parse_args()

    report = validate(load_config(Path(args.config)))
    payload = json.dumps(report, indent=2, ensure_ascii=False, sort_keys=True) + "\n"
    if args.output:
        output = Path(args.output)
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(payload, encoding="utf-8")
    print(payload, end="")
    return 0 if report["status"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
