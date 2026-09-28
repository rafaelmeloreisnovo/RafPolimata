#!/usr/bin/env python3
import argparse
import hashlib
import json
import os
import pathlib
import shutil
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]

def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="build/maturity/reproducibility.json")
    args = ap.parse_args()
    out = ROOT / args.out
    out.parent.mkdir(parents=True, exist_ok=True)

    epoch = subprocess.check_output(["git", "show", "-s", "--format=%ct", "HEAD"], cwd=ROOT, text=True).strip()
    env = dict(os.environ, SOURCE_DATE_EPOCH=epoch)
    with tempfile.TemporaryDirectory() as td:
        a = pathlib.Path(td) / "a"
        b = pathlib.Path(td) / "b"
        subprocess.run(["bash", "sdk/rafpolimata_v1/build.sh", str(a)], cwd=ROOT, env=env, check=True)
        subprocess.run(["bash", "sdk/rafpolimata_v1/build.sh", str(b)], cwd=ROOT, env=env, check=True)
        pairs = {
            "object": (a / "rafpolimata_v1.o", b / "rafpolimata_v1.o"),
            "static_library": (a / "librafpolimata_v1.a", b / "librafpolimata_v1.a")
        }
        hashes = {}
        for name, (pa, pb) in pairs.items():
            ha, hb = sha256(pa), sha256(pb)
            hashes[name] = {"build_a": ha, "build_b": hb, "identical": ha == hb}
        ok = all(v["identical"] for v in hashes.values())

    receipt = {
        "schema": "rafpolimata.reproducible-build.v1",
        "source_sha": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
        "source_date_epoch": int(epoch),
        "same_environment_double_build": "PASS" if ok else "FAIL",
        "hashes": hashes,
        "independent_provider_reproduction": "TOKEN_VAZIO (CLOSURE_L12)"
    }
    out.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    if not ok:
        raise SystemExit("reproducible SDK build mismatch")
    print(f"REPRODUCIBLE_SDK_PASS out={out}")

if __name__ == "__main__":
    main()
