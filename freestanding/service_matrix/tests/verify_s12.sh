#!/bin/sh
# TOOLING_TEST only: hosted sh, nm, readelf, compiler; never L0 runtime.
set -eu
self=$(CDPATH='' cd -- "$(dirname "$0")/.." && pwd)
cc=${CC:-clang}
command -v "$cc" >/dev/null
command -v nm >/dev/null
command -v readelf >/dev/null
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT HUP INT TERM
common='-std=c11 -Os -ffreestanding -fno-builtin -fno-stack-protector -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-exceptions -Wall -Wextra -Werror'
"$cc" $common -c "$self/raf_fs_probe.c" -o "$out/host.o"
if nm -u "$out/host.o" | grep -E '[A-Za-z_]'; then echo 'FAIL: undefined host symbol' >&2; exit 3; fi
"$cc" -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer "$self/tests/probe_test.c" "$self/raf_fs_probe.c" -o "$out/test"
"$out/test"
for item in 'armv7a-linux-gnueabi ARM' 'aarch64-linux-gnu AArch64'; do
  set -- $item
  tag=$1; expected=$2
  "$cc" -target "$tag" $common -c "$self/raf_fs_probe.c" -o "$out/$tag.o"
  if nm -u "$out/$tag.o" | grep -E '[A-Za-z_]'; then echo "FAIL: undefined $tag symbol" >&2; exit 4; fi
  readelf -h "$out/$tag.o" | grep -q "Machine:.*$expected"
  printf 'PASS target=%s (relocatable object; runtime NOT_RUN)\n' "$tag"
done
printf 'PASS L0 host+two ARM object profiles; runtime/device/CI proof NOT_ATTESTED\n'
"$cc" --version | sed -n '1p'
