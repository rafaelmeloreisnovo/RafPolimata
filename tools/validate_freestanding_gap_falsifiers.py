#!/usr/bin/env python3
"""Fail-closed freestanding gap falsifier.

CLOSURE_L13: audit-readiness unknowns remain governed, not resolved.
This validator does not prove runtime, license completeness or independent
reproduction. It rejects untyped or over-promoted evidence gaps in the changed
freestanding reconstruction tranche.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from dataclasses import asdict, dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, List, Optional, Set

TOKEN = re.compile(r"TOKEN_VAZIO", re.IGNORECASE)
CLOSURE = re.compile(r"CLOSURE_([LG]\d+)", re.IGNORECASE)
CLAIM_ALLOWED_TRUE = re.compile(r"claim_allowed\s*[:=]\s*[`\"']?true", re.IGNORECASE)
DOCUMENTED_TO_PASS = re.compile(r"DOCUMENTED[^\n]{0,80}\bPASS\b|\bPASS\b[^\n]{0,80}DOCUMENTED", re.IGNORECASE)
MEASURED = re.compile(r"MEASURED_LOCAL|MEASURED_LOCAL_PASS|MEASURED_LOCAL_SUCCESS", re.IGNORECASE)
REPRODUCED = re.compile(r"\b(REPRODUCED|CROSS_HOST|PROVEN)\b", re.IGNORECASE)
REQUIRED_MEASURED_FIELDS = ("head_sha", "run", "job")
TEXT_SUFFIXES = {".md", ".json", ".yml", ".yaml", ".py", ".sh", ".txt"}
SKIP_PARTS = {".git", "__pycache__", "docs/generated", "results"}
SELF_PATH = "tools/validate_freestanding_gap_falsifiers.py"

TOKEN_TYPES = {
    "TOKEN_VAZIO_LICENSE": ("license", "licen", "copying", "notice", "spdx"),
    "TOKEN_VAZIO_DEVICE": ("device", "arm32", "arm64", "termux", "frida", "physical"),
    "TOKEN_VAZIO_PROVIDER": ("provider", "ci", "github actions", "workflow", "runner"),
    "TOKEN_VAZIO_REPRODUCTION": ("independent", "reproduction", "cross_host", "reproduced"),
    "TOKEN_VAZIO_ARTIFACT": ("artifact", "hash", "elf", "abi", "object"),
    "TOKEN_VAZIO_LOG": ("log", "raw", "receipt", "run id", "job"),
}


@dataclass
class Finding:
    path: str
    line: int
    severity: str
    rule: str
    detail: str

    def to_dict(self) -> Dict[str, object]:
        return asdict(self)


def run_git(root: Path, args: List[str]) -> str:
    proc = subprocess.run(["git", "-C", str(root), *args], text=True, capture_output=True)
    if proc.returncode != 0:
        raise RuntimeError((proc.stderr or proc.stdout).strip() or "git command failed")
    return proc.stdout


def changed_files(root: Path, base_ref: Optional[str]) -> List[Path]:
    if not base_ref:
        return sorted(p for p in root.rglob("*") if p.is_file())
    out = run_git(root, ["diff", "--name-only", "--diff-filter=ACMRT", base_ref, "HEAD", "--"])
    return [root / line.strip() for line in out.splitlines() if line.strip()]


def is_scannable(root: Path, path: Path) -> bool:
    try:
        rel = path.resolve().relative_to(root).as_posix()
    except ValueError:
        return False
    if any(part in rel for part in SKIP_PARTS):
        return False
    return path.is_file() and path.suffix.lower() in TEXT_SUFFIXES


def closure_ids(text: str) -> Set[str]:
    return {m.group(1).upper() for m in CLOSURE.finditer(text)}


def classify_token(line: str) -> str:
    low = line.lower()
    for token_type, hints in TOKEN_TYPES.items():
        if any(h in low for h in hints):
            return token_type
    return "TOKEN_VAZIO_UNTYPED"


def scan_file(root: Path, path: Path, findings: List[Finding]) -> Dict[str, int]:
    rel = path.resolve().relative_to(root).as_posix()
    counts = {"tokens": 0, "typed_tokens": 0, "measured": 0}
    if rel == SELF_PATH:
        return counts

    text = path.read_text(encoding="utf-8", errors="replace")
    file_closures = closure_ids(text)
    lines = text.splitlines()

    for idx, line in enumerate(lines, 1):
        if CLAIM_ALLOWED_TRUE.search(line):
            findings.append(Finding(rel, idx, "ERROR", "CLAIM_ALLOWED_TRUE", "claim_allowed=true is forbidden in this fail-closed tranche"))
        if DOCUMENTED_TO_PASS.search(line):
            findings.append(Finding(rel, idx, "ERROR", "DOCUMENTED_PASS_OVERPROMOTION", "DOCUMENTED must not be described as PASS"))
        if REPRODUCED.search(line) and "TOKEN_VAZIO" not in line and "CROSS_HOST" in line.upper():
            findings.append(Finding(rel, idx, "WARNING", "CROSS_HOST_ASSERTION_REQUIRES_REVIEW", "cross-host language requires independent evidence binding"))
        if TOKEN.search(line):
            counts["tokens"] += 1
            token_type = classify_token(line)
            if token_type != "TOKEN_VAZIO_UNTYPED":
                counts["typed_tokens"] += 1
            if not file_closures and not closure_ids(line):
                findings.append(Finding(rel, idx, "ERROR", "TOKEN_WITHOUT_CLOSURE", "TOKEN_VAZIO requires a governing CLOSURE_*"))
            if token_type == "TOKEN_VAZIO_UNTYPED":
                findings.append(Finding(rel, idx, "ERROR", "TOKEN_UNTYPED", "TOKEN_VAZIO must be typed by license/device/provider/reproduction/artifact/log context"))

    if MEASURED.search(text):
        counts["measured"] += 1
        missing = [field for field in REQUIRED_MEASURED_FIELDS if field not in text]
        if rel.startswith("docs/receipts/") and missing:
            findings.append(Finding(rel, 1, "ERROR", "MEASURED_RECEIPT_MISSING_BINDING", "measured receipt missing: " + ",".join(missing)))
    return counts


def main(argv: Optional[List[str]] = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, default=Path("."))
    ap.add_argument("--changed-since")
    ap.add_argument("--output", type=Path, default=Path("ci/reports/freestanding-gap-falsifiers.json"))
    ap.add_argument("--strict", action="store_true")
    ap.add_argument("--summary", action="store_true")
    args = ap.parse_args(argv)

    root = args.repo.resolve()
    findings: List[Finding] = []
    totals = {"files": 0, "tokens": 0, "typed_tokens": 0, "measured_mentions": 0}

    files = changed_files(root, args.changed_since)
    for path in files:
        if not is_scannable(root, path):
            continue
        totals["files"] += 1
        counts = scan_file(root, path, findings)
        totals["tokens"] += counts["tokens"]
        totals["typed_tokens"] += counts["typed_tokens"]
        totals["measured_mentions"] += counts["measured"]

    summary = {
        **totals,
        "errors": sum(1 for f in findings if f.severity == "ERROR"),
        "warnings": sum(1 for f in findings if f.severity == "WARNING"),
        "scope": "changed_since" if args.changed_since else "full_repository",
        "changed_since": args.changed_since or "TOKEN_VAZIO_PROVIDER_BASE_REF",
    }
    report = {
        "schema": "rafaelia.freestanding_gap_falsifier.v1",
        "timestamp_utc": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
        "claim_allowed": False,
        "status": "PASS" if summary["errors"] == 0 else "FAIL",
        "summary": summary,
        "findings": [f.to_dict() for f in findings],
        "invariants": [
            "SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM",
            "DOCUMENTED != EXECUTED",
            "IMPLEMENTED_UNTESTED != PASS",
            "TOKEN_VAZIO != 0",
            "MEASURED_LOCAL != REPRODUCED",
        ],
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if args.summary:
        print("FREESTANDING_GAP_FALSIFIERS")
        for k, v in summary.items():
            print(f"{k}={v}")
        print(f"status={report['status']}")
        for finding in findings:
            print(f"{finding.severity} {finding.path}:{finding.line} {finding.rule}: {finding.detail}")

    if args.strict and summary["errors"]:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
