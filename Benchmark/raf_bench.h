/* raf_bench.h — harness de benchmark, mediana de BENCH_K=31 amostras
 * Mediana elimina outliers: scheduler preemption, IRQ, thermal throttle
 * Insertion sort em array de stack — zero malloc, O(k^2) = O(961) = trivial
 * ARM64: cntvct_el0 — hardware counter; frequency read from cntfrq_el0
 * x86-64: rdtsc + lfence — raw TSC ticks until a calibrated frequency exists
 * ARM32: clock_gettime(CLOCK_MONOTONIC) raw syscall — values already in ns
 * Timer overhead is architecture-specific and must not be reported as zero. */
#pragma once
#include "raf_types.h"
#include "raf_sys.h"

/* Insertion sort em stack — BENCH_K=31 iterações máximas                   */
static void isort31(u64 a[BENCH_K]) {
    for (s32 i = 1; i < BENCH_K; i++) {
        u64  key = a[i];
        s32  j   = i - 1;
        while (j >= 0 && a[j] > key) { a[j+1] = a[j]; j--; }
        a[j+1] = key;
    }
}

/* Mediana: índice BENCH_MED=15 após sort — valor central exato              */
static u64 bench_median(u64 a[BENCH_K]) {
    isort31(a);
    return a[BENCH_MED];
}

/* Macro de benchmark — captura tsc antes/depois, armazena delta             */
#define BENCH_RUN(samples, code_block)    \
    do {                                   \
        for (u32 _bi = 0; _bi < BENCH_K; _bi++) { \
            u64 _t0 = raf_tsc();           \
            { code_block }                 \
            u64 _t1 = raf_tsc();           \
            (samples)[_bi] = (raf_timer_ok() && _t1 >= _t0) ? (_t1 - _t0) : 0; \
        }                                  \
    } while(0)

/* Resultado de benchmark com estatísticas completas                         */
typedef struct {
    u64 med;   /* mediana — métrica principal                               */
    u64 p5;    /* percentil 5 (índice 1)                                    */
    u64 p95;   /* percentil 95 (índice 29)                                  */
    u64 min;   /* mínimo (índice 0)                                         */
    u64 max;   /* máximo (índice 30)                                        */
} BenchResult;

static BenchResult bench_analyze(u64 a[BENCH_K]) {
    isort31(a);
    BenchResult r;
    r.min = a[0];    r.p5  = a[1];
    r.med = a[15];   r.p95 = a[29];
    r.max = a[30];
    return r;
}

/* Convert ticks to ns only when a counter frequency is known.
 * ARM32 clock_gettime32 values are already nanoseconds; x86 stays raw ticks
 * until an explicit TSC calibration exists.                                */
static u64 ticks_to_ns(u64 ticks, u64 freq_hz) {
#ifdef RAF_ARCH_A32
    /* ARM32 raf_tsc() already returns nanoseconds. Avoid a 64-bit division
     * that can introduce an external __aeabi_* helper in a no-libc link. */
    (void)freq_hz;
    return ticks;
#else
    if (!freq_hz) return ticks; /* raw ticks/cycles; not nanoseconds */
    return (ticks * 1000000ULL) / (freq_hz / 1000ULL);
#endif
}

/* Report formatado — sem printf, sem heap. Unit follows timer evidence.     */
static void bench_report(const char *name, BenchResult r, u64 freq) {
    if (!raf_timer_ok()) {
        raf_puts(name); raf_puts("  status=FAIL_TIMER\n");
        return;
    }
    const char *unit = freq ? "ns" : "ticks";
    raf_puts(name);
    raf_puts("  med=");  raf_putu64(ticks_to_ns(r.med, freq));
    raf_puts(unit); raf_puts(" p5=");  raf_putu64(ticks_to_ns(r.p5,  freq));
    raf_puts(unit); raf_puts(" p95="); raf_putu64(ticks_to_ns(r.p95, freq));
    raf_puts(unit); raf_puts("\n");
}
