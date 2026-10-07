#!/bin/sh
set -eu

CC=${CC:-clang}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/freestanding/tests/apx_probe.c"
OUT=${TMPDIR:-/tmp}/rafaelia-fs-apx
mkdir -p "$OUT"

NM_TOOL=$(command -v llvm-nm || command -v nm)
test -n "$NM_TOOL"

FLAGS="-target x86_64-unknown-none-elf -march=x86-64 -mapxf -O2 -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer"
OBJ="$OUT/apx.o"
ASM="$OUT/apx.s"
MACROS="$OUT/apx.macros"

printf '' | $CC -target x86_64-unknown-none-elf -mapxf -dM -E -x c - > "$MACROS"
grep -q '__APX_F__' "$MACROS"
grep -q '__EGPR__' "$MACROS"

$CC $FLAGS -I"$ROOT/freestanding" -c "$SRC" -o "$OBJ"
$CC $FLAGS -I"$ROOT/freestanding" -S "$SRC" -o "$ASM"

test -z "$("$NM_TOOL" -u "$OBJ")"

grep -E -q '%r16([^0-9]|$)' "$ASM"
grep -E -q '%r17([^0-9]|$)' "$ASM"
grep -E -q '%r31([^0-9]|$)' "$ASM"
grep -E -q 'addq[[:space:]]+%r17,[[:space:]]*%r16' "$ASM"

! grep -E -q '(^|[[:space:]])callq?[[:space:]]' "$ASM"
! grep -E -q '(^|[[:space:]])(pushq|popq)[[:space:]]|%rsp' "$ASM"

readelf -h "$OBJ" | grep -F 'Machine:' | grep -E 'X86-64|Advanced Micro Devices X86-64' >/dev/null

echo "RAFAELIA x86 APX EGPR profile: PASS; R16/R17/R31 present; helpers=0; stack=0"
