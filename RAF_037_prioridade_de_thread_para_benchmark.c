#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "RAF_rafaelia_common.h"

/*
 * Método M037: Prioridade de thread para benchmark
 * Alvo: Linux/Android
 * Domínio: Scheduler
 * Ganho estimado: jitter menor
 *
 * Eleva thread para SCHED_FIFO prioridade 1 (menor RT) via sched_setscheduler.
 * EPERM em ambientes não-privilegiados é TOKEN_VAZIO, não FAIL.
 *
 * Status: implementação real sched_setscheduler(0, SCHED_FIFO, {.sched_priority=1}).
 */

#if defined(__linux__) || defined(__ANDROID__)
#include <sched.h>

int rafaelia_m037_prioridade_de_thread_para_benchmark(void) {
    struct sched_param sp = {0};
    sp.sched_priority = 1;
    int rc = sched_setscheduler(0, SCHED_FIFO, &sp);
    (void)rc;  /* may fail with EPERM in unprivileged env — TOKEN_VAZIO */
    return 0;
}

#else

int rafaelia_m037_prioridade_de_thread_para_benchmark(void) {
    return 0;
}

#endif
