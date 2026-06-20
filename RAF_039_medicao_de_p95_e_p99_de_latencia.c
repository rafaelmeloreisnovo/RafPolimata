#include "RAF_rafaelia_common.h"

#if defined(__linux__) || defined(__ANDROID__)
#include <time.h>
#endif

/*
 * Método M039: Medição de p95 e p99 de latência
 * Alvo: Todos
 * Domínio: Benchmark
 * Ganho estimado: ciência
 *
 * Não olhar só média.
 *
 * Status: skeleton C — coleta N amostras de latência (delta de contador),
 * ordena (insertion sort, N pequeno, sem heap) e lê p95/p99 por índice.
 */

#define M039_N_SAMPLES 32

static inline uint64_t rafaelia_read_counter_m039(void) {
#if defined(__aarch64__)
    uint64_t v = 0;
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(v));
    return v;
#elif defined(__linux__) || defined(__ANDROID__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ull) + (uint64_t)ts.tv_nsec;
#else
    return 0;
#endif
}

static inline void rafaelia_sort_u64_m039(uint64_t *a, int n) {
    for (int i = 1; i < n; i++) {
        uint64_t key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) { a[j + 1] = a[j]; j--; }
        a[j + 1] = key;
    }
}

int rafaelia_m039_medicao_de_p95_e_p99_de_latencia(void) {
    uint64_t samples[M039_N_SAMPLES];

    for (int i = 0; i < M039_N_SAMPLES; i++) {
        uint64_t t0 = rafaelia_read_counter_m039();
        uint64_t t1 = rafaelia_read_counter_m039();
        samples[i] = (t1 >= t0) ? (t1 - t0) : 0;
    }

    rafaelia_sort_u64_m039(samples, M039_N_SAMPLES);

    int p95_idx = (M039_N_SAMPLES * 95) / 100;
    int p99_idx = (M039_N_SAMPLES * 99) / 100;
    if (p95_idx >= M039_N_SAMPLES) p95_idx = M039_N_SAMPLES - 1;
    if (p99_idx >= M039_N_SAMPLES) p99_idx = M039_N_SAMPLES - 1;

    uint64_t p95 = samples[p95_idx];
    uint64_t p99 = samples[p99_idx];

    /* sanity: p99 must never be smaller than p95 once sorted ascending */
    return (p99 >= p95) ? 0 : -1;
}
