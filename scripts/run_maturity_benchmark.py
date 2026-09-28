#!/usr/bin/env python3
import argparse
import json
import os
import pathlib
import platform
import resource
import shutil
import statistics
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]

def run(cmd, **kw):
    return subprocess.run(cmd, check=True, text=True, **kw)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="build/maturity/benchmark.json")
    args = ap.parse_args()

    out = ROOT / args.out
    out.parent.mkdir(parents=True, exist_ok=True)
    cc = os.environ.get("CC") or shutil.which("clang") or shutil.which("gcc") or shutil.which("cc")
    if not cc:
        raise SystemExit("compiler missing")

    binary = out.parent / "benchmark_crc32c_host"
    cmd = [
        cc, "-std=c11", "-O3", "-Wall", "-Wextra", "-Werror", "-Wno-unused-function",
        "-I", str(ROOT / "Benchmark"),
        str(ROOT / "tests/maturity/benchmark_crc32c.c"),
        "-o", str(binary),
    ]
    machine = platform.machine().lower()
    if machine in {"x86_64", "amd64"}:
        cmd.insert(2, "-msse4.2")
    elif machine in {"aarch64", "arm64"}:
        cmd.insert(2, "-march=armv8-a+crc")

    run(cmd)
    proc = run([str(binary)], capture_output=True)
    usage = resource.getrusage(resource.RUSAGE_CHILDREN)

    samples = []
    for line in proc.stdout.splitlines():
        fields = {}
        for token in line.split():
            if "=" in token:
                k, v = token.split("=", 1)
                fields[k] = int(v)
        if "ns" in fields and "bytes" in fields:
            samples.append(fields)
    if len(samples) != 31:
        raise SystemExit(f"expected 31 benchmark samples, got {len(samples)}")

    ns = sorted(s["ns"] for s in samples)
    cycles = sorted(s["cycles"] for s in samples)
    bytes_per_sample = samples[0]["bytes"]
    median_ns = ns[len(ns)//2]
    median_cycles = cycles[len(cycles)//2]
    source_sha = run(["git", "rev-parse", "HEAD"], cwd=ROOT, capture_output=True).stdout.strip()
    compiler_version = run([cc, "--version"], capture_output=True).stdout.splitlines()[0]

    receipt = {
        "schema": "rafpolimata.benchmark-receipt.v1",
        "source_sha": source_sha,
        "benchmark": "crc32c_specialized_host",
        "host": {
            "system": platform.system(),
            "release": platform.release(),
            "machine": platform.machine(),
            "compiler": compiler_version
        },
        "workload": {
            "bytes_per_sample": bytes_per_sample,
            "samples": len(samples),
            "alignment_rotation": "offset_0_to_7"
        },
        "timing": {
            "unit": "ns",
            "min_ns": min(ns),
            "median_ns": median_ns,
            "p95_ns": ns[29],
            "max_ns": max(ns),
            "variance_ns2": statistics.pvariance(ns),
            "bytes_per_second_at_median": bytes_per_sample * 1_000_000_000.0 / median_ns
        },
        "cycles": {
            "counter": "rdtsc_lfence" if machine in {"x86_64", "amd64"} else "unavailable",
            "median_cycles": median_cycles,
            "cycles_per_byte_at_median": (median_cycles / bytes_per_sample) if median_cycles else None
        },
        "memory": {
            "static_input_buffer_bytes": 1024 * 1024 + 8,
            "binary_size_bytes": binary.stat().st_size,
            "maxrss_kib_hosted_runner": usage.ru_maxrss
        },
        "energy_joules": {
            "state": "TOKEN_VAZIO (CLOSURE_L12)",
            "reason": "runner exposes no calibrated per-process energy counter"
        },
        "raw_samples": samples
    }
    out.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print(f"MATURITY_BENCHMARK_PASS out={out} sha={source_sha} median_ns={median_ns}")

if __name__ == "__main__":
    main()
