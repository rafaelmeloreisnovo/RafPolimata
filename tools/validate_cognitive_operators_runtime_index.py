#!/usr/bin/env python3
"""Fail-closed validator for the RAFAELIA cognitive operators 71-126 runtime index."""
from __future__ import annotations
import json
import sys
from collections import Counter
from pathlib import Path

SCHEMA = "raf.cognitive-operators-runtime-index.v1"
ALLOWED_CLASSES = {
    "ADAPTED_STANDARD", "CORRECTED_STANDARD", "FORMAL_SPECIFICATION",
    "MODEL_ANALOGY", "RESEARCH_AGENDA_ONLY",
}
EXPECTED_COUNTS = {
    "ADAPTED_STANDARD": 25,
    "MODEL_ANALOGY": 20,
    "CORRECTED_STANDARD": 4,
    "FORMAL_SPECIFICATION": 4,
    "RESEARCH_AGENDA_ONLY": 3,
}
BASE_SYMBOLS = {"Omega_infty", "Psi_v", "Xi", "B_psi", "T_Omega", "Omega_b"}


def validate(data: dict) -> list[str]:
    errors: list[str] = []
    if data.get("schema") != SCHEMA:
        errors.append("schema")
    if data.get("operator_count") != 56 or data.get("operator_range") != [71, 126]:
        errors.append("range_or_count")
    if data.get("claim_allowed") is not False:
        errors.append("claim_allowed")
    source = data.get("canonical_source")
    if not isinstance(source, dict) or not source.get("repository") or not source.get("directory"):
        errors.append("canonical_source")
    operators = data.get("operators")
    if not isinstance(operators, list):
        return errors + ["operators"]
    ids = [item.get("id") for item in operators if isinstance(item, dict)]
    if len(operators) != 56 or ids != list(range(71, 127)) or len(set(ids)) != 56:
        errors.append("ids")
    counts: Counter[str] = Counter()
    for pos, item in enumerate(operators):
        if not isinstance(item, dict):
            errors.append(f"operator_{pos}_type")
            continue
        cls = item.get("class")
        counts[cls] += 1
        if cls not in ALLOWED_CLASSES:
            errors.append(f"operator_{item.get('id')}_class")
        if not isinstance(item.get("name"), str) or not item["name"].strip():
            errors.append(f"operator_{item.get('id')}_name")
        bindings = item.get("bindings")
        if not isinstance(bindings, list) or not bindings or any(b not in BASE_SYMBOLS for b in bindings):
            errors.append(f"operator_{item.get('id')}_bindings")
    if dict(counts) != EXPECTED_COUNTS or data.get("class_counts") != EXPECTED_COUNTS:
        errors.append("class_counts")
    return errors


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: validate_cognitive_operators_runtime_index.py <operators.index.json>", file=sys.stderr)
        return 2
    try:
        data = json.loads(Path(argv[1]).read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(json.dumps({"status": "FAIL", "error": str(exc)}, ensure_ascii=False))
        return 3
    errors = validate(data)
    print(json.dumps({"status": "PASS" if not errors else "FAIL", "errors": errors,
                      "operator_count": len(data.get("operators", [])),
                      "claim_allowed": data.get("claim_allowed")}, ensure_ascii=False, sort_keys=True))
    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
