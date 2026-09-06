#!/bin/sh
set -eu

CC=${CC:-clang}
NM=${NM:-nm}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
SRC="$ROOT/freestanding/tests/register_metadata_probe.c"
OUT=${TMPDIR:-/tmp}/rafaelia-fs-register-metadata
mkdir -p "$OUT"
COMMON="-ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fdata-sections -Os"

compile_metadata() {
    name=$1
    target=$2
    obj="$OUT/$name.o"

    "$CC" -target "$target" $COMMON -I"$ROOT/freestanding" -c "$SRC" -o "$obj"
    if "$NM" -u "$obj" | grep -q .; then
        printf '%s\n' "FAIL: $name metadata object has unresolved helper(s)"
        "$NM" -u "$obj"
        exit 1
    fi
    if ! "$NM" -g --defined-only "$obj" | grep -q 'raf_fs_register_metadata_probe'; then
        printf '%s\n' "FAIL: $name metadata symbol missing"
        exit 1
    fi
    printf '%s\n' "PASS: $name metadata"
}

compile_metadata power64 powerpc64le-unknown-none
compile_metadata loongarch64 loongarch64-unknown-none
compile_metadata s390x s390x-unknown-none

printf '%s\n' "RAFAELIA additional OS-neutral register metadata: 3/3 PASS"
