#include "RAF_rafaelia_common.h"

/*
 * Método M055: Cache local de resultado técnico
 * Alvo: Todos
 * Domínio: Cache
 * Ganho estimado: 2x-50x
 *
 * Evita recomputar estados.
 *
 * Status: skeleton C — cache direto-mapeado de tamanho fixo (sem malloc).
 * Recomputa apenas em cache-miss; cache-hit retorna valor armazenado.
 */

#define M055_CACHE_SLOTS 16

typedef struct {
    int      valid;
    uint32_t key;
    uint32_t value;
} rafaelia_cache_slot_m055;

static rafaelia_cache_slot_m055 _m055_cache[M055_CACHE_SLOTS];
static uint32_t _m055_recompute_count = 0;

static inline uint32_t rafaelia_expensive_compute_m055(uint32_t key) {
    _m055_recompute_count++;
    return key * key; /* stand-in for a costly deterministic computation */
}

static inline uint32_t rafaelia_cached_compute_m055(uint32_t key) {
    uint32_t slot = key % M055_CACHE_SLOTS;
    if (_m055_cache[slot].valid && _m055_cache[slot].key == key)
        return _m055_cache[slot].value; /* cache hit */

    uint32_t result = rafaelia_expensive_compute_m055(key); /* cache miss */
    _m055_cache[slot].valid = 1;
    _m055_cache[slot].key   = key;
    _m055_cache[slot].value = result;
    return result;
}

int rafaelia_m055_cache_local_de_resultado_tecnico(void) {
    for (int i = 0; i < M055_CACHE_SLOTS; i++) _m055_cache[i].valid = 0;
    _m055_recompute_count = 0;

    uint32_t key = 7;
    uint32_t r1 = rafaelia_cached_compute_m055(key); /* miss → recompute */
    uint32_t r2 = rafaelia_cached_compute_m055(key); /* hit  → no recompute */

    /* sanity: same key must yield same value, and only one recompute happened */
    return (r1 == r2 && _m055_recompute_count == 1) ? 0 : -1;
}
