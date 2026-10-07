#!/bin/sh
set -eu

CC=${CC:-clang}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/freestanding/tests/vector_probe.c"
META="$ROOT/freestanding/tests/register_metadata_probe.c"
OUT=${TMPDIR:-/tmp}/rafaelia-fs-mve
mkdir -p "$OUT"

NM_TOOL=$(command -v llvm-nm || command -v nm)
test -n "$NM_TOOL"

COMMON="-O2 -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer"
TARGET="armv8.1m.main-none-eabi"
FLAGS="-march=armv8.1-m.main+mve"

OBJ="$OUT/mve-vector.o"
ASM="$OUT/mve-vector.s"
META_OBJ="$OUT/mve-meta.o"

"$CC" -target "$TARGET" $FLAGS $COMMON -I"$ROOT/freestanding" -c "$SRC" -o "$OBJ"
"$CC" -target "$TARGET" $FLAGS $COMMON -I"$ROOT/freestanding" -S "$SRC" -o "$ASM"
"$CC" -target "$TARGET" $FLAGS $COMMON -I"$ROOT/freestanding" -c "$META" -o "$META_OBJ"

for obj in "$OBJ" "$META_OBJ"; do
    if "$NM_TOOL" -u "$obj" | grep -q .; then
        echo "FAIL: MVE object has unresolved helper(s): $obj" >&2
        "$NM_TOOL" -u "$obj" >&2
        exit 1
    fi
done

if grep -E -q '(^|[[:space:]])(bl|blx)[[:space:]]' "$ASM"; then
    echo "FAIL: MVE vector probe contains call instruction" >&2
    grep -E -n '(^|[[:space:]])(bl|blx)[[:space:]]' "$ASM" >&2
    exit 1
fi

if grep -E -q '(^|[[:space:]])(push|pop|vpush|vpop)[[:space:]]|\[sp|[[:space:]]sp[ ,]' "$ASM"; then
    echo "FAIL: MVE vector probe contains stack traffic" >&2
    grep -E -n '(^|[[:space:]])(push|pop|vpush|vpop)[[:space:]]|\[sp|[[:space:]]sp[ ,]' "$ASM" >&2
    exit 1
fi

for marker in     'vldrb\.u8[[:space:]]+q0'     'vstrb\.8[[:space:]]+q0'     'vand[[:space:]]+q1,[[:space:]]*q1,[[:space:]]*q0'     'vbic[[:space:]]+q2,[[:space:]]*q2,[[:space:]]*q0'     'vorr[[:space:]]+q3,[[:space:]]*q1,[[:space:]]*q2'     'vstrb\.8[[:space:]]+q3'
do
    if ! grep -E -q "$marker" "$ASM"; then
        echo "FAIL: MVE expected codegen marker missing: $marker" >&2
        exit 1
    fi
done

readelf -h "$OBJ" | grep -F 'Machine:' | grep -E 'ARM|Arm' >/dev/null

echo "RAFAELIA Arm MVE/Helium executor: PASS; Q0..Q3; 128-bit; helpers=0; stack=0; tail=0"
