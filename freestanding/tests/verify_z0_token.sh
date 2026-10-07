#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
TMP=${TMPDIR:-/tmp}/raf-z0-token-$$
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
mkdir -p "$TMP"

CC=${CC:-clang}
NM_TOOL=$(command -v llvm-nm || command -v nm)
CFLAGS='-O2 -nostdinc -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident'

$CC $CFLAGS -Wall -Wextra -Werror -c "$ROOT/freestanding/tests/raf_z0_probe.c" -o "$TMP/z0-host.o"
test -z "$($NM_TOOL -u "$TMP/z0-host.o")"

$CC -O2 -Wall -Wextra -Werror "$ROOT/freestanding/tests/raf_z0_token_test.c" -o "$TMP/z0-semantic"
"$TMP/z0-semantic"

compile_target() {
    name=$1
    shift
    $CC "$@" $CFLAGS -Wall -Wextra -Werror -c "$ROOT/freestanding/tests/raf_z0_probe.c" -o "$TMP/z0-$name.o"
    test -z "$($NM_TOOL -u "$TMP/z0-$name.o")"
}

compile_target x86_64 --target=x86_64-unknown-none-elf
compile_target i686 --target=i686-unknown-none-elf
compile_target armv7 --target=armv7a-none-eabi -march=armv7-a
compile_target aarch64 --target=aarch64-none-elf
compile_target rv32 --target=riscv32-unknown-elf -march=rv32im -mabi=ilp32
compile_target rv64 --target=riscv64-unknown-elf -march=rv64im -mabi=lp64

printf '%s\n' 'RAFAELIA Z0 contextless tokenization: PASS'
