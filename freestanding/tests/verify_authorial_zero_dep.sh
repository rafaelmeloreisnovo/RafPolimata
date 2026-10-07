#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"

sh freestanding/tests/verify_contract.sh
sh tools/rafbbs/rafbbs_build.sh freestanding

PURE_FILES="
tools/rafbbs/rafbbs_types.h
tools/rafbbs/rafbbs_freestanding.h
tools/rafbbs/rafbbs_baremetal.h
tools/rafbbs/rafbbs_crc32_core.h
tools/rafbbs/rafbbs_sha256_core.h
"

fail=0
for file in $PURE_FILES; do
    if grep -n -E '^[[:space:]]*#include[[:space:]]*<' "$file"; then
        echo "FAIL: hosted/system header in authorial freestanding file: $file" >&2
        fail=1
    fi
    if grep -n -E '(^|[^A-Za-z0-9_])(malloc|calloc|realloc|free|system|fopen|fread|fwrite|fclose)[[:space:]]*\(' "$file"; then
        echo "FAIL: hosted/runtime primitive in authorial freestanding file: $file" >&2
        fail=1
    fi
done

[ "$fail" -eq 0 ] || exit 1
echo "RAFAELIA authorial zero-dependency V1: PASS"
