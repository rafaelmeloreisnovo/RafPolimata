#include "raf_bus_throughput.h"
#include <stdio.h>
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

static void report_mbps(const char *label, unsigned long long samples_ns[RAF_BENCH_SAMPLES], u32 bytes) {
    sort31(samples_ns);
    double mb = (double)bytes / (1024.0 * 1024.0);
    double p50 = mb / ((double)samples_ns[15] / 1e9);
    double p95 = mb / ((double)samples_ns[29] / 1e9);
    double p99 = mb / ((double)samples_ns[30] / 1e9);
    printf("%-18s p50=%.1f MB/s p95=%.1f MB/s p99=%.1f MB/s\n", label, p50, p95, p99);
}

static void report_ops(const char *label, unsigned long long samples_ns[RAF_BENCH_SAMPLES]) {
    sort31(samples_ns);
    double p50 = 1e9 / (double)samples_ns[15];
    double p95 = 1e9 / (double)samples_ns[29];
    double p99 = 1e9 / (double)samples_ns[30];
    printf("%-18s p50=%.0f ops/s p95=%.0f ops/s p99=%.0f ops/s\n", label, p50, p95, p99);
}

int main(void) {
    static u8 src[RAF_BUS_MEM_SIZE];
    static u8 dst[RAF_BUS_MEM_SIZE];
    for (u32 i = 0u; i < RAF_BUS_MEM_SIZE; i++) src[i] = (u8)(i & 0xFFu);

    static unsigned long long samples[RAF_BENCH_SAMPLES];
    u32 i;

    /* memory bus — sequential */
    for (i = 0u; i < RAF_BENCH_PREWARM; i++) (void)raf_bus_mem_sequential(dst, src, RAF_BUS_MEM_SIZE);
    for (i = 0u; i < RAF_BENCH_WARMUP; i++) (void)raf_bus_mem_sequential(dst, src, RAF_BUS_MEM_SIZE);
    for (i = 0u; i < RAF_BENCH_SAMPLES; i++) {
        unsigned long long t0 = tick_ns();
        (void)raf_bus_mem_sequential(dst, src, RAF_BUS_MEM_SIZE);
        samples[i] = tick_ns() - t0;
    }
    report_mbps("mem_sequential", samples, RAF_BUS_MEM_SIZE);

    /* memory bus — strided */
    for (i = 0u; i < RAF_BENCH_PREWARM; i++) (void)raf_bus_mem_strided(dst, src, RAF_BUS_MEM_SIZE);
    for (i = 0u; i < RAF_BENCH_WARMUP; i++) (void)raf_bus_mem_strided(dst, src, RAF_BUS_MEM_SIZE);
    for (i = 0u; i < RAF_BENCH_SAMPLES; i++) {
        unsigned long long t0 = tick_ns();
        (void)raf_bus_mem_strided(dst, src, RAF_BUS_MEM_SIZE);
        samples[i] = tick_ns() - t0;
    }
    report_mbps("mem_strided", samples, RAF_BUS_MEM_SIZE);

    /* dispatch bus — runtime backend selector */
    RafRouteInput in = {RAF_CAP_ARM64_NEON | RAF_CAP_GPU_BATCH, 0u, 64u, 128u, RAF_ROUTE_STATE_VALIDATED};
    for (i = 0u; i < RAF_BENCH_PREWARM; i++) (void)raf_bus_dispatch_op(in);
    for (i = 0u; i < RAF_BENCH_WARMUP; i++) (void)raf_bus_dispatch_op(in);
    for (i = 0u; i < RAF_BENCH_SAMPLES; i++) {
        unsigned long long t0 = tick_ns();
        (void)raf_bus_dispatch_op(in);
        samples[i] = tick_ns() - t0;
    }
    report_ops("dispatch_bus", samples);

    return 0;
}
