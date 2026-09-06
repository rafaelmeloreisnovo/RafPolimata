#!/bin/sh
set -eu

CC=${CC:-clang}
NM=${NM:-nm}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/freestanding/tests/probe.c"
OUT=${TMPDIR:-/tmp}/rafaelia-fs-matrix
mkdir -p "$OUT"

COMMON="-ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer -Os"

compile() {
    target=$1
    obj="$OUT/${target}.o"

    "$CC" -target "$target" $COMMON -I"$ROOT/freestanding" -c "$SRC" -o "$obj"

    # A freestanding object may export the deliberate probe symbol, but it must
    # not require memcpy/memset/compiler-rt/stack-check or any other helper.
    if "$NM" -u "$obj" | grep -q '[^[:space:]]'; then
        printf '%s\n' "FAIL: unresolved helper(s) in $target"
        "$NM" -u "$obj"
        exit 1
    fi

    if ! "$NM" -g --defined-only "$obj" | grep -Eq '[[:space:]][Tt][[:space:]]+raf_fs_compile_probe$'; then
        printf '%s\n' "FAIL: expected compile probe symbol missing in $target"
        "$NM" -g --defined-only "$obj"
        exit 1
    fi
}

compile x86_64-linux-gnu
compile i686-linux-gnu
compile armv7a-linux-gnueabihf
compile aarch64-linux-gnu
compile riscv32-linux-gnu
compile riscv64-linux-gnu

printf '%s\n' "RAFAELIA freestanding compile/helper matrix: 6/6 PASS; unresolved helpers=0"
