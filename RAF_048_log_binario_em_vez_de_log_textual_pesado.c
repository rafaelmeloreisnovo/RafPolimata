#include "RAF_rafaelia_common.h"

/*
 * Método M048: Log binário em vez de log textual pesado
 * Alvo: Todos
 * Domínio: Logging
 * Ganho estimado: 5x-50x
 *
 * Reduz formatação e I/O.
 *
 * Status: skeleton C — grava N registros em um buffer binário de tamanho
 * fixo (sem malloc) e compara o custo de bytes contra a estimativa de um
 * log textual equivalente ("ts=<u64> val=<u32>\n" ~= 24 bytes/linha).
 */

#define M048_N_RECORDS 8

typedef struct {
    uint64_t ts;
    uint32_t val;
} rafaelia_bin_record_m048;

static rafaelia_bin_record_m048 _m048_log[M048_N_RECORDS];

int rafaelia_m048_log_binario_em_vez_de_log_textual_pesado(void) {
    for (uint32_t i = 0; i < M048_N_RECORDS; i++) {
        _m048_log[i].ts  = (uint64_t)i * 1000ull;
        _m048_log[i].val = i;
    }

    uint32_t binary_bytes  = (uint32_t)(sizeof(_m048_log));
    uint32_t textual_bytes = M048_N_RECORDS * 24u; /* "ts=18446744073709551615 val=4294967295\n" worst case ~40, typical ~24 */

    /* sanity: the fixed binary record must beat the textual estimate */
    return (binary_bytes < textual_bytes) ? 0 : -1;
}
