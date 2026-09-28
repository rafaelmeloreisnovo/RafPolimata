#!/usr/bin/env bash
set -euo pipefail
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="$ROOT/build/maturity"
CC_BIN="${CC:-$(command -v clang || command -v gcc || command -v cc)}"
mkdir -p "$OUT"

case "$(uname -m)" in
  x86_64|amd64) ARCH_FLAGS=(-msse4.2) ;;
  *) echo "maturity equivalence: NOT_RUN host specialized runtime currently requires x86_64"; exit 2 ;;
esac

if [[ "${SANITIZE:-0}" == "1" ]]; then
  MODE_FLAGS=(-O1 -fsanitize=address,undefined -fno-omit-frame-pointer)
  OUT_BIN="$OUT/test_equivalence_properties_sanitized"
else
  MODE_FLAGS=(-O2)
  OUT_BIN="$OUT/test_equivalence_properties"
fi

"$CC_BIN" -std=c11 "${MODE_FLAGS[@]}" -Wall -Wextra -Werror -Wno-unused-function   "${ARCH_FLAGS[@]}" -I"$ROOT/Benchmark" -I"$ROOT/sdk/rafpolimata_v1/include"   "$ROOT/tests/maturity/test_equivalence_properties.c"   "$ROOT/sdk/rafpolimata_v1/src/rafpolimata_v1.c"   -o "$OUT_BIN"

if [[ "${SANITIZE:-0}" == "1" ]]; then
  ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 "$OUT_BIN"
else
  "$OUT_BIN"
fi
