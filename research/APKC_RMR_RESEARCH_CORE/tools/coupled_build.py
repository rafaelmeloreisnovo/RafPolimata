#!/usr/bin/env python3
"""Verify, generate and compile the APKC–RMR coupled research unit.

SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
Coupling-ID: APKC-RMR-RESEARCH-CORE-V1-20260726
Contract-Role: COUPLED_BUILD_VERIFIER
License-Role: RESEARCH_NONCOMMERCIAL_ONLY
Normative comment: no compilation is authorized after a failed seal check.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

COUPLING_ID = "APKC-RMR-RESEARCH-CORE-V1-20260726"
ROOT = Path(__file__).resolve().parents[1]
LOCK = ROOT / "integrity" / "coupling.lock.json"
CONTRACT = ROOT / "contracts" / "coupling.v1.json"
GENERATED = ROOT / "generated" / "apkc_rmr_coupling_generated.h"


class CouplingError(RuntimeError):
    pass


def git_blob_sha1(path: Path) -> str:
    data = path.read_bytes()
    header = f"blob {len(data)}\0".encode("ascii")
    return hashlib.sha1(header + data).hexdigest()


def artifact_root(records: list[dict[str, str]]) -> str:
    value = hashlib.sha256()
    for record in sorted(records, key=lambda item: item["path"]):
        value.update(record["path"].encode("utf-8"))
        value.update(b"\0")
        value.update(record["git_blob_sha1"].encode("ascii"))
        value.update(b"\n")
    return value.hexdigest()


def load_json(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8"))


def verify(root: Path = ROOT) -> dict[str, object]:
    lock = load_json(root / "integrity" / "coupling.lock.json")
    contract = load_json(root / "contracts" / "coupling.v1.json")
    errors: list[str] = []
    observed: list[dict[str, str]] = []

    if lock.get("schema") != "raf.apkc-rmr-coupling-lock.v2":
        errors.append("invalid lock schema")
    if contract.get("schema") != "raf.apkc-rmr-coupling-contract.v1":
        errors.append("invalid contract schema")
    if lock.get("coupling_id") != COUPLING_ID:
        errors.append("lock coupling id mismatch")
    if contract.get("coupling_id") != COUPLING_ID:
        errors.append("contract coupling id mismatch")

    source_markers = [
        str(item) for item in contract.get("required_source_markers", [])
    ]
    for item in lock.get("artifacts", []):
        rel = str(item["path"])
        path = root / rel
        if not path.is_file():
            errors.append(f"missing artifact: {rel}")
            continue
        actual = git_blob_sha1(path)
        expected = str(item["git_blob_sha1"])
        if actual != expected:
            errors.append(f"identity mismatch: {rel}")
        text = path.read_text(encoding="utf-8", errors="replace")
        for marker in item.get("required_markers", []):
            if str(marker) not in text:
                errors.append(f"required marker absent in {rel}: {marker}")
        if item.get("source_contract", False):
            for marker in source_markers:
                if marker not in text:
                    errors.append(f"source marker absent in {rel}: {marker}")
        observed.append({"path": rel, "git_blob_sha1": actual})

    computed_root = artifact_root(observed)
    if computed_root != lock.get("artifact_root_sha256"):
        errors.append("artifact root mismatch")

    state = "PASS_COUPLED" if not errors else "FAIL"
    return {
        "schema": "raf.apkc-rmr-coupled-build-receipt.v1",
        "coupling_id": COUPLING_ID,
        "state": state,
        "claim_allowed": False,
        "commercial_authorized": False,
        "artifact_root_sha256": computed_root,
        "errors": errors,
        "observed": observed,
    }


def generate_header(receipt: dict[str, object]) -> None:
    if receipt["state"] != "PASS_COUPLED":
        raise CouplingError("cannot generate header from failed receipt")
    GENERATED.parent.mkdir(parents=True, exist_ok=True)
    GENERATED.write_text(
        "/*\n"
        " * Generated file. Do not edit.\n"
        f" * Coupling-ID: {COUPLING_ID}\n"
        f" * Artifact-Root-SHA256: {receipt['artifact_root_sha256']}\n"
        " */\n"
        "#ifndef APKC_RMR_COUPLING_GENERATED_H\n"
        "#define APKC_RMR_COUPLING_GENERATED_H\n"
        f"#define APKC_RMR_COUPLING_ID \"{COUPLING_ID}\"\n"
        f"#define APKC_RMR_ARTIFACT_ROOT_SHA256 \"{receipt['artifact_root_sha256']}\"\n"
        "#endif\n",
        encoding="utf-8",
    )


def run_checked(command: list[str], cwd: Path) -> None:
    result = subprocess.run(
        command,
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        raise CouplingError(
            f"command failed ({result.returncode}): {' '.join(command)}\n"
            f"{result.stdout}{result.stderr}"
        )


def compile_unit(cc: str, build_dir: Path) -> None:
    compiler = shutil.which(cc) if os.path.sep not in cc else cc
    if not compiler:
        raise CouplingError(f"compiler not found: {cc}")
    build_dir.mkdir(parents=True, exist_ok=True)
    flags = [
        "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
        "-ffreestanding", "-fno-builtin", "-fno-stack-protector",
        "-fvisibility=hidden", "-Icore", "-Istubs", "-Igenerated",
    ]
    sources = [
        ("core/apkc_rmr_research_core.c", build_dir / "core.o"),
        ("stubs/apkc_rmr_research_stubs.c", build_dir / "stubs.o"),
    ]
    for source, output in sources:
        run_checked([compiler, *flags, "-c", source, "-o", str(output)], ROOT)

    nm = shutil.which("nm")
    undefined: list[str] = []
    if nm:
        for _, output in sources:
            result = subprocess.run(
                [nm, "-u", str(output)], cwd=ROOT, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True,
            )
            undefined.extend(
                line for line in result.stdout.splitlines() if line.strip()
            )
    if undefined:
        raise CouplingError("undefined external symbols: " + "; ".join(undefined))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--verify-only", action="store_true")
    parser.add_argument("--generate", action="store_true")
    parser.add_argument("--compile", action="store_true")
    parser.add_argument("--cc", default=os.environ.get("CC", "cc"))
    parser.add_argument("--build-dir", default="build/coupled")
    args = parser.parse_args()

    receipt = verify()
    print(json.dumps(receipt, indent=2, sort_keys=True))
    if receipt["state"] != "PASS_COUPLED":
        return 1
    if args.generate or args.compile:
        generate_header(receipt)
    if args.compile:
        compile_unit(args.cc, ROOT / args.build_dir)
        receipt["compile_state"] = "PASS"
        print(json.dumps(receipt, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except CouplingError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
