#!/bin/sh
set -eu
# Governance: runtime/device evidence gaps are bound to CLOSURE_L12.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
fail=0

die() {
    printf '%s\n' "FAIL: $*" >&2
    fail=1
}

must_have() {
    file=$1
    pattern=$2
    label=$3
    grep -Fq "$pattern" "$ROOT/$file" || die "$label ($file)"
}

must_not_have() {
    file=$1
    pattern=$2
    label=$3
    if grep -Fq "$pattern" "$ROOT/$file"; then
        die "$label ($file)"
    fi
}

must_have "Benchmark/raf_sys.h" "NOT physical bare-metal" "raw syscall layer must declare its OS boundary"
must_not_have "Benchmark/raf_sys.h" "syscall bare-metal" "raw syscall adapter must not be labeled bare-metal"
must_have "Benchmark/raf_main.c" "RAFAELIA ENTERPRISE NO-LIBC USERSPACE" "runtime banner must identify userspace"
must_not_have "Benchmark/raf_main.c" "ENTERPRISE BARE-METAL" "userspace runtime must not claim physical bare-metal"
must_not_have "Benchmark/raf_main.c" "tsc=PMCCNTR" "ARM32 report must match clock_gettime implementation"
must_not_have "Benchmark/raf_bench.h" "ARM32: PMCCNTR" "benchmark timer documentation must match implementation"
must_have "Benchmark/raf_main.c" "physical bare-metal | TOKEN_VAZIO" "physical firmware evidence must remain explicit"
must_have "docs/canonical/2026-09-23/RUNTIME_AND_TARGETS.md" "RAW_SYSCALL_USERSPACE" "canonical runtime taxonomy missing"

native_a32_count=$(grep -Ec -- '-o[[:space:]]+raf_enterprise_a32([[:space:]]|$)' "$ROOT/Benchmark/build2.sh" || true)
if [ "$native_a32_count" -ne 1 ]; then
    die "Benchmark/build2.sh must define exactly one native raf_enterprise_a32 output; found $native_a32_count"
fi

if [ "$fail" -ne 0 ]; then
    exit 1
fi

printf '%s\n' "RAFAELIA execution boundary audit: PASS"
printf '%s\n' "FREESTANDING_L0 != RAW_SYSCALL_USERSPACE != HOSTED != PHYSICAL_BARE_METAL"
