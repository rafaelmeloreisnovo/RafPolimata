#include "RAF_rafaelia_common.h"

/*
 * Método M025: Uso de cntfrq_el0 para converter ciclos em tempo
 * Alvo: ARM64
 * Domínio: Timing
 * Ganho estimado: precisão
 *
 * Lê cntfrq_el0 (frequência do contador de sistema em Hz) e converte
 * delta de cntvct_el0 em nanossegundos. Fallback: clock_gettime em x86.
 *
 * Status: implementação real ARM64 (mrs cntfrq_el0/cntvct_el0) + fallback POSIX.
 */

#if defined(__linux__) || defined(__ANDROID__)
#include <time.h>
#endif

int rafaelia_m025_uso_de_cntfrq_el0_para_converter_ciclos_em_tempo(void) {
#if defined(__aarch64__)
    uint64_t freq = 0, t0 = 0, t1 = 0;
    __asm__ __volatile__("mrs %0, cntfrq_el0" : "=r"(freq));
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(t0));
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(t1));
    if (!freq) freq = 1000000000ULL;
    uint64_t elapsed_ns = (t1 - t0) * 1000000000ULL / freq;
    return (elapsed_ns < 1000000000ULL) ? 0 : -1;
#elif defined(__linux__) || defined(__ANDROID__)
    struct timespec ts0, ts1;
    clock_gettime(CLOCK_MONOTONIC, &ts0);
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    uint64_t diff_ns = (uint64_t)(ts1.tv_sec  - ts0.tv_sec)  * 1000000000ULL
                     + (uint64_t)(ts1.tv_nsec - ts0.tv_nsec);
    return (diff_ns < 1000000000ULL) ? 0 : -1;
#else
    return 0;
#endif
}
