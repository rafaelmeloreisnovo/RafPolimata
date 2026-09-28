#!/usr/bin/env python3
import argparse
import hashlib
import json
import os
import pathlib
import shutil
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]

def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def tool_identity(name):
    path = shutil.which(name)
    if not path:
        return {"name": name, "state": "NOT_FOUND"}
    real = pathlib.Path(path).resolve()
    version = subprocess.run([path, "--version"], text=True, capture_output=True)
    first = (version.stdout or version.stderr).splitlines()[0] if (version.stdout or version.stderr) else ""
    return {"name": name, "path": str(real), "sha256": sha256(real), "version": first}

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--artifact", required=True)
    ap.add_argument("--out", default="build/maturity/supply-chain.json")
    args = ap.parse_args()
    artifact = ROOT / args.artifact
    if not artifact.is_file():
        raise SystemExit(f"artifact missing: {artifact}")

    cc = os.environ.get("CC") or ("clang" if shutil.which("clang") else "gcc")
    ar = os.environ.get("AR") or ("llvm-ar" if shutil.which("llvm-ar") else "ar")
    source_paths = [
        ROOT / "sdk/rafpolimata_v1/include/rafpolimata_v1.h",
        ROOT / "sdk/rafpolimata_v1/src/rafpolimata_v1.c",
        ROOT / "contracts/rafpolimata_api_abi_v1.json"
    ]
    receipt = {
        "schema": "rafpolimata.supply-chain-receipt.v1",
        "source_sha": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
        "source_tree": subprocess.check_output(["git", "rev-parse", "HEAD^{tree}"], cwd=ROOT, text=True).strip(),
        "artifact": {"path": args.artifact, "sha256": sha256(artifact), "bytes": artifact.stat().st_size},
        "source_files": [{"path": p.relative_to(ROOT).as_posix(), "sha256": sha256(p)} for p in source_paths],
        "toolchain": [tool_identity(cc), tool_identity(ar), tool_identity("python3")],
        "dependency_manifest": {
            "scope": "sdk/rafpolimata_v1",
            "runtime_external_dependencies": [],
            "build_dependencies": ["C compiler", "deterministic ar", "shell", "python3 for receipts only"]
        },
        "repository_wide_license_compatibility": "REVIEW_REQUIRED",
        "claim_scope": "bounded_sdk_v1_supply_chain_not_whole_repository"
    }
    out = ROOT / args.out
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print(f"SUPPLY_CHAIN_PASS out={out} artifact_sha256={receipt['artifact']['sha256']}")

if __name__ == "__main__":
    main()
