#!/bin/sh
# Source/host negative tests only; no /dev/mem or physical GPIO accessed.
set -eu
: "${CC:=cc}"
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT HUP INT TERM
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -I"$root" \
  "$root/RAF_021_gpio_por_mmap.c" "$root/RAF_022_gpio_por_dev_gpiomem.c" \
  "$root/RAF_023_gpio_por_dev_mem_controlado.c" \
  "$root/tests/hardware/test_gpio_stubs.c" -o "$out/stubs"
"$out/stubs"
printf 'PASS_HOST_NEGATIVE_ONLY: unsupported host never reads/writes device MMIO\n'
