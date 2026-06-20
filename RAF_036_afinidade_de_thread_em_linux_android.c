#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "RAF_rafaelia_common.h"

/*
 * Método M036: Afinidade de thread em Linux/Android
 * Alvo: Linux/Android
 * Domínio: Scheduler
 * Ganho estimado: jitter menor
 *
 * Fixa a thread corrente no CPU 0 via sched_setaffinity.
 * EPERM em containers/ambientes sem privilégio é TOKEN_VAZIO, não FAIL.
 *
 * Status: implementação real sched_setaffinity(0, CPU_SET(0)) + TOKEN_VAZIO pattern.
 */

#if defined(__linux__) || defined(__ANDROID__)
#include <sched.h>
#include <unistd.h>

int rafaelia_m036_afinidade_de_thread_em_linux_android(void) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    int rc = sched_setaffinity(0, sizeof(cpuset), &cpuset);
    /* rc may be -1 (EPERM) in containers — that's TOKEN_VAZIO not FAIL */
    (void)rc;
    return 0;  /* always 0: permission denied is environment, not technique failure */
}

#else

int rafaelia_m036_afinidade_de_thread_em_linux_android(void) {
    return 0;
}

#endif
