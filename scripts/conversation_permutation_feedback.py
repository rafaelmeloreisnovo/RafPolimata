#!/usr/bin/env python3
"""Bounded prompt↔response feedback scheduler.

Chronology is immutable: the engine NEVER reorders conversation turns.  It only
permutes the order in which four follow-up lenses are applied to each observed
interaction: VERIFY, GAP, RISK and URGENCY.  The 4! schedules are enumerated
exhaustively and ranked deterministically from explicit [0,1] needs.

This is an auditable scheduling heuristic.  It does not inspect model weights,
attention, embeddings or hidden states and does not prove semantic truth.
"""
from __future__ import annotations

import argparse
import hashlib
import itertools
import json
from pathlib import Path
from typing import Any

LENSES = ("VERIFY", "GAP", "RISK", "URGENCY")
POSITION_WEIGHTS = (4.0, 3.0, 2.0, 1.0)


class FeedbackError(ValueError):
    pass


def _number01(value: Any, name: str) -> float:
    if not isinstance(value, (int, float)) or isinstance(value, bool):
        raise FeedbackError(f"{name}:not_number")
    value = float(value)
    if not 0.0 <= value <= 1.0:
        raise FeedbackError(f"{name}:out_of_range")
    return value


def _needs(interaction: dict[str, Any]) -> dict[str, float]:
    m = interaction.get("metrics")
    if not isinstance(m, dict):
        raise FeedbackError("metrics")
    evidence = _number01(m.get("evidence"), "evidence")
    gap = _number01(m.get("gap"), "gap")
    risk = _number01(m.get("risk"), "risk")
    urgency = _number01(m.get("urgency"), "urgency")
    return {
        "VERIFY": 1.0 - evidence,
        "GAP": gap,
        "RISK": risk,
        "URGENCY": urgency,
    }


def _schedule_score(order: tuple[str, ...], needs: dict[str, float]) -> float:
    return sum(POSITION_WEIGHTS[i] * needs[name] for i, name in enumerate(order))


def rank_schedules(interaction: dict[str, Any], seed: str) -> dict[str, Any]:
    iid = interaction.get("interaction_id")
    if not isinstance(iid, str) or not iid:
        raise FeedbackError("interaction_id")
    needs = _needs(interaction)
    ranked = []
    for order in itertools.permutations(LENSES):
        digest = hashlib.sha256((seed + "|" + iid + "|" + ">".join(order)).encode()).hexdigest()
        ranked.append({
            "order": list(order),
            "score": round(_schedule_score(order, needs), 6),
            "tie_break": digest[:16],
        })
    ranked.sort(key=lambda x: (-x["score"], x["tie_break"], x["order"]))
    return {
        "interaction_id": iid,
        "chronology_index": interaction.get("chronology_index"),
        "needs": needs,
        "permutation_count": len(ranked),
        "selected_schedule": ranked[0]["order"],
        "selected_score": ranked[0]["score"],
        "all_schedules": ranked,
    }


def evaluate(payload: dict[str, Any]) -> dict[str, Any]:
    if payload.get("schema_version") != "rafaelia.conversation-permutation-feedback/v1":
        raise FeedbackError("schema_version")
    interactions = payload.get("interactions")
    if not isinstance(interactions, list) or not interactions:
        raise FeedbackError("interactions")
    seed = payload.get("seed", "RAFAELIA-PERMUTATION-V1")
    if not isinstance(seed, str) or not seed:
        raise FeedbackError("seed")

    seen = set()
    previous_index = -1
    results = []
    for item in interactions:
        if not isinstance(item, dict):
            raise FeedbackError("interaction_not_object")
        iid = item.get("interaction_id")
        if iid in seen:
            raise FeedbackError("duplicate_interaction_id")
        seen.add(iid)
        idx = item.get("chronology_index")
        if not isinstance(idx, int) or isinstance(idx, bool) or idx < 0:
            raise FeedbackError(f"{iid}:chronology_index")
        if idx <= previous_index:
            raise FeedbackError("chronology_not_strictly_increasing")
        previous_index = idx
        results.append(rank_schedules(item, seed))

    urgency_queue = sorted(
        results,
        key=lambda x: (
            -max(x["needs"].values()),
            x["chronology_index"],
            x["interaction_id"],
        ),
    )
    blocking = payload.get("blocking_gaps", [])
    if not isinstance(blocking, list):
        raise FeedbackError("blocking_gaps")

    return {
        "schema": "rafaelia.conversation-permutation-feedback-result/v1",
        "chronology_preserved": True,
        "interaction_count": len(results),
        "permutations_per_interaction": 24,
        "interactions": results,
        "urgency_queue": [x["interaction_id"] for x in urgency_queue],
        "blocking_gaps": blocking,
        "claim_allowed": False,
        "F_ok": ["chronology_preserved", "exhaustive_4_factor_permutations", "deterministic_tie_break"],
        "F_gap": blocking,
        "F_next": ["bind_full_conversation_export", "resolve_highest_need_first", "emit_append_only_receipt"],
    }


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("input", type=Path)
    p.add_argument("--out", type=Path)
    args = p.parse_args()
    try:
        result = evaluate(json.loads(args.input.read_text(encoding="utf-8")))
        code = 0
    except (OSError, json.JSONDecodeError, FeedbackError) as exc:
        result = {"schema":"rafaelia.conversation-permutation-feedback-result/v1","status":"FAIL","reason":str(exc),"claim_allowed":False}
        code = 1
    text = json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding="utf-8")
    print(text, end="")
    return code

if __name__ == "__main__":
    raise SystemExit(main())
