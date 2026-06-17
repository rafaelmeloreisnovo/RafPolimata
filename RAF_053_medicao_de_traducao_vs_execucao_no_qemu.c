#include "RAF_rafaelia_common.h"

#if defined(__linux__) || defined(__ANDROID__)
#define _POSIX_C_SOURCE 200809L
#include <time.h>
#endif

/*
 * Método M053: Medição de tradução vs execução no QEMU
 * Alvo: QEMU/TCG
 * Domínio: VM
 * Ganho estimado: ciência — entender onde o tempo vai
 *
 * QEMU/TCG distingue duas fases de custo:
 *   1. Tradução (cold): TCG converte guest ISA → IR → host code (caminho frio)
 *   2. Execução (warm): código já traduzido em cache — muito mais rápido
 *
 * Esta probe mede ambas: primeira execução de um bloco (inclui tradução)
 * vs segunda execução (bloco já no code-cache TCG).
 *
 * Em host nativo (sem TCG) ambos os valores serão similares.
 * Em QEMU: cold_ns >> warm_ns pela sobrecarga de tradução.
 */

typedef struct {
    uint64_t cold_ns;    /* primeira execução — inclui custo de tradução TCG */
    uint64_t warm_ns;    /* segunda execução — bloco já no code-cache */
    uint64_t ratio_x10;  /* cold/warm * 10, inteiro (sem float) */
} rafaelia_tcg_timing_m053_t;

/*
 * _m053_read_ns — lê tempo monotônico em nanosegundos.
 * ARM64: usa cntvct_el0 (ciclos de sistema) convertido via cntfrq_el0.
 * Linux/Android x86: usa clock_gettime(CLOCK_MONOTONIC).
 * Outros: retorna 0 (TOKEN_VAZIO).
 */
static uint64_t _m053_read_ns(void)
{
#if defined(__aarch64__)
    uint64_t cntvct, cntfrq;
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(cntvct));
    __asm__ __volatile__("mrs %0, cntfrq_el0" : "=r"(cntfrq));
    /* ns = cntvct * 1_000_000_000 / cntfrq */
    if (cntfrq == 0u) return 0u;
    return (cntvct / cntfrq) * 1000000000ull
         + (cntvct % cntfrq) * 1000000000ull / cntfrq;
#elif defined(__linux__) || defined(__ANDROID__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ull) + (uint64_t)ts.tv_nsec;
#else
    return 0u;
#endif
}

/*
 * _m053_workload — synthetic workload kept in a separate function so the
 * compiler treats it as a distinct basic block (helping TCG identify the
 * translation unit boundary).
 */
static volatile uint64_t _m053_acc = 0u;

static void _m053_workload(void)
{
    for (uint64_t i = 0u; i < 1000u; i++) {
        _m053_acc ^= i;
    }
}

int rafaelia_m053_medicao_de_traducao_vs_execucao_no_qemu(void)
{
    rafaelia_tcg_timing_m053_t t = {0u, 0u, 0u};
    uint64_t a;

    /* Cold measurement: first execution may include TCG translation overhead */
    a = _m053_read_ns();
    _m053_workload();
    t.cold_ns = _m053_read_ns() - a;

    /* Warm measurement: block already in TCG code-cache */
    a = _m053_read_ns();
    _m053_workload();
    t.warm_ns = _m053_read_ns() - a;

    /*
     * Compute integer ratio cold/warm * 10.
     * If warm_ns == 0 (very fast host or clock resolution too coarse),
     * assume ratio 1:1 (ratio_x10 = 10).
     */
    t.ratio_x10 = (t.warm_ns > 0u)
                ? (t.cold_ns * 10u) / t.warm_ns
                : 10u;

    /*
     * Sanity check: ratio must be positive.
     * On QEMU with TCG we expect ratio_x10 >> 10 (cold >> warm).
     * On native hardware ratio_x10 ≈ 10 (similar times).
     * Either is acceptable — the probe is for measurement, not a pass/fail gate.
     */
    (void)_m053_acc; /* side-effect: prevents optimisation of the workload away */
    return (t.ratio_x10 > 0u) ? 0 : -1;
}
