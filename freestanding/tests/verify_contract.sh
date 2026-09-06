#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CODE="$ROOT/freestanding/include $ROOT/freestanding/arch"

fail=0

check_pattern() {
    label=$1
    pattern=$2
    if grep -R -n -E "$pattern" $CODE; then
        printf '%s\n' "FAIL: $label"
        fail=1
    fi
}

check_pattern "hosted header leaked into L0" '^[[:space:]]*#include[[:space:]]*<' 
check_pattern "dynamic allocator leaked into L0" '(^|[^A-Za-z0-9_])(malloc|calloc|realloc|free)[[:space:]]*\('
check_pattern "syscall instruction leaked into L0" '(^|[^A-Za-z0-9_])(syscall|ecall|svc[[:space:]]*#?0|int[[:space:]]+\$0x80)([^A-Za-z0-9_]|$)'
check_pattern "synthetic do/while macro shell leaked into L0" '^[[:space:]]*#define.*\bdo[[:space:]]*\{'
check_pattern "synthetic while(0) leaked into L0" '^[[:space:]]*\}[[:space:]]*while[[:space:]]*\([[:space:]]*0[[:space:]]*\)'

if [ "$fail" -ne 0 ]; then
    exit 1
fi

printf '%s\n' "RAFAELIA freestanding contract: PASS"
