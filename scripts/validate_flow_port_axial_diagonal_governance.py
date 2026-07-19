#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path


def validate(config: dict[str, object]) -> dict[str, object]:
    construction = config["construction"]
    cyclic = config["cyclic_base"]
    claims = config["claims"]
    axes = construction["axes"]
    directions = {tuple(item) for item in construction["matrix_directions"]}
    expected = {
        (0, 1), (0, -1), (1, 0), (-1, 0),
        (1, 1), (-1, -1), (-1, 1), (1, -1),
    }
    angle_sets = [tuple(axis["angles_degrees"]) for axis in axes]
    base7 = cyclic["base7_example"]
    checks = {
        "schema": config["schema"] == "rafpolimata.geometry.flow_ports_axial_diagonal.v2",
        "exact_claim_gate_open": config["claim_allowed"] is True,
        "exact_geometry_pass": config["exact_geometry_state"] == "PASS",
        "operator_state_pass": config["operator_state"] == "PASS",
        "empirical_fluid_not_applicable": config["empirical_fluid_interpretation"] == "NOT_APPLICABLE",
        "four_axes": construction["axis_count"] == 4 and len(axes) == 4,
        "eight_ports": construction["port_count"] == 8
        and sum(len(axis["ports"]) for axis in axes) == 8,
        "opposed_angles": all(
            math.isclose((angles[1] - angles[0]) % 360, 180.0)
            for angles in angle_sets
        ),
        "axial_diagonal_directions_complete": directions == expected,
        "diagonal_normalization_declared": construction["diagonal_normalization"] == "1/sqrt(2)",
        "implementation_routed_to_chipquantum": config["implementation"]["repository"] == "rafaelmeloreisnovo/ChipQuantum",
        "paper_routed_to_rll": config["paper"]["repository"] == "instituto-Rafael/relativity-living-light",
        "exact_operator_claims_present": sum(
            claim["class"] == "E" and claim["state"] == "PASS"
            for claim in claims
        ) >= 9,
        "venturi_is_exact_operator": any(
            claim["id"] == "FLOW-E-007" and claim["state"] == "PASS"
            for claim in claims
        ),
        "vortex_is_exact_operator": any(
            claim["id"] == "FLOW-E-008" and claim["state"] == "PASS"
            for claim in claims
        ),
        "base7_seam_preserved": base7 == {
            "decimal_value": 7,
            "positional_base7": "10",
            "quotient": 1,
            "remainder": 0,
            "cycle_index": 0,
            "phase": 7,
            "lifted_angle": "2*pi",
            "wrapped_angle": 0,
        },
    }
    passed = sum(checks.values())
    return {
        "schema": "rafpolimata.validation.flow_ports_axial_diagonal.v2",
        "checks_total": len(checks),
        "checks_passed": passed,
        "checks_failed": len(checks) - passed,
        "status": "PASS" if all(checks.values()) else "FAIL",
        "claim_allowed": all(checks.values()),
        "operator_state": "PASS" if all(checks.values()) else "AUDIT",
        "empirical_fluid_interpretation": "NOT_APPLICABLE",
        "checks": checks,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", default="configs/flow-port-axial-diagonal.json")
    parser.add_argument("--output", default="results/flow-port-axial-diagonal-governance.json")
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
