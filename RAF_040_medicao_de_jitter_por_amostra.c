#include "RAF_rafaelia_common.h"

#if defined(__linux__) || defined(__ANDROID__)
#include <time.h>
#endif

/*
 * Método M040: Medição de jitter por amostra
 * Alvo: Todos
 * Domínio: Benchmark
 * Ganho estimado: tempo real
 *
 * Registra variação temporal.
 *
 * Status: skeleton C — jitter = desvio máximo entre amostras consecutivas
 * e a média das amostras (sem heap, sem ponto flutuante: tudo em u64).
 */

#define M040_N_SAMPLES 16

static inline uint64_t rafaelia_read_counter_m040(void) {
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

int rafaelia_m040_medicao_de_jitter_por_amostra(void) {
    uint64_t deltas[M040_N_SAMPLES];
    uint64_t sum = 0;

    for (int i = 0; i < M040_N_SAMPLES; i++) {
        uint64_t t0 = rafaelia_read_counter_m040();
        uint64_t t1 = rafaelia_read_counter_m040();
        deltas[i] = (t1 >= t0) ? (t1 - t0) : 0;
        sum += deltas[i];
    }

    uint64_t mean = sum / M040_N_SAMPLES;
    uint64_t max_jitter = 0;

    for (int i = 0; i < M040_N_SAMPLES; i++) {
        uint64_t dev = (deltas[i] >= mean) ? (deltas[i] - mean) : (mean - deltas[i]);
        if (dev > max_jitter) max_jitter = dev;
    }

    /* sanity: a single deviation from the mean can never exceed the sum */
    return (max_jitter <= sum) ? 0 : -1;
}
