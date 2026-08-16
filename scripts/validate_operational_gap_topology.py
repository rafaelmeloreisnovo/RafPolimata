#!/usr/bin/env python3
"""Fail-closed validator for the RafPolimata operational gap topology.

Stdlib-only by design. It validates semantics that JSON Schema alone does not cover:
unique IDs, valid edge endpoints, fail-closed claims, closure completeness, and
acyclic hard dependency (`requires`) edges.
"""
from __future__ import annotations

import argparse
import json
from collections import Counter, defaultdict, deque
from pathlib import Path
from typing import Any

SCHEMA = "raf.operational-gap-topology.v1"
VALID_STATES = {"REFERENCE", "AUDIT", "IMPLEMENTED", "PENDING", "PASS", "FAIL", "TOKEN_VAZIO"}
UNKNOWN_OR_BLOCKED = {"TOKEN_VAZIO", "PENDING", "FAIL"}
VALID_URGENCY = {"P0", "P1", "P2", "P3"}
VALID_IMPACT = {"CRITICAL", "HIGH", "MEDIUM", "LOW"}
VALID_RELATIONS = {"requires", "blocks", "supports", "governed_by", "mitigates", "observed_in", "commercializes", "evidences"}
VALID_STRENGTH = {"HARD", "SOFT"}


def load(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise SystemExit(f"operational-topology: invalid JSON: {exc}") from exc
    if not isinstance(value, dict):
        raise SystemExit("operational-topology: root must be an object")
    return value


def _nonempty_list(value: Any) -> bool:
    return isinstance(value, list) and bool(value) and all(isinstance(item, str) and item.strip() for item in value)


def _has_requires_cycle(gap_ids: set[str], edges: list[dict[str, Any]]) -> bool:
    adjacency: dict[str, set[str]] = defaultdict(set)
    indegree = {gap_id: 0 for gap_id in gap_ids}
    for edge in edges:
        if edge.get("relation") != "requires":
            continue
        source, target = edge.get("from"), edge.get("to")
        if source not in gap_ids or target not in gap_ids or source == target:
            continue
        if target not in adjacency[source]:
            adjacency[source].add(target)
            indegree[target] += 1
    queue = deque(sorted(node for node, degree in indegree.items() if degree == 0))
    visited = 0
    while queue:
        node = queue.popleft()
        visited += 1
        for target in sorted(adjacency[node]):
            indegree[target] -= 1
            if indegree[target] == 0:
                queue.append(target)
    return visited != len(gap_ids)


def validate(data: dict[str, Any]) -> dict[str, Any]:
    errors: list[str] = []
    warnings: list[str] = []

    if data.get("schema") != SCHEMA:
        errors.append(f"schema must be {SCHEMA}")
    if data.get("state_model", {}).get("unknown_state") != "TOKEN_VAZIO":
        errors.append("state_model.unknown_state must remain TOKEN_VAZIO")

    gaps = data.get("gaps")
    edges = data.get("edges")
    if not isinstance(gaps, list) or not gaps:
        errors.append("gaps must be a non-empty list")
        gaps = []
    if not isinstance(edges, list):
        errors.append("edges must be a list")
        edges = []

    gap_ids = [str(gap.get("id", "")) for gap in gaps if isinstance(gap, dict)]
    duplicates = sorted(key for key, count in Counter(gap_ids).items() if key and count > 1)
    if duplicates:
        errors.append(f"duplicate gap ids: {duplicates}")
    gap_set = set(gap_ids)

    for index, gap in enumerate(gaps):
        if not isinstance(gap, dict):
            errors.append(f"gaps[{index}] must be an object")
            continue
        gap_id = str(gap.get("id", f"gaps[{index}]"))
        state = gap.get("state")
        urgency = gap.get("urgency")
        impact = gap.get("impact")
        if state not in VALID_STATES:
            errors.append(f"{gap_id}: invalid state {state!r}")
        if urgency not in VALID_URGENCY:
            errors.append(f"{gap_id}: invalid urgency {urgency!r}")
        if impact not in VALID_IMPACT:
            errors.append(f"{gap_id}: invalid impact {impact!r}")
        if state in UNKNOWN_OR_BLOCKED and gap.get("claim_allowed") is not False:
            errors.append(f"{gap_id}: {state} requires claim_allowed=false")
        if state == "PASS" and not _nonempty_list(gap.get("evidence")):
            errors.append(f"{gap_id}: PASS requires non-empty evidence")
        if urgency in {"P0", "P1"} and not _nonempty_list(gap.get("required_roles")):
            errors.append(f"{gap_id}: {urgency} requires explicit required_roles")
        if urgency in {"P0", "P1"} and not _nonempty_list(gap.get("reference_controls")):
            errors.append(f"{gap_id}: {urgency} requires reference_controls")
        if not _nonempty_list(gap.get("provenance")):
            errors.append(f"{gap_id}: provenance must be non-empty")
        if not isinstance(gap.get("owner_role"), str) or not gap.get("owner_role", "").strip():
            errors.append(f"{gap_id}: owner_role is required")
        if not isinstance(gap.get("next_action"), str) or not gap.get("next_action", "").strip():
            errors.append(f"{gap_id}: next_action is required")
        closure = gap.get("closure")
        if not isinstance(closure, dict):
            errors.append(f"{gap_id}: closure object is required")
            continue
        if not isinstance(closure.get("condition"), str) or not closure.get("condition", "").strip():
            errors.append(f"{gap_id}: closure.condition is required")
        if not _nonempty_list(closure.get("required_artifacts")):
            errors.append(f"{gap_id}: closure.required_artifacts must be non-empty")
        if not _nonempty_list(closure.get("required_gates")):
            errors.append(f"{gap_id}: closure.required_gates must be non-empty")
        if gap.get("uncertainty_class") == "OWNER_DECISION" and gap.get("owner_decision_required") is not True:
            errors.append(f"{gap_id}: OWNER_DECISION requires owner_decision_required=true")

    edge_ids: list[str] = []
    degree = Counter()
    for index, edge in enumerate(edges):
        if not isinstance(edge, dict):
            errors.append(f"edges[{index}] must be an object")
            continue
        edge_id = str(edge.get("id", f"edges[{index}]"))
        edge_ids.append(edge_id)
        source, target = edge.get("from"), edge.get("to")
        if source not in gap_set:
            errors.append(f"{edge_id}: unknown from endpoint {source!r}")
        if target not in gap_set:
            errors.append(f"{edge_id}: unknown to endpoint {target!r}")
        if source == target:
            errors.append(f"{edge_id}: self-edge is not allowed")
        if edge.get("relation") not in VALID_RELATIONS:
            errors.append(f"{edge_id}: invalid relation {edge.get('relation')!r}")
        if edge.get("strength") not in VALID_STRENGTH:
            errors.append(f"{edge_id}: invalid strength {edge.get('strength')!r}")
        if source in gap_set:
            degree[source] += 1
        if target in gap_set:
            degree[target] += 1

    duplicate_edges = sorted(key for key, count in Counter(edge_ids).items() if key and count > 1)
    if duplicate_edges:
        errors.append(f"duplicate edge ids: {duplicate_edges}")

    isolated = sorted(gap_id for gap_id in gap_set if degree[gap_id] == 0)
    if isolated:
        warnings.append(f"isolated gaps with no topology edge: {isolated}")

    if _has_requires_cycle(gap_set, [edge for edge in edges if isinstance(edge, dict)]):
        errors.append("requires relation contains a dependency cycle")

    counts_by_state = Counter(str(gap.get("state")) for gap in gaps if isinstance(gap, dict))
    counts_by_urgency = Counter(str(gap.get("urgency")) for gap in gaps if isinstance(gap, dict))
    blocking = sorted(
        str(gap["id"])
        for gap in gaps
        if isinstance(gap, dict)
        and gap.get("urgency") in {"P0", "P1"}
        and gap.get("state") in UNKNOWN_OR_BLOCKED
    )
    report = {
        "schema": "raf.operational-gap-topology-validation.v1",
        "state": "FAIL" if errors else "PASS",
        "claim_allowed": False,
        "summary": {
            "gaps": len(gaps),
            "edges": len(edges),
            "blocking_p0_p1": len(blocking),
            "states": dict(sorted(counts_by_state.items())),
            "urgencies": dict(sorted(counts_by_urgency.items())),
        },
        "blocking_p0_p1": blocking,
        "errors": errors,
        "warnings": warnings,
    }
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate RafPolimata operational gap topology")
    parser.add_argument("path", nargs="?", default="configs/operational-gap-topology.v1.json")
    parser.add_argument("--write-report")
    args = parser.parse_args(argv)

    report = validate(load(Path(args.path)))
    payload = json.dumps(report, ensure_ascii=False, sort_keys=True, indent=2) + "\n"
    if args.write_report:
        target = Path(args.write_report)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(payload, encoding="utf-8")
    else:
        print(payload, end="")
    return 0 if report["state"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
