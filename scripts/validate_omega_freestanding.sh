#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT=${OUT_DIR:-"$ROOT/build/freestanding-host"}
REPORT=${REPORT_PATH:-"$ROOT/ci/reports/omega-freestanding.md"}
CC=${CC:-cc}
NM=${NM:-nm}
SIZE=${SIZE:-size}

mkdir -p "$OUT" "$(dirname "$REPORT")"

fail()
{
    printf '%s\n' "$1" >&2
    exit 1
}

command -v "$CC" >/dev/null 2>&1 || fail "missing C compiler: $CC"
command -v "$NM" >/dev/null 2>&1 || fail "missing nm: $NM"
command -v "$SIZE" >/dev/null 2>&1 || fail "missing size: $SIZE"

SRC="$ROOT/freestanding/omega/omega_core.c"
HDR="$ROOT/freestanding/omega/omega_core.h"
TEST="$ROOT/tests/freestanding/omega_core_test.c"
OBJ="$OUT/omega_core.o"
BIN="$OUT/omega_core_test"

for f in "$SRC" "$HDR" "$TEST"; do
    test -f "$f" || fail "missing source: $f"
done

if grep -REn '#[[:space:]]*include[[:space:]]*[<"](stdio|stdlib|string|stdint|unistd|fcntl|math)\.h[>"]' \
    "$ROOT/freestanding/omega" > "$OUT/banned-includes.txt" 2>/dev/null; then
    fail "freestanding core imports hosted headers"
fi

if grep -REn '\b(malloc|calloc|realloc|free|memcpy|memmove|memset|printf|fprintf|puts|write|read|open|close)\b[[:space:]]*\(' \
    "$ROOT/freestanding/omega" > "$OUT/banned-calls.txt" 2>/dev/null; then
    fail "freestanding core calls hosted/runtime functions"
fi

COMMON='-std=c11 -Wall -Wextra -Werror -pedantic -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fvisibility=hidden -ffunction-sections -fdata-sections -g0'
# shellcheck disable=SC2086
"$CC" $COMMON -Os -I"$ROOT/freestanding/omega" -c "$SRC" -o "$OBJ"

UNDEFINED=$($NM -u "$OBJ" 2>/dev/null || true)
test -z "$UNDEFINED" || {
    printf '%s\n' "$UNDEFINED" > "$OUT/undefined-symbols.txt"
    fail "undefined symbols found in freestanding object"
}

# The test harness is hosted; only the core object is required to remain freestanding.
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
    -I"$ROOT/freestanding/omega" "$SRC" "$TEST" -o "$BIN"
"$BIN"

SIZE_LINE=$($SIZE "$OBJ" | tail -n 1)
TEXT_SIZE=$(printf '%s\n' "$SIZE_LINE" | awk '{print $1}')
case "$TEXT_SIZE" in
    ''|*[!0-9]*) fail "could not parse object text size" ;;
esac
[ "$TEXT_SIZE" -le 4096 ] || fail "text size budget exceeded: $TEXT_SIZE > 4096"

{
    echo '# Omega freestanding host gate'
    echo
    echo "- compiler: $($CC --version 2>/dev/null | head -n 1)"
    echo "- object: $OBJ"
    echo "- text bytes: $TEXT_SIZE"
    echo '- undefined symbols: 0'
    echo '- hosted headers in core: 0'
    echo '- hosted calls in core: 0'
    echo '- deterministic twin-state test: PASS'
    echo '- result: PASS'
} > "$REPORT"

printf 'OMEGA FREESTANDING HOST PASS text=%s report=%s\n' "$TEXT_SIZE" "$REPORT"
