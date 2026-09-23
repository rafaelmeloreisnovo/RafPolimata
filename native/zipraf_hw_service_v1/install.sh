#!/bin/sh
set -eu
PREFIX=${PREFIX:-"$HOME/.local"}
CC=${CC:-cc}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BIN="$PREFIX/bin"
mkdir -p "$BIN"
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -I"$ROOT/include" \
  "$ROOT/c/zipraf_hw_service.c" "$ROOT/bbs/zipraf_hw_bbs.c" -o "$BIN/zipraf-hw"
printf '\033[32mZIPRAF-HW installed:\033[0m %s\n' "$BIN/zipraf-hw"
printf 'No root, no network fetch, no ambient permission escalation.\n'
