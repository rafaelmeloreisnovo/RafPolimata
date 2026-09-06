#!/bin/sh
set -eu

CC=${CC:-clang}
NM=${NM:-nm}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/freestanding/tests/scalable_probe.c"
OUT=${TMPDIR:-/tmp}/rafaelia-fs-scalable
mkdir -p "$OUT"
COMMON="-ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer -Os"

compile_scalable() {
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
    if grep -E -q '(%rsp|%rbp|[[:space:]]sp,|\[sp|push[lq]?[[:space:]]|pop[lq]?[[:space:]])' "$asm"; then
        printf '%s\n' "FAIL: $name contains stack traffic"
        grep -E -n '(%rsp|%rbp|[[:space:]]sp,|\[sp|push[lq]?[[:space:]]|pop[lq]?[[:space:]])' "$asm"
        exit 1
    fi
    if ! grep -q "$marker" "$asm"; then
        printf '%s\n' "FAIL: $name missing expected scalable-vector marker: $marker"
        exit 1
    fi
    printf '%s\n' "PASS: $name"
}

compile_scalable aarch64-sve aarch64-linux-gnu "-march=armv8.2-a+sve" 'whilelo'
compile_scalable rv32-v riscv32-linux-gnu "-march=rv32gcv -mabi=ilp32d" 'vsetvli'
compile_scalable rv64-v riscv64-linux-gnu "-march=rv64gcv -mabi=lp64d" 'vsetvli'

printf '%s\n' "RAFAELIA scalable-vector profiles: 3/3 PASS"
