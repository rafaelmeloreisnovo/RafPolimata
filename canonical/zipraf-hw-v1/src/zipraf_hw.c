#include "../include/zipraf_hw.h"

zh_u64 zh_patch_u64(zh_u64 current, zh_u64 value, zh_u64 mask) {
    return current ^ ((current ^ value) & mask);
}

zh_u32 zh_vector_lanes(zh_u32 register_bits, zh_u32 element_bits) {
    if (element_bits == 0u) return 0u;
    return register_bits / element_bits;
}

zh_u32 zh_block_fit(zh_u32 container_bytes, zh_u32 block_bytes) {
    if (block_bytes == 0u) return 0u;
    return container_bytes / block_bytes;
}

zh_u32 zh_write_amplification_q16(zh_u32 semantic_bytes, zh_u32 physical_granule_bytes) {
    zh_u64 num;
    if (semantic_bytes == 0u || physical_granule_bytes == 0u) return 0u;
    num = ((zh_u64)physical_granule_bytes) << 16;
    num /= (zh_u64)semantic_bytes;
    return num > 0xffffffffULL ? 0xffffffffu : (zh_u32)num;
}

zh_u32 zh_cycles_per_byte_q16(zh_u64 cycles, zh_u64 useful_bytes) {
    zh_u64 num;
    if (useful_bytes == 0ULL) return 0xffffffffu;
    if (cycles > (0xffffffffffffffffULL >> 16)) return 0xffffffffu;
    num = (cycles << 16) / useful_bytes;
    return num > 0xffffffffULL ? 0xffffffffu : (zh_u32)num;
}

zh_i32 zh_measurement_compatible(const struct zh_capability *cap, const struct zh_measurement *m) {
    if (!cap || !m) return 0;
    if (m->state != ZH_PASS) return 0;
    return (m->required_features & ~cap->features) == 0u;
}

zh_i32 zh_select_measured_backend(const struct zh_capability *cap,
                                  const struct zh_measurement *m,
                                  zh_u32 count,
                                  zh_u32 *backend_id_out) {
    zh_u32 i;
    zh_u32 found = 0u;
    zh_u32 best_id = 0u;
    zh_u32 best_q16 = 0xffffffffu;
    if (!cap || !m || !backend_id_out) return -1;
    for (i = 0u; i < count; ++i) {
        zh_u32 score;
        if (!zh_measurement_compatible(cap, &m[i])) continue;
        score = zh_cycles_per_byte_q16(m[i].p95_cycles, m[i].useful_bytes);
        if (!found || score < best_q16 || (score == best_q16 && m[i].backend_id < best_id)) {
            found = 1u;
            best_q16 = score;
            best_id = m[i].backend_id;
        }
    }
    if (!found) return 1; /* TOKEN_VAZIO: no evidence-backed compatible backend. */
    *backend_id_out = best_id;
    return 0;
}
