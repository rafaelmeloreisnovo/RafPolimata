#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CC_BIN="${CC:-cc}"
READELF_BIN="${READELF:-llvm-readelf}"
command -v "$READELF_BIN" >/dev/null 2>&1 || READELF_BIN="readelf"
TMP_ROOT="$(mktemp -d "${TMPDIR:-/tmp}/rafaelia-compiler-station.XXXXXX")"
trap 'rm -rf "$TMP_ROOT"' EXIT

python3 "$ROOT/scripts/raf_c_rewrite.py" --selftest
python3 "$ROOT/scripts/raf_kernel_lower.py" --selftest

bash "$ROOT/scripts/apkc_strict_native_build.sh" c arm64 "$TMP_ROOT/c64.so" "$ROOT/tests/fixtures/strict_kernel.c"
bash "$ROOT/scripts/apkc_strict_native_build.sh" c arm32 "$TMP_ROOT/c32.so" "$ROOT/tests/fixtures/strict_kernel.c"
bash "$ROOT/scripts/apkc_strict_native_build.sh" py arm64 "$TMP_ROOT/py64.so" "$ROOT/tests/fixtures/kernel.py"

"$READELF_BIN" -h "$TMP_ROOT/c64.so" | grep -Eq 'AArch64|Machine:[[:space:]]+AArch64'
"$READELF_BIN" -h "$TMP_ROOT/c32.so" | grep -Eq 'ARM|Machine:[[:space:]]+ARM'
"$READELF_BIN" -Ws "$TMP_ROOT/py64.so" | grep -q 'ANativeActivity_onCreate'
"$READELF_BIN" -Ws "$TMP_ROOT/py64.so" | grep -q 'android_main'
"$READELF_BIN" -Ws "$TMP_ROOT/py64.so" | grep -q 'mix'

"$CC_BIN" -std=c11 -Wall -Wextra -Werror \
  "$ROOT/raf_main.c" "$ROOT/raf_frontend.c" "$ROOT/raf_cpu.c" \
  "$ROOT/raf_asm_emit.c" "$ROOT/raf_precomp.c" -I"$ROOT" -o "$TMP_ROOT/raf_compile"

"$TMP_ROOT/raf_compile" "$ROOT/tests/fixtures/raf_return_42.c" "$TMP_ROOT/a" --native >/dev/null
"$TMP_ROOT/raf_compile" "$ROOT/tests/fixtures/raf_return_42.c" "$TMP_ROOT/b" --native >/dev/null
"$TMP_ROOT/raf_compile" "$ROOT/tests/fixtures/raf_return_1337.py" "$TMP_ROOT/c" --native >/dev/null
cmp "$TMP_ROOT/a.bin" "$TMP_ROOT/b.bin"
if cmp -s "$TMP_ROOT/a.bin" "$TMP_ROOT/c.bin"; then
  echo 'source-dependent gate: FAIL — distinct source values emitted identical bytes' >&2
  exit 1
fi
python3 "$ROOT/scripts/validate_ops_manifest.py" "$TMP_ROOT/a.ops" --expect-rollback 0
python3 "$ROOT/scripts/compare_ops_manifest.py" "$TMP_ROOT/a.ops" "$TMP_ROOT/b.ops"

set +e
"$TMP_ROOT/raf_compile" "$ROOT/tests/fixtures/raf_return_invalid.c" "$TMP_ROOT/invalid" --native >/dev/null 2>&1
invalid_rc=$?
set -e
[[ $invalid_rc -ne 0 ]] || { echo 'non-constant expression must fail closed' >&2; exit 1; }
python3 "$ROOT/scripts/validate_ops_manifest.py" "$TMP_ROOT/invalid.ops" --expect-rollback -2

echo 'RAFAELIA compiler station: PASS'
