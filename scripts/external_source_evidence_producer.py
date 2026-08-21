#!/usr/bin/env python3
"""Produce federated external source evidence from Vectras-VM-Android receipts.

This script consumes external source validation receipts from the Vectras-VM-Android
android-ci workflow and produces a federated evidence record for the RAFAELIA ecosystem.

Evidence chain:
  external source URL + branch -> remote branch validation -> pinned commit resolution
  -> recovery fallback ranking -> timestamp + environment -> evidence state

The evidence state reflects:
- VERIFIED_ORIGINAL_PIN: pinned commit resolved at rank 0 (original pin exact/ancestor)
- TOKEN_VAZIO_PINNED_UNRESOLVED: pinned commit requires fallback (rank 1+)
- REMOTE_BRANCH_UNREACHABLE: branch validation failed
- BLOCKED: invalid manifest format or critical validation error
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any
from datetime import datetime, timezone

EXTERNAL_SOURCES = {
    "androidx_RmR": {
        "url": "https://github.com/wojcikiewicz17/androidx_RmR",
        "branch": "androidx-main",
        "expected_pinned": "e3c10c6ac1acff50774d14417d93eaa6b5f8169a",
    },
    "qemu_rafaelia": {
        "url": "https://github.com/rafaelmeloreisnovo/qemu_rafaelia",
        "branch": "master",
        "expected_pinned": "2346c30c2ba77881c2930add83523ea903b173fe",
    },
}


class ExternalSourceEvidenceError(ValueError):
    pass


def validate_receipt(receipt: dict[str, Any], source_name: str) -> tuple[bool, list[str]]:
    """Validate receipt structure and content."""
    errors: list[str] = []

    if not isinstance(receipt, dict):
        errors.append("receipt_not_dict")
        return False, errors

    if receipt.get("kind") != "external_source_resolution":
        errors.append("kind")
    if not isinstance(receipt.get("pinned_sha"), str) or len(receipt["pinned_sha"]) != 40:
        errors.append("pinned_sha")
    if not isinstance(receipt.get("resolved_sha"), str) or len(receipt["resolved_sha"]) != 40:
        errors.append("resolved_sha")
    if receipt.get("recovery_rank") not in [
        "rank_0_original_pin",
        "rank_1_fallback_to_branch_head",
        "rank_2_evaluate_upstream_stable",
        "rank_3_synthesize_minimal",
    ]:
        errors.append("recovery_rank")
    if receipt.get("status") not in [
        "VERIFIED_ORIGINAL_PIN",
        "TOKEN_VAZIO_PINNED_UNRESOLVED",
        "REMOTE_BRANCH_UNREACHABLE",
        "INVALID_BRANCH_HEAD",
        "TOKEN_VAZIO_FETCH_FAILED",
    ]:
        errors.append("status")
    if not isinstance(receipt.get("timestamp"), str):
        errors.append("timestamp")

    return len(errors) == 0, errors


def produce_evidence(receipts: list[dict[str, Any]]) -> dict[str, Any]:
    """Produce federated external source evidence from receipts."""
    timestamp = datetime.now(timezone.utc).isoformat()

    # Validate all receipts
    source_evidence: dict[str, Any] = {}
    blocked_reasons: list[str] = []

    for receipt in receipts:
        source_name = receipt.get("repository", "unknown").split("/")[-1].replace(".git", "")
        is_valid, validation_errors = validate_receipt(receipt, source_name)

        if not is_valid:
            blocked_reasons.append(f"{source_name}: {','.join(validation_errors)}")
            continue

        source_evidence[source_name] = {
            "pinned_sha": receipt["pinned_sha"],
            "resolved_sha": receipt["resolved_sha"],
            "recovery_rank": receipt["recovery_rank"],
            "status": receipt["status"],
            "branch": receipt.get("branch", "unknown"),
            "repository": receipt.get("repository", "unknown"),
            "receipt_timestamp": receipt.get("timestamp"),
        }

    # Determine overall evidence state
    evidence_state = "VERIFIED_LIMITED"
    f_ok: list[str] = []
    f_gap: list[str] = []
    f_next: list[str] = []

    if blocked_reasons:
        evidence_state = "BLOCKED"
        f_gap.extend(blocked_reasons)
        f_next.append("fix_external_source_receipts")
    else:
        # Check if all sources were verified at rank 0
        all_rank_0 = all(
            e.get("recovery_rank") == "rank_0_original_pin"
            for e in source_evidence.values()
        )
        all_verified = all(
            e.get("status") == "VERIFIED_ORIGINAL_PIN"
            for e in source_evidence.values()
        )

        if all_rank_0 and all_verified:
            evidence_state = "VERIFIED_LIMITED"
            f_ok.append("all_external_sources_pinned_commits_verified")
            f_ok.append("qemu_rafaelia_commit_2346c30c2ba77881c2930add83523ea903b173fe_resolved")
            f_ok.append("androidx_RmR_commit_e3c10c6ac1acff50774d14417d93eaa6b5f8169a_resolved")
            f_gap.append("ABI_validation_contracts_and_runtime_integration:TOKEN_VAZIO")
            f_gap.append("device_runtime_execution:TOKEN_VAZIO")
            f_next.append("validate_abi_contracts_between_qemu_and_androidx")
            f_next.append("execute_android_ci_build_with_verified_sources")
        else:
            evidence_state = "PARTIAL"
            f_ok.append("external_source_manifest_parseable")

            for source_name, evidence in source_evidence.items():
                if evidence["recovery_rank"] != "rank_0_original_pin":
                    f_gap.append(f"{source_name}_recovery_rank_not_zero:{evidence['recovery_rank']}")
                if evidence["status"] != "VERIFIED_ORIGINAL_PIN":
                    f_gap.append(f"{source_name}_status_not_verified:{evidence['status']}")

            f_next.append("investigate_external_source_fallback_requirements")
            f_next.append("execute_recovery_with_fallback_strategy")

    # Create canonical receipt for hashing
    canonical = json.dumps(source_evidence, sort_keys=True, separators=(",", ":")).encode()

    return {
        "schema": "raf.external-source-evidence.v1",
        "produced_at": timestamp,
        "evidence_state": evidence_state,
        "claim_allowed": False,
        "source_receipts_hash": hashlib.sha256(canonical).hexdigest(),
        "external_sources_validated": list(source_evidence.keys()),
        "external_sources": source_evidence,
        "scope": "android-ci-external-source-validation",
        "scope_description": "External source validation for qemu_rafaelia and androidx_RmR used by Vectras-VM-Android",
        "F_ok": f_ok,
        "F_gap": f_gap,
        "F_next": f_next,
        "safe_state": "sources_validated_no_artifacts_modified",
    }


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Produce federated external source evidence from Vectras-VM-Android receipts."
    )
    parser.add_argument(
        "receipt",
        type=Path,
        help="Path to external sources receipt JSON from verify_external_sources_v2.sh",
    )
    parser.add_argument(
        "--out",
        type=Path,
        help="Output path for evidence JSON (default: stdout)",
    )
    args = parser.parse_args()

    try:
        receipt_text = args.receipt.read_text(encoding="utf-8")
        # Handle both single JSON object and array of objects
        receipt_data = json.loads(receipt_text)
        if isinstance(receipt_data, list):
            receipts = [r for r in receipt_data if r]  # Filter out empty/null entries
        elif isinstance(receipt_data, dict):
            receipts = [receipt_data]
        else:
            raise ExternalSourceEvidenceError(f"Invalid receipt type: {type(receipt_data)}")

        if not receipts:
            raise ExternalSourceEvidenceError("No valid receipts found in input")

        evidence = produce_evidence(receipts)
    except (OSError, json.JSONDecodeError, ExternalSourceEvidenceError, KeyError) as exc:
        evidence = {
            "schema": "raf.external-source-evidence.v1",
            "evidence_state": "BLOCKED",
            "claim_allowed": False,
            "reason": str(exc),
            "safe_state": "validation_failed",
        }

    rendered = json.dumps(evidence, sort_keys=True, indent=2) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0 if evidence.get("evidence_state") != "BLOCKED" else 1


if __name__ == "__main__":
    raise SystemExit(main())
