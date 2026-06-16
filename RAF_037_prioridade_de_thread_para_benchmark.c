#include "RAF_rafaelia_common.h"

#if defined(__linux__) || defined(__ANDROID__)
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#endif

/*
 * Método M037: Prioridade de thread para benchmark
 * Alvo: Linux/Android
 * Domínio: Scheduler
 * Ganho estimado: jitter menor
 *
 * Eleva prioridade quando permitido.
 *
 * Status: skeleton C low-level para Linux/Android/ARM.
 */

static inline uint64_t rafaelia_read_counter_m037(void) {
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

int rafaelia_m037_prioridade_de_thread_para_benchmark(void) {
    uint64_t t0 = rafaelia_read_counter_m037();
    uint64_t t1 = rafaelia_read_counter_m037();
    return (t1 >= t0) ? 0 : -1;
}
