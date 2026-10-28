#!/bin/sh
# HOST ONLY. Reject missing compiler tools, keep source under caller control.
# L0 headers do not depend on shell/compiler/runtime at execution time.
set -eu
D=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
: "${CC:=cc}" "${CXX:=c++}" "${CLANG:=clang}" "${NM:=nm}"
for TOOL in "$CC" "$CXX" "$CLANG" "$NM"; do
  command -v "$TOOL" >/dev/null 2>&1 || { echo "TOKEN_VAZIO_TOOLCHAIN_UNAVAILABLE=$TOOL"; exit 77; }
done
TMP=$(mktemp -d "${TMPDIR:-/tmp}/rafci-v43.XXXXXXXX")
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
for TEST in test_gate_v4.c test_gate_v43.c test_gate_v43_fuzz.c; do
  "$CC" -std=c11 -Os -Wall -Wextra -Werror -pedantic -I"$D" "$D/$TEST" -o "$TMP/$TEST"
  "$TMP/$TEST"
done
cat > "$TMP/cpp.cpp" <<'END'
#include "raf_ci_gate_v43.h"
#include "raf_ci_gate_v4_wire.h"
static_assert(sizeof(raf_ci_gate_v4)==24,"ABI drift");
int main(){const unsigned char p[]="on: push\njobs:\n  ok:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo yes\n";
 unsigned int why=9u;raf_ci_gate_v4 g=raf_ci_gate_v43_check(p,sizeof(p)-1u,&why);
 return g.state==RAF_CI_G4_REVIEW && why==0u?0:1;}
END
"$CXX" -std=c++17 -Os -Wall -Wextra -Werror -pedantic -I"$D" "$TMP/cpp.cpp" -o "$TMP/cpp"
"$TMP/cpp"
TARGETS=${RAF_TARGETS:-"x86_64-unknown-none i686-unknown-none armv7a-none-eabi aarch64-none-elf riscv32-unknown-elf riscv64-unknown-elf"}
for TARGET in $TARGETS; do
  for TEST in test_gate_v43.c test_gate_v43_fuzz.c; do
    "$CLANG" -target "$TARGET" -std=c11 -Os -Wall -Wextra -Werror -ffreestanding -fno-builtin -fno-stack-protector -nostdinc -I"$D" -c "$D/$TEST" -o "$TMP/$TARGET-$TEST.o"
    if [ -n "$("$NM" -u "$TMP/$TARGET-$TEST.o")" ]; then
      echo "FAIL_UNDEFINED_SYMBOLS=$TARGET-$TEST"; exit 1
    fi
  done
  echo "PASS_FREESTANDING_OBJECTS=$TARGET"
done
if [ "${RAF_SANITIZE:-0}" = "1" ]; then
  "$CLANG" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I"$D" "$D/test_gate_v43_fuzz.c" -o "$TMP/san"
  "$TMP/san"
  echo PASS_ASAN_UBSAN_10000
else
  echo TOKEN_VAZIO_SANITIZERS_NOT_RUN
fi
echo 'PASS_LOCAL_SCOPED TARGETS_CHECKED='$TARGETS' PROVIDER_CI=NOT_RUN DEVICE=NOT_RUN FULL_YAML=UNVERIFIED PROMOTION=HOLD claim_allowed=false'
