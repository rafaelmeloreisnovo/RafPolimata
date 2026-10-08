#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CC=${CC:-cc}
CLANG=${CLANG:-clang}
NM=${NM:-nm}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM

"$CC" -std=c11 -Wall -Wextra -Werror -O2 -I"$ROOT/freestanding/include" \
    "$ROOT/freestanding/tests/raf_fs_evidence_gate_test.c" -o "$TMP/gate-test"
"$TMP/gate-test"
printf '%s\n' "PASS: 21 evidence/negative checks (hosted test harness only)"

for target in armv7a-none-eabi aarch64-none-elf; do
    "$CLANG" -target "$target" -std=c11 -ffreestanding -fno-builtin \
        -fno-stack-protector -nostdinc -O2 -I"$ROOT/freestanding/include" \
        -c "$ROOT/freestanding/tests/raf_fs_evidence_gate_test.c" -o "$TMP/$target.o"
    if "$NM" -u "$TMP/$target.o" | grep -q '[[:alnum:]_]'; then
        printf '%s\n' "FAIL: unresolved symbol in $target" >&2
        exit 1
    fi
    printf '%s\n' "PASS: $target freestanding object, no unresolved helpers"
done
printf '%s\n' "BOUNDARY: no device/runtime or independent scientific validation claimed"
