#!/usr/bin/env python3
"""RAFAELIA epistemic claim gate V1.

Stdlib-only, fail-closed validator for claim promotion metadata.
It does NOT decide scientific truth. It decides whether a declared
claim_allowed=true is procedurally supported by the recorded gates.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

KINDS = {"DEMONSTRATION", "CONVENTION", "HYPOTHESIS", "PARABLE", "TOKEN_VAZIO"}
STATES = {
    "METAPHOR", "FORMALIZED", "METHOD_DEFINED", "SIMULATED",
    "EVIDENCE_LINKED", "REPLICATED", "CLAIM_ALLOWED",
    "TOKEN_VAZIO", "CONTRADICTION",
}
REPLICATION_RANK = {
    "NONE": 0,
    "VERIFIED_INTERNAL": 1,
    "REPRODUCED_SAME_ENV": 2,
    "REPRODUCED_CLEAN_ENV": 3,
    "REPLICATED_INDEPENDENT": 4,
    "EXTERNAL_CLAIM_READY": 5,
}
REQUIRED = {
    "claim_id", "proposition", "claim_kind", "state", "domain",
    "falsifier", "protocol", "data_provenance", "code_provenance",
    "evidence", "negative_evidence", "replication", "limitations",
    "token_vazio", "claim_allowed",
}


def nonempty(value: Any) -> bool:
    return isinstance(value, str) and bool(value.strip())


def has_material_provenance(items: Any) -> bool:
    if not isinstance(items, list) or not items:
        return False
    for item in items:
        if not isinstance(item, dict):
            continue
        # At least an identity plus one immutable/versioned locator.
        identity = item.get("source") or item.get("path") or item.get("repo") or item.get("artifact")
        locator = item.get("sha256") or item.get("commit") or item.get("version") or item.get("id")
        if identity and locator:
            return True
    return False


def evaluate(record: dict[str, Any]) -> tuple[bool, list[str], list[str]]:
    errors: list[str] = []
    blockers: list[str] = []

    missing_keys = sorted(REQUIRED - set(record))
    if missing_keys:
        errors.append("missing required keys: " + ", ".join(missing_keys))
        return False, errors, blockers

    kind = record.get("claim_kind")
    state = record.get("state")
    if kind not in KINDS:
        errors.append(f"invalid claim_kind: {kind!r}")
    if state not in STATES:
        errors.append(f"invalid state: {state!r}")
    if not nonempty(record.get("claim_id")):
        errors.append("claim_id must be non-empty")
    if not nonempty(record.get("proposition")):
        errors.append("proposition must be non-empty")
    if not nonempty(record.get("domain")):
        errors.append("domain must be non-empty")

    protocol = record.get("protocol")
    if not isinstance(protocol, dict):
        errors.append("protocol must be an object")
        protocol = {}
    if not nonempty(protocol.get("version")):
        blockers.append("protocol.version missing")
    if protocol.get("frozen") is not True:
        blockers.append("protocol is not frozen")

    token_vazio = record.get("token_vazio")
    if not isinstance(token_vazio, list):
        errors.append("token_vazio must be a list")
        token_vazio = []
    critical_gaps = [
        x for x in token_vazio
        if isinstance(x, dict) and x.get("critical") is True
    ]
    if critical_gaps:
        blockers.append(f"critical TOKEN_VAZIO count={len(critical_gaps)}")

    replication = record.get("replication")
    if not isinstance(replication, dict):
        errors.append("replication must be an object")
        replication = {}
    level = replication.get("level", "NONE")
    if level not in REPLICATION_RANK:
        errors.append(f"invalid replication.level: {level!r}")
        level = "NONE"

    # Type-specific epistemic rules.
    if kind in {"PARABLE", "TOKEN_VAZIO"}:
        blockers.append(f"{kind} cannot be promoted to CLAIM_ALLOWED")

    if kind == "HYPOTHESIS":
        if not nonempty(record.get("h0")):
            blockers.append("HYPOTHESIS requires h0")
        if not nonempty(record.get("h1")):
            blockers.append("HYPOTHESIS requires h1")
        if not nonempty(record.get("falsifier")):
            blockers.append("HYPOTHESIS requires explicit falsifier")
        if not nonempty(record.get("metric")):
            blockers.append("HYPOTHESIS requires metric")
        if record.get("threshold") is None:
            blockers.append("HYPOTHESIS requires threshold/decision rule")
        if not has_material_provenance(record.get("data_provenance")):
            blockers.append("HYPOTHESIS requires versioned/hashed data provenance")
        if not has_material_provenance(record.get("code_provenance")):
            blockers.append("HYPOTHESIS requires versioned/hashed code provenance")
        if not isinstance(record.get("evidence"), list) or not record.get("evidence"):
            blockers.append("HYPOTHESIS requires material evidence")
        if REPLICATION_RANK[level] < REPLICATION_RANK["REPRODUCED_CLEAN_ENV"]:
            blockers.append("HYPOTHESIS requires at least REPRODUCED_CLEAN_ENV for promotion")
        intended_scope = record.get("intended_scope", "INTERNAL")
        if intended_scope == "PUBLIC_SCIENTIFIC" and REPLICATION_RANK[level] < REPLICATION_RANK["REPLICATED_INDEPENDENT"]:
            blockers.append("PUBLIC_SCIENTIFIC claim requires independent replication")

    if kind == "DEMONSTRATION":
        if not nonempty(record.get("falsifier")):
            blockers.append("DEMONSTRATION requires counterexample/check-failure criterion")
        if not isinstance(record.get("evidence"), list) or not record.get("evidence"):
            blockers.append("DEMONSTRATION requires proof/check evidence")

    if kind == "CONVENTION":
        # Convention may be asserted as a convention, never silently as empirical proof.
        if record.get("intended_scope") == "PUBLIC_SCIENTIFIC":
            blockers.append("CONVENTION cannot be promoted as PUBLIC_SCIENTIFIC evidence")

    if state == "CONTRADICTION":
        blockers.append("state=CONTRADICTION")
    if state == "TOKEN_VAZIO":
        blockers.append("state=TOKEN_VAZIO")

    computed_allowed = not errors and not blockers
    return computed_allowed, errors, blockers


def main() -> int:
    parser = argparse.ArgumentParser(description="Fail-closed RAFAELIA epistemic claim gate")
    parser.add_argument("record", type=Path, help="claim record JSON")
    args = parser.parse_args()

    try:
        record = json.loads(args.record.read_text(encoding="utf-8"))
    except Exception as exc:
        print(json.dumps({"status": "INVALID", "error": str(exc)}, ensure_ascii=False, indent=2))
        return 2

    if not isinstance(record, dict):
        print(json.dumps({"status": "INVALID", "error": "root must be object"}, indent=2))
        return 2

    computed, errors, blockers = evaluate(record)
    declared = record.get("claim_allowed") is True
    coherent = not errors and (not declared or computed)

    output = {
        "status": "PASS" if coherent else "FAIL",
        "declared_claim_allowed": declared,
        "computed_claim_allowed": computed,
        "errors": errors,
        "blockers": blockers,
    }
    print(json.dumps(output, ensure_ascii=False, sort_keys=True, indent=2))

    if errors:
        return 2
    if declared and not computed:
        return 3
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
