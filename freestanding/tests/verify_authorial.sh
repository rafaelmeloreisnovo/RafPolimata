#!/bin/sh
set -eu

CC=${CC:-clang}
NM=${NM:-nm}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/freestanding/tests/authorial_probe.c"
HDR="$ROOT/freestanding/include/raf_fs_authorial.h"
OUT=${TMPDIR:-/tmp}/rafaelia-fs-authorial
mkdir -p "$OUT"

COMMON="-std=c99 -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer -Os -nostdinc"

fail_source() {
    label=$1
    pattern=$2
    if grep -n -E "$pattern" "$HDR" "$SRC"; then
        printf '%s\n' "FAIL: $label"
        exit 1
    fi
}

compile() {
    target=$1
    obj="$OUT/${target}.o"

    "$CC" -target "$target" $COMMON -I"$ROOT/freestanding" -c "$SRC" -o "$obj"

    if "$NM" -u "$obj" | grep -q '[^[:space:]]'; then
        printf '%s\n' "FAIL: unresolved helper(s) in authorial $target"
        "$NM" -u "$obj"
        exit 1
    fi

    if ! "$NM" -g --defined-only "$obj" | grep -Eq '[[:space:]][Tt][[:space:]]+raf_fs_authorial_probe$'; then
        printf '%s\n' "FAIL: expected authorial probe symbol missing in $target"
        "$NM" -g --defined-only "$obj"
        exit 1
    fi
}

fail_source "hosted header leaked into authorial L0" '^[[:space:]]*#include[[:space:]]*<'
fail_source "allocator leaked into authorial L0" '(^|[^A-Za-z0-9_])(malloc|calloc|realloc|free)[[:space:]]*\('
fail_source "syscall instruction leaked into authorial L0" '"(syscall|ecall|svc[[:space:]]*#?0|int[[:space:]]+\$0x80)"'
fail_source "assembly leaked into authorial L0" '(^|[^A-Za-z0-9_])(__asm__|asm)[[:space:]]*'

compile x86_64-unknown-none
compile i686-unknown-none
compile armv7a-none-eabi
compile aarch64-none-elf
compile riscv32-unknown-elf
compile riscv64-unknown-elf

printf '%s\n' "RAFAELIA authorial freestanding item gate: 6/6 PASS; external deps=0; unresolved helpers=0"
