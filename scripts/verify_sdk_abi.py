#!/usr/bin/env python3
import argparse
import json
import pathlib
import shutil
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--library", default="build/sdk/rafpolimata_v1/librafpolimata_v1.a")
    args = ap.parse_args()
    contract = json.loads((ROOT / "contracts/rafpolimata_api_abi_v1.json").read_text())
    expected = sorted(contract["public_symbols"])
    nm = shutil.which("llvm-nm") or shutil.which("nm")
    if not nm:
        raise SystemExit("nm missing")
    proc = subprocess.run([nm, "-g", "--defined-only", str(ROOT / args.library)], text=True, capture_output=True, check=True)
    actual = sorted({line.split()[-1] for line in proc.stdout.splitlines() if line.split() and line.split()[-1].startswith("rafp_v1_")})
    if actual != expected:
        raise SystemExit(f"ABI symbol mismatch expected={expected} actual={actual}")
    print(f"SDK_ABI_PASS version={contract['abi_version']} symbols={len(actual)}")

if __name__ == "__main__":
    main()
