#!/usr/bin/env python3
"""Compile a Living Book domain cell into a non-executable bounded IR."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

ALLOWED_ACTIONS = {"INDEX_ONLY", "PROPOSE_ANALYSIS", "PROPOSE_TRANSLATION", "PROPOSE_TEST"}
FORBIDDEN_CAPABILITIES = {"network", "publish", "merge", "delete", "disclose_private", "execute_untrusted", "shell_eval"}


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")


def digests(value: Any) -> dict[str, str]:
    data = canonical_bytes(value)
    return {
        "sha256": hashlib.sha256(data).hexdigest(),
        "sha3_256": hashlib.sha3_256(data).hexdigest(),
        "blake2b_256": hashlib.blake2b(data, digest_size=32).hexdigest(),
    }


def validate_cell(cell: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    payload = cell.get("payload")
    if cell.get("schema") != "rafaelia.living-book.domain-cell/v1":
        errors.append("unsupported cell schema")
    if not isinstance(payload, dict):
        return errors + ["payload must be object"]
    if cell.get("integrity", {}).get("digests") != digests(payload):
        errors.append("cell digest mismatch")
    if payload.get("seed", {}).get("claim_allowed") is not False:
        errors.append("seed claim must remain blocked")
    if payload.get("seed", {}).get("source_disclosure") != "NO_RAW_PRIVATE_TEXT":
        errors.append("raw private source disclosure forbidden")
    if payload.get("privacy_security", {}).get("raw_private_text_committed") is not False:
        errors.append("raw private text forbidden")
    if payload.get("privacy_security", {}).get("secrets_allowed") is not False:
        errors.append("secrets forbidden")
    if payload.get("governance", {}).get("ai_can_approve") is not False:
        errors.append("AI approval forbidden")
    if payload.get("governance", {}).get("claim_allowed") is not False:
        errors.append("claim promotion forbidden")
    return errors


def compile_ir(cell: dict[str, Any], module_id: str, action: str, intent_id: str) -> dict[str, Any]:
    errors = validate_cell(cell)
    if action not in ALLOWED_ACTIONS:
        errors.append(f"action not allowed: {action}")
    modules = {m.get("id"): m for m in cell.get("payload", {}).get("modules", [])}
    module = modules.get(module_id)
    if module is None:
        errors.append(f"unknown module: {module_id}")
    elif module.get("required_user_knowledge") != []:
        errors.append("module requires technical user knowledge")
    if errors:
        raise ValueError("; ".join(errors))

    cell_digests = cell["integrity"]["digests"]
    capabilities = ["read_declared_metadata", "produce_proposal", "translate_to_domain"]
    if set(capabilities) & FORBIDDEN_CAPABILITIES:
        raise AssertionError("internal forbidden capability")
    ir_body = {
        "intent_id": intent_id,
        "cell_id": cell["cell_id"],
        "cell_digests": cell_digests,
        "module_id": module_id,
        "module_kind": module.get("kind"),
        "action": action,
        "capabilities": capabilities,
        "forbidden_capabilities": sorted(FORBIDDEN_CAPABILITIES),
        "output_contract": {
            "language": cell["payload"]["domain"]["primary_language"],
            "translate_to_domain": module.get("kind") != "DOMAIN",
            "raw_seed_text_in_output": False,
            "private_content_in_output": False,
        },
        "policy_gates": {
            "human_approval_required_for_execution": True,
            "approval_binding": "EXACT_CELL_SHA256",
            "execution_allowed": False,
            "publication_allowed": False,
            "claim_allowed": False,
        },
        "expected_receipt": cell["payload"]["workflow_proof"]["required_receipt_fields"],
        "state": "COMPILED_NON_EXECUTABLE_IR",
    }
    return {
        "schema": "rafpolimata.living-book-domain-ir/v1",
        "ir": ir_body,
        "integrity": {
            "canonicalization": "json-sort-keys-utf8-no-whitespace/v1",
            "digests": digests(ir_body),
        },
    }


def load(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as handle:
        value = json.load(handle)
    if not isinstance(value, dict):
        raise ValueError("top-level JSON must be object")
    return value


def atomic_write(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_suffix(path.suffix + ".tmp")
    tmp.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    tmp.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cell", required=True, type=Path)
    parser.add_argument("--module", required=True)
    parser.add_argument("--action", required=True)
    parser.add_argument("--intent-id", required=True)
    parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args()
    try:
        ir = compile_ir(load(args.cell), args.module, args.action, args.intent_id)
        atomic_write(args.out, ir)
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print(f"FAIL: {exc}")
        return 1
    print(f"PASS: wrote non-executable IR to {args.out}")
    print(json.dumps(ir["integrity"]["digests"], sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
