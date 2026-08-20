#!/usr/bin/env python3
"""RAFAELIA neurocognitive/biophysical claim contract validator.

Deterministic static validation only. It never upgrades scientific validity.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path

REQUIRED = {
    "claim_id", "status", "priority", "claim_allowed", "claim",
    "falsifier", "next_gate",
}
FORBIDDEN_PROMOTIONS = {"PASS", "REPLICATED_INDEPENDENT"}

def validate_claim(c: dict) -> list[str]:
    e: list[str] = []
    missing = sorted(REQUIRED - set(c))
    if missing:
        e.append("missing:" + ",".join(missing))
    if c.get("claim_allowed") is not False:
        e.append("claim_allowed_must_be_false")
    text = str(c.get("claim", "")).lower()

    if "dna" in text and "sequence" in text:
        if any(x in text for x in ("methylation", "adduct", "expression")):
            if not any(x in text for x in ("not ", "not automatically", "≠", "distinct")):
                e.append("possible_dna_endpoint_conflation")

    if "emf" in text or "electromagnetic" in text:
        gate = str(c.get("next_gate", "")).lower()
        if not any(x in gate for x in ("dosimetry", "dose", "sar")):
            e.append("emf_missing_dosimetry_gate")

    if "upe" in text or "photon" in text:
        gate = str(c.get("next_gate", "")).lower()
        if c.get("status") in FORBIDDEN_PROMOTIONS and "source" not in gate:
            e.append("upe_promotion_without_source_attribution")
        if not any(x in gate for x in ("source", "background", "adversarial")):
            e.append("upe_missing_source_background_gate")

    if c.get("status") == "PASS":
        e.append("pass_not_authorized_by_static_validator")
    return e

def validate_jsonl(path: Path) -> tuple[int, list[dict]]:
    issues = []
    count = 0
    for line_no, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if not line.strip():
            continue
        count += 1
        try:
            obj = json.loads(line)
        except json.JSONDecodeError as exc:
            issues.append({"line": line_no, "errors": [f"invalid_json:{exc.msg}"]})
            continue
        errs = validate_claim(obj)
        if errs:
            issues.append({"line": line_no, "claim_id": obj.get("claim_id"), "errors": errs})
    return count, issues

def main(argv=None) -> int:
    p = argparse.ArgumentParser()
    p.add_argument("jsonl", type=Path)
    p.add_argument("--json", action="store_true", dest="as_json")
    args = p.parse_args(argv)
    count, issues = validate_jsonl(args.jsonl)
    out = {
        "schema": "RAFAELIA_NEUROBIO_STATIC_VALIDATION_V1",
        "records": count,
        "issues": issues,
        "result": "PASS_STATIC_CONTRACT" if not issues else "FAIL_STATIC_CONTRACT",
        "scientific_claim_allowed": False,
        "note": "Static contract validation is not biological evidence.",
    }
    if args.as_json:
        print(json.dumps(out, ensure_ascii=False, sort_keys=True))
    else:
        print(out["result"], f"records={count}", f"issues={len(issues)}")
    return 0 if not issues else 1

if __name__ == "__main__":
    raise SystemExit(main())
