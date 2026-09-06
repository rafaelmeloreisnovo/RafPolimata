#!/bin/sh
set -eu

CC=${CC:-clang}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/syscall/tests/probe.c"
OUT=${TMPDIR:-/tmp}/rafaelia-syscall-matrix
mkdir -p "$OUT"

COMMON="-ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer -Os"

compile() {
    target=$1
    "$CC" -target "$target" $COMMON -I"$ROOT" -c "$SRC" -o "$OUT/${target}.o"
}

compile x86_64-linux-gnu
compile i686-linux-gnu
compile armv7a-linux-gnueabihf
compile aarch64-linux-gnu
compile riscv32-linux-gnu
compile riscv64-linux-gnu

printf '%s\n' "RAFAELIA raw Linux syscall compile matrix: 6/6 PASS"
