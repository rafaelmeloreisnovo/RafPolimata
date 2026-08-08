from __future__ import annotations

import hashlib
import importlib.util
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "scripts/validate_rafcodephi_bootstrap_evidence.py"
SPEC = importlib.util.spec_from_file_location("bootstrap_evidence", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def make_inputs(tmp_path: Path) -> tuple[Path, Path]:
    arm = "1" * 64
    aarch64 = "2" * 64
    manifest = tmp_path / "manifest.txt"
    manifest.write_text(
        "\n".join(
            [
                "schema=rafcodephi.real-bootstrap-sourcebuild/v1",
                "package_name=com.termux.rafacodephi",
                "prefix=/data/data/com.termux.rafacodephi/files/usr",
                "api_package=com.termux.rafacodephi.api",
                "api_receiver_component=com.termux.rafacodephi.api/com.termux.api.TermuxApiReceiver",
                "api_access_control=SIGNATURE_PERMISSION_NO_SHARED_UID",
                "bridge_allowed=false",
                "legacy_prefix_allowed=false",
                "termux_api_cli=EMBEDDED",
                "package_repo_runtime_state=BLOCKED_CUSTOM_REPOSITORY_NOT_PUBLISHED",
                "apt_update_guard=RAFCODEPHI_PACKAGE_REPOSITORY_NOT_PUBLISHED",
                f"sha256_arm={arm}",
                f"sha256_aarch64={aarch64}",
                "claim_allowed_device_runtime=false",
                "device_runtime_proof=TOKEN_VAZIO",
                "",
            ]
        ),
        encoding="utf-8",
    )
    manifest_sha = hashlib.sha256(manifest.read_bytes()).hexdigest()
    matrix = tmp_path / "matrix.json"
    matrix.write_text(
        json.dumps(
            {
                "schema": "rafcodephi.real-bootstrap-import-matrix/v1",
                "structural_state": "PASS",
                "package_name": "com.termux.rafacodephi",
                "api_package": "com.termux.rafacodephi.api",
                "api_receiver_component": "com.termux.rafacodephi.api/com.termux.api.TermuxApiReceiver",
                "api_access_control": "SIGNATURE_PERMISSION_NO_SHARED_UID",
                "package_repo_runtime_state": "BLOCKED_CUSTOM_REPOSITORY_NOT_PUBLISHED",
                "apt_update_guard": "RAFCODEPHI_PACKAGE_REPOSITORY_NOT_PUBLISHED",
                "architectures": {
                    "arm": {
                        "sha256": "3" * 64,
                        "bootstrap_sha256": arm,
                        "package_repo_runtime_state": "BLOCKED_CUSTOM_REPOSITORY_NOT_PUBLISHED",
                        "apt_update_guard": "RAFCODEPHI_PACKAGE_REPOSITORY_NOT_PUBLISHED",
                    },
                    "aarch64": {
                        "sha256": "4" * 64,
                        "bootstrap_sha256": aarch64,
                        "package_repo_runtime_state": "BLOCKED_CUSTOM_REPOSITORY_NOT_PUBLISHED",
                        "apt_update_guard": "RAFCODEPHI_PACKAGE_REPOSITORY_NOT_PUBLISHED",
                    },
                },
                "source_manifest_sha256": manifest_sha,
                "paired_architectures_complete": True,
                "claim_allowed_device_runtime": False,
                "device_runtime_proof": "TOKEN_VAZIO",
            },
            sort_keys=True,
        )
        + "\n",
        encoding="utf-8",
    )
    return manifest, matrix


def test_structural_pair_preserves_runtime_tokens_and_blockers(tmp_path: Path) -> None:
    manifest, matrix = make_inputs(tmp_path)
    report = MODULE.validate(manifest, matrix)
    assert report["structural_state"] == "PASS"
    assert report["overall_state"] == "STRUCTURAL_PASS_LIMITED"
    assert report["device_runtime_state"] == "TOKEN_VAZIO"
    assert report["api_runtime_state"] == "TOKEN_VAZIO"
    assert report["apt_update_guard"] == "RAFCODEPHI_PACKAGE_REPOSITORY_NOT_PUBLISHED"
    assert report["claim_allowed"] is False
    assert set(report["architectures"]) == {"arm", "aarch64"}
    assert "BLOCKED_CUSTOM_REPOSITORY_NOT_PUBLISHED" in report["blockers"]


def test_manifest_to_import_hash_drift_fails_closed(tmp_path: Path) -> None:
    manifest, matrix = make_inputs(tmp_path)
    doc = json.loads(matrix.read_text(encoding="utf-8"))
    doc["architectures"]["arm"]["bootstrap_sha256"] = "9" * 64
    matrix.write_text(json.dumps(doc), encoding="utf-8")
    try:
        MODULE.validate(manifest, matrix)
    except MODULE.EvidenceError as exc:
        assert str(exc) == "BOOTSTRAP_HASH_CHAIN_BROKEN:arm"
    else:
        raise AssertionError("broken bootstrap hash chain was accepted")


def test_repository_state_drift_fails_closed(tmp_path: Path) -> None:
    manifest, matrix = make_inputs(tmp_path)
    doc = json.loads(matrix.read_text(encoding="utf-8"))
    doc["package_repo_runtime_state"] = "PUBLISHED_VERIFIED"
    matrix.write_text(json.dumps(doc), encoding="utf-8")
    try:
        MODULE.validate(manifest, matrix)
    except MODULE.EvidenceError as exc:
        assert str(exc) == "IMPORT_MATRIX_PACKAGE_REPO_STATE"
    else:
        raise AssertionError("repository state drift was accepted")


def test_unbound_device_pass_is_not_promoted_to_claim(tmp_path: Path) -> None:
    manifest, matrix = make_inputs(tmp_path)
    device = tmp_path / "device.json"
    device.write_text(
        json.dumps(
            {
                "package_name": "com.termux.rafacodephi",
                "require_real_pkg": "true",
                "final_status": "DEVICE_REAL_PKG_VALIDATED",
            }
        ),
        encoding="utf-8",
    )
    report = MODULE.validate(manifest, matrix, device)
    assert report["overall_state"] == "DEVICE_RECEIPT_UNBOUND"
    assert "DEVICE_RECEIPT_ARTIFACT_BINDING_REQUIRED" in report["blockers"]
    assert report["claim_allowed"] is False
