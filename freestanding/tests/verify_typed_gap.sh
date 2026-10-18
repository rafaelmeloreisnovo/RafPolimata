#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
CC=${CC:-clang}
NM=${NM:-nm}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
for command in "$CC" "$NM"; do command -v "$command" >/dev/null 2>&1 || exit 127; done
sh "$ROOT/freestanding/tests/verify_contract.sh"
SOURCE="$ROOT/freestanding/tests/raf_fs_typed_gap_test.c"
for opt in 0 2 3; do
    "$CC" -std=c11 -Wall -Wextra -Werror "-O$opt" "$SOURCE" -o "$TMP/typed-gap-$opt"
    "$TMP/typed-gap-$opt"
done
printf '%s\n' 'PASS: typed gap 33 checks at O0/O2/O3 (host)'
for target in x86_64-unknown-none i686-unknown-none armv7a-none-eabi aarch64-none-elf riscv32-unknown-elf riscv64-unknown-elf; do
    "$CC" -target "$target" -std=c11 -Wall -Wextra -Werror -O2       -ffreestanding -fno-builtin -fno-stack-protector -nostdinc       -c "$SOURCE" -o "$TMP/$target.o"
    "$NM" -u "$TMP/$target.o" > "$TMP/$target.undefined"
    test ! -s "$TMP/$target.undefined" || { cat "$TMP/$target.undefined"; exit 1; }
    printf 'PASS: %s unresolved=0\n' "$target"
done
printf '%s\n' 'EVIDENCE: compile+hosted oracle only, not hardware/provider/rights claim'
