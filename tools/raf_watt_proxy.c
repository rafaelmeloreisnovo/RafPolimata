/* tools/raf_watt_proxy.c — fixed-cost workload for processing-per-watt
 * measurement (closes checklist S25).
 *
 * This binary does NOT measure power itself. It runs a deterministic,
 * reproducible compute kernel (the same phi_fst geometric coherence
 * metric used by Apkc/coherence.h, reimplemented here in plain hosted
 * C11 to avoid mixing this host tool with the freestanding Apkc/sys.h
 * syscall layer) over a fixed buffer, repeated a fixed number of times,
 * so that an external tool — perf stat / RAPL via
 * scripts/raf_watt_proxy_probe.sh — has a stable load to wrap energy or
 * IPC/cache-miss measurement around.
 *
 * Never fabricates a wattage number. Reports only what it actually
 * measured: elapsed time and a checksum (anti-dead-code-elimination
 * witness), to stdout as a single machine-parsable line.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define RAF_WATT_BUF_SIZE (64u * 1024u)
#define RAF_WATT_ITERS    2000u

/* Mirrors Apkc/coherence.h phi_fst() — see that file for the formula's
 * rationale. Kept in sync deliberately; not #included to avoid pulling
 * in the freestanding syscall layer (Apkc/sys.h) into a hosted tool. */
static uint32_t phi_fst_proxy(const uint8_t *buf, uint32_t n) {
    if (!n) return 0u;

    uint32_t freq[256] = {0};
    for (uint32_t i = 0u; i < n; i++) freq[buf[i]]++;

    uint32_t unique = 0u;
    for (int i = 0; i < 256; i++) if (freq[i]) unique++;
    uint32_t H = (unique * 0x10000u) / 256u;

    static const uint32_t KAM7[7] = {40503u, 40503u, 40503u, 40503u, 40503u, 40503u, 40503u};
    uint64_t dot = 0u, ns = 0u;
    for (int i = 0; i < 7; i++) {
        dot += (uint64_t)freq[i] * KAM7[i];
        ns  += (uint64_t)freq[i] * freq[i];
    }
    uint32_t C = 0u;
    if (ns) {
        C = (uint32_t)((dot * 0x10000u) / (ns | 1u));
        if (C > 0x10000u) C = 0x10000u;
    }

    uint32_t oneMinH = (H < 0x10000u) ? (0x10000u - H) : 0u;
    return (uint32_t)(((uint64_t)oneMinH * C) >> 16);
}

static uint64_t mono_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ull) + (uint64_t)ts.tv_nsec;
}

int main(void) {
    static uint8_t buf[RAF_WATT_BUF_SIZE];
    for (uint32_t i = 0u; i < RAF_WATT_BUF_SIZE; i++) buf[i] = (uint8_t)((i * 2654435761u) & 0xFFu);

    uint64_t t0 = mono_ns();
    uint32_t checksum = 0u;
    for (uint32_t it = 0u; it < RAF_WATT_ITERS; it++) {
        checksum ^= phi_fst_proxy(buf, RAF_WATT_BUF_SIZE);
        buf[it & (RAF_WATT_BUF_SIZE - 1u)] ^= (uint8_t)it;
    }
    uint64_t t1 = mono_ns();

    printf("raf_watt_proxy: iters=%u bytes_per_iter=%u elapsed_ns=%llu checksum=%08x\n",
           RAF_WATT_ITERS, RAF_WATT_BUF_SIZE,
           (unsigned long long)(t1 - t0), (unsigned)checksum);
    return 0;
}
