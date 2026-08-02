#!/usr/bin/env python3
"""Validate the RAFAELIA peer-review mathematical 7-direction matrix.

No third-party dependencies are required. The validator is intentionally
fail-closed: missing fields become validation errors, never inferred success.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

SCHEMA = "raf.peer-review-math-7d-matrix.v1"
DIRECTIONS = (
    "fact",
    "gap",
    "invariant",
    "variant",
    "proof",
    "parable",
    "feedback",
)
ALLOWED_STATES = {
    "READY_FOR_SCOPE_REVIEW",
    "EXPERIMENT_REQUIRED",
    "RESEARCH_AGENDA_ONLY",
}
EXPECTED_IDS = {f"PRM-{n:02d}" for n in range(1, 13)}
OPEN_BOUNDARIES = {
    "PRM-08": "NAVIER_STOKES_ORIGINAL_NOT_CLAIMED",
    "PRM-09": "YANG_MILLS_MASS_GAP_NOT_CLAIMED",
    "PRM-11": "RIEMANN_PROOF_NOT_CLAIMED",
}


class ValidationError(ValueError):
    """Raised when the manifest violates a hard invariant."""


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise ValidationError(message)


def validate_manifest(data: dict[str, Any]) -> list[str]:
    _require(data.get("schema") == SCHEMA, f"schema must be {SCHEMA!r}")
    _require(data.get("privacy", {}).get("claim_allowed") is False, "claim_allowed must be false")
    _require(data.get("decision", {}).get("automatic_merge") is False, "automatic_merge must be false")
    _require(
        data.get("privacy", {}).get("private_corpus_body") == "FORBIDDEN",
        "private corpus body must be forbidden",
    )

    method = data.get("method", {})
    _require(tuple(method.get("directions", ())) == DIRECTIONS, "seven directions are incomplete or reordered")

    packages = data.get("packages")
    _require(isinstance(packages, list), "packages must be a list")
    _require(len(packages) == 12, "exactly 12 packages are required")

    ids = [item.get("id") for item in packages]
    _require(len(set(ids)) == len(ids), "package ids must be unique")
    _require(set(ids) == EXPECTED_IDS, "package ids must be PRM-01 through PRM-12")

    state_counts = {state: 0 for state in ALLOWED_STATES}
    for item in packages:
        pid = item["id"]
        _require(item.get("state") in ALLOWED_STATES, f"{pid}: invalid state")
        state_counts[item["state"]] += 1
        _require(item.get("claim_allowed") is False, f"{pid}: claim_allowed must be false")
        for field in ("title", "original_scope", "open_problem_boundary", "next_gate", *DIRECTIONS):
            value = item.get(field)
            _require(isinstance(value, str) and value.strip(), f"{pid}: missing non-empty {field}")

        expected_boundary = OPEN_BOUNDARIES.get(pid)
        if expected_boundary:
            _require(
                item["open_problem_boundary"] == expected_boundary,
                f"{pid}: open-problem boundary was weakened",
            )

        if item["state"] == "RESEARCH_AGENDA_ONLY":
            forbidden = ("PROVED", "SOLVED", "DEMONSTRATED_EQUIVALENCE")
            combined = " ".join(str(item.get(k, "")) for k in item)
            _require(not any(token in combined.upper() for token in forbidden), f"{pid}: agenda overclaim")

    counts = data.get("counts", {})
    _require(counts.get("packages") == 12, "counts.packages must be 12")
    _require(counts.get("canonical_formulations") == 60, "canonical_formulations must be 60")
    _require(counts.get("open_problem_solution_claims") == 0, "open problem claims must remain zero")
    for state, observed in state_counts.items():
        _require(counts.get(state) == observed, f"count mismatch for {state}")

    immutable = data.get("immutability", {})
    immutable_fields = set(immutable.get("immutable_fields", []))
    mutable_fields = set(immutable.get("mutable_fields", []))
    _require({"id", "title", "original_scope", "open_problem_boundary"} <= immutable_fields,
             "immutable field set is incomplete")
    _require(not immutable_fields & mutable_fields, "mutable and immutable fields overlap")

    gates = data.get("global_gates")
    _require(isinstance(gates, list) and len(gates) == 8, "G0-G7 gates are required")
    _require([gate.get("id") for gate in gates] == [f"G{n}" for n in range(8)], "gates must be ordered G0-G7")

    return [
        f"schema={SCHEMA}",
        "packages=12",
        "directions=7",
        "gates=8",
        "claim_allowed=false",
        "open_problem_solution_claims=0",
    ]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("manifest", type=Path)
    args = parser.parse_args()
    data = json.loads(args.manifest.read_text(encoding="utf-8"))
    try:
        facts = validate_manifest(data)
    except ValidationError as exc:
        print(f"FAIL: {exc}")
        return 1
    print("PASS")
    for fact in facts:
        print(f"- {fact}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
