#!/bin/sh
set -eu

CC=${CC:-clang}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/freestanding/tests/vector_probe.c"
OUT=${TMPDIR:-/tmp}/rafaelia-fs-power-vsx
mkdir -p "$OUT"

NM_TOOL=$(command -v llvm-nm || command -v nm)
test -n "$NM_TOOL"

FLAGS="-target powerpc64le-unknown-none -mcpu=power8 -mvsx -maltivec -O2 -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer"
OBJ="$OUT/power-vsx.o"
ASM="$OUT/power-vsx.s"

$CC $FLAGS -I"$ROOT/freestanding" -c "$SRC" -o "$OBJ"
$CC $FLAGS -I"$ROOT/freestanding" -S "$SRC" -o "$ASM"

test -z "$("$NM_TOOL" -u "$OBJ")"

for marker in   'lvx[[:space:]]+0,'   'stvx[[:space:]]+0,'   'vand[[:space:]]+1,[[:space:]]*1,[[:space:]]*0'   'vandc[[:space:]]+2,[[:space:]]*2,[[:space:]]*0'   'vor[[:space:]]+3,[[:space:]]*1,[[:space:]]*2'   'vxor[[:space:]]+0,[[:space:]]*0,[[:space:]]*0'
do
  grep -E -q "$marker" "$ASM"
done

! grep -E -q '(^|[[:space:]])(bl|bla|bctrl|blrl)([[:space:]]|$)' "$ASM"
! grep -E -q 'addi[[:space:]]+1,[[:space:]]*1,' "$ASM"

readelf -h "$OBJ" | grep -F 'Machine:' | grep -E 'PowerPC|Power' >/dev/null

echo "RAFAELIA POWER64 VSX/VMX slice: PASS; VR0..VR3=VSR32..35; helpers=0; stack=0; tail=0"
