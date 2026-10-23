#!/usr/bin/env python3
"""Validate ChatGPT operational-session evidence receipts.

This validator classifies a ChatGPT session statement as operational evidence.
It explicitly rejects training/dataset use and fails closed when an automatic
correction is claimed without the complete reproducibility/rollback envelope.

SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
SESSION_CONTEXT != TRAINING_DATASET
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_CONTRACT = ROOT / "data/contracts/chatgpt_operational_session_evidence.v1.json"
DEFAULT_RECEIPT = ROOT / "data/evidence/chatgpt/2026-10-20_operational_session_evidence.v1.json"
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def validate_receipt(receipt: dict[str, Any], contract: dict[str, Any]) -> list[str]:
    errors: list[str] = []

    if receipt.get("schema") != "rafaelia.chatgpt-operational-evidence/v1":
        errors.append("unexpected receipt schema")
    if contract.get("schema") != "rafaelia.chatgpt-operational-evidence.contract/v1":
        errors.append("unexpected contract schema")

    source = receipt.get("source")
    if not isinstance(source, dict):
        return errors + ["source must be an object"]

    for field in contract.get("required_source_fields", []):
        if field not in source:
            errors.append(f"missing source field: {field}")

    if source.get("source_surface") != "ChatGPT":
        errors.append("source_surface must be ChatGPT")
    if source.get("training_use") is not False:
        errors.append("training_use must be false")
    if source.get("dataset_use") is not False:
        errors.append("dataset_use must be false")
    if source.get("operational_evidence_use") is not True:
        errors.append("operational_evidence_use must be true")
    if not SHA256_RE.fullmatch(str(source.get("instruction_sha256_utf8", ""))):
        errors.append("instruction_sha256_utf8 must be a lowercase 64-hex SHA-256")

    evidence = receipt.get("evidence")
    if not isinstance(evidence, dict):
        errors.append("evidence must be an object")
    else:
        allowed_states = set(contract.get("correction_gate", {}).get("states", []))
        if evidence.get("state") not in allowed_states:
            errors.append("evidence state is not allowed")

    correction = receipt.get("automatic_correction")
    if not isinstance(correction, dict):
        errors.append("automatic_correction must be an object")
    else:
        eligible = correction.get("eligible") is True
        if eligible:
            envelope = correction.get("envelope")
            if not isinstance(envelope, dict):
                errors.append("eligible automatic correction requires envelope")
            else:
                for field in contract.get("correction_gate", {}).get("auto_correct_requires", []):
                    value = envelope.get(field)
                    if value in (None, "", [], {}):
                        errors.append(f"eligible automatic correction missing: {field}")
            if evidence.get("state") not in {"REPRODUCED", "AUTO_CORRECT_ELIGIBLE", "PATCH_APPLIED", "PASS"}:
                errors.append("eligible automatic correction requires reproduced-or-later evidence state")
        elif correction.get("state") == "AUTO_CORRECT_ELIGIBLE":
            errors.append("AUTO_CORRECT_ELIGIBLE state requires eligible=true")

    claims = receipt.get("claims")
    if not isinstance(claims, dict) or claims.get("allowed") is not False:
        errors.append("claims.allowed must remain false at evidence-intake stage")

    custody = receipt.get("custody")
    if not isinstance(custody, dict):
        errors.append("custody must be an object")
    else:
        if custody.get("append_only") is not True:
            errors.append("custody.append_only must be true")
        if custody.get("historical_rewrite_forbidden") is not True:
            errors.append("custody.historical_rewrite_forbidden must be true")

    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--contract", type=Path, default=DEFAULT_CONTRACT)
    parser.add_argument("--receipt", type=Path, default=DEFAULT_RECEIPT)
    args = parser.parse_args()

    contract = load_json(args.contract)
    receipt = load_json(args.receipt)
    errors = validate_receipt(receipt, contract)
    payload = {
        "schema": "rafaelia.chatgpt-operational-evidence.validation/v1",
        "state": "PASS" if not errors else "FAIL",
        "errors": errors,
        "training_use": receipt.get("source", {}).get("training_use"),
        "dataset_use": receipt.get("source", {}).get("dataset_use"),
        "operational_evidence_use": receipt.get("source", {}).get("operational_evidence_use"),
        "automatic_correction_eligible": receipt.get("automatic_correction", {}).get("eligible"),
        "claim_allowed": False,
    }
    print(json.dumps(payload, indent=2, sort_keys=True))
    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main())
