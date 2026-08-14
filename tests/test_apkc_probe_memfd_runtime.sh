#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/scripts/apkc_probe_memfd_runtime.c"
TMP="${TMPDIR:-/tmp}/apkc-memfd-runtime-probe.$$"
BIN="$TMP/apkc_probe_memfd_runtime"
OUT="$TMP/apkc_probe_memfd_runtime.out"
mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT
CC_BIN="${CC:-$(command -v clang || command -v cc)}"
"$CC_BIN" -std=c11 -O2 -Wall -Wextra -Werror "$SRC" -o "$BIN"
"$BIN" | tee "$OUT"
grep -q '^memfd_create=PASS$' "$OUT"
grep -q '^readback_seals=PASS$' "$OUT"
grep -q '^seals=0xf$' "$OUT"
grep -q '^sealed_write_rejected=PASS$' "$OUT"
grep -q '^proc_self_fd_exec_inheritance=PASS$' "$OUT"
grep -q '^runtime_probe=PASS$' "$OUT"
grep -q '^android_compatibility=TOKEN_VAZIO$' "$OUT"
grep -q '^claim_allowed=false$' "$OUT"
printf 'RESULT pass=8 fail=0 claim_allowed=false\n'
