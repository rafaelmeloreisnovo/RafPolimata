#!/usr/bin/env python3
"""test_format_fixtures.py — structural golden tests for audit lacuna L17.

Pure Python, x86-runnable. No ARM toolchain, no apkc execution.

Asserts invariants over artifacts ALREADY COMMITTED under Apkc/proofs/.
Any artifact that is absent OR still carries the `TOKEN_VAZIO` sentinel is
SKIPPED (not failed) and counted as TOKEN_VAZIO — because producing real
content for those cases requires running apkc on an ARM64 host.

Exit code is 0 unless a *present, non-empty, non-TOKEN_VAZIO* artifact fails
a structural invariant. Run:

    python3 tests/test_format_fixtures.py
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "Apkc" / "proofs" / "out"
RUN_TV = ROOT / "Apkc" / "proofs" / "runs" / "2026-06-14-tv"

TOKEN = "TOKEN_VAZIO"

# Counters for the summary line.
N_PASS = 0
N_SKIP = 0
N_FAIL = 0


def _read(path: Path):
    """Return (text, status) where status is 'ok', 'absent', or 'token_vazio'."""
    if not path.exists():
        return "", "absent"
    text = path.read_text(encoding="utf-8", errors="replace")
    if not text.strip():
        return text, "absent"
    if TOKEN in text:
        return text, "token_vazio"
    return text, "ok"


def case(name: str):
    """Decorator-ish helper: returns a logger bound to a corpus case name."""
    def log(kind: str, msg: str):
        global N_PASS, N_SKIP, N_FAIL
        if kind == "PASS":
            N_PASS += 1
            print(f"[PASS] {name}: {msg}")
        elif kind == "SKIP":
            N_SKIP += 1
            print(f"[SKIP/{TOKEN}] {name}: {msg}")
        else:
            N_FAIL += 1
            print(f"[FAIL] {name}: {msg}", file=sys.stderr)
    return log


def skip_if_unavailable(log, path: Path):
    """If artifact is absent/empty/TOKEN_VAZIO, log SKIP and return True."""
    text, status = _read(path)
    if status == "absent":
        log("SKIP", f"{path.relative_to(ROOT)} absent or empty")
        return True, text
    if status == "token_vazio":
        log("SKIP", f"{path.relative_to(ROOT)} is {TOKEN}")
        return True, text
    return False, text


# ---------------------------------------------------------------------------
# Case 1: readelf-arm32 golden — ELF32 / ARM / EABI markers (if real content)
# ---------------------------------------------------------------------------
def test_readelf_arm32():
    log = case("readelf-arm32")
    path = OUT / "readelf-arm32.txt"
    skip, text = skip_if_unavailable(log, path)
    if skip:
        return
    # Expected structural markers in a real ARM32 readelf summary.
    checks = {
        "ELF32 class": r"ELF32",
        "ARM machine": r"Machine:\s*ARM\b",
        "EABI marker": r"EABI",
        "shared object": r"shared object",
        "PASS status": r"Status:\s*PASS",
    }
    missing = [label for label, pat in checks.items()
               if not re.search(pat, text, re.IGNORECASE)]
    if missing:
        log("FAIL", f"missing markers: {missing}")
        return
    # Bonus: if a libhello.so sha256 is present it must be 64 hex chars.
    m = re.search(r"libhello\.so\s+sha256:\s*([0-9a-fA-F]+)", text)
    if m and len(m.group(1)) != 64:
        log("FAIL", f"libhello.so sha256 not 64 hex chars: {m.group(1)!r}")
        return
    log("PASS", "ELF32 / Machine ARM / EABI / shared object / Status PASS present")


# ---------------------------------------------------------------------------
# Case 2: readelf-arm64 golden — AArch64 markers (if real content)
# ---------------------------------------------------------------------------
def test_readelf_arm64():
    log = case("readelf-arm64")
    path = OUT / "readelf-arm64.txt"
    skip, text = skip_if_unavailable(log, path)
    if skip:
        return
    if not re.search(r"AArch64|ELF64", text, re.IGNORECASE):
        log("FAIL", "no AArch64/ELF64 marker in non-empty readelf-arm64 artifact")
        return
    log("PASS", "AArch64/ELF64 marker present")


# ---------------------------------------------------------------------------
# Case 3: unzip membership — AndroidManifest.xml / classes.dex / lib/.../*.so
# ---------------------------------------------------------------------------
def test_unzip_members():
    log = case("unzip-members")
    path = OUT / "unzip.txt"
    skip, text = skip_if_unavailable(log, path)
    if skip:
        return
    expected = ["AndroidManifest.xml", "classes.dex"]
    missing = [m for m in expected if m not in text]
    if missing:
        log("FAIL", f"unzip listing missing members: {missing}")
        return
    if not re.search(r"lib/[A-Za-z0-9_\-]+/lib\w+\.so", text):
        log("FAIL", "unzip listing missing a lib/<abi>/lib*.so member")
        return
    log("PASS", "AndroidManifest.xml + classes.dex + lib/<abi>/*.so present")


# ---------------------------------------------------------------------------
# Case 4: aapt xmltree — manifest parses, package + permission visible
# ---------------------------------------------------------------------------
def test_aapt_xmltree():
    log = case("aapt-xmltree")
    path = OUT / "aapt-xmltree.txt"
    skip, text = skip_if_unavailable(log, path)
    if skip:
        return
    if "manifest" not in text.lower():
        log("FAIL", "aapt xmltree has no 'manifest' element")
        return
    log("PASS", "aapt xmltree contains a manifest element")


# ---------------------------------------------------------------------------
# Case 5: dex-sha1 — well-formed 40-hex SHA-1 (if real content)
# ---------------------------------------------------------------------------
def test_dex_sha1():
    log = case("dex-sha1")
    path = OUT / "dex-sha1.txt"
    skip, text = skip_if_unavailable(log, path)
    if skip:
        return
    if not re.search(r"\b[0-9a-fA-F]{40}\b", text):
        log("FAIL", "no 40-hex SHA-1 in non-empty dex-sha1 artifact")
        return
    log("PASS", "contains a well-formed 40-hex SHA-1")


# ---------------------------------------------------------------------------
# Case 6 & 7: sha256 ledgers — every line is (64-hex)(name)(bytes=N)
# Handles both the fenced ledger (out/artifact-sha256.txt) and the plain
# ledger (runs/2026-06-14-tv/sha256.txt).
# ---------------------------------------------------------------------------
LEDGER_LINE = re.compile(
    r"^([0-9a-fA-F]{64})\s+(\S.*?)\s+bytes=(\d+)\s*$"
)


def _check_ledger(log, path: Path):
    skip, text = skip_if_unavailable(log, path)
    if skip:
        return
    # Strip markdown / fence / comment lines; keep candidate data lines.
    data_lines = []
    for raw in text.splitlines():
        line = raw.strip()
        if not line:
            continue
        if line.startswith("#") or line.startswith("```"):
            continue
        # Prose lines (Scope:, Policy:) won't start with 64 hex chars.
        if re.match(r"^[0-9a-fA-F]{64}\s", line):
            data_lines.append(line)
    if not data_lines:
        log("SKIP", f"{path.relative_to(ROOT)} has no hash data lines")
        return
    bad = [ln for ln in data_lines if not LEDGER_LINE.match(ln)]
    if bad:
        log("FAIL", f"{path.relative_to(ROOT)} malformed ledger lines: {bad[:2]}")
        return
    log("PASS",
        f"{path.relative_to(ROOT)}: {len(data_lines)} ledger lines all "
        f"'<64hex> <name> bytes=N'")


def test_sha256_ledger_out():
    _check_ledger(case("sha256-ledger-out"), OUT / "artifact-sha256.txt")


def test_sha256_ledger_run():
    _check_ledger(case("sha256-ledger-run"), RUN_TV / "sha256.txt")


def main():
    print("=== test_format_fixtures (L17 corpus) ===")
    print(f"proofs/out : {OUT}")
    print(f"proofs/run : {RUN_TV}\n")

    test_readelf_arm32()
    test_readelf_arm64()
    test_unzip_members()
    test_aapt_xmltree()
    test_dex_sha1()
    test_sha256_ledger_out()
    test_sha256_ledger_run()

    print()
    print(f"format-fixtures: {N_PASS} PASS, {N_SKIP} SKIP/{TOKEN}, {N_FAIL} FAIL")
    # Only real, present, non-TOKEN_VAZIO artifacts failing a structural
    # invariant cause a non-zero exit. SKIP/TOKEN_VAZIO are expected.
    return 1 if N_FAIL else 0


if __name__ == "__main__":
    sys.exit(main())
