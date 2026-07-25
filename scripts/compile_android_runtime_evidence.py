#!/usr/bin/env python3
"""Compile a bounded Android runtime receipt into an epistemic evidence record."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

ALLOWED_BINARIES = {
    "qemu-system-x86_64",
    "qemu-system-x86_64-rafaelia",
    "qemu-system-x86_64-rafacodephi",
    "qemu-system-aarch64",
    "qemu-system-aarch64-rafaelia",
    "qemu-system-i386",
}


class EvidenceError(ValueError):
    pass


def compile_evidence(receipt: dict[str, Any]) -> dict[str, Any]:
    errors: list[str] = []
    if receipt.get("schema") != "raf.android-runtime-receipt.v1":
        errors.append("schema")
    if receipt.get("protocol_version") != 2:
        errors.append("protocol_version")
    if receipt.get("binary_name") not in ALLOWED_BINARIES:
        errors.append("binary_name")
    if receipt.get("private_paths_exposed") is not False:
        errors.append("private_paths_exposed")
    if receipt.get("claim_allowed") is not False:
        errors.append("claim_allowed")
    if receipt.get("safe_state") != "vm-stopped-no-image-mutation":
        errors.append("safe_state")
    if errors:
        raise EvidenceError(",".join(errors))

    dispatch = receipt.get("dispatch_state")
    exit_code = receipt.get("execution_exit_code")
    boot_hash = receipt.get("guest_boot_artifact_sha256")

    if dispatch != "DISPATCHED":
        state = "BLOCKED"
        f_ok = []
        f_gap = ["dispatch_not_accepted"]
        f_next = ["repair_dispatch_contract"]
    elif exit_code is None:
        state = "PARTIAL"
        f_ok = ["android_service_dispatch_accepted"]
        f_gap = ["execution_exit_code:TOKEN_VAZIO", "guest_boot:TOKEN_VAZIO"]
        f_next = ["capture_termux_execution_receipt"]
    elif exit_code != 0:
        state = "CONTRADICTION"
        f_ok = ["execution_receipt_present"]
        f_gap = [f"execution_exit_code:{exit_code}"]
        f_next = ["preserve_logs_and_debug_without_vm_promotion"]
    elif not isinstance(boot_hash, str) or len(boot_hash) != 64:
        state = "TESTED"
        f_ok = ["termux_execution_exit_code_zero"]
        f_gap = ["guest_boot_artifact_sha256:TOKEN_VAZIO"]
        f_next = ["capture_minimal_guest_boot_artifact"]
    else:
        try:
            int(boot_hash, 16)
        except ValueError as exc:
            raise EvidenceError("guest_boot_artifact_sha256") from exc
        state = "VERIFIED_LIMITED"
        f_ok = ["termux_execution_exit_code_zero", "guest_boot_artifact_addressed"]
        f_gap = ["independent_device_replication:TOKEN_VAZIO"]
        f_next = ["repeat_on_second_abi_or_device"]

    canonical = json.dumps(receipt, sort_keys=True, separators=(",", ":")).encode()
    return {
        "schema": "raf.android-runtime-evidence.v1",
        "transaction_id": receipt["transaction_id"],
        "receipt_sha256": hashlib.sha256(canonical).hexdigest(),
        "evidence_state": state,
        "claim_allowed": False,
        "F_ok": f_ok,
        "F_gap": f_gap,
        "F_next": f_next,
        "safe_state": "vm-stopped-no-image-mutation",
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("receipt", type=Path)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()

    try:
        receipt = json.loads(args.receipt.read_text(encoding="utf-8"))
        evidence = compile_evidence(receipt)
    except (OSError, json.JSONDecodeError, EvidenceError, KeyError) as exc:
        evidence = {
            "schema": "raf.android-runtime-evidence.v1",
            "evidence_state": "BLOCKED",
            "claim_allowed": False,
            "reason": str(exc),
            "safe_state": "vm-stopped-no-image-mutation",
        }

    rendered = json.dumps(evidence, sort_keys=True, indent=2) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0 if evidence["evidence_state"] != "BLOCKED" else 1


if __name__ == "__main__":
    raise SystemExit(main())
