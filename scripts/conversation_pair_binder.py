#!/usr/bin/env python3
"""Bind ChatGPT export shards into auditable user→assistant interaction pairs.

The binder follows each assistant node's `parent` ancestry to the nearest textual
user ancestor. It never invents a linear order for branches, never assigns
conversation-level feedback to one response, and never derives evidence/gap/
risk/urgency scores from thumbs-up/down.

Output is suitable as an upstream source for later RAFAELIA triage once an
independent metrics record exists.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path
from typing import Any, Iterable

SHARD_RE = re.compile(r"conversations-(\d{3})\.json$")


class BinderError(ValueError):
    pass


def text_of(message: dict[str, Any] | None) -> str | None:
    if not isinstance(message, dict):
        return None
    content = message.get("content")
    if not isinstance(content, dict):
        return None
    parts = content.get("parts")
    if not isinstance(parts, list):
        return None
    out: list[str] = []
    for part in parts:
        if isinstance(part, str):
            out.append(part)
        elif isinstance(part, dict):
            candidate = part.get("text")
            if isinstance(candidate, str):
                out.append(candidate)
    text = "\n".join(x for x in out if x).strip()
    return text or None


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def conversation_id(conv: dict[str, Any]) -> str:
    value = conv.get("id") or conv.get("conversation_id")
    if not isinstance(value, str) or not value:
        raise BinderError("conversation_without_id")
    return value


def nearest_user_ancestor(mapping: dict[str, Any], node: dict[str, Any]) -> dict[str, Any] | None:
    parent = node.get("parent")
    seen: set[str] = set()
    while isinstance(parent, str) and parent and parent not in seen:
        seen.add(parent)
        parent_node = mapping.get(parent)
        if not isinstance(parent_node, dict):
            return None
        message = parent_node.get("message")
        role = ((message or {}).get("author") or {}).get("role") if isinstance(message, dict) else None
        if role == "user" and text_of(message):
            return parent_node
        parent = parent_node.get("parent")
    return None


def bind_conversation(conv: dict[str, Any], source_shard: str) -> list[dict[str, Any]]:
    cid = conversation_id(conv)
    mapping = conv.get("mapping")
    if not isinstance(mapping, dict):
        return []
    rows: list[dict[str, Any]] = []
    for node_id, node in mapping.items():
        if not isinstance(node, dict):
            continue
        assistant = node.get("message")
        if not isinstance(assistant, dict):
            continue
        if ((assistant.get("author") or {}).get("role")) != "assistant":
            continue
        assistant_text = text_of(assistant)
        if not assistant_text:
            continue
        user_node = nearest_user_ancestor(mapping, node)
        if not user_node:
            continue
        user = user_node.get("message")
        user_text = text_of(user)
        if not isinstance(user, dict) or not user_text:
            continue
        uid = user.get("id") or user_node.get("id")
        aid = assistant.get("id") or node_id
        interaction_id = f"{cid}:{uid}:{aid}"
        rows.append({
            "interaction_id": interaction_id,
            "conversation_id": cid,
            "conversation_title": conv.get("title"),
            "source_shard": source_shard,
            "user_message_id": uid,
            "assistant_message_id": aid,
            "user_create_time": user.get("create_time"),
            "assistant_create_time": assistant.get("create_time"),
            "user_text": user_text,
            "assistant_text": assistant_text,
            "lineage": {
                "assistant_node_id": node_id,
                "assistant_parent_node_id": node.get("parent"),
                "nearest_user_node_id": user_node.get("id"),
            },
            "conversation_feedback": [],
            "feedback_scope": "conversation_only_if_present",
            "operational_metrics": {
                "state": "TOKEN_VAZIO_UNDERIVED",
                "evidence": None,
                "gap": None,
                "risk": None,
                "urgency": None,
            },
            "claim_allowed": False,
        })
    return rows


def load_feedback(path: Path | None) -> dict[str, list[dict[str, Any]]]:
    if path is None:
        return {}
    raw = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(raw, list):
        raise BinderError("feedback_not_list")
    out: dict[str, list[dict[str, Any]]] = {}
    for item in raw:
        if not isinstance(item, dict):
            continue
        cid = item.get("conversation_id")
        rating = item.get("rating")
        if not isinstance(cid, str) or rating not in {"thumbs_up", "thumbs_down"}:
            continue
        out.setdefault(cid, []).append({
            "feedback_id": item.get("id"),
            "rating": rating,
            "create_time": item.get("create_time"),
            "update_time": item.get("update_time"),
            "scope": "conversation",
        })
    return out


def shard_number(path: Path) -> int | None:
    m = SHARD_RE.search(path.name)
    return int(m.group(1)) if m else None


def build_report(paths: Iterable[Path], feedback_path: Path | None = None) -> dict[str, Any]:
    paths = list(paths)
    if not paths:
        raise BinderError("no_shards")
    feedback = load_feedback(feedback_path)
    interactions: list[dict[str, Any]] = []
    shard_receipts: list[dict[str, Any]] = []
    seen_interactions: set[str] = set()
    observed_numbers: list[int] = []
    conversation_count = 0

    for path in paths:
        raw = json.loads(path.read_text(encoding="utf-8"))
        if not isinstance(raw, list):
            raise BinderError(f"{path.name}:root_not_list")
        number = shard_number(path)
        if number is not None:
            observed_numbers.append(number)
        shard_pairs = 0
        for conv in raw:
            if not isinstance(conv, dict):
                continue
            conversation_count += 1
            rows = bind_conversation(conv, path.name)
            cid = conversation_id(conv)
            cfeedback = feedback.get(cid, [])
            for row in rows:
                iid = row["interaction_id"]
                if iid in seen_interactions:
                    continue
                seen_interactions.add(iid)
                # Conversation feedback is repeated as contextual metadata only;
                # it is NOT response-level attribution.
                row["conversation_feedback"] = cfeedback
                interactions.append(row)
                shard_pairs += 1
        shard_receipts.append({
            "file_name": path.name,
            "bytes": path.stat().st_size,
            "sha256": sha256_file(path),
            "conversation_count": len(raw),
            "interaction_pair_count": shard_pairs,
        })

    interactions.sort(key=lambda r: (
        float(r["assistant_create_time"]) if isinstance(r.get("assistant_create_time"), (int, float)) else float("inf"),
        r["conversation_id"],
        r["assistant_message_id"],
    ))
    for idx, row in enumerate(interactions):
        row["chronology_index"] = idx

    gaps: list[int] = []
    if observed_numbers:
        lo, hi = min(observed_numbers), max(observed_numbers)
        gaps = sorted(set(range(lo, hi + 1)) - set(observed_numbers))

    return {
        "schema": "rafaelia.conversation-pair-binder/v1",
        "shards_observed": len(paths),
        "shard_numbers": sorted(observed_numbers),
        "missing_shard_numbers_within_observed_range": gaps,
        "conversation_count": conversation_count,
        "interaction_pair_count": len(interactions),
        "feedback_record_count": sum(len(v) for v in feedback.values()),
        "feedback_conversation_count": len(feedback),
        "feedback_scope": "conversation_only",
        "chronology_preserved": True,
        "branch_lineage_preserved": True,
        "operational_metrics_derived": False,
        "shard_receipts": shard_receipts,
        "interactions": interactions,
        "claim_allowed": False,
        "F_gap": [
            "TOKEN_VAZIO_OPERATIONAL_METRICS_PER_INTERACTION",
            *[f"TOKEN_VAZIO_MISSING_SHARD_{n:03d}" for n in gaps],
        ],
    }


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("shards", nargs="+", type=Path)
    p.add_argument("--feedback", type=Path)
    p.add_argument("--out", required=True, type=Path)
    args = p.parse_args()
    try:
        report = build_report(args.shards, args.feedback)
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(report, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print(json.dumps({k: report[k] for k in (
            "shards_observed", "conversation_count", "interaction_pair_count",
            "feedback_record_count", "missing_shard_numbers_within_observed_range",
            "operational_metrics_derived", "claim_allowed")}, ensure_ascii=False, indent=2))
        return 0
    except (OSError, json.JSONDecodeError, BinderError) as exc:
        print(json.dumps({"status":"FAIL","reason":str(exc),"claim_allowed":False}))
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
