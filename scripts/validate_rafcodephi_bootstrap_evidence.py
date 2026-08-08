#!/usr/bin/env python3
"""Normalize RAFCODEPHI bootstrap receipts without promoting missing runtime proof."""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path

HEX64 = re.compile(r"^[0-9a-f]{64}$")
PACKAGE = "com.termux.rafacodephi"
API_PACKAGE = f"{PACKAGE}.api"
PREFIX = f"/data/data/{PACKAGE}/files/usr"
RECEIVER = f"{API_PACKAGE}/com.termux.api.TermuxApiReceiver"
APT_UPDATE_GUARD = "RAFCODEPHI_PACKAGE_REPOSITORY_NOT_PUBLISHED"
ARCHES = ("arm", "aarch64")


class EvidenceError(RuntimeError):
    pass


def require(condition: bool, token: str) -> None:
    if not condition:
        raise EvidenceError(token)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def parse_kv(path: Path) -> dict[str, str]:
    result: dict[str, str] = {}
    for number, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        require("=" in line, f"SOURCE_MANIFEST_LINE_INVALID:{number}")
        key, value = line.split("=", 1)
        require(bool(key) and key not in result, f"SOURCE_MANIFEST_KEY_INVALID:{key}")
        result[key] = value
    return result


def load_json(path: Path, token: str) -> dict[str, object]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise EvidenceError(f"{token}:{exc}") from exc
    require(isinstance(value, dict), f"{token}:ROOT_NOT_OBJECT")
    return value


def validate(
    source_manifest_path: Path,
    import_matrix_path: Path,
    device_receipt_path: Path | None = None,
) -> dict[str, object]:
    manifest = parse_kv(source_manifest_path)
    matrix = load_json(import_matrix_path, "IMPORT_MATRIX_INVALID")

    expected_manifest = {
        "schema": "rafcodephi.real-bootstrap-sourcebuild/v1",
        "package_name": PACKAGE,
        "prefix": PREFIX,
        "api_package": API_PACKAGE,
        "api_receiver_component": RECEIVER,
        "api_access_control": "SIGNATURE_PERMISSION_NO_SHARED_UID",
        "bridge_allowed": "false",
        "legacy_prefix_allowed": "false",
        "termux_api_cli": "EMBEDDED",
        "package_repo_runtime_state": "BLOCKED_CUSTOM_REPOSITORY_NOT_PUBLISHED",
        "apt_update_guard": APT_UPDATE_GUARD,
        "claim_allowed_device_runtime": "false",
        "device_runtime_proof": "TOKEN_VAZIO",
    }
    for key, expected in expected_manifest.items():
        require(manifest.get(key) == expected, f"SOURCE_MANIFEST_MISMATCH:{key}")

    require(matrix.get("schema") == "rafcodephi.real-bootstrap-import-matrix/v1", "IMPORT_MATRIX_SCHEMA")
    require(matrix.get("structural_state") == "PASS", "IMPORT_MATRIX_NOT_PASS")
    require(matrix.get("package_name") == PACKAGE, "IMPORT_MATRIX_PACKAGE")
    require(matrix.get("api_package") == API_PACKAGE, "IMPORT_MATRIX_API_PACKAGE")
    require(matrix.get("api_receiver_component") == RECEIVER, "IMPORT_MATRIX_API_RECEIVER")
    require(
        matrix.get("package_repo_runtime_state") == "BLOCKED_CUSTOM_REPOSITORY_NOT_PUBLISHED",
        "IMPORT_MATRIX_PACKAGE_REPO_STATE",
    )
    require(matrix.get("apt_update_guard") == APT_UPDATE_GUARD, "IMPORT_MATRIX_APT_UPDATE_GUARD")
    require(
        matrix.get("api_access_control") == "SIGNATURE_PERMISSION_NO_SHARED_UID",
        "IMPORT_MATRIX_API_ACCESS_CONTROL",
    )
    require(matrix.get("paired_architectures_complete") is True, "IMPORT_MATRIX_PAIR_INCOMPLETE")
    require(matrix.get("claim_allowed_device_runtime") is False, "IMPORT_MATRIX_CLAIM_PROMOTION")
    require(matrix.get("device_runtime_proof") == "TOKEN_VAZIO", "IMPORT_MATRIX_DEVICE_BOUNDARY")

    manifest_sha = digest(source_manifest_path)
    require(matrix.get("source_manifest_sha256") == manifest_sha, "SOURCE_MANIFEST_HASH_MISMATCH")
    matrix_arches = matrix.get("architectures")
    require(isinstance(matrix_arches, dict) and set(matrix_arches) == set(ARCHES), "IMPORT_MATRIX_ARCH_SET")

    package_repo_state = manifest["package_repo_runtime_state"]
    architecture_evidence: dict[str, object] = {}
    for arch in ARCHES:
        bootstrap_sha = manifest.get(f"sha256_{arch}", "")
        require(bool(HEX64.fullmatch(bootstrap_sha)), f"SOURCE_BOOTSTRAP_HASH_INVALID:{arch}")
        entry = matrix_arches[arch]
        require(isinstance(entry, dict), f"IMPORT_MATRIX_ARCH_INVALID:{arch}")
        require(entry.get("bootstrap_sha256") == bootstrap_sha, f"BOOTSTRAP_HASH_CHAIN_BROKEN:{arch}")
        receipt_sha = entry.get("sha256")
        require(isinstance(receipt_sha, str) and bool(HEX64.fullmatch(receipt_sha)), f"IMPORT_RECEIPT_HASH_INVALID:{arch}")
        require(
            entry.get("package_repo_runtime_state") == package_repo_state,
            f"PACKAGE_REPO_STATE_DRIFT:{arch}",
        )
        require(entry.get("apt_update_guard") == APT_UPDATE_GUARD, f"APT_UPDATE_GUARD_DRIFT:{arch}")
        architecture_evidence[arch] = {
            "bootstrap_sha256": bootstrap_sha,
            "import_receipt_sha256": receipt_sha,
            "termux_api_cli": "EMBEDDED",
        }

    blockers = []
    if package_repo_state != "PUBLISHED_VERIFIED":
        blockers.append(package_repo_state)

    device_state = "TOKEN_VAZIO"
    api_state = "TOKEN_VAZIO"
    overall_state = "STRUCTURAL_PASS_LIMITED"
    inputs: dict[str, str] = {
        "source_manifest_sha256": manifest_sha,
        "import_matrix_sha256": digest(import_matrix_path),
    }
    if device_receipt_path is None:
        blockers.extend(["DEVICE_RUNTIME_PROOF_REQUIRED", "PAIRED_API_CALL_PROOF_REQUIRED"])
    else:
        device = load_json(device_receipt_path, "DEVICE_RECEIPT_INVALID")
        inputs["device_receipt_sha256"] = digest(device_receipt_path)
        require(device.get("package_name") == PACKAGE, "DEVICE_RECEIPT_PACKAGE")
        device_state = str(device.get("final_status", "TOKEN_VAZIO"))
        if device_state == "DEVICE_REAL_PKG_VALIDATED":
            # Current device_pkg_smoke.json does not bind the observed device to
            # the exact bootstrap/APK/API hashes. Preserve that distinction.
            overall_state = "DEVICE_RECEIPT_UNBOUND"
            blockers.extend(
                [
                    "DEVICE_RECEIPT_ARTIFACT_BINDING_REQUIRED",
                    "PAIRED_API_CALL_PROOF_REQUIRED",
                ]
            )
        else:
            overall_state = "BLOCKED"
            blockers.append(f"DEVICE_STATUS:{device_state}")

    return {
        "schema": "rafcodephi.bootstrap-evidence/v1",
        "identity": {
            "main_app_package": PACKAGE,
            "api_package": API_PACKAGE,
            "prefix": PREFIX,
            "api_receiver_component": RECEIVER,
            "api_access_control": "SIGNATURE_PERMISSION_NO_SHARED_UID",
        },
        "inputs": inputs,
        "architectures": architecture_evidence,
        "structural_state": "PASS",
        "package_repo_runtime_state": package_repo_state,
        "apt_update_guard": APT_UPDATE_GUARD,
        "device_runtime_state": device_state,
        "api_runtime_state": api_state,
        "overall_state": overall_state,
        "blockers": sorted(set(blockers)),
        "claim_allowed": False,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-manifest", required=True, type=Path)
    parser.add_argument("--import-matrix", required=True, type=Path)
    parser.add_argument("--device-receipt", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    try:
        report = validate(args.source_manifest, args.import_matrix, args.device_receipt)
    except (OSError, EvidenceError) as exc:
        print(json.dumps({"ok": False, "state": "TOKEN_VAZIO", "error": str(exc)}, sort_keys=True))
        return 1
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(payload, encoding="utf-8")
    print(payload, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
