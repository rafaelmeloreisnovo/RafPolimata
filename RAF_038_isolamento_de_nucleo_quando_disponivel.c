#include "RAF_rafaelia_common.h"

#if defined(__linux__) || defined(__ANDROID__)
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#endif

/*
 * Método M038: Isolamento de núcleo quando disponível
 * Alvo: Linux
 * Domínio: Scheduler
 * Ganho estimado: jitter menor
 *
 * Reduz interferência de scheduler.
 *
 * Status: skeleton C low-level para Linux/Android/ARM.
 */

static inline uint64_t rafaelia_read_counter_m038(void) {
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

int rafaelia_m038_isolamento_de_nucleo_quando_disponivel(void) {
    uint64_t t0 = rafaelia_read_counter_m038();
    uint64_t t1 = rafaelia_read_counter_m038();
    return (t1 >= t0) ? 0 : -1;
}
