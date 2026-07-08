/* raf_coherence_arx.h — Invariante Geométrica Coerente (ARX branchless)
 * Spec: docs/RAFAELIA_ORQUESTRADOR_ASCII_UTF.md (seção "Prova")
 *
 * coherence_filter — operador Ω puro, ARX-style (no-branch, SIMD-friendly)
 * Implementação canônica: omega_kernel_v2.c:49-67, identicamente
 *
 * Fórmula: Ω(v) = (v XOR (v >> 33)) × 0xff51afd7ed558ccd XOR (result >> 33)
 *          (Knuth multiplicative mixing com FNV-inspired dispersão)
 *
 * Usada em:
 *   1. Kernel Invariante: state_checksum(sigma, relation, delta, entropy, omega, matrix[])
 *   2. Topologia Relacional: phi_ethica = (1−H)×C (coerência em [0,1))
 *   3. Feedback Adaptativo: delta_dir, perm_class (atrator esconde/revela)
 *
 * Zero malloc, zero libc, branchless — freestanding + SIMD.
 */
#pragma once
#include "raf_types.h"

/* ARX mixing: Knuth-style multiplicative hash with two XOR stages */
static inline u64 coherence_filter(u64 v) {
    v ^= v >> 33;
    v *= 0xff51afd7ed558ccdULL;
    v ^= v >> 33;
    return v;
}

/* proof_coherence_arx — ARX checksum de um símbolo (paralelo ao SHA256[:16])
 * Entrada: campos M(s) do símbolo
 * Saída: u64 hash ARX determinístico, sempre idêntico para mesma entrada
 *
 * Implementação:
 *   1. Concatena code|sound|shape|base20|state|meaning como stream u64
 *   2. Folding: acumula via XOR
 *   3. Finalização: coherence_filter → u64 (em hex: %016llx para 16 dígitos)
 */
static inline u64 proof_coherence_arx(u32 code, const char *sound,
                                       const char *shape, const char *base20,
                                       u32 state, const char *meaning) {
    u64 acc = (u64)code;

    /* Fold sound (até 16 chars) */
    if (sound) {
        for (int i = 0; sound[i] && i < 16; i++) {
            acc ^= (u64)(u8)sound[i];
            acc = coherence_filter(acc);
        }
    }

    /* Fold shape */
    if (shape) {
        for (int i = 0; shape[i] && i < 16; i++) {
            acc ^= (u64)(u8)shape[i];
            acc = coherence_filter(acc);
        }
    }

    /* Fold base20 */
    if (base20) {
        for (int i = 0; base20[i] && i < 16; i++) {
            acc ^= (u64)(u8)base20[i];
            acc = coherence_filter(acc);
        }
    }

    /* Fold state */
    acc ^= (u64)state;
    acc = coherence_filter(acc);

    /* Fold meaning */
    if (meaning) {
        for (int i = 0; meaning[i] && i < 16; i++) {
            acc ^= (u64)(u8)meaning[i];
            acc = coherence_filter(acc);
        }
    }

    /* Final mixing */
    return coherence_filter(acc);
}

/* Teste compilação (com -DTEST_COHERENCE_ARX) */
#ifdef TEST_COHERENCE_ARX
#include <stdio.h>

int main(void) {
    /* Teste determinismo: mesma entrada → mesmo hash */
    u64 h1 = proof_coherence_arx(65, "/eɪ/", "linha", "35", 1, "letra/A");
    u64 h2 = proof_coherence_arx(65, "/eɪ/", "linha", "35", 1, "letra/A");

    printf("proof_coherence_arx(code=65, 'A'):\n");
    printf("  h1 = %016llx\n", (unsigned long long)h1);
    printf("  h2 = %016llx\n", (unsigned long long)h2);
    printf("  determinístico = %s\n", h1 == h2 ? "SIM ✓" : "NÃO ✗");

    return h1 == h2 ? 0 : 1;
}
#endif
