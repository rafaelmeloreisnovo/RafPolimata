#!/usr/bin/env python3
"""Fail-closed validator for RafGitTools -> RafPolimata custody bridge envelopes."""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONTRACT = ROOT / "configs" / "rafgittools-custody-consumer.v1.json"
SECRET_PREFIXES = ("ghp_", "github_pat_", "gho_", "ghu_", "ghs_", "ghr_", "sk-")
REQUIRED_FIELDS = {
    "schemaVersion", "bridgeId", "producer", "consumer", "sourceRef",
    "artifactRef", "executionRef", "evidenceRefs", "state", "claimAllowed",
    "predecessorReceipt", "supersedesReceipt", "capabilityLabels", "observedAt",
}
STRING_REF_FIELDS = (
    "sourceRef", "artifactRef", "executionRef", "predecessorReceipt", "supersedesReceipt"
)


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def _non_empty_string(value: object) -> bool:
    return isinstance(value, str) and bool(value)


def validate(envelope: dict, contract: dict) -> list[str]:
    errors: list[str] = []
    if not isinstance(envelope, dict):
        return ["envelope must be an object"]

    missing = sorted(REQUIRED_FIELDS - set(envelope))
    if missing:
        errors.append(f"missing fields: {missing}")

    unexpected = sorted(set(envelope) - REQUIRED_FIELDS)
    if unexpected:
        errors.append(f"unexpected fields: {unexpected}")

    if envelope.get("schemaVersion") != contract.get("accepted_bridge_schema"):
        errors.append("bridge schema mismatch")
    if envelope.get("producer") != contract.get("producer"):
        errors.append("producer authority mismatch")
    if envelope.get("consumer") != contract.get("consumer"):
        errors.append("consumer authority mismatch")
    if envelope.get("state") not in set(contract.get("accepted_states", [])):
        errors.append("state not accepted")
    if envelope.get("claimAllowed") is not False:
        errors.append("claimAllowed must remain false")

    if not _non_empty_string(envelope.get("bridgeId")):
        errors.append("bridgeId must be a non-empty string")
    if not _non_empty_string(envelope.get("observedAt")):
        errors.append("observedAt must be a non-empty string")

    for field in STRING_REF_FIELDS:
        if not _non_empty_string(envelope.get(field)):
            errors.append(f"{field} must be a non-empty string")

    refs = envelope.get("evidenceRefs")
    if not isinstance(refs, list) or any(not _non_empty_string(v) for v in refs):
        errors.append("evidenceRefs must be a string array")

    labels = envelope.get("capabilityLabels")
    if not isinstance(labels, list):
        errors.append("capabilityLabels must be an array")
    else:
        if len(labels) != len(set(labels)):
            errors.append("capabilityLabels must be unique")
        for label in labels:
            if not isinstance(label, str) or not re.fullmatch(r"[A-Z][a-z0-9_]*", label):
                errors.append(f"invalid capability label: {label!r}")
                continue
            if label.lower().startswith(SECRET_PREFIXES):
                errors.append("secret-looking credential material is forbidden")

    return errors


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: validate_rafgittools_custody_bridge.py ENVELOPE.json", file=sys.stderr)
        return 2
    contract = load(CONTRACT)
    envelope = load(Path(argv[1]))
    errors = validate(envelope, contract)
    if errors:
        for error in errors:
            print(f"FAIL: {error}")
        return 1
    print("PASS rafgittools-custody-bridge-v1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
