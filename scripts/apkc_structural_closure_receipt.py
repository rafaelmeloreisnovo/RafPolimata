#!/usr/bin/env python3
"""Create a fail-closed C04 receipt from ApkC build and format evidence."""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
from typing import Any

SCHEMA = "raf.apkc-structural-closure.v1"
SOURCE_PROOF_SCHEMA = "raf.apkc.source-to-binary-proof.v3"
FIRST_PART_SCHEMA = "raf.apkc-first-part-gate.v1"
PREFLIGHT_SCHEMA = "raf.apkc-runtime-preflight.v1"
FORMAT_MODULE_PATH = Path(__file__).with_name("validate_apkc_formats.py")


def load_format_module():
    name = "validate_apkc_formats_c04"
    spec = importlib.util.spec_from_file_location(name, FORMAT_MODULE_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot import {FORMAT_MODULE_PATH}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--apk", required=True, type=Path)
    parser.add_argument(
        "--source-proof",
        type=Path,
        default=Path("Apkc/proofs/out/apkc-compile.status.json"),
    )
    parser.add_argument(
        "--first-part-gate",
        type=Path,
        default=Path("results/apkc-first-part-gate.json"),
    )
    parser.add_argument(
        "--runtime-preflight",
        type=Path,
        default=Path("results/apkc-runtime-preflight.json"),
    )
    parser.add_argument("--source-commit", required=True)
    parser.add_argument("--output", required=True, type=Path)
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    if not path.is_file():
        return "TOKEN_VAZIO_ARTIFACT_MISSING"
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def artifact(path: Path) -> dict[str, Any]:
    return {
        "path": str(path),
        "exists": path.is_file(),
        "size_bytes": path.stat().st_size if path.is_file() else None,
        "sha256": sha256_file(path),
        "blake3": "TOKEN_VAZIO_BLAKE3_NOT_CALCULATED",
    }


def load_json(path: Path) -> tuple[dict[str, Any] | None, str | None]:
    if not path.is_file():
        return None, f"missing JSON artifact: {path}"
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        return None, f"invalid JSON {path}: {error}"
    if not isinstance(value, dict):
        return None, f"JSON root is not object: {path}"
    return value, None


def source_proof_state(proof: dict[str, Any] | None, source_commit: str) -> tuple[str, list[str]]:
    errors: list[str] = []
    if proof is None:
        return "TOKEN_VAZIO", errors
    if proof.get("schema") != SOURCE_PROOF_SCHEMA:
        errors.append(f"unexpected source proof schema: {proof.get('schema')}")
    if proof.get("commit") != source_commit:
        errors.append(
            f"source proof commit mismatch: expected {source_commit}, got {proof.get('commit')}"
        )
    for abi in ("aarch64", "arm32"):
        section = proof.get(abi)
        if not isinstance(section, dict):
            errors.append(f"missing source proof section: {abi}")
            continue
        for gate in ("build", "identity", "reproducibility"):
            if section.get(gate) != "PASS":
                errors.append(f"{abi}.{gate} is not PASS: {section.get(gate)}")
        sha = str(section.get("sha256", ""))
        if len(sha) != 64 or any(char not in "0123456789abcdefABCDEF" for char in sha):
            errors.append(f"{abi}.sha256 invalid")
    if proof.get("state") != "PASS":
        errors.append(f"source proof state is not PASS: {proof.get('state')}")
    if proof.get("claim_allowed") is not False:
        errors.append("source proof claim_allowed must remain false")
    return ("PASS" if not errors else "FAIL"), errors


def first_part_state(document: dict[str, Any] | None) -> tuple[str, list[str]]:
    if document is None:
        return "TOKEN_VAZIO", []
    errors: list[str] = []
    if document.get("schema") != FIRST_PART_SCHEMA:
        errors.append(f"unexpected first-part schema: {document.get('schema')}")
    if document.get("state") != "PASS":
        errors.append(f"first-part gate is not PASS: {document.get('state')}")
    if document.get("claim_allowed") is not False:
        errors.append("first-part claim_allowed must remain false")
    contradictions = document.get("contradictions")
    if contradictions not in ([], None):
        errors.append("first-part gate contains contradictions")
    return ("PASS" if not errors else "FAIL"), errors


def preflight_state(document: dict[str, Any] | None) -> dict[str, Any]:
    if document is None:
        return {
            "state": "TOKEN_VAZIO",
            "schema_valid": False,
            "runtime_blocked": "TOKEN_VAZIO",
            "blocker_count": "TOKEN_VAZIO",
            "critical_blockers": "TOKEN_VAZIO",
        }
    schema_valid = document.get("schema") == PREFLIGHT_SCHEMA
    summary = document.get("summary") if isinstance(document.get("summary"), dict) else {}
    state = document.get("state", "TOKEN_VAZIO")
    return {
        "state": state,
        "schema_valid": schema_valid,
        "runtime_blocked": state == "BLOCKED",
        "blocker_count": summary.get("source_blocker_count", "TOKEN_VAZIO"),
        "critical_blockers": summary.get("critical_blockers", "TOKEN_VAZIO"),
        "claim_allowed": document.get("claim_allowed", "TOKEN_VAZIO"),
    }


def main() -> int:
    args = parse_args()
    format_module = load_format_module()

    source_proof, source_error = load_json(args.source_proof)
    first_part, first_error = load_json(args.first_part_gate)
    preflight, preflight_error = load_json(args.runtime_preflight)

    errors: list[str] = []
    missing: list[str] = []
    for path, error in (
        (args.source_proof, source_error),
        (args.first_part_gate, first_error),
        (args.runtime_preflight, preflight_error),
    ):
        if error:
            missing.append(str(path))

    source_state, source_errors = source_proof_state(source_proof, args.source_commit)
    first_state, first_errors = first_part_state(first_part)
    preflight_summary = preflight_state(preflight)
    errors.extend(source_errors)
    errors.extend(first_errors)

    apk_report = format_module.validate_apk(args.apk, require_both=True)
    apk_state = apk_report.get("state", "TOKEN_VAZIO")
    if apk_state == "FAIL":
        errors.extend(f"APK: {item}" for item in apk_report.get("errors", []))
    if apk_state == "TOKEN_VAZIO":
        missing.append(str(args.apk))

    dex_pass = bool(apk_report.get("dex")) and all(
        item.get("state") == "PASS" for item in apk_report.get("dex", [])
    )
    elf_pass = bool(apk_report.get("elf")) and all(
        item.get("state") == "PASS" for item in apk_report.get("elf", [])
    )
    both_abis = set(apk_report.get("abis", [])) >= {"arm64-v8a", "armeabi-v7a"}

    hard_failure = (
        bool(errors)
        or apk_state == "FAIL"
        or source_state == "FAIL"
        or first_state == "FAIL"
    )
    if hard_failure:
        state = "FAIL"
    elif missing or source_state != "PASS" or first_state != "PASS" or apk_state != "PASS":
        state = "INCOMPLETE"
    elif not (dex_pass and elf_pass and both_abis):
        state = "FAIL"
        errors.append("structural sub-gates are not all PASS")
    else:
        state = "PASS_STRUCTURAL"

    report = {
        "schema": SCHEMA,
        "cycle_id": "C04",
        "state": state,
        "claim_allowed": False,
        "structural_claim_allowed": state == "PASS_STRUCTURAL",
        "claim_scope": "APK_ZIP_DEX_ELF_AND_DUAL_ABI_STRUCTURE_ONLY",
        "source_commit": args.source_commit,
        "source_to_binary": {
            "state": source_state,
            "expected_schema": SOURCE_PROOF_SCHEMA,
            "document": source_proof if source_proof is not None else "TOKEN_VAZIO",
        },
        "first_part_gate": {
            "state": first_state,
            "document": first_part if first_part is not None else "TOKEN_VAZIO",
        },
        "runtime_preflight": preflight_summary,
        "apk_validation": apk_report,
        "checks": {
            "source_proof_pass": source_state == "PASS",
            "first_part_gate_pass": first_state == "PASS",
            "apk_zip_pass": apk_state == "PASS",
            "dex_pass": dex_pass,
            "elf_pass": elf_pass,
            "both_abis": both_abis,
            "source_commit_matches": bool(source_proof) and source_proof.get("commit") == args.source_commit,
            "runtime_preflight_schema_valid": preflight_summary["schema_valid"],
            "runtime_blockers_do_not_expand_structural_claim": True,
        },
        "artifacts": [
            artifact(args.apk),
            artifact(args.source_proof),
            artifact(args.first_part_gate),
            artifact(args.runtime_preflight),
        ],
        "missing_required_artifacts": sorted(set(missing)),
        "errors": sorted(set(errors)),
        "runtime_boundary": {
            "source_contract_safe": (
                False if preflight_summary["runtime_blocked"] is True else "TOKEN_VAZIO"
            ),
            "apk_signed": "TOKEN_VAZIO",
            "apk_installed": "TOKEN_VAZIO",
            "package_launched": "TOKEN_VAZIO",
            "nativeactivity_logcat": "TOKEN_VAZIO",
            "device_arm32": "TOKEN_VAZIO",
            "device_arm64": "TOKEN_VAZIO",
            "performance_claim": "FORBIDDEN_OUT_OF_SCOPE",
        },
        "falsifiers": [
            "source_proof_schema_not_current_v3",
            "source_proof_commit_mismatch",
            "source_build_or_reproducibility_not_pass",
            "first_part_gate_not_pass_or_contains_contradiction",
            "apk_missing_or_invalid_zip",
            "dex_internal_sha1_or_adler32_invalid",
            "elf_wrong_machine_or_invalid_layout",
            "required_abi_missing",
            "structural_pass_promoted_to_install_or_runtime",
        ],
    }

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"{state} ApkC structural closure: {args.output}")
    return 0 if state == "PASS_STRUCTURAL" else 1


if __name__ == "__main__":
    raise SystemExit(main())
