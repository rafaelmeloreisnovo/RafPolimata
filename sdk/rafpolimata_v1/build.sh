#!/usr/bin/env bash
set -euo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
OUT="${1:-$ROOT/../../build/sdk/rafpolimata_v1}"
CC_BIN="${CC:-}"
AR_BIN="${AR:-}"

if [[ -z "$CC_BIN" ]]; then
  CC_BIN="$(command -v clang || command -v gcc || command -v cc)"
fi
if [[ -z "$AR_BIN" ]]; then
  AR_BIN="$(command -v llvm-ar || command -v ar)"
fi
[[ -n "$CC_BIN" && -n "$AR_BIN" ]] || { echo "SDK build: missing compiler/ar" >&2; exit 127; }

mkdir -p "$OUT"
COMMON=(-std=c11 -O2 -fno-ident -fno-stack-protector -Wall -Wextra -Werror -I"$ROOT/include")
"$CC_BIN" "${COMMON[@]}" -ffreestanding -fno-builtin -c "$ROOT/src/rafpolimata_v1.c" -o "$OUT/rafpolimata_v1.o"
"$AR_BIN" rcD "$OUT/librafpolimata_v1.a" "$OUT/rafpolimata_v1.o"
"$CC_BIN" "${COMMON[@]}" "$ROOT/examples/smoke.c" "$OUT/librafpolimata_v1.a" -Wl,--build-id=none -o "$OUT/rafpolimata_v1_smoke"
"$OUT/rafpolimata_v1_smoke"

printf 'SDK_V1_PASS compiler=%s ar=%s out=%s\n' "$CC_BIN" "$AR_BIN" "$OUT"
