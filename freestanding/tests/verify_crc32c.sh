#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
HOST_CC=${CC:-cc}
for tool in "$HOST_CC" clang nm; do
    command -v "$tool" >/dev/null 2>&1 || {
        printf 'CRC32C gate: missing tool %s\n' "$tool" >&2
        exit 127
    }
done

"$HOST_CC" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
    -I"$ROOT/freestanding/include" \
    "$ROOT/freestanding/tests/raf_fs_crc32c_test.c" -o "$TMP/crc32c-host"
"$TMP/crc32c-host"

for target in x86_64-unknown-none i386-unknown-none armv7-none-eabi \
              aarch64-none-elf riscv32-unknown-elf riscv64-unknown-elf
do
    clang -target "$target" -std=c11 -O2 -ffreestanding -fno-builtin \
        -fno-stack-protector -nostdinc -I"$ROOT/freestanding/include" \
        -c "$ROOT/freestanding/tests/raf_fs_crc32c_test.c" \
        -o "$TMP/$target.o"
    undef=$(nm -u "$TMP/$target.o")
    if [ -n "$undef" ]; then
        printf 'CRC32C gate: unresolved symbols for %s: %s\n' "$target" "$undef" >&2
        exit 1
    fi
done

printf '%s\n' 'RAFAELIA CRC32C L0: PASS host KAT/chunk/bounds + six-ISA no-undefined compile'
