#include "RAF_rafaelia_common.h"

/*
 * Método M054: Batching de operações repetidas
 * Alvo: Todos
 * Domínio: Performance
 * Ganho estimado: 2x-20x
 *
 * Agrupa trabalho para reduzir dispatch.
 *
 * Status: skeleton C — compara o número de "dispatches" (chamadas de
 * função simuladas) entre processamento item-a-item e processamento em
 * lotes de tamanho fixo, sobre o mesmo conjunto de trabalho.
 */

#define M054_N_ITEMS    64
#define M054_BATCH_SIZE 8

static uint32_t _m054_work[M054_N_ITEMS];

static inline void rafaelia_dispatch_one_m054(uint32_t *item) { *item += 1; }

static inline void rafaelia_dispatch_batch_m054(uint32_t *items, uint32_t n) {
    for (uint32_t i = 0; i < n; i++) items[i] += 1;
}

int rafaelia_m054_batching_de_operacoes_repetidas(void) {
    uint32_t dispatch_count_individual = 0;
    uint32_t dispatch_count_batched    = 0;

    for (uint32_t i = 0; i < M054_N_ITEMS; i++) _m054_work[i] = 0;

    /* item-a-item: 1 dispatch por item */
    for (uint32_t i = 0; i < M054_N_ITEMS; i++) {
        rafaelia_dispatch_one_m054(&_m054_work[i]);
        dispatch_count_individual++;
    }

    /* em lotes: 1 dispatch por grupo de M054_BATCH_SIZE itens */
    for (uint32_t i = 0; i < M054_N_ITEMS; i += M054_BATCH_SIZE) {
        rafaelia_dispatch_batch_m054(&_m054_work[i], M054_BATCH_SIZE);
        dispatch_count_batched++;
    }

    /* sanity: batching deve reduzir estritamente a contagem de dispatch */
    return (dispatch_count_batched < dispatch_count_individual) ? 0 : -1;
}
