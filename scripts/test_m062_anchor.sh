#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="$(mktemp)"
trap 'rm -f "$OUT"' EXIT

cc -std=c11 -Wall -Wextra -Werror -pedantic \
  -DRAFAELIA_M062_SELFTEST_MAIN \
  "$ROOT/RAF_062_quaternary_anchor_eight_gate.c" \
  -o "$OUT"

"$OUT"

grep -Fq 'M062_ORGANS       4u' "$ROOT/RAF_062_quaternary_anchor_eight_gate.c"
grep -Fq 'M062_GATES        8u' "$ROOT/RAF_062_quaternary_anchor_eight_gate.c"
grep -Fq 'M062_INPUT_TRANSITION_OR_NOISE' "$ROOT/RAF_062_quaternary_anchor_eight_gate.c"
grep -Fq 'rafaelia_m062_try_fifth' "$ROOT/RAF_062_quaternary_anchor_eight_gate.c"

if grep -En '\b(malloc|calloc|realloc|free)\s*\(' "$ROOT/RAF_062_quaternary_anchor_eight_gate.c"; then
  echo 'FALHA: heap proibido no M062' >&2
  exit 1
fi

echo 'M062 four-organ anchor: PASS'
