/* Governance: CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE.
 * TOKEN_VAZIO vocabulary in this file preserves missing physical/runtime
 * evidence; it is not numeric zero and never promotes a claim.
 */
#include "../include/zipraf_bench_core.h"

static zh_u32 zh_bench_mod_small(zh_u32 value, zh_u32 mod) {
    if (mod == 0u) return 0xffffffffu;
    value &= 0xffu;
    while (value >= mod) value -= mod;
    return value;
}

static zh_u32 zh_bench_mix32(zh_u32 x) {
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    x += 0x6d2b79f5u;
    x ^= x >> 11;
    return x;
}

static void zh_bench_sort31(zh_u64 v[ZH_BENCH_SAMPLE_COUNT]) {
    zh_u32 i;
    for (i = 1u; i < ZH_BENCH_SAMPLE_COUNT; ++i) {
        zh_u64 key = v[i];
        zh_u32 j = i;
        while (j > 0u && v[j - 1u] > key) {
            v[j] = v[j - 1u];
            --j;
        }
        v[j] = key;
    }
}

zh_u32 zh_bench_factor_bit(zh_u32 factor_id) {
    if (factor_id >= ZH_BENCH_FACTOR_COUNT) return 0u;
    return 1u << factor_id;
}

zh_u32 zh_bench_factor_layer(zh_u32 factor_id) {
    if (factor_id <= ZH_FACTOR_PGO) return ZH_BENCH_LAYER_TOOLCHAIN;
    if (factor_id <= ZH_FACTOR_INSTRUCTION_ALIGNMENT) return ZH_BENCH_LAYER_ISA;
    if (factor_id == ZH_FACTOR_BRANCH_PREDICTOR) return ZH_BENCH_LAYER_MICROARCH;
    if (factor_id <= ZH_FACTOR_NUMA_TOPOLOGY) return ZH_BENCH_LAYER_MEMORY;
    if (factor_id <= ZH_FACTOR_POWER_GOVERNOR) return ZH_BENCH_LAYER_POWER_THERMAL;
    if (factor_id <= ZH_FACTOR_PAGE_FAULT) return ZH_BENCH_LAYER_OS_SCHEDULER;
    if (factor_id <= ZH_FACTOR_JIT_TRANSLATION) return ZH_BENCH_LAYER_VIRTUALIZATION;
    if (factor_id <= ZH_FACTOR_FILESYSTEM_IO) return ZH_BENCH_LAYER_OBSERVER_IO;
    if (factor_id < ZH_BENCH_FACTOR_COUNT) return ZH_BENCH_LAYER_PHYSICAL_EXTERNAL;
    return 0xffffffffu;
}

zh_u32 zh_bench_factor_state(zh_u32 observed_mask,
                             zh_u32 active_mask,
                             zh_u32 factor_id) {
    zh_u32 bit = zh_bench_factor_bit(factor_id);
    if (bit == 0u || (observed_mask & bit) == 0u)
        return ZH_BENCH_FACTOR_UNKNOWN;
    return (active_mask & bit) != 0u ?
        ZH_BENCH_FACTOR_ACTIVE : ZH_BENCH_FACTOR_INACTIVE;
}

zh_u32 zh_bench_order_index(zh_u32 seed,
                            zh_u32 round,
                            zh_u32 position,
                            zh_u32 variant_count) {
    zh_u32 mixed;
    zh_u32 rotation;
    zh_u32 p;
    zh_u32 reverse;
    if (variant_count == 0u || variant_count > ZH_BENCH_MAX_VARIANTS)
        return 0xffffffffu;
    mixed = zh_bench_mix32(seed ^ (round << 8) ^ (round >> 3));
    rotation = zh_bench_mod_small(mixed, variant_count);
    p = zh_bench_mod_small(position, variant_count);
    reverse = (mixed >> 8) & 1u;
    if (reverse == 0u)
        return zh_bench_mod_small(rotation + p, variant_count);
    if (p == 0u) return rotation;
    return zh_bench_mod_small(rotation + variant_count - p, variant_count);
}

zh_u32 zh_bench_analyze31(const zh_u64 samples[ZH_BENCH_SAMPLE_COUNT],
                          zh_u32 valid_mask,
                          struct zh_bench_stats *out) {
    zh_u64 sorted[ZH_BENCH_SAMPLE_COUNT];
    zh_u32 i;
    if (!out) return ZH_FAIL;
    out->state = ZH_TOKEN_VAZIO;
    out->n = 0u;
    out->min = 0u;
    out->p05 = 0u;
    out->median = 0u;
    out->p95 = 0u;
    out->max = 0u;
    if (!samples || valid_mask != ZH_BENCH_VALID_MASK)
        return ZH_TOKEN_VAZIO;
    for (i = 0u; i < ZH_BENCH_SAMPLE_COUNT; ++i)
        sorted[i] = samples[i];
    zh_bench_sort31(sorted);
    out->state = ZH_PASS;
    out->n = ZH_BENCH_SAMPLE_COUNT;
    out->min = sorted[0];
    out->p05 = sorted[1];
    out->median = sorted[15];
    out->p95 = sorted[29];
    out->max = sorted[30];
    return ZH_PASS;
}

zh_u32 zh_bench_compare(const struct zh_bench_stats *prehot,
                        const struct zh_bench_stats *hot) {
    if (!prehot || !hot) return ZH_BENCH_REL_TOKEN_VAZIO;
    if (prehot->state != ZH_PASS || hot->state != ZH_PASS)
        return ZH_BENCH_REL_TOKEN_VAZIO;
    if (hot->median < prehot->median) return ZH_BENCH_REL_HOT_FASTER;
    if (hot->median > prehot->median) return ZH_BENCH_REL_HOT_SLOWER;
    return ZH_BENCH_REL_EQUAL;
}

void zh_bench_receipt_init(struct zh_bench_receipt_v1 *out,
                           zh_u32 seed,
                           zh_u32 variant_count,
                           zh_u32 speed_group,
                           zh_u32 unit,
                           zh_u32 factor_observed_mask,
                           zh_u32 factor_active_mask,
                           zh_u32 quality_observed_mask,
                           zh_u32 quality_pass_mask,
                           const struct zh_bench_stats *prehot,
                           const struct zh_bench_stats *hot) {
    if (!out) return;
    out->seed = seed;
    out->variant_count = variant_count;
    out->speed_group = speed_group;
    out->unit = unit;
    out->factor_observed_mask = factor_observed_mask;
    out->factor_active_mask = factor_active_mask & factor_observed_mask;
    out->quality_observed_mask = quality_observed_mask;
    out->quality_pass_mask = quality_pass_mask & quality_observed_mask;
    if (prehot) out->prehot = *prehot;
    else {
        out->prehot.state = ZH_TOKEN_VAZIO;
        out->prehot.n = 0u;
        out->prehot.min = 0u;
        out->prehot.p05 = 0u;
        out->prehot.median = 0u;
        out->prehot.p95 = 0u;
        out->prehot.max = 0u;
    }
    if (hot) out->hot = *hot;
    else {
        out->hot.state = ZH_TOKEN_VAZIO;
        out->hot.n = 0u;
        out->hot.min = 0u;
        out->hot.p05 = 0u;
        out->hot.median = 0u;
        out->hot.p95 = 0u;
        out->hot.max = 0u;
    }
    out->relation = zh_bench_compare(&out->prehot, &out->hot);
    out->speed_ratio_num = out->prehot.median;
    out->speed_ratio_den = out->hot.median;
}

static void zh_put_u32le(zh_u8 *p, zh_u32 v) {
    p[0] = (zh_u8)v;
    p[1] = (zh_u8)(v >> 8);
    p[2] = (zh_u8)(v >> 16);
    p[3] = (zh_u8)(v >> 24);
}

static void zh_put_u64le(zh_u8 *p, zh_u64 v) {
    zh_u32 i;
    for (i = 0u; i < 8u; ++i)
        p[i] = (zh_u8)(v >> (i * 8u));
}

static zh_u32 zh_get_u32le(const zh_u8 *p) {
    return ((zh_u32)p[0]) |
           ((zh_u32)p[1] << 8) |
           ((zh_u32)p[2] << 16) |
           ((zh_u32)p[3] << 24);
}

static zh_u64 zh_get_u64le(const zh_u8 *p) {
    zh_u32 i;
    zh_u64 v = 0u;
    for (i = 0u; i < 8u; ++i)
        v |= ((zh_u64)p[i]) << (i * 8u);
    return v;
}

static void zh_put_stats(zh_u8 *p, const struct zh_bench_stats *s) {
    zh_put_u64le(p + 0u, s->min);
    zh_put_u64le(p + 8u, s->p05);
    zh_put_u64le(p + 16u, s->median);
    zh_put_u64le(p + 24u, s->p95);
    zh_put_u64le(p + 32u, s->max);
}

static void zh_get_stats(struct zh_bench_stats *s,
                         const zh_u8 *p,
                         zh_u32 state) {
    s->state = state;
    s->n = state == ZH_PASS ? ZH_BENCH_SAMPLE_COUNT : 0u;
    s->min = zh_get_u64le(p + 0u);
    s->p05 = zh_get_u64le(p + 8u);
    s->median = zh_get_u64le(p + 16u);
    s->p95 = zh_get_u64le(p + 24u);
    s->max = zh_get_u64le(p + 32u);
}

zh_i32 zh_bench_receipt_encode(const struct zh_bench_receipt_v1 *r,
                               zh_u8 *out,
                               zh_u32 out_len) {
    if (!r || !out || out_len < ZH_BENCH_RECEIPT_V1_SIZE) return -1;
    out[0] = 'Z'; out[1] = 'B'; out[2] = 'R'; out[3] = '1';
    zh_put_u32le(out + 4u, 1u);
    zh_put_u32le(out + 8u, r->seed);
    zh_put_u32le(out + 12u, r->variant_count);
    zh_put_u32le(out + 16u, r->speed_group);
    zh_put_u32le(out + 20u, r->unit);
    zh_put_u32le(out + 24u, r->factor_observed_mask);
    zh_put_u32le(out + 28u, r->factor_active_mask);
    zh_put_u32le(out + 32u, r->quality_observed_mask);
    zh_put_u32le(out + 36u, r->quality_pass_mask);
    zh_put_u32le(out + 40u, r->prehot.state);
    zh_put_u32le(out + 44u, r->hot.state);
    zh_put_u32le(out + 48u, r->relation);
    zh_put_u32le(out + 52u, ZH_BENCH_SAMPLE_COUNT);
    zh_put_stats(out + 56u, &r->prehot);
    zh_put_stats(out + 96u, &r->hot);
    zh_put_u64le(out + 136u, r->speed_ratio_num);
    zh_put_u64le(out + 144u, r->speed_ratio_den);
    return 0;
}

zh_i32 zh_bench_receipt_decode(struct zh_bench_receipt_v1 *r,
                               const zh_u8 *in,
                               zh_u32 in_len) {
    zh_u32 pre_state;
    zh_u32 hot_state;
    if (!r || !in || in_len < ZH_BENCH_RECEIPT_V1_SIZE) return -1;
    if (in[0] != 'Z' || in[1] != 'B' || in[2] != 'R' || in[3] != '1')
        return -2;
    if (zh_get_u32le(in + 4u) != 1u ||
        zh_get_u32le(in + 52u) != ZH_BENCH_SAMPLE_COUNT)
        return -3;
    r->seed = zh_get_u32le(in + 8u);
    r->variant_count = zh_get_u32le(in + 12u);
    r->speed_group = zh_get_u32le(in + 16u);
    r->unit = zh_get_u32le(in + 20u);
    r->factor_observed_mask = zh_get_u32le(in + 24u);
    r->factor_active_mask = zh_get_u32le(in + 28u) & r->factor_observed_mask;
    r->quality_observed_mask = zh_get_u32le(in + 32u);
    r->quality_pass_mask = zh_get_u32le(in + 36u) & r->quality_observed_mask;
    pre_state = zh_get_u32le(in + 40u);
    hot_state = zh_get_u32le(in + 44u);
    r->relation = zh_get_u32le(in + 48u);
    zh_get_stats(&r->prehot, in + 56u, pre_state);
    zh_get_stats(&r->hot, in + 96u, hot_state);
    r->speed_ratio_num = zh_get_u64le(in + 136u);
    r->speed_ratio_den = zh_get_u64le(in + 144u);
    return 0;
}
