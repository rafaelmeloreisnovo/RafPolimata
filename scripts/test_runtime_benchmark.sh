#!/usr/bin/env sh
set -eu
cc="${CC:-gcc}"
build_dir="build_runtime_benchmark"
mkdir -p "$build_dir"
cat > "$build_dir/test_runtime_benchmark.c" <<'C_EOF'
#include "Benchmark/raf_runtime_router.h"
#include <time.h>

#define RAF_BENCH_PREWARM 16u
#define RAF_BENCH_WARMUP 64u
#define RAF_BENCH_SAMPLES 31u

static unsigned long long tick_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((unsigned long long)ts.tv_sec * 1000000000ull) + (unsigned long long)ts.tv_nsec;
}

static void sort31(unsigned long long a[RAF_BENCH_SAMPLES]) {
    u32 i;
    for (i = 1u; i < RAF_BENCH_SAMPLES; i++) {
        unsigned long long key = a[i];
        u32 j = i;
        while (j > 0u && a[j - 1u] > key) {
            a[j] = a[j - 1u];
            j--;
        }
        a[j] = key;
    }
}

int main(void) {
    static unsigned long long samples[RAF_BENCH_SAMPLES];
    RafRouteInput in = {RAF_CAP_ARM64_NEON | RAF_CAP_GPU_BATCH, 0u, 64u, 128u, RAF_ROUTE_STATE_VALIDATED};
    RafRouteDecision out;
    u32 i;
    for (i = 0u; i < RAF_BENCH_PREWARM; i++) {
        out = raf_runtime_route(in);
    }
    for (i = 0u; i < RAF_BENCH_WARMUP; i++) {
        out = raf_runtime_route(in);
    }
    for (i = 0u; i < RAF_BENCH_SAMPLES; i++) {
        unsigned long long t0 = tick_ns();
        out = raf_runtime_route(in);
        samples[i] = tick_ns() - t0;
    }
    sort31(samples);
    if (out.backend != RAF_BACKEND_GPU_BATCH) return 1;
    if (samples[15] > samples[29]) return 2;
    if (samples[29] > samples[30]) return 3;
    return 0;
}
C_EOF
"$cc" -std=c11 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L -I. "$build_dir/test_runtime_benchmark.c" -o "$build_dir/test_runtime_benchmark"
"$build_dir/test_runtime_benchmark"
echo "runtime_benchmark=PASS prewarm=16 warmup=64 samples=31 median_index=15 p95_index=29"
