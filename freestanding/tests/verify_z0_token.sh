#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CC=${CC:-clang}
NM=${NM:-$(command -v llvm-nm || command -v nm)}
OUT=${TMPDIR:-/tmp}/raf-z0-presence-token-v1

command -v "$CC" >/dev/null 2>&1 || { echo "FAIL: compiler unavailable: $CC" >&2; exit 127; }
test -n "$NM" || { echo "FAIL: nm unavailable" >&2; exit 127; }

rm -rf "$OUT"
mkdir -p "$OUT"

COMMON="-std=c11 -O2 -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -nostdinc -I$ROOT/freestanding/include"

for target in     x86_64-unknown-none     i686-unknown-none     armv7a-none-eabi     aarch64-none-elf     riscv32-unknown-elf     riscv64-unknown-elf
do
    obj="$OUT/$target.o"
    "$CC" -target "$target" $COMMON -c "$ROOT/freestanding/tests/z0_probe.c" -o "$obj"
    undef=$("$NM" -u "$obj")
    if [ -n "$undef" ]; then
        echo "FAIL: unresolved helper(s) for $target" >&2
        printf '%s\n' "$undef" >&2
        exit 1
    fi
done

"$CC" -std=c11 -O2 -ffreestanding -fno-builtin     -I"$ROOT/freestanding/include"     "$ROOT/freestanding/tests/z0_selftest.c"     -o "$OUT/z0_selftest"
"$OUT/z0_selftest"

echo "RAFAELIA Z0 presence-token V1: PASS; states=ABSENT,EMPTY,SPACE,NUL,BYTE,SEQUENCE,INVALID; context=0; attention=0; learned_weights=0; 6/6 ISA objects; unresolved helpers=0"
