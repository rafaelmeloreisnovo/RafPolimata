#include "raf_runtime_router.h"
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
    for (i = 0u; i < RAF_BENCH_PREWARM; i++) out = raf_runtime_route(in);
    for (i = 0u; i < RAF_BENCH_WARMUP; i++) out = raf_runtime_route(in);
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
