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

require_file_contract() {
    file=$1
    for marker in \
        'RAFAELIA-L0-FILE-CONTRACT' \
        'PURPOSE:' \
        'SCOPE:' \
        'PRECONDITIONS:' \
        'REGISTER_OWNERSHIP:' \
        'CLOBBERS:' \
        'MEMORY_ORDER:' \
        'TAIL_SHADOW:' \
        'EVIDENCE:'
    do
        if ! grep -q "$marker" "$file"; then
            printf '%s\n' "FAIL: missing $marker in ${file#$ROOT/}"
            fail=1
        fi
    done
}

check_pattern "hosted header leaked into L0" '^[[:space:]]*#include[[:space:]]*<' 
check_pattern "dynamic allocator leaked into L0" '(^|[^A-Za-z0-9_])(malloc|calloc|realloc|free)[[:space:]]*\('
check_pattern "syscall instruction leaked into L0" '(^|[^A-Za-z0-9_])(syscall|ecall|svc[[:space:]]*#?0|int[[:space:]]+\$0x80)([^A-Za-z0-9_]|$)'
check_pattern "synthetic do/while macro shell leaked into L0" '^[[:space:]]*#define.*\bdo[[:space:]]*\{'
check_pattern "synthetic while(0) leaked into L0" '^[[:space:]]*\}[[:space:]]*while[[:space:]]*\([[:space:]]*0[[:space:]]*\)'
check_pattern "hosted fallback annotation leaked into canonical L0" 'RAF_FS_HOSTED_FALLBACK'
check_pattern "weak production fallback leaked into canonical L0" '__attribute__[[:space:]]*\(\([[:space:]]*weak[[:space:]]*\)\)'

for file in "$ROOT"/freestanding/include/*.h "$ROOT"/freestanding/arch/*.h; do
    [ -f "$file" ] || continue
    require_file_contract "$file"
done

if [ ! -f "$ROOT/freestanding/gaps.v1.json" ]; then
    printf '%s\n' "FAIL: missing freestanding/gaps.v1.json"
    fail=1
else
    if ! grep -q 'CLOSURE_L11' "$ROOT/freestanding/gaps.v1.json"; then
        printf '%s\n' "FAIL: implementation gaps are not bound to CLOSURE_L11"
        fail=1
    fi
    if ! grep -q 'CLOSURE_L12' "$ROOT/freestanding/gaps.v1.json"; then
        printf '%s\n' "FAIL: runtime/device gaps are not bound to CLOSURE_L12"
        fail=1
    fi
fi

if [ "$fail" -ne 0 ]; then
    exit 1
fi

printf '%s\n' "RAFAELIA freestanding contract: PASS"
