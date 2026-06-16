#include "RAF_rafaelia_common.h"

#if defined(__linux__) || defined(__ANDROID__)
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#endif

/*
 * Método M022: GPIO por /dev/gpiomem
 * Alvo: Raspberry/Linux
 * Domínio: GPIO
 * Ganho estimado: segurança
 *
 * Acesso GPIO sem /dev/mem total.
 *
 * Status: skeleton C low-level para Linux/Android/ARM.
 */

static inline uint64_t rafaelia_read_counter_m022(void) {
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

int rafaelia_m022_gpio_por_dev_gpiomem(void) {
    uint64_t t0 = rafaelia_read_counter_m022();
    uint64_t t1 = rafaelia_read_counter_m022();
    return (t1 >= t0) ? 0 : -1;
}
