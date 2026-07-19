#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path


def validate(config: dict[str, object]) -> dict[str, object]:
    construction = config["construction"]
    claims = config["claims"]
    axes = construction["axes"]
    directions = {tuple(item) for item in construction["matrix_directions"]}
    expected = {
        (0, 1),
        (0, -1),
        (1, 0),
        (-1, 0),
        (1, 1),
        (-1, -1),
        (-1, 1),
        (1, -1),
    }
    angle_sets = [tuple(axis["angles_degrees"]) for axis in axes]
    checks = {
        "schema": config["schema"]
        == "rafpolimata.geometry.flow_ports_axial_diagonal.v1",
        "global_claim_gate_closed": config["claim_allowed"] is False,
        "exact_geometry_pass": config["exact_geometry_state"] == "PASS",
        "physical_state_token_vazio": config["physical_state"] == "TOKEN_VAZIO",
        "four_axes": construction["axis_count"] == 4 and len(axes) == 4,
        "eight_ports": construction["port_count"] == 8
        and sum(len(axis["ports"]) for axis in axes) == 8,
        "opposed_angles": all(
            math.isclose((angles[1] - angles[0]) % 360, 180.0)
            for angles in angle_sets
        ),
        "axial_diagonal_directions_complete": directions == expected,
        "diagonal_normalization_declared": construction["diagonal_normalization"]
        == "1/sqrt(2)",
        "implementation_routed_to_chipquantum": config["implementation"]["repository"]
        == "rafaelmeloreisnovo/ChipQuantum",
        "paper_routed_to_rll": config["paper"]["repository"]
        == "instituto-Rafael/relativity-living-light",
        "exact_claims_present": sum(
            claim["class"] == "E" and claim["state"] == "PASS"
            for claim in claims
        )
        >= 5,
        "convention_not_promoted": any(
            claim["class"] == "C" and claim["state"] == "REFERENCE"
            for claim in claims
        ),
        "venturi_blocked": any(
            claim["id"] == "FLOW-H-007" and claim["state"] == "TOKEN_VAZIO"
            for claim in claims
        ),
        "vortex_blocked": any(
            claim["id"] == "FLOW-H-008" and claim["state"] == "TOKEN_VAZIO"
            for claim in claims
        ),
        "physical_gates_complete": len(config["promotion_gates"]["physical_model"])
        >= 10,
    }
    passed = sum(checks.values())
    return {
        "schema": "rafpolimata.validation.flow_ports_axial_diagonal.v1",
        "checks_total": len(checks),
        "checks_passed": passed,
        "checks_failed": len(checks) - passed,
        "status": "PASS" if all(checks.values()) else "FAIL",
        "claim_allowed": False,
        "checks": checks,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--config",
        default="configs/flow-port-axial-diagonal.json",
    )
    parser.add_argument(
        "--output",
        default="results/flow-port-axial-diagonal-governance.json",
    )
    args = parser.parse_args()
    config = json.loads(Path(args.config).read_text(encoding="utf-8"))
    report = validate(config)
    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    Path(args.output).write_text(
        json.dumps(report, indent=2, ensure_ascii=False, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    print(json.dumps(report, indent=2, ensure_ascii=False, sort_keys=True))
    return 0 if report["status"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
