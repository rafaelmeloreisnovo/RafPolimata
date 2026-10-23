#!/bin/sh
# Host-only deterministic negative tests; NOT a hardware/ABI execution receipt.
set -eu
: "${CC:=cc}"
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT HUP INT TERM
sources="$root/RAF_029_spi_por_registrador_bcm.c $root/RAF_030_i2c_por_registrador_bcm.c $root/RAF_031_pwm_por_clock_manager.c $root/RAF_016_i2c_twi_com_timeout.c"
testsrc="$root/tests/hardware/test_mmio_failclosed.c"
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -I"$root" $sources "$testsrc" -o "$out/host"
"$out/host"
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -DRASPBERRYPI -DRAF_HW_POLL_LIMIT=128u -I"$root" $sources "$testsrc" -o "$out/mock"
"$out/mock"
printf 'PASS_HOST_MOCK_ONLY: typed negatives, bounded polling, no clock-pointer subtraction\n'
