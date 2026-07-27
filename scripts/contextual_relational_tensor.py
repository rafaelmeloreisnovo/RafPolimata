#!/usr/bin/env python3
"""Deterministic relational scoring for contextual reconstruction.

This is an external, auditable feature matrix. It does not inspect or modify
native model weights, attention, embeddings, or hidden states.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

BENEFIT_WEIGHTS = {
    "lexical_match": 0.05,
    "source_support": 0.30,
    "provenance": 0.25,
    "layer_coherence": 0.15,
    "unit_completeness": 0.10,
    "temporal_fit": 0.15,
}
PENALTY_WEIGHTS = {
    "counterevidence": 0.25,
    "gap_penalty": 0.35,
}
FEATURES = tuple(BENEFIT_WEIGHTS) + tuple(PENALTY_WEIGHTS)


class TensorError(ValueError):
    pass


def _number01(value: Any, name: str) -> float:
    if not isinstance(value, (int, float)) or isinstance(value, bool):
        raise TensorError(f"{name}:not_number")
    value = float(value)
    if not 0.0 <= value <= 1.0:
        raise TensorError(f"{name}:out_of_range")
    return value


def score_path(path: dict[str, Any]) -> dict[str, Any]:
    path_id = path.get("path_id")
    if not isinstance(path_id, str) or not path_id:
        raise TensorError("path_id")
    features = path.get("features")
    if not isinstance(features, dict):
        raise TensorError(f"{path_id}:features")

    normalized = {
        name: _number01(features.get(name), f"{path_id}:{name}")
        for name in FEATURES
    }
    benefit = sum(
        normalized[name] * weight for name, weight in BENEFIT_WEIGHTS.items()
    )
    penalty = sum(
        normalized[name] * weight for name, weight in PENALTY_WEIGHTS.items()
    )
    score = max(-1.0, min(1.0, benefit - penalty))
    eligible = (
        normalized["source_support"] >= 0.50
        and normalized["provenance"] >= 0.50
        and normalized["layer_coherence"] >= 0.50
        and normalized["gap_penalty"] < 0.50
    )

    return {
        "path_id": path_id,
        "score": round(score, 6),
        "eligible": eligible,
        "features": normalized,
        "reasons": [] if eligible else [
            name
            for name, condition in (
                ("source_support_below_gate", normalized["source_support"] < 0.50),
                ("provenance_below_gate", normalized["provenance"] < 0.50),
                ("layer_coherence_below_gate", normalized["layer_coherence"] < 0.50),
                ("blocking_gap_pressure", normalized["gap_penalty"] >= 0.50),
            )
            if condition
        ],
    }


def evaluate_case(case: dict[str, Any]) -> dict[str, Any]:
    if case.get("schema_version") != "rafaelia.contextual-relational-tensor/v1":
        raise TensorError("schema_version")
    paths = case.get("paths")
    if not isinstance(paths, list) or not paths:
        raise TensorError("paths")
    blockers = case.get("blocking_gaps")
    if not isinstance(blockers, list):
        raise TensorError("blocking_gaps")

    scored = sorted(
        (score_path(path) for path in paths),
        key=lambda item: (-item["score"], item["path_id"]),
    )
    eligible = [item for item in scored if item["eligible"]]
    selected = eligible[0]["path_id"] if eligible and not blockers else None

    return {
        "schema": "rafaelia.contextual-relational-tensor-result/v1",
        "case_id": case.get("case_id"),
        "weights": {
            "benefit": BENEFIT_WEIGHTS,
            "penalty": PENALTY_WEIGHTS,
        },
        "ranked_paths": scored,
        "selected_path": selected,
        "abstain": selected is None,
        "blocking_gaps": blockers,
        "claim_allowed": False,
        "F_ok": ["explicit_feature_matrix", "deterministic_ranking"],
        "F_gap": blockers if blockers else ([] if selected else ["no_eligible_path"]),
        "F_next": (
            ["resolve_blocking_gaps"]
            if blockers
            else (["human_review_selected_path"] if selected else ["improve_source_and_provenance"])
        ),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("case", type=Path)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()

    try:
        result = evaluate_case(json.loads(args.case.read_text(encoding="utf-8")))
        code = 0
    except (OSError, json.JSONDecodeError, TensorError) as exc:
        result = {
            "schema": "rafaelia.contextual-relational-tensor-result/v1",
            "status": "FAIL",
            "reason": str(exc),
            "claim_allowed": False,
        }
        code = 1

    text = json.dumps(result, ensure_ascii=False, sort_keys=True, indent=2) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding="utf-8")
    print(text, end="")
    return code


if __name__ == "__main__":
    raise SystemExit(main())
