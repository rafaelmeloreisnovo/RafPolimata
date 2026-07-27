#!/usr/bin/env bash
set -Eeuo pipefail

export LC_ALL=C
export TZ=UTC
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CC_BIN="${CC:-cc}"
READELF_BIN="${READELF:-llvm-readelf}"
command -v "$READELF_BIN" >/dev/null 2>&1 || READELF_BIN="readelf"
TMP_ROOT="$(mktemp -d "${TMPDIR:-/tmp}/rafaelia-compiler-station.XXXXXX")"
trap 'rm -rf "$TMP_ROOT"' EXIT

python3 "$ROOT/scripts/raf_c_rewrite.py" --selftest
python3 "$ROOT/scripts/raf_kernel_lower.py" --selftest

"$CC_BIN" -std=c11 -Wall -Wextra -Werror -Wshadow -Wconversion -Wpedantic \
  -fno-builtin -I"$ROOT" "$ROOT/tests/raf_libc_emu_selftest.c" \
  -o "$TMP_ROOT/libc-emu-selftest"
"$TMP_ROOT/libc-emu-selftest"

bash "$ROOT/scripts/apkc_strict_native_build.sh" c arm64 \
  "$TMP_ROOT/c64.so" "$ROOT/tests/fixtures/strict_kernel.c"
bash "$ROOT/scripts/apkc_strict_native_build.sh" c arm64 \
  "$TMP_ROOT/c64-repeat.so" "$ROOT/tests/fixtures/strict_kernel.c"
bash "$ROOT/scripts/apkc_strict_native_build.sh" c arm32 \
  "$TMP_ROOT/c32.so" "$ROOT/tests/fixtures/strict_kernel.c"
bash "$ROOT/scripts/apkc_strict_native_build.sh" cpp arm64 \
  "$TMP_ROOT/cpp64.so" "$ROOT/tests/fixtures/strict_kernel.cpp"
bash "$ROOT/scripts/apkc_strict_native_build.sh" py arm64 \
  "$TMP_ROOT/py64.so" "$ROOT/tests/fixtures/kernel.py"

cmp "$TMP_ROOT/c64.so" "$TMP_ROOT/c64-repeat.so"
for artifact in c64 c64-repeat c32 cpp64 py64; do
  test -s "$TMP_ROOT/$artifact.so"
  test -s "$TMP_ROOT/$artifact.so.receipt.json"
  python3 - "$TMP_ROOT/$artifact.so.receipt.json" <<'PY'
import json
from pathlib import Path
import sys
receipt = json.loads(Path(sys.argv[1]).read_text(encoding="utf-8"))
assert receipt["schema"] == "rafaelia.apkc.strict-native-receipt.v2"
assert receipt["status"] == "STRICT_ELF_PASS"
assert receipt["claim_allowed"] is True
assert receipt["source_bytes"] > 0
assert receipt["source_bytes"] <= receipt["source_limit_bytes"] == 1 << 20
assert receipt["runtime_external_dependencies"] == []
assert all(value == "PASS" for value in receipt["gates"].values())
PY
done

"$READELF_BIN" -h "$TMP_ROOT/c64.so" | grep -Eq 'AArch64|Machine:[[:space:]]+AArch64'
"$READELF_BIN" -h "$TMP_ROOT/c32.so" | grep -Eq 'ARM|Machine:[[:space:]]+ARM'
"$READELF_BIN" -Ws "$TMP_ROOT/py64.so" | grep -q 'ANativeActivity_onCreate'
"$READELF_BIN" -Ws "$TMP_ROOT/py64.so" | grep -q 'android_main'
"$READELF_BIN" -Ws "$TMP_ROOT/py64.so" | grep -q 'mix'
"$READELF_BIN" -Ws "$TMP_ROOT/cpp64.so" | grep -q 'raf_cpp_patch'

"$CC_BIN" -std=c11 -Wall -Wextra -Werror \
  "$ROOT/raf_main.c" "$ROOT/raf_frontend.c" "$ROOT/raf_cpu.c" \
  "$ROOT/raf_asm_emit.c" "$ROOT/raf_precomp.c" -I"$ROOT" -o "$TMP_ROOT/raf_compile"

"$TMP_ROOT/raf_compile" "$ROOT/tests/fixtures/raf_return_42.c" "$TMP_ROOT/a" --native >/dev/null
"$TMP_ROOT/raf_compile" "$ROOT/tests/fixtures/raf_return_42.c" "$TMP_ROOT/b" --native >/dev/null
"$TMP_ROOT/raf_compile" "$ROOT/tests/fixtures/raf_return_1337.py" "$TMP_ROOT/c" --native >/dev/null
cmp "$TMP_ROOT/a.bin" "$TMP_ROOT/b.bin"
if cmp -s "$TMP_ROOT/a.bin" "$TMP_ROOT/c.bin"; then
  echo 'source-dependent gate: FAIL — distinct values emitted identical bytes' >&2
  exit 1
fi
python3 "$ROOT/scripts/validate_ops_manifest.py" "$TMP_ROOT/a.ops" --expect-rollback 0
python3 "$ROOT/scripts/validate_ops_manifest.py" "$TMP_ROOT/b.ops" --expect-rollback 0
python3 "$ROOT/scripts/validate_ops_manifest.py" "$TMP_ROOT/c.ops" --expect-rollback 0
python3 "$ROOT/scripts/compare_ops_manifest.py" "$TMP_ROOT/a.ops" "$TMP_ROOT/b.ops"
grep -qx 'ops_schema=4' "$TMP_ROOT/a.ops"
grep -qx 'transaction_state=COMMITTED' "$TMP_ROOT/a.ops"
grep -qx 'ir_value=42' "$TMP_ROOT/a.ops"

# Root compiler rollback over a previous successful output.
"$TMP_ROOT/raf_compile" "$ROOT/tests/fixtures/raf_return_42.c" "$TMP_ROOT/stale" --native >/dev/null
test -s "$TMP_ROOT/stale.bin"
set +e
"$TMP_ROOT/raf_compile" "$ROOT/tests/fixtures/raf_return_invalid.c" "$TMP_ROOT/stale" --native >/dev/null 2>&1
stale_rc=$?
set -e
[[ $stale_rc -ne 0 ]] || { echo 'invalid source unexpectedly committed' >&2; exit 1; }
for suffix in s hex bin; do
  [[ ! -e "$TMP_ROOT/stale.$suffix" ]] || { echo "stale artifact survived: stale.$suffix" >&2; exit 1; }
done
python3 "$ROOT/scripts/validate_ops_manifest.py" "$TMP_ROOT/stale.ops" --expect-rollback -2
grep -qx 'transaction_state=ROLLED_BACK' "$TMP_ROOT/stale.ops"

for invalid_fixture in \
  raf_return_invalid.c \
  raf_return_ambiguous.c \
  raf_return_plusplus.c \
  raf_return_comment_tail.c; do
  base="$TMP_ROOT/${invalid_fixture%.c}"
  set +e
  "$TMP_ROOT/raf_compile" "$ROOT/tests/fixtures/$invalid_fixture" "$base" --native >/dev/null 2>&1
  invalid_rc=$?
  set -e
  [[ $invalid_rc -ne 0 ]] || { echo "$invalid_fixture must fail closed" >&2; exit 1; }
  python3 "$ROOT/scripts/validate_ops_manifest.py" "$base.ops" --expect-rollback -2
  [[ ! -e "$base.bin" ]] || { echo "$invalid_fixture left native bytes" >&2; exit 1; }
done

# Root expression depth and token budgets.
python3 - "$TMP_ROOT/deep.c" "$TMP_ROOT/wide.c" <<'PY'
from pathlib import Path
import sys
Path(sys.argv[1]).write_text("int f(void){return " + "(" * 70 + "1" + ")" * 70 + ";}\n")
Path(sys.argv[2]).write_text("int f(void){return " + "+".join("1" for _ in range(1100)) + ";}\n")
PY
for bounded in deep wide; do
  set +e
  "$TMP_ROOT/raf_compile" "$TMP_ROOT/$bounded.c" "$TMP_ROOT/$bounded" --native >/dev/null 2>&1
  bounded_rc=$?
  set -e
  [[ $bounded_rc -ne 0 ]] || { echo "$bounded expression escaped limits" >&2; exit 1; }
  python3 "$ROOT/scripts/validate_ops_manifest.py" "$TMP_ROOT/$bounded.ops" --expect-rollback -2
done

# Strict builder rollback and source-size limit must invalidate an old pair.
bash "$ROOT/scripts/apkc_strict_native_build.sh" c arm64 \
  "$TMP_ROOT/strict-stale.so" "$ROOT/tests/fixtures/strict_kernel.c"
test -s "$TMP_ROOT/strict-stale.so" && test -s "$TMP_ROOT/strict-stale.so.receipt.json"
set +e
bash "$ROOT/scripts/apkc_strict_native_build.sh" c arm64 \
  "$TMP_ROOT/strict-stale.so" "$ROOT/tests/fixtures/unsupported_header.c" >/dev/null 2>&1
unsupported_rc=$?
set -e
[[ $unsupported_rc -ne 0 ]] || { echo 'unsupported header must fail closed' >&2; exit 1; }
[[ ! -e "$TMP_ROOT/strict-stale.so" && ! -e "$TMP_ROOT/strict-stale.so.receipt.json" ]]

python3 - "$TMP_ROOT/oversized.s" <<'PY'
from pathlib import Path
import sys
Path(sys.argv[1]).write_bytes(b'x' * ((1 << 20) + 1))
PY
bash "$ROOT/scripts/apkc_strict_native_build.sh" c arm64 \
  "$TMP_ROOT/oversized-target.so" "$ROOT/tests/fixtures/strict_kernel.c"
set +e
bash "$ROOT/scripts/apkc_strict_native_build.sh" asm arm64 \
  "$TMP_ROOT/oversized-target.so" "$TMP_ROOT/oversized.s" >/dev/null 2>&1
oversized_rc=$?
set -e
[[ $oversized_rc -ne 0 ]] || { echo 'oversized ASM source must fail' >&2; exit 1; }
[[ ! -e "$TMP_ROOT/oversized-target.so" && ! -e "$TMP_ROOT/oversized-target.so.receipt.json" ]]

# Tampering with any signed field must invalidate the .ops receipt.
cp "$TMP_ROOT/a.ops" "$TMP_ROOT/tampered.ops"
sed -i 's/^ir_value=42$/ir_value=43/' "$TMP_ROOT/tampered.ops"
if python3 "$ROOT/scripts/validate_ops_manifest.py" "$TMP_ROOT/tampered.ops" >/dev/null 2>&1; then
  echo 'tampered manifest unexpectedly validated' >&2
  exit 1
fi

echo 'RAFAELIA compiler station HOTFIX: PASS'
