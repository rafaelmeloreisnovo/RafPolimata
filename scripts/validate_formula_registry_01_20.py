#!/usr/bin/env python3
"""Validate research/formula_registry_01_20/registry.v1.json using stdlib only."""
from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REGISTRY = ROOT / "research" / "formula_registry_01_20" / "registry.v1.json"
DEFAULT_REPORT = ROOT / "results" / "formula_registry_01_20_report.json"

REQUIRED_FIELDS = {
    "id", "name", "family", "priority", "state", "expression_original",
    "primary_gap", "dimensional_gate", "correction", "falsifier", "claim_allowed",
}
EXPECTED_IDS = [f"F{i:02d}" for i in range(1, 21)]
EXPECTED_FAMILIES = {
    "dynamical_topology",
    "information_governance",
    "waves_physics",
    "bio_signals",
    "compression_statistics",
}
P0_REQUIRED = {"F01", "F04", "F06", "F09", "F11", "F13", "F16"}
BLOCKED_OR_CONTRADICTED = {"F06", "F09", "F11"}


def validate(path: Path) -> dict[str, object]:
    doc = json.loads(path.read_text(encoding="utf-8"))
    if doc.get("schema") != "rafaelia.formula-registry.v1":
        raise SystemExit("invalid_schema")
    if doc.get("claim_allowed") is not False:
        raise SystemExit("registry_claim_must_be_false")
    if "token_vazio" not in str(doc.get("token_vazio_rule", "")).lower():
        raise SystemExit("missing_token_vazio_rule")
    records = doc.get("records")
    if not isinstance(records, list) or len(records) != 20:
        raise SystemExit("registry_must_have_20_records")

    ids: list[str] = []
    families: Counter[str] = Counter()
    priorities: Counter[str] = Counter()
    states: Counter[str] = Counter()
    dimensional: Counter[str] = Counter()

    for record in records:
        if not isinstance(record, dict):
            raise SystemExit("record_must_be_object")
        missing = REQUIRED_FIELDS - record.keys()
        if missing:
            raise SystemExit(f"missing_fields:{record.get('id')}:{sorted(missing)}")
        rid = record["id"]
        ids.append(rid)
        if record["claim_allowed"] is not False:
            raise SystemExit(f"claim_inflation:{rid}")
        if not str(record["primary_gap"]).strip():
            raise SystemExit(f"missing_gap:{rid}")
        if not str(record["falsifier"]).strip():
            raise SystemExit(f"missing_falsifier:{rid}")
        if not str(record["correction"]).strip():
            raise SystemExit(f"missing_correction:{rid}")
        if record["family"] not in EXPECTED_FAMILIES:
            raise SystemExit(f"unknown_family:{rid}:{record['family']}")
        if record["priority"] not in {"P0", "P1", "P2"}:
            raise SystemExit(f"invalid_priority:{rid}:{record['priority']}")
        families[record["family"]] += 1
        priorities[record["priority"]] += 1
        states[record["state"]] += 1
        dimensional[record["dimensional_gate"]] += 1

    if ids != EXPECTED_IDS:
        raise SystemExit(f"ids_must_be_ordered_01_20:{ids}")
    p0_ids = {r["id"] for r in records if r["priority"] == "P0"}
    if p0_ids != P0_REQUIRED:
        raise SystemExit(f"unexpected_p0_set:{sorted(p0_ids)}")

    by_id = {r["id"]: r for r in records}
    if by_id["F06"]["dimensional_gate"] != "FAIL_AS_WRITTEN":
        raise SystemExit("F06_must_remain_dimension_failure")
    if by_id["F09"]["state"] != "CONTRADICTION_ORIGINAL":
        raise SystemExit("F09_original_divergence_must_be_preserved")
    if by_id["F11"]["state"] != "BLOCKED_AS_WRITTEN":
        raise SystemExit("F11_must_remain_blocked")
    for rid in BLOCKED_OR_CONTRADICTED:
        if by_id[rid]["claim_allowed"] is not False:
            raise SystemExit(f"blocked_record_promoted:{rid}")

    return {
        "schema": doc["schema"],
        "record_count": len(records),
        "ids": ids,
        "families": dict(sorted(families.items())),
        "priorities": dict(sorted(priorities.items())),
        "states": dict(sorted(states.items())),
        "dimensional_gates": dict(sorted(dimensional.items())),
        "claim_allowed": False,
        "critical_blocked_or_contradicted": sorted(BLOCKED_OR_CONTRADICTED),
        "verdict": "PASS",
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--registry", type=Path, default=DEFAULT_REGISTRY)
    parser.add_argument("--output", type=Path, default=DEFAULT_REPORT)
    args = parser.parse_args()
    registry = args.registry if args.registry.is_absolute() else ROOT / args.registry
    output = args.output if args.output.is_absolute() else ROOT / args.output
    result = validate(registry)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, ensure_ascii=False, sort_keys=True))


if __name__ == "__main__":
    main()
