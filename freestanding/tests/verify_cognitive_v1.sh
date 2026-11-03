#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
CC=${CC:-clang}
NM=${NM:-nm}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
for tool in "$CC" "$NM"; do command -v "$tool" >/dev/null 2>&1 || exit 127; done
SOURCE="$ROOT/freestanding/tests/raf_fs_cognitive_v1_test.c"
PROBE="$ROOT/freestanding/tests/raf_fs_cognitive_v1_probe.c"
for opt in 0 2 3; do
    "$CC" -std=c11 -Wall -Wextra -Werror "-O$opt" "$SOURCE" -o "$TMP/cognitive-$opt"
    "$TMP/cognitive-$opt"
done
printf '%s\n' 'PASS: 37 falsifiers (1000 cell roundtrips) at host O0/O2/O3'
for target in x86_64-unknown-none i686-unknown-none armv7a-none-eabi aarch64-none-elf riscv32-unknown-elf riscv64-unknown-elf; do
    "$CC" -target "$target" -std=c11 -Wall -Wextra -Werror -O2 -ffreestanding -fno-builtin -fno-stack-protector -nostdinc -c "$PROBE" -o "$TMP/$target.o"
    "$NM" -u "$TMP/$target.o" > "$TMP/$target.undefined"
    test ! -s "$TMP/$target.undefined" || { cat "$TMP/$target.undefined"; exit 1; }
    printf 'PASS: %s unresolved=0\n' "$target"
done
printf '%s\n' 'BOUNDARY: no Supabase I/O; device/provider/authenticity E2E TOKEN_VAZIO'
