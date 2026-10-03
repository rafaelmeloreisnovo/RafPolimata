#!/usr/bin/env python3
"""Verify RafCI bounded same-artifact physical execution receipts.

CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE

A PASS here is bounded to externally supplied source/artifact pins, physical
Termux ABI observation, execution exit status and verified receipt files. The
receipt cannot appoint its own expected source or artifact identity. This does
not imply production readiness, universal compatibility or independent
replication.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
import re
import tempfile
from typing import Any

HEX64 = re.compile(r"^[0-9a-f]{64}$")
HEX40 = re.compile(r"^[0-9a-f]{40}$")


class PhysicalReceiptError(RuntimeError):
    pass


def require(value: bool, message: str) -> None:
    if not value:
        raise PhysicalReceiptError(message)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def abi_matches(scope: str, abi: str, uname_m: str) -> bool:
    if scope == "arm32":
        return abi in {"armeabi-v7a", "armeabi"} or uname_m.startswith("armv7") or uname_m == "armv8l"
    if scope == "arm64":
        return abi == "arm64-v8a" or uname_m == "aarch64"
    return False


def verify_manifest(root: Path) -> str:
    manifest = root / "receipt.sha256"
    require(manifest.is_file(), "receipt.sha256 missing")
    lines = manifest.read_text(encoding="utf-8").splitlines()
    require(lines, "receipt.sha256 empty")
    seen: set[str] = set()
    for line in lines:
        parts = line.split(None, 1)
        require(len(parts) == 2 and bool(HEX64.fullmatch(parts[0].lower())), "invalid manifest line")
        expected = parts[0].lower()
        rel = parts[1].strip()
        if rel.startswith("*"):
            rel = rel[1:]
        if rel.startswith("./"):
            rel = rel[2:]
        require(rel and not rel.startswith("/") and ".." not in Path(rel).parts, "unsafe manifest path")
        path = root / rel
        require(path.is_file(), f"manifest member missing: {rel}")
        require(digest(path) == expected, f"manifest hash mismatch: {rel}")
        seen.add(rel)
    require("receipt.json" in seen, "receipt.json not covered by manifest")
    require("target.stdout.bin" in seen, "stdout evidence not covered by manifest")
    require("target.stderr.bin" in seen, "stderr evidence not covered by manifest")
    return digest(manifest)


def verify(
    root: Path,
    expected_source_sha: str,
    expected_artifact_sha256: str,
) -> dict[str, Any]:
    root = root.resolve()
    expected_source_sha = expected_source_sha.lower()
    expected_artifact_sha256 = expected_artifact_sha256.lower()
    require(bool(HEX40.fullmatch(expected_source_sha)), "external expected source SHA must be 40 hex")
    require(bool(HEX64.fullmatch(expected_artifact_sha256)), "external expected artifact SHA-256 must be 64 hex")

    receipt_path = root / "receipt.json"
    require(receipt_path.is_file(), "receipt.json missing")
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    require(isinstance(receipt, dict), "receipt must be an object")
    require(receipt.get("schema") == "rafaelia.rafci.physical-execution-receipt/v1", "schema drift")
    require(receipt.get("closure") == "CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE", "closure drift")
    require(receipt.get("claim_allowed") is False, "claim_allowed must remain false")
    require(receipt.get("capture_state") == "EVIDENCE_READY_BOUNDED", "capture state is not promotable")

    source_commit = str(receipt.get("source_commit", "")).lower()
    require(bool(HEX40.fullmatch(source_commit)), "source commit is not exact 40-hex")
    require(source_commit == expected_source_sha, "source commit does not match external expected head")

    env = receipt.get("environment")
    require(isinstance(env, dict), "environment missing")
    require(env.get("physical_termux_observed") is True, "physical Termux not observed")
    require(env.get("abi_scope_match") is True, "producer ABI match false")
    require(env.get("raw_device_serial_stored") is False, "raw device serial exposure")
    require(env.get("raw_termux_prefix_stored") is False, "raw Termux prefix exposure")

    scope = str(receipt.get("target_scope", ""))
    require(scope in {"arm32", "arm64"}, "unsupported target scope")
    abi = str(env.get("android_abi", ""))
    uname_m = str(env.get("uname_machine", ""))
    require(abi_matches(scope, abi, uname_m), "observed ABI does not match target scope")

    artifact = receipt.get("artifact")
    require(isinstance(artifact, dict), "artifact block missing")
    require(artifact.get("raw_path_stored") is False, "raw artifact path exposure")
    receipt_expected = str(artifact.get("expected_sha256", "")).lower()
    before = str(artifact.get("sha256_before", "")).lower()
    after = str(artifact.get("sha256_after", "")).lower()
    require(bool(HEX64.fullmatch(receipt_expected)), "receipt expected artifact SHA-256 invalid")
    require(
        expected_artifact_sha256 == receipt_expected == before == after,
        "external artifact pin / measured artifact identity contradiction",
    )

    execution = receipt.get("execution")
    require(isinstance(execution, dict), "execution block missing")
    require(execution.get("authorized_by_explicit_execute_flag") is True, "explicit execute authorization missing")
    require(execution.get("executed") is True, "target was not executed")
    require(execution.get("exit_code") == 0, "target exit code is not zero")
    require(execution.get("arguments") == [], "v1 only accepts zero-argument execution")
    require(bool(HEX64.fullmatch(str(execution.get("stdout_sha256", "")).lower())), "stdout hash invalid")
    require(bool(HEX64.fullmatch(str(execution.get("stderr_sha256", "")).lower())), "stderr hash invalid")
    require(digest(root / "target.stdout.bin") == execution["stdout_sha256"], "stdout evidence drift")
    require(digest(root / "target.stderr.bin") == execution["stderr_sha256"], "stderr evidence drift")

    forbidden = receipt.get("forbidden_automatic_actions")
    require(isinstance(forbidden, dict), "forbidden action ledger missing")
    require(all(value is False for value in forbidden.values()), "receipt reports forbidden automatic action")

    manifest_sha = verify_manifest(root)
    receipt_sha = digest(receipt_path)
    return {
        "schema": "rafaelia.rafci.physical-execution-evidence/v1",
        "state": "PASS_PHYSICAL_EXECUTION_BOUNDED",
        "gate": "gate.physical-execution",
        "scope": scope,
        "source_commit": source_commit,
        "artifact_sha256": expected_artifact_sha256,
        "external_source_pin_verified": True,
        "external_artifact_pin_verified": True,
        "receipt_sha256": receipt_sha,
        "manifest_sha256": manifest_sha,
        "independent_replication": "TOKEN_VAZIO",
        "claim_allowed": False,
        "closure": "CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE",
    }


def write_fixture(root: Path, *, scope: str = "arm32") -> None:
    root.mkdir(parents=True, exist_ok=True)
    (root / "target.stdout.bin").write_bytes(b"")
    (root / "target.stderr.bin").write_bytes(b"")
    empty_sha = hashlib.sha256(b"").hexdigest()
    artifact_sha = "a" * 64
    abi = "armeabi-v7a" if scope == "arm32" else "arm64-v8a"
    uname_m = "armv7l" if scope == "arm32" else "aarch64"
    receipt = {
        "schema": "rafaelia.rafci.physical-execution-receipt/v1",
        "repository": "rafaelmeloreisnovo/RafPolimata",
        "source_commit": "b" * 40,
        "closure": "CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE",
        "capture_state": "EVIDENCE_READY_BOUNDED",
        "target_scope": scope,
        "environment": {
            "physical_termux_observed": True,
            "abi_scope_match": True,
            "android_abi": abi,
            "uname_machine": uname_m,
            "android_release": "selftest",
            "android_sdk": "selftest",
            "raw_device_serial_stored": False,
            "raw_termux_prefix_stored": False,
        },
        "artifact": {
            "name": "selftest.bin",
            "raw_path_stored": False,
            "expected_sha256": artifact_sha,
            "sha256_before": artifact_sha,
            "sha256_after": artifact_sha,
        },
        "execution": {
            "authorized_by_explicit_execute_flag": True,
            "executed": True,
            "exit_code": 0,
            "started_utc": "2026-10-03T00:00:00Z",
            "ended_utc": "2026-10-03T00:00:01Z",
            "stdout_sha256": empty_sha,
            "stderr_sha256": empty_sha,
            "arguments": [],
        },
        "forbidden_automatic_actions": {
            "package_install": False,
            "process_attach": False,
            "hook": False,
            "runtime_patch": False,
            "privilege_escalation": False,
            "apk_build": False,
            "vm_boot": False,
        },
        "claim_allowed": False,
        "F_ok": ["selftest"],
        "F_gap": [],
        "F_next": ["selftest"],
    }
    (root / "receipt.json").write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    members = ["receipt.json", "target.stderr.bin", "target.stdout.bin"]
    (root / "receipt.sha256").write_text(
        "".join(f"{digest(root / name)}  ./{name}\n" for name in members),
        encoding="utf-8",
    )


def rewrite_manifest(root: Path) -> None:
    members = ["receipt.json", "target.stderr.bin", "target.stdout.bin"]
    (root / "receipt.sha256").write_text(
        "".join(f"{digest(root / member)}  ./{member}\n" for member in members),
        encoding="utf-8",
    )


def selftest() -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="rafci-physical-v1-") as temp:
        root = Path(temp) / "good"
        write_fixture(root)
        good = verify(root, "b" * 40, "a" * 64)

        cases: list[tuple[str, Any]] = [
            ("artifact-mutation", lambda x: x["artifact"].__setitem__("sha256_after", "c" * 64)),
            ("abi-mismatch", lambda x: x["environment"].__setitem__("android_abi", "arm64-v8a")),
            ("nonzero-exit", lambda x: x["execution"].__setitem__("exit_code", 7)),
            ("not-physical", lambda x: x["environment"].__setitem__("physical_termux_observed", False)),
            ("claim-promotion", lambda x: x.__setitem__("claim_allowed", True)),
        ]
        rejected: list[str] = []
        original = json.loads((root / "receipt.json").read_text(encoding="utf-8"))
        for name, mutate in cases:
            case_root = Path(temp) / name
            write_fixture(case_root)
            receipt = copy.deepcopy(original)
            mutate(receipt)
            (case_root / "receipt.json").write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n", encoding="utf-8")
            rewrite_manifest(case_root)
            try:
                verify(case_root, "b" * 40, "a" * 64)
            except PhysicalReceiptError:
                rejected.append(name)
            else:
                raise PhysicalReceiptError(f"selftest falsifier accepted: {name}")

        try:
            verify(root, "c" * 40, "a" * 64)
        except PhysicalReceiptError:
            rejected.append("external-source-pin-mismatch")
        else:
            raise PhysicalReceiptError("selftest accepted external source pin mismatch")

        try:
            verify(root, "b" * 40, "c" * 64)
        except PhysicalReceiptError:
            rejected.append("external-artifact-pin-mismatch")
        else:
            raise PhysicalReceiptError("selftest accepted external artifact pin mismatch")

        require(len(rejected) == len(cases) + 2, "not all falsifiers rejected")
        return {
            "schema": "rafaelia.rafci.physical-execution-selftest/v1",
            "state": "PASS_CONTRACT_ONLY",
            "synthetic_good_state": good["state"],
            "rejected_falsifiers": rejected,
            "physical_execution_observed": "TOKEN_VAZIO_SELFTEST_IS_SYNTHETIC",
            "claim_allowed": False,
        }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--receipt-dir", type=Path)
    parser.add_argument("--expected-source-sha")
    parser.add_argument("--expected-artifact-sha256")
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()

    if args.selftest:
        result = selftest()
    else:
        require(args.receipt_dir is not None, "--receipt-dir required unless --selftest")
        require(args.expected_source_sha is not None, "--expected-source-sha required for physical receipt verification")
        require(args.expected_artifact_sha256 is not None, "--expected-artifact-sha256 required for physical receipt verification")
        result = verify(args.receipt_dir, args.expected_source_sha, args.expected_artifact_sha256)

    rendered = json.dumps(result, indent=2, sort_keys=True) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, json.JSONDecodeError, KeyError, PhysicalReceiptError) as exc:
        print(f"RAFCI_PHYSICAL_RECEIPT_FAIL: {exc}")
        raise SystemExit(1)
