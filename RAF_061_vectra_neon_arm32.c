#include "RAF_rafaelia_common.h"

/*
 * Método M061: Vectra kernel NEON ARM32 — VMLA/VMLS pipeline extremo
 * Alvo: ARM32 com NEON (armeabi-v7a, -mfpu=neon -march=armv7-a)
 * Domínio: SIMD / Benchmark
 * Ganho estimado: 4× throughput vs escalar para FMA 4-wide Q4 pipeline
 * Status: ARM32+NEON gated; TOKEN_VAZIO=0 em outros alvos
 *
 * Kernel: 4 registradores NEON Q (128 bits cada = 4×float32):
 *   q0-q3: acumuladores VMLA/VMLS cross-coupled
 *   q4-q5: permutação/XOR para evitar loop folding pelo compilador
 *
 * Origem: L1.md — protótipo Vectra real medido em ARM32 Android
 * Referência: ARM Architecture Reference Manual ARMv7-A/R, NEON PCS
 */

/* ── VMLA/VMLS loop constants (IEEE 754 float em uint32) ─────────────── */
/* v20 = 0.1 em float32 = 0x3D4CCCCD */
#define M061_V20_BITS  0x3D4CCCCDu
/* phi = 1.618033... (razão áurea) em float32 = 0x3FCF1BBCu */
#define M061_PHI_BITS  0x3FCF1BBCu
/* Iterações padrão do kernel */
#define M061_ITERS_DEFAULT 800000u
/* Iterações do selftest (menor para CI rápido) */
#define M061_ITERS_SELFTEST 1000u

/*
 * rafaelia_m061_vectra_neon_loop:
 *   Roda o kernel VMLA/VMLS por `iters` iterações em ARM32+NEON.
 *   Retorna nanosegundos decorridos, ou 0 (TOKEN_VAZIO) em outros alvos.
 */
uint64_t rafaelia_m061_vectra_neon_loop(uint32_t iters) {
#if defined(__arm__) && defined(__ARM_NEON)
    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    uint32_t v20 = M061_V20_BITS;
    uint32_t phi = M061_PHI_BITS;
    uint32_t loops = iters;

    __asm__ volatile(
        /* Carregar constantes nos Q regs como vetores float32x4 */
        "vdup.32 q0, %[v20]        \n"
        "vdup.32 q1, %[phi]        \n"
        "vdup.32 q2, %[v20]        \n"
        "vdup.32 q3, %[phi]        \n"

        "1:                        \n"
        /* VMLA cross-coupled — q0 += q1 * q2, etc */
        "vmla.f32 q0, q1, q2       \n"
        "vmla.f32 q3, q0, q1       \n"
        /* VMLS para manter pipeline alimentado */
        "vmls.f32 q1, q3, q2       \n"
        "vmls.f32 q2, q0, q3       \n"
        /* Permutação via VEXT + VREV64 + VEOR — evita dead-loop folding */
        "vext.32 q4, q0, q1, #1    \n"
        "vrev64.32 q5, q3          \n"
        "veor q0, q4, q5           \n"

        "subs %[cnt], %[cnt], #1   \n"
        "bne 1b                    \n"

        : [cnt] "+r"(loops)
        : [v20] "r"(v20), [phi] "r"(phi)
        : "q0","q1","q2","q3","q4","q5","cc"
    );

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    uint64_t ns = (uint64_t)(t_end.tv_sec - t_start.tv_sec) * 1000000000ULL
                + (uint64_t)(t_end.tv_nsec - t_start.tv_nsec);
    return ns;
#else
    (void)iters;
    return 0u; /* TOKEN_VAZIO — hardware ARM32+NEON ausente */
#endif
}

/*
 * rafaelia_m061_selftest:
 *   ARM32+NEON: corre M061_ITERS_SELFTEST, verifica elapsed_ns > 0.
 *   Outros: verifica TOKEN_VAZIO = 0.
 *   Retorna 0 em PASS, -1 em FAIL.
 */
int rafaelia_m061_selftest(void) {
    uint64_t elapsed = rafaelia_m061_vectra_neon_loop(M061_ITERS_SELFTEST);

#if defined(__arm__) && defined(__ARM_NEON)
    /* Em ARM32+NEON: deve ter medido tempo real positivo */
    if (elapsed == 0u) {
        return -1; /* falha: loop rodou mas retornou 0ns */
    }
#else
    /* Em outros alvos: deve retornar 0 (TOKEN_VAZIO) */
    if (elapsed != 0u) {
        return -1; /* falha: expected TOKEN_VAZIO */
    }
#endif
    return 0;
}
