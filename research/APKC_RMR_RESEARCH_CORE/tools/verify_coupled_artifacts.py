#!/usr/bin/env python3
"""Fail-closed verifier for the APKC–RMR coupled research nucleus.

SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
Coupling-ID: APKC-RMR-RESEARCH-CORE-V1-20260726
"""
from __future__ import annotations

import hashlib
import json
import shutil
import subprocess
import sys
from pathlib import Path

COUPLING_ID = "APKC-RMR-RESEARCH-CORE-V1-20260726"
ROOT = Path(__file__).resolve().parents[1]
LOCK = ROOT / "integrity" / "coupling.lock.json"


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def optional_blake3(path: Path) -> str:
    tool = shutil.which("b3sum")
    if tool is None:
        return "TOKEN_VAZIO_B3SUM_NOT_AVAILABLE"
    result = subprocess.run(
        [tool, str(path)],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return result.stdout.split()[0].lower()


def verify() -> dict[str, object]:
    lock = json.loads(LOCK.read_text(encoding="utf-8"))
    errors: list[str] = []

    if lock.get("schema") != "raf.apkc-rmr-coupling-lock.v1":
        errors.append("invalid schema")
    if lock.get("coupling_id") != COUPLING_ID:
        errors.append("coupling id mismatch")

    observed: list[dict[str, str]] = []
    for item in lock.get("artifacts", []):
        rel = item["path"]
        path = ROOT / rel
        if not path.is_file():
            errors.append(f"missing: {rel}")
            continue
        actual = sha256(path)
        expected = item["sha256"]
        if actual != expected:
            errors.append(f"sha256 mismatch: {rel}")
        text = path.read_text(encoding="utf-8", errors="replace")
        if COUPLING_ID not in text:
            errors.append(f"coupling id absent: {rel}")
        observed.append(
            {
                "path": rel,
                "sha256": actual,
                "blake3": optional_blake3(path),
            }
        )

    notice = (ROOT / "NOTICE.md").read_text(encoding="utf-8")
    if "Communication alone does not grant permission" not in notice:
        errors.append("commercial authorization notice missing")

    state = "PASS_SEALED" if not errors else "FAIL"
    return {
        "schema": "raf.apkc-rmr-coupling-receipt.v1",
        "coupling_id": COUPLING_ID,
        "state": state,
        "claim_allowed": False,
        "commercial_authorized": False,
        "errors": errors,
        "observed": observed,
    }


def main() -> int:
    receipt = verify()
    print(json.dumps(receipt, indent=2, sort_keys=True))
    return 0 if receipt["state"] == "PASS_SEALED" else 1


if __name__ == "__main__":
    sys.exit(main())
