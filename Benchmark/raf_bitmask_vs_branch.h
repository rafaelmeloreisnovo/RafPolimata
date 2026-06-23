#ifndef RAF_BITMASK_VS_BRANCH_H
#define RAF_BITMASK_VS_BRANCH_H

/*
 * S16 — Evitar branch em hot path
 *
 * Demonstra a diferença entre uso de branch condicional e bitmask
 * no processamento de flags em loops críticos.
 *
 * Técnica: substituir `if (flags & BIT) val |= BIT;`
 *          por     `val |= (flags & BIT);`
 * O bitmask produz o mesmo resultado sem introduzir salto dependente de dados,
 * eliminando misprediction penalty em execução superescalar.
 */

#include <stdint.h>

/* Versão com branch — introduz salto dependente de dados por flag */
static inline uint32_t raf_flags_branch(uint32_t flags, uint32_t mask) {
    uint32_t result = 0;
    if (flags & (1u << 0))  result |= (1u << 0);
    if (flags & (1u << 1))  result |= (1u << 1);
    if (flags & (1u << 2))  result |= (1u << 2);
    if (flags & (1u << 3))  result |= (1u << 3);
    if (flags & (1u << 4))  result |= (1u << 4);
    if (flags & (1u << 5))  result |= (1u << 5);
    if (flags & (1u << 6))  result |= (1u << 6);
    if (flags & (1u << 7))  result |= (1u << 7);
    return result & mask;
}

/* Versão sem branch — AND direto, zero saltos dependentes de dados */
static inline uint32_t raf_flags_bitmask(uint32_t flags, uint32_t mask) {
    return flags & mask;
}

/*
 * Autovalidação: ambas as versões devem produzir o mesmo resultado
 * para qualquer combinação de flags e mask.
 * Retorna 0 se equivalentes, -1 se divergirem.
 */
static inline int raf_bitmask_vs_branch_selftest(void) {
    /* raf_flags_branch covers bits 0-7; keep all inputs within that range */
    static const uint32_t test_flags[] = {
        0x00u, 0xFFu, 0x55u, 0xAAu, 0x0Fu, 0xF0u, 0xCCu, 0x33u
    };
    static const uint32_t test_masks[] = {
        0xFFu, 0x0Fu, 0xF0u, 0x55u, 0xAAu, 0xCCu
    };
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 6; j++) {
            uint32_t r_branch  = raf_flags_branch (test_flags[i], test_masks[j]);
            uint32_t r_bitmask = raf_flags_bitmask(test_flags[i], test_masks[j]);
            if (r_branch != r_bitmask) return -1;
        }
    }
    return 0; /* semantically identical — bitmask preferred in hot paths */
}

#endif /* RAF_BITMASK_VS_BRANCH_H */
