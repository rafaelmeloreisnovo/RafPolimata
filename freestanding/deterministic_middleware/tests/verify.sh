#!/bin/sh
# Hosted TOOLING_TEST: no shell/clang/nm/readelf in PURE_CORE.
set -eu
self=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT HUP INT TERM
cc=${CC:-clang}
common='-std=c11 -Os -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-exceptions -Wall -Wextra -Werror'
"$cc" -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer "$self/raf_mw.c" "$self/tests/test_route.c" -o "$out/scoped_test"
"$out/scoped_test"
for spec in 'x86_64-unknown-linux-gnu X86-64' 'armv7a-linux-gnueabi ARM' 'aarch64-linux-gnu AArch64'; do
  set -- $spec
  target=$1;machine=$2
  "$cc" -target "$target" $common -c "$self/raf_mw.c" -o "$out/$target.o"
  nm -u "$out/$target.o" > "$out/$target.undefined"
  test ! -s "$out/$target.undefined"
  readelf -h "$out/$target.o" | grep -q "Machine:.*$machine"
  echo "PASS OBJECT: $target no undefined external symbols (NOT executed on target)"
done
printf 'PASS host fixtures + 3 target objects; no runtime/device/GPU/NUMA claim\n'
