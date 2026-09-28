#!/usr/bin/env bash
set -euo pipefail
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
OUT="$ROOT/build/maturity"
CC_BIN="${CC:-$(command -v clang || command -v gcc || command -v cc)}"
mkdir -p "$OUT"

case "$(uname -m)" in
  x86_64|amd64) EXTRA=(-msse4.2) ;;
  *) echo "maturity equivalence: NOT_RUN host specialized runtime currently requires x86_64"; exit 2 ;;
esac

"$CC_BIN" -std=c11 -O2 -Wall -Wextra -Werror -Wno-unused-function   "${EXTRA[@]}" -I"$ROOT/Benchmark" -I"$ROOT/sdk/rafpolimata_v1/include"   "$ROOT/tests/maturity/test_equivalence_properties.c"   "$ROOT/sdk/rafpolimata_v1/src/rafpolimata_v1.c"   -o "$OUT/test_equivalence_properties"

"$OUT/test_equivalence_properties"
