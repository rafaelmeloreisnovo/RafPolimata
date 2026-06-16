#include "RAF_rafaelia_common.h"

/*
 * Método M056: Comparação automática contra implementação padrão
 * Alvo: Todos
 * Domínio: Benchmark
 * Ganho estimado: prova
 *
 * Gera baseline e delta.
 *
 * Status: skeleton C — executa uma implementação "baseline" (referência,
 * sem otimização) e uma "otimizada" sobre a mesma entrada, compara
 * resultado (correção) e contagem de operações (custo) automaticamente.
 */

#define M056_N 32

static inline uint32_t rafaelia_sum_baseline_m056(const uint32_t *a, int n, uint32_t *ops) {
    uint32_t acc = 0;
    for (int i = 0; i < n; i++) { acc += a[i]; (*ops)++; }
    return acc;
}

static inline uint32_t rafaelia_sum_optimized_m056(const uint32_t *a, int n, uint32_t *ops) {
    /* unrolled-by-4: mesma correção, menos "dispatches" de iteração contados */
    uint32_t acc = 0;
    int i = 0;
    for (; i + 4 <= n; i += 4) {
        acc += a[i] + a[i+1] + a[i+2] + a[i+3];
        (*ops)++;
    }
    for (; i < n; i++) { acc += a[i]; (*ops)++; }
    return acc;
}

int rafaelia_m056_comparacao_automatica_contra_implementacao_padrao(void) {
    uint32_t input[M056_N];
    for (int i = 0; i < M056_N; i++) input[i] = (uint32_t)i + 1;

    uint32_t ops_baseline = 0, ops_optimized = 0;
    uint32_t r_baseline  = rafaelia_sum_baseline_m056(input, M056_N, &ops_baseline);
    uint32_t r_optimized = rafaelia_sum_optimized_m056(input, M056_N, &ops_optimized);

    /* prova: mesmo resultado (correção) e delta de custo mensurável (ops menor) */
    int correct        = (r_baseline == r_optimized);
    int cheaper_or_equal = (ops_optimized <= ops_baseline);

    return (correct && cheaper_or_equal) ? 0 : -1;
}
