#!/usr/bin/env python3
from __future__ import annotations
import json
import subprocess
import sys
from pathlib import Path

RAW_MARKERS = ("Apkc/apkc.c", "$APKC/apkc.c", "${APKC}/apkc.c")
HARDENING_MARKERS = ("patch_apkc_source_cap.py", "patch_apkc_runtime_source.py", "apkc-hardened-source")
EXEC_SUFFIXES = {".sh", ".py", ".bash", ".yml", ".yaml"}
EXEC_NAMES = {"Makefile"}


def tracked_files(root: Path) -> list[Path]:
    p = subprocess.run(
        ["git", "-C", str(root), "ls-files", "-z"],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return [root / x.decode("utf-8") for x in p.stdout.split(b"\0") if x]


def is_exec_control(path: Path) -> bool:
    return path.name in EXEC_NAMES or path.suffix in EXEC_SUFFIXES


def classify_text(path: Path, text: str) -> tuple[str, str]:
    if not is_exec_control(path):
        return ("IGNORED_NON_EXEC", "documentation/data/non-control-plane file")
    if not any(m in text for m in RAW_MARKERS):
        return ("NO_RAW_REF", "no canonical raw ApkC source reference")
    if any(m in text for m in HARDENING_MARKERS):
        return ("PASS_HARDENED", "raw source reference is paired with mandatory hardening entrypoint")
    return (
        "FAIL_RAW_BYPASS",
        "executable/control-plane file references raw ApkC source without a recognized hardening entrypoint",
    )


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    rows = []
    failures = []
    for path in tracked_files(root):
        try:
            text = path.read_text(encoding="utf-8")
        except (UnicodeDecodeError, OSError):
            continue
        state, reason = classify_text(path.relative_to(root), text)
        if state in ("IGNORED_NON_EXEC", "NO_RAW_REF"):
            continue
        row = {"path": str(path.relative_to(root)), "state": state, "reason": reason}
        rows.append(row)
        if state.startswith("FAIL"):
            failures.append(row)

    out = {
        "schema": "raf.apkc.raw-source-gate.v1",
        "claim_allowed": False,
        "checked_control_plane_refs": len(rows),
        "failures": len(failures),
        "state": "PASS" if not failures else "FAIL",
        "rows": rows,
    }
    print(json.dumps(out, indent=2, sort_keys=True))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
