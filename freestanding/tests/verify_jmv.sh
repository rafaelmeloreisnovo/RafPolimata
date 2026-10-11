#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CC=${CC:-cc}
CLANG=${CLANG:-clang}
NM=${NM:-nm}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM

sh "$ROOT/freestanding/tests/verify_contract.sh"
"$CC" -std=c11 -Wall -Wextra -Werror -O2 \
  -I"$ROOT/freestanding/include" "$ROOT/freestanding/tests/raf_fs_jmv_test.c" -o "$TMP/jmv-test"
"$TMP/jmv-test"
printf '%s\n' 'PASS: JMV host synthetic positive/negative and saturation cases'

for target in x86_64-unknown-none i686-unknown-none armv7a-none-eabi \
              aarch64-none-elf riscv32-unknown-elf riscv64-unknown-elf; do
  "$CLANG" -target "$target" -std=c11 -Wall -Wextra -Werror \
    -O2 -ffreestanding -fno-builtin -fno-stack-protector \
    -fno-unwind-tables -fno-asynchronous-unwind-tables \
    -nostdinc -I"$ROOT/freestanding/include" \
    -c "$ROOT/freestanding/tests/raf_fs_jmv_test.c" -o "$TMP/$target.o"
  "$NM" -u "$TMP/$target.o" > "$TMP/unresolved"
  if [ -s "$TMP/unresolved" ]; then
    printf '%s\n' "FAIL: external symbol for $target" >&2
    cat "$TMP/unresolved" >&2
    exit 1
  fi
  printf 'PASS: %s no undefined symbols\n' "$target"
done
printf '%s\n' 'BOUNDARY: CI is not JVM compatibility, physical runtime or independent evidence'
