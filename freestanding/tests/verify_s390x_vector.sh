#!/bin/sh
set -eu

CC=${CC:-clang}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/freestanding/tests/vector_probe.c"
OUT=${TMPDIR:-/tmp}/rafaelia-fs-s390x-vector
mkdir -p "$OUT"

NM_TOOL=$(command -v llvm-nm || command -v nm)
test -n "$NM_TOOL"

COMMON="-O2 -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer"
OBJ="$OUT/s390x-z13-vector.o"
ASM="$OUT/s390x-z13-vector.s"

"$CC" -target s390x-unknown-none -march=z13 -mzvector $COMMON -I"$ROOT/freestanding" -c "$SRC" -o "$OBJ"
"$CC" -target s390x-unknown-none -march=z13 -mzvector $COMMON -I"$ROOT/freestanding" -S "$SRC" -o "$ASM"

if "$NM_TOOL" -u "$OBJ" | grep -q .; then
    echo "FAIL: s390x vector object has unresolved helper(s)" >&2
    "$NM_TOOL" -u "$OBJ" >&2
    exit 1
fi

if grep -E -q '(^|[[:space:]])(brasl|basr|balr)[[:space:]]' "$ASM"; then
    echo "FAIL: s390x vector probe contains call/branch-and-link instruction" >&2
    grep -E -n '(^|[[:space:]])(brasl|basr|balr)[[:space:]]' "$ASM" >&2
    exit 1
fi

if grep -E -q '%r15([^0-9]|$)' "$ASM"; then
    echo "FAIL: s390x vector probe contains stack-register traffic" >&2
    grep -E -n '%r15([^0-9]|$)' "$ASM" >&2
    exit 1
fi

for marker in     'vl[[:space:]]+%v16'     'vst[[:space:]]+%v16'     'vzero[[:space:]]+%v16'     'vsel[[:space:]]+%v19,[[:space:]]*%v18,[[:space:]]*%v17,[[:space:]]*%v16'     'vst[[:space:]]+%v19'
do
    if ! grep -E -q "$marker" "$ASM"; then
        echo "FAIL: s390x expected high-vector marker missing: $marker" >&2
        exit 1
    fi
done

if grep -E -q '%v([0-9]|1[0-5])([^0-9]|$)' "$ASM"; then
    echo "FAIL: s390x executor touched VR0..VR15, which overlap FPR0..FPR15" >&2
    grep -E -n '%v([0-9]|1[0-5])([^0-9]|$)' "$ASM" >&2
    exit 1
fi

readelf -h "$OBJ" | grep -F 'Machine:' | grep -E 'IBM S/390|S390' >/dev/null

echo "RAFAELIA s390x z13 high-vector executor: PASS; VR16..VR19 only; helpers=0; stack=0; tail=0"
