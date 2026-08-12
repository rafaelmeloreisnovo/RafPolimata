#!/bin/sh
set -eu
SELF_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SRC="${1:-}"
BASE_OUT="${2:-$SELF_DIR/OMEGA_DEVICE_RUN}"
[ -n "$SRC" ] || { echo "usage: RUN_OMEGA_ON_DEVICE.sh /path/RAFAELIA_COMPLETE_v4.zip [out]" >&2; exit 1; }
"$SELF_DIR/MATERIALIZE_AND_VERIFY.sh" "$SRC" "$BASE_OUT/source"
exec "$BASE_OUT/source/omega_final_gate.sh" "$BASE_OUT/receipt"
