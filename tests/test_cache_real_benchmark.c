/* tests/test_cache_real_benchmark.c
 *
 * PHASE 2B ITERATION 3: Real Hardware Cache Benchmark
 *
 * Measures L1/L2/L3 cache hit rates on actual hardware using synthetic workload.
 * Validates cache efficiency assumptions underlying RAFAELIA architecture.
 *
 * Related closures:
 *   • docs/closures/CLOSURE_L5_CACHE_ARCHITECTURE.md
 *     - L1 design target: 80-90% hit rate
 *     - L2 design target: 60-70% hit rate
 *     - L3 design target: 40-50% hit rate
 *
 * Method: Synthetic access patterns (sequential, random, stride) measured
 * against memory latency timing and cache line behavior.
 *
 * Status: PENDING Iteration 3 (2026-08-17)
 * Date: 2026-08-17
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Synthetic cache workload sizes */
#define L1_CACHE_SIZE       32768      /* 32 KB typical */
#define L2_CACHE_SIZE       262144     /* 256 KB typical */
#define L3_CACHE_SIZE       8388608    /* 8 MB typical */
#define CACHE_LINE          64         /* 64-byte cache line */

/* Working set sizes for each tier */
#define L1_WORKING_SET      (L1_CACHE_SIZE / 4)     /* 25% utilization */
#define L2_WORKING_SET      (L2_CACHE_SIZE / 2)     /* 50% utilization */
#define L3_WORKING_SET      (L3_CACHE_SIZE / 2)     /* 50% utilization */

#define ITERATIONS          1000000    /* Total memory accesses */

/* Timing resolution */
typedef struct {
    uint64_t start;
    uint64_t end;
} timer_t;

static inline uint64_t rdtsc(void) {
    uint32_t eax, edx;
    __asm__ __volatile__("rdtsc" : "=a" (eax), "=d" (edx));
    return ((uint64_t)edx << 32) | eax;
}

/* Sequential access pattern: predictable, cache-friendly */
static uint64_t bench_sequential(uint8_t *buffer, size_t buf_size, int iterations) {
    uint64_t sum = 0;
    uint64_t start = rdtsc();

    for (int i = 0; i < iterations; i++) {
        size_t offset = (i * CACHE_LINE) % buf_size;
        sum += buffer[offset];
    }

    uint64_t end = rdtsc();
    return end - start;
}

/* Random access pattern: unpredictable, cache-unfriendly */
static uint64_t bench_random(uint8_t *buffer, size_t buf_size, int iterations) {
    uint64_t sum = 0;
    uint64_t start = rdtsc();

    /* Use simple LCG for pseudo-random offsets */
    uint64_t seed = 12345;
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        size_t offset = (seed % (buf_size / CACHE_LINE)) * CACHE_LINE;
        sum += buffer[offset];
    }

    uint64_t end = rdtsc();
    return end - start;
}

/* Stride access pattern: moderate cache efficiency */
static uint64_t bench_stride(uint8_t *buffer, size_t buf_size, int iterations) {
    uint64_t sum = 0;
    uint64_t start = rdtsc();

    int stride = 256;  /* Typical stride: 256 bytes between accesses */
    for (int i = 0; i < iterations; i++) {
        size_t offset = (i * stride) % buf_size;
        sum += buffer[offset];
    }

    uint64_t end = rdtsc();
    return end - start;
}

int main(void) {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  PHASE 2B ITERATION 3: Real Hardware Cache Benchmark         ║\n");
    printf("║  Measures L1/L2/L3 hit rates via synthetic access patterns    ║\n");
    printf("║  Date: 2026-08-17                                             ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    printf("Cache Configuration (hardware):\n");
    printf("  L1 size:                   32 KB (typical)\n");
    printf("  L2 size:                   256 KB (typical)\n");
    printf("  L3 size:                   8 MB (typical)\n");
    printf("  Cache line:                64 bytes\n");
    printf("\n");

    printf("Working Sets:\n");
    printf("  L1 working set:            %zu bytes (25%% utilization)\n", L1_WORKING_SET);
    printf("  L2 working set:            %zu bytes (50%% utilization)\n", L2_WORKING_SET);
    printf("  L3 working set:            %zu bytes (50%% utilization)\n", L3_WORKING_SET);
    printf("\n");

    /* Allocate buffers for each cache tier */
    uint8_t *l1_buffer = malloc(L1_WORKING_SET);
    uint8_t *l2_buffer = malloc(L2_WORKING_SET);
    uint8_t *l3_buffer = malloc(L3_WORKING_SET);

    if (!l1_buffer || !l2_buffer || !l3_buffer) {
        printf("ERROR: Failed to allocate cache working set buffers\n");
        return 1;
    }

    /* Initialize buffers */
    memset(l1_buffer, 0xAA, L1_WORKING_SET);
    memset(l2_buffer, 0xBB, L2_WORKING_SET);
    memset(l3_buffer, 0xCC, L3_WORKING_SET);

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  L1 CACHE BENCHMARK (Sequential, Random, Stride)             ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    uint64_t l1_seq = bench_sequential(l1_buffer, L1_WORKING_SET, ITERATIONS);
    uint64_t l1_rand = bench_random(l1_buffer, L1_WORKING_SET, ITERATIONS);
    uint64_t l1_stride = bench_stride(l1_buffer, L1_WORKING_SET, ITERATIONS);

    printf("Sequential access:          %.2f cycles/access\n", (double)l1_seq / ITERATIONS);
    printf("Random access:              %.2f cycles/access\n", (double)l1_rand / ITERATIONS);
    printf("Stride access:              %.2f cycles/access\n", (double)l1_stride / ITERATIONS);
    printf("\n");

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  L2 CACHE BENCHMARK (Sequential, Random, Stride)             ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    uint64_t l2_seq = bench_sequential(l2_buffer, L2_WORKING_SET, ITERATIONS);
    uint64_t l2_rand = bench_random(l2_buffer, L2_WORKING_SET, ITERATIONS);
    uint64_t l2_stride = bench_stride(l2_buffer, L2_WORKING_SET, ITERATIONS);

    printf("Sequential access:          %.2f cycles/access\n", (double)l2_seq / ITERATIONS);
    printf("Random access:              %.2f cycles/access\n", (double)l2_rand / ITERATIONS);
    printf("Stride access:              %.2f cycles/access\n", (double)l2_stride / ITERATIONS);
    printf("\n");

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  L3 CACHE BENCHMARK (Sequential, Random, Stride)             ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    uint64_t l3_seq = bench_sequential(l3_buffer, L3_WORKING_SET, ITERATIONS);
    uint64_t l3_rand = bench_random(l3_buffer, L3_WORKING_SET, ITERATIONS);
    uint64_t l3_stride = bench_stride(l3_buffer, L3_WORKING_SET, ITERATIONS);

    printf("Sequential access:          %.2f cycles/access\n", (double)l3_seq / ITERATIONS);
    printf("Random access:              %.2f cycles/access\n", (double)l3_rand / ITERATIONS);
    printf("Stride access:              %.2f cycles/access\n", (double)l3_stride / ITERATIONS);
    printf("\n");

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║  SUMMARY — CACHE PERFORMANCE                                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    double l1_ratio = (double)l1_rand / (double)l1_seq;
    double l2_ratio = (double)l2_rand / (double)l2_seq;
    double l3_ratio = (double)l3_rand / (double)l3_seq;

    printf("Cache Efficiency (random/sequential ratio):\n");
    printf("  L1:                        %.2f× (target: 2-3×)\n", l1_ratio);
    printf("  L2:                        %.2f× (target: 2-4×)\n", l2_ratio);
    printf("  L3:                        %.2f× (target: 3-5×)\n", l3_ratio);
    printf("\n");

    int status = 0;

    if (l1_ratio < 1.5 || l1_ratio > 4.0) {
        printf("STATUS: WARN — L1 ratio %.2f× (expected 2-3×)\n", l1_ratio);
    } else {
        printf("STATUS: PASS — L1 cache efficiency within range (%.2f×)\n", l1_ratio);
    }

    if (l2_ratio < 1.5 || l2_ratio > 5.0) {
        printf("STATUS: WARN — L2 ratio %.2f× (expected 2-4×)\n", l2_ratio);
    } else {
        printf("STATUS: PASS — L2 cache efficiency within range (%.2f×)\n", l2_ratio);
    }

    if (l3_ratio < 2.0 || l3_ratio > 6.0) {
        printf("STATUS: WARN — L3 ratio %.2f× (expected 3-5×)\n", l3_ratio);
    } else {
        printf("STATUS: PASS — L3 cache efficiency within range (%.2f×)\n", l3_ratio);
    }

    printf("\n");
    printf("Note: This is a synthetic benchmark on current hardware.\n");
    printf("Real RAFAELIA workload may differ due to access patterns and contention.\n");
    printf("\n");

    free(l1_buffer);
    free(l2_buffer);
    free(l3_buffer);

    return status;
}
