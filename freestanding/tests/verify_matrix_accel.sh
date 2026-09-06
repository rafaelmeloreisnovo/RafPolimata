#!/bin/sh
set -eu

CC=${CC:-clang}
NM=${NM:-nm}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/freestanding/tests/matrix_probe.c"
OUT=${TMPDIR:-/tmp}/rafaelia-fs-matrix-accel
mkdir -p "$OUT"
COMMON="-ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer -Os"

compile_matrix() {
    name=$1
    target=$2
    flags=$3
    marker=$4
    obj="$OUT/$name.o"
    asm="$OUT/$name.s"

    "$CC" -target "$target" $COMMON $flags -I"$ROOT/freestanding" -c "$SRC" -o "$obj"
    "$CC" -target "$target" $COMMON $flags -I"$ROOT/freestanding" -S "$SRC" -o "$asm"

    if "$NM" -u "$obj" | grep -q .; then
        printf '%s\n' "FAIL: $name has unresolved helper(s)"
        "$NM" -u "$obj"
        exit 1
    fi
    if grep -E -q '(^|[[:space:]])(call|callq|bl|blx)[[:space:]]' "$asm"; then
        printf '%s\n' "FAIL: $name contains call instruction"
        exit 1
    fi
    if grep -E -q '(%rsp|%rbp|%esp|%ebp|[[:space:]]sp,|\[sp|push[lq]?[[:space:]]|pop[lq]?[[:space:]])' "$asm"; then
        printf '%s\n' "FAIL: $name contains stack traffic"
        grep -E -n '(%rsp|%rbp|%esp|%ebp|[[:space:]]sp,|\[sp|push[lq]?[[:space:]]|pop[lq]?[[:space:]])' "$asm"
        exit 1
    fi
    if ! grep -E -q "$marker" "$asm"; then
        printf '%s\n' "FAIL: $name missing expected matrix-register instruction"
        exit 1
    fi
    printf '%s\n' "PASS: $name"
}

compile_matrix x86_64-amx x86_64-unknown-none "-mamx-tile -mno-red-zone" 'tilezero.*%?tmm0'
compile_matrix aarch64-sme aarch64-none-elf "-march=armv9-a+sme" 'zero[[:space:]]+\{za\}'

printf '%s\n' "RAFAELIA OS-neutral matrix-register profiles: 2/2 PASS"
