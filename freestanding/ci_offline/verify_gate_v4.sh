#!/bin/sh
# HOST ONLY. No GitHub workflow executes; no source YAML command is evaluated.
set -eu
D=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
: "${CC:=cc}" "${CLANG:=clang}" "${CXX:=c++}" "${NM:=nm}"
for T in "$CC" "$CLANG" "$CXX" "$NM"; do
  command -v "$T" >/dev/null 2>&1 || { echo "TOKEN_VAZIO_TOOLCHAIN_UNAVAILABLE=$T"; exit 77; }
done
TMP=$(mktemp -d "${TMPDIR:-/tmp}/rafci-g4.XXXXXXXX")
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
for SRC in test_gate_v4.c test_gate_v4_fuzz.c; do
  "$CC" -std=c11 -O2 -Wall -Wextra -Werror -pedantic -I"$D" "$D/$SRC" -o "$TMP/$SRC"
  "$TMP/$SRC"
done
printf '#include "raf_ci_gate_v4.h"\nint main(){return sizeof(raf_ci_gate_v4)==24u?0:1;}\n' > "$TMP/cpp.cpp"
"$CXX" -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -I"$D" "$TMP/cpp.cpp" -o "$TMP/cpp"
"$TMP/cpp"
for TARGET in x86_64-unknown-none i686-unknown-none armv7a-none-eabi aarch64-none-elf riscv32-unknown-elf riscv64-unknown-elf; do
  for SRC in test_gate_v4.c test_gate_v4_fuzz.c; do
    "$CLANG" -target "$TARGET" -std=c11 -Os -Wall -Wextra -Werror -ffreestanding -fno-builtin -fno-stack-protector -nostdinc -I"$D" -c "$D/$SRC" -o "$TMP/$TARGET-$SRC.o"
    [ -z "$("$NM" -u "$TMP/$TARGET-$SRC.o")" ] || { echo "FAIL_UNDEFINED=$TARGET/$SRC"; exit 1; }
  done
  echo "PASS_OBJECT_NO_UNDEFINED=$TARGET"
done
if [ "${RAF_SANITIZE:-0}" = 1 ]; then
  "$CLANG" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I"$D" "$D/test_gate_v4_fuzz.c" -o "$TMP/san"
  "$TMP/san"
  echo PASS_ASAN_UBSAN_10000
else
  echo TOKEN_VAZIO_SANITIZER_NOT_RUN
fi
echo "PASS_LOCAL_SCOPED GITHUB_CI=NOT_RUN DEVICE=NOT_RUN FULL_YAML=UNVERIFIED CLAIM_ALLOWED=false PROMOTION=HOLD"
