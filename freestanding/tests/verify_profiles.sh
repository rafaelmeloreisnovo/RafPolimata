#!/bin/sh
set -eu

CC=${CC:-clang}
NM=${NM:-nm}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/freestanding/tests/vector_probe.c"
OUT=${TMPDIR:-/tmp}/rafaelia-fs-profiles
mkdir -p "$OUT"

COMMON="-ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer -Os"

compile_profile() {
    name=$1
    target=$2
    flags=$3
    marker=$4
    obj="$OUT/$name.o"
    asm="$OUT/$name.s"

    "$CC" -target "$target" $COMMON $flags -I"$ROOT/freestanding" -c "$SRC" -o "$obj"
    "$CC" -target "$target" $COMMON $flags -I"$ROOT/freestanding" -S "$SRC" -o "$asm"

    if "$NM" -u "$obj" | grep -q .; then
        printf '%s\n' "FAIL: $name has unresolved external helper(s)"
        "$NM" -u "$obj"
        exit 1
    fi

    if grep -E -q '(^|[[:space:]])(call|callq|bl|blx)[[:space:]]' "$asm"; then
        printf '%s\n' "FAIL: $name contains external/internal call instruction"
        grep -E -n '(^|[[:space:]])(call|callq|bl|blx)[[:space:]]' "$asm"
        exit 1
    fi

    if grep -E -q '(%rsp|%rbp|[[:space:]]sp,|\[sp|push[lq]?[[:space:]]|pop[lq]?[[:space:]])' "$asm"; then
        printf '%s\n' "FAIL: $name probe contains stack traffic"
        grep -E -n '(%rsp|%rbp|[[:space:]]sp,|\[sp|push[lq]?[[:space:]]|pop[lq]?[[:space:]])' "$asm"
        exit 1
    fi

    if ! grep -q "$marker" "$asm"; then
        printf '%s\n' "FAIL: $name did not emit expected vector register class marker: $marker"
        exit 1
    fi

    printf '%s\n' "PASS: $name"
}

compile_profile x86_64-avx2 x86_64-linux-gnu "-mavx2 -mno-red-zone -mno-vzeroupper" 'ymm0'
compile_profile x86_64-avx512 x86_64-linux-gnu "-mavx512f -mno-red-zone -mno-vzeroupper" 'zmm0'
compile_profile armv7-neon armv7a-linux-gnueabihf "-march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=softfp" 'd0'
compile_profile aarch64-neon aarch64-linux-gnu "-march=armv8-a" 'q0'

printf '%s\n' "RAFAELIA fixed-vector profiles: 4/4 PASS"
