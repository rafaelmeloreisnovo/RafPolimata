#!/usr/bin/env python3
"""Produce explicit test accounting for RafPolimata's local compiler gate.

The project-owned shell test remains the authority for the nine test blocks.
This adapter invokes that exact tracked script by argv (never ``shell=True``),
forwards its streams, and writes a bounded JSON accounting artifact for the
Foundation computational gate.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path


STEPS = (
    ("strict-host-compiler-build", "strict host compiler build"),
    ("segment-v1-contract", "segment v1 header, fixed records and bounded-reader tests"),
    ("raw-native-output-contract", "raw native output contract"),
    ("optional-output-base", "optional output-base parsing"),
    ("source-boundary-negative", "unknown extension and exact source bound"),
    ("source-honesty-invariants", "source-level honesty invariants"),
    ("ecosystem-evidence-state", "ecosystem evidence-state validation"),
    ("toroidal-router-contract", "toroidal research router validation"),
    ("build-doctor-boundary", "ecosystem build doctor unit tests and self-audit"),
)
FALSIFIERS = (
    ("unknown-extension", "an unknown source extension must be rejected"),
    ("oversized-source", "a source one byte above RAF_SOURCE_MAX must be rejected"),
    ("transaction-rollback", "negative source paths must emit the expected rollback state"),
    ("build-doctor-boundary", "build-doctor must preserve TOKEN_VAZIO runtime boundaries"),
)
STEP_PATTERN = re.compile(r"^\[(\d+)/9\]\s", re.MULTILINE)


def summary_for(exit_code: int, combined_output: str) -> dict[str, object]:
    seen = [int(value) for value in STEP_PATTERN.findall(combined_output)]
    last_started = max(seen, default=0)
    succeeded = (
        exit_code == 0
        and seen == list(range(1, len(STEPS) + 1))
        and "PASS rafpolimata-runtime-truth-local" in combined_output
    )
    tests: list[dict[str, str]] = []
    for index, (identifier, _) in enumerate(STEPS, start=1):
        if succeeded:
            result = "PASS"
        elif index < last_started:
            # set -e stops at the first failed block; completed previous blocks passed.
            result = "PASS"
        elif index == last_started and last_started:
            result = "FAIL"
        else:
            result = "NOT_EXECUTED"
        tests.append({"id": identifier, "result": result})
    passed = sum(test["result"] == "PASS" for test in tests)
    failed = sum(test["result"] == "FAIL" for test in tests)
    executed = passed + failed
    skipped = len(tests) - executed
    falsifier_status = "EXERCISED" if succeeded else "NOT_EXERCISED"
    return {
        "schema": "rafaelia.test-summary/v1",
        "counts": {
            "discovered": len(STEPS),
            "executed": executed,
            "passed": passed,
            "failed": failed,
            "skipped": skipped,
        },
        "tests": tests,
        "falsifiers": [
            {"id": identifier, "condition": condition, "status": falsifier_status}
            for identifier, condition in FALSIFIERS
        ],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args(argv)
    root = Path(__file__).resolve().parents[1]
    script = root / "scripts" / "validate_runtime_truth_local.sh"
    if not script.is_file():
        print(f"TOKEN_VAZIO_INPUT_MISSING: {script.relative_to(root)}", file=sys.stderr)
        return 2
    if args.out.exists():
        print(f"FAIL: refusing to overwrite test summary: {args.out}", file=sys.stderr)
        return 1
    result = subprocess.run(
        [script.as_posix()],
        cwd=root,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
        text=True,
    )
    sys.stdout.write(result.stdout)
    sys.stderr.write(result.stderr)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    combined_output = result.stdout + "\n" + result.stderr
    args.out.write_text(
        json.dumps(summary_for(result.returncode, combined_output), ensure_ascii=False, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    print(f"TEST_SUMMARY={args.out.as_posix()}")
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
