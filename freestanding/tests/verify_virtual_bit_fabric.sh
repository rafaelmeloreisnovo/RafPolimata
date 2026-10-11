#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CC=${CC:-cc}
CLANG=${CLANG:-clang}
NM=${NM:-nm}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
sh "$ROOT/freestanding/tests/verify_contract.sh"
for opt in 0 2 3; do
    "$CC" -std=c11 "-O$opt" -Wall -Wextra -Werror -pedantic \
        "$ROOT/freestanding/tests/raf_fs_virtual_bit_fabric_test.c" -o "$TMP/vbf-O$opt"
    "$TMP/vbf-O$opt"
    printf 'PASS: host model + watchdog at -O%s\n' "$opt"
done
"$CLANG" -std=c11 -O1 -g -Wall -Wextra -Werror \
    -fsanitize=undefined -fno-sanitize-recover=all \
    "$ROOT/freestanding/tests/raf_fs_virtual_bit_fabric_test.c" -o "$TMP/vbf-ubsan"
"$TMP/vbf-ubsan"
printf '%s\n' 'PASS: host UBSan model + watchdog'
for target in x86_64-unknown-none i686-unknown-none armv7a-none-eabi \
              aarch64-none-elf riscv32-unknown-elf riscv64-unknown-elf; do
    "$CLANG" -target "$target" -std=c11 -O2 -Wall -Wextra -Werror \
        -ffreestanding -fno-builtin -fno-stack-protector \
        -fno-unwind-tables -fno-asynchronous-unwind-tables \
        -nostdinc -I"$ROOT/freestanding/include" \
        -c "$ROOT/freestanding/tests/raf_fs_virtual_bit_fabric_probe.c" -o "$TMP/$target.o"
    "$NM" -u "$TMP/$target.o" > "$TMP/undef"
    if [ -s "$TMP/undef" ]; then
        printf 'FAIL: unresolved L0 symbols for %s\n' "$target" >&2
        cat "$TMP/undef" >&2
        exit 1
    fi
    printf 'PASS: %s zero unresolved symbols\n' "$target"
done
printf '%s\n' 'SCOPE: virtual bit-path model, not a CPU emulation or physical microarchitectural log'
