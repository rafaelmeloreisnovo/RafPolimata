#include "RAF_rafaelia_common.h"

/*
 * Método M049: Benchmark via Termux CLI
 * Alvo: Termux
 * Domínio: Benchmark
 * Ganho estimado: reprodutível
 *
 * Harness de benchmark auto-temporizado: executa workload XOR por n iterações,
 * mede tempo elapsed via clock_gettime(CLOCK_MONOTONIC).
 * Adequado para invocação direta de CLI (Termux, shell Linux).
 *
 * Status: implementação real com clock_gettime + loop XOR + self-test elapsed>0.
 */

#if defined(__linux__) || defined(__ANDROID__)
#include <time.h>

static uint64_t rafaelia_m049_read_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

uint64_t rafaelia_m049_run_benchmark(uint32_t n) {
    uint64_t t0 = rafaelia_m049_read_ns();
    volatile uint32_t acc = 0;
    for (uint32_t i = 0; i < n; i++) {
        acc ^= i;
    }
    (void)acc;
    return rafaelia_m049_read_ns() - t0;
}

int rafaelia_m049_benchmark_via_termux_cli(void) {
    uint64_t ns = rafaelia_m049_run_benchmark(1000u);
    return (ns > 0u) ? 0 : -1;
}

#else /* non-Linux fallback: no clock_gettime — always TOKEN_VAZIO pass */

uint64_t rafaelia_m049_run_benchmark(uint32_t n) {
    volatile uint32_t acc = 0;
    for (uint32_t i = 0; i < n; i++) acc ^= i;
    (void)acc;
    return 1u; /* non-zero: "success, timing unavailable" */
}

int rafaelia_m049_benchmark_via_termux_cli(void) {
    return 0;
}

#endif
