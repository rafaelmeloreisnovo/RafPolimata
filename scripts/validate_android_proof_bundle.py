#!/usr/bin/env python3
"""Validate Android proof bundles under runtime-evidence closure CLOSURE_L2."""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

SHA256 = re.compile(r"^[0-9a-f]{64}$")
SHA40 = re.compile(r"^[0-9a-f]{40}$")
TV = "TOKEN_VAZIO"


def non_token(value: Any) -> bool:
    return isinstance(value, str) and bool(value.strip()) and not value.startswith(TV)


def pass_status(value: Any) -> bool:
    return isinstance(value, str) and value.upper() == "PASS"


def fail_status(value: Any) -> bool:
    return isinstance(value, str) and value.upper() == "FAIL"


def false_value(value: Any) -> bool:
    return value is False or (isinstance(value, str) and value.strip().lower() in {"false", "0", "no"})


def file_has(root: Path, name: Any, needles: tuple[str, ...] = ()) -> bool:
    if not non_token(name):
        return False
    if " or " in str(name):
        return False
    path = root / str(name)
    if not path.is_file():
        return False
    if needles:
        text = path.read_text(encoding="utf-8", errors="replace")
        return all(needle in text for needle in needles)
    return True


def validate(manifest_path: Path, out_dir: Path | None = None) -> dict[str, Any]:
    errors: list[str] = []
    warnings: list[str] = []
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except Exception as exc:
        return {"status": "FAIL", "errors": [f"manifest unreadable: {exc}"], "warnings": [], "gates": {}, "claim_allowed": False}
    if not isinstance(manifest, dict):
        return {"status": "FAIL", "errors": ["manifest must be object"], "warnings": [], "gates": {}, "claim_allowed": False}
    if manifest.get("schema") != "rafpolimata.android_proof_chain.v1":
        errors.append("schema must be rafpolimata.android_proof_chain.v1")

    root = out_dir or manifest_path.parent
    commit = manifest.get("git_commit")
    if non_token(commit) and not SHA40.fullmatch(str(commit)):
        errors.append("git_commit must be immutable 40-hex SHA or TOKEN_VAZIO")

    artifacts = manifest.get("artifacts")
    if not isinstance(artifacts, dict):
        artifacts = {}
        errors.append("artifacts must be object")

    apkc = artifacts.get("apkc_binary", {}) if isinstance(artifacts.get("apkc_binary"), dict) else {}
    apk = artifacts.get("apk", {}) if isinstance(artifacts.get("apk"), dict) else {}
    arm64 = artifacts.get("arm64_elf", {}) if isinstance(artifacts.get("arm64_elf"), dict) else {}
    install = artifacts.get("install", {}) if isinstance(artifacts.get("install"), dict) else {}
    launch = artifacts.get("launch", {}) if isinstance(artifacts.get("launch"), dict) else {}
    logcat = artifacts.get("logcat", {}) if isinstance(artifacts.get("logcat"), dict) else {}

    apkc_sha = apkc.get("sha256")
    apk_sha = apk.get("sha256")
    if non_token(apkc_sha) and not SHA256.fullmatch(str(apkc_sha)):
        errors.append("apkc_binary.sha256 invalid")
    if non_token(apk_sha) and not SHA256.fullmatch(str(apk_sha)):
        errors.append("apk.sha256 invalid")

    g1 = bool(SHA40.fullmatch(str(commit or ""))) and bool(SHA256.fullmatch(str(apkc_sha or ""))) and file_has(root, apkc.get("compile_log"))
    g2 = g1 and bool(SHA256.fullmatch(str(apk_sha or ""))) and file_has(root, apk.get("generate_log")) and file_has(root, apk.get("zip_list"), ("AndroidManifest.xml", "classes.dex"))
    g3 = g2 and pass_status(arm64.get("status")) and file_has(root, arm64.get("readelf_log"), ("Class:", "ELF64", "Machine:", "AArch64"))

    install_log = install.get("log")
    if isinstance(install_log, str) and " or " in install_log:
        install_log = None
    g4 = (
        g3
        and pass_status(install.get("status"))
        and file_has(root, install_log)
        and pass_status(launch.get("status"))
        and file_has(root, launch.get("log"))
        and pass_status(logcat.get("status"))
        and file_has(root, logcat.get("log"))
        and false_value(logcat.get("fatal_exception"))
        and false_value(logcat.get("dlopen_failure"))
    )
    g5 = g4 and non_token(manifest.get("date_utc"))

    gates = {
        "source_to_binary": "PASS" if g1 else TV,
        "binary_to_apk": "PASS" if g2 else TV,
        "arm64_real": "PASS" if g3 else TV,
        "android_runtime": "PASS" if g4 else TV,
        "single_run_reproducibility": "PASS" if g5 else TV,
    }

    negative: list[str] = []
    for label, node in (("arm64_elf", arm64), ("install", install), ("launch", launch), ("logcat", logcat)):
        if fail_status(node.get("status")):
            negative.append(label)
    if logcat and (logcat.get("fatal_exception") is True or logcat.get("dlopen_failure") is True):
        negative.append("logcat_failure_flag")
    if negative:
        errors.append("explicit negative runtime evidence: " + ",".join(sorted(set(negative))))

    claimed = manifest.get("promotion")
    if isinstance(claimed, dict):
        for gate, computed in gates.items():
            claimed_value = claimed.get(gate, TV)
            if pass_status(claimed_value) and computed != "PASS":
                errors.append(f"promotion.{gate}=PASS without sufficient same-run evidence")
            elif computed == "PASS" and not pass_status(claimed_value):
                warnings.append(f"promotion.{gate} remains conservative although evidence computes PASS")
    else:
        errors.append("promotion must be object")

    if errors:
        status = "FAIL"
    elif all(value == "PASS" for value in gates.values()):
        status = "PASS"
    elif g2:
        status = "PASS_LIMITED"
    else:
        status = TV

    return {
        "schema_version": "rafpolimata.android-proof-bundle-validation/v1",
        "status": status,
        "claim_allowed": status == "PASS",
        "gates": gates,
        "errors": errors,
        "warnings": warnings,
        "limitations": [
            "PASS_LIMITED never proves physical Android runtime.",
            "A manifest claim is not accepted unless same-run files satisfy the computed gate."
        ]
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", required=True)
    parser.add_argument("--out-dir")
    parser.add_argument("--write-report")
    args = parser.parse_args()
    report = validate(Path(args.manifest), Path(args.out_dir) if args.out_dir else None)
    text = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.write_report:
        output = Path(args.write_report)
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text, encoding="utf-8")
    print(text, end="")
    return 1 if report["status"] == "FAIL" else 0


if __name__ == "__main__":
    raise SystemExit(main())
