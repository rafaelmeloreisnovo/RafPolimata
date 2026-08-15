#!/usr/bin/env python3
from __future__ import annotations
import json
import re
import subprocess
from pathlib import Path

RAW_MARKERS = ("Apkc/apkc.c", "$APKC/apkc.c", "${APKC}/apkc.c")
HARDENING_MARKERS = (
    "patch_apkc_source_cap.py",
    "patch_apkc_runtime_source.py",
    "patch_apkc_runtime_source_c_escape.py",
    "apkc-hardened-source",
)
EXEC_SUFFIXES = {".sh", ".py", ".bash", ".yml", ".yaml"}
EXEC_NAMES = {"Makefile"}
COMPILER_TOKEN_RE = re.compile(
    r"(?:^|[\s:'\"/])(?:clang(?:\+\+)?|gcc|g\+\+|cc)(?=\s|$)"
    r"|\$(?:CC|\{CC\})(?=\s|$|[\"'])"
)
COMMAND_COMPILER_RE = re.compile(
    r"(?:^|[;&|]\s*|\brun:\s*(?:\|\s*)?)"
    r"(?:['\"]?(?:clang(?:\+\+)?|gcc|g\+\+|cc)['\"]?|['\"]?\$(?:CC|\{CC\})['\"]?)"
    r"(?=\s|$)"
)
BUILD_FLAG_RE = re.compile(r"(?:^|\s)(?:-o|-c|-fsyntax-only|-nostdlib|-ffreestanding)(?:\s|$)")
RAW_ALIAS_RE = re.compile(
    r"(?m)^\s*(?:export\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*(?::=|\?=|\+=|=)\s*['\"]?Apkc/apkc\.c['\"]?\s*$"
)


def tracked_files(root: Path) -> list[Path]:
    p = subprocess.run(
        ["git", "-C", str(root), "ls-files", "-z"],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return [root / x.decode("utf-8") for x in p.stdout.split(b"\0") if x]


def is_exec_control(path: Path) -> bool:
    if path.parts and path.parts[0] == "docs":
        return False
    return path.name in EXEC_NAMES or path.suffix in EXEC_SUFFIXES


def _logical_lines(text: str) -> list[str]:
    """Join backslash-continued shell/YAML command lines without executing them."""
    out: list[str] = []
    buf = ""
    for raw in text.splitlines():
        line = raw.rstrip()
        if buf:
            buf += " " + line.lstrip()
        else:
            buf = line
        if line.endswith("\\"):
            buf = buf[:-1].rstrip()
            continue
        out.append(buf)
        buf = ""
    if buf:
        out.append(buf)
    return out


def _active_raw_build(text: str) -> bool:
    aliases = {m.group(1) for m in RAW_ALIAS_RE.finditer(text)}
    for line in _logical_lines(text):
        stripped = line.lstrip()
        if not stripped or stripped.startswith(("#", "//", "/*", "*")):
            continue
        raw_on_line = any(marker in line for marker in RAW_MARKERS)
        alias_on_line = any((f"${name}" in line or f"${{{name}}}" in line) for name in aliases)
        if not (raw_on_line or alias_on_line):
            continue
        # Compiler invocation is a build even without -o (default a.out behavior).
        # The flag-aware fallback catches common wrapped commands such as env/sudo.
        if COMMAND_COMPILER_RE.search(stripped):
            return True
        if COMPILER_TOKEN_RE.search(line) and BUILD_FLAG_RE.search(line):
            return True
    return False


def classify_text(path: Path, text: str) -> tuple[str, str]:
    if not is_exec_control(path):
        return ("IGNORED_NON_EXEC", "documentation/data/non-control-plane file")
    if not any(m in text for m in RAW_MARKERS):
        return ("NO_RAW_REF", "no canonical raw ApkC source reference")
    # A hardening marker elsewhere in the same file must never mask a separate
    # direct compilation of the canonical raw source.
    if _active_raw_build(text):
        return (
            "FAIL_RAW_BYPASS",
            "active compiler command consumes raw ApkC source; hardening marker elsewhere cannot authorize it",
        )
    if any(m in text for m in HARDENING_MARKERS):
        return ("PASS_HARDENED", "raw source reference is transformer input to a recognized hardening entrypoint")
    return (
        "PASS_REFERENCE_ONLY",
        "raw ApkC source is referenced for inspection, fixture text, commentary, or non-build control logic",
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
        "schema": "raf.apkc.raw-source-gate.v2",
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
