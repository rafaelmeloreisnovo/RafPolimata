#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CC=cc
CLANG=clang
NM=nm
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
sh "$ROOT/freestanding/tests/verify_contract.sh"
"$CC" -std=c11 -O2 -Wall -Wextra -Werror \
    -I"$ROOT/freestanding/include" \
    "$ROOT/freestanding/tests/raf_fs_formats_numa_test.c" -o "$TMP/test"
"$TMP/test"
printf '%s\n' 'PASS: synthetic ELF/DEX/NUMA positive and adversarial cases'
"$CLANG" -std=c11 -O1 -g -Wall -Wextra -Werror \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    "$ROOT/freestanding/tests/raf_fs_formats_numa_test.c" -o "$TMP/sanitized"
"$TMP/sanitized"
printf '%s\n' 'PASS: host ASan/UBSan synthetic ELF/DEX/NUMA'
for target in x86_64-unknown-none i686-unknown-none armv7a-none-eabi \
              aarch64-none-elf riscv32-unknown-elf riscv64-unknown-elf; do
    "$CLANG" -target "$target" -std=c11 -O2 -Wall -Wextra -Werror \
        -ffreestanding -fno-builtin -fno-stack-protector -nostdinc \
        -I"$ROOT/freestanding/include" -c \
        "$ROOT/freestanding/tests/raf_fs_formats_numa_test.c" \
        -o "$TMP/$target.o"
    "$NM" -u "$TMP/$target.o" > "$TMP/undef"
    if [ -s "$TMP/undef" ]; then
        printf 'FAIL: unresolved helper on %s\n' "$target" >&2
        cat "$TMP/undef" >&2
        exit 1
    fi
    printf 'PASS: freestanding no-undefined %s\n' "$target"
done
printf '%s\n' 'SCOPE: structural inspection / advisory placement only; no ELF load, DEX VM, NUMA OS binding'
