/* Governance: CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE.
 * TOKEN_VAZIO vocabulary in this file preserves missing physical/runtime
 * evidence; it is not numeric zero and never promotes a claim.
 */
#include <stdio.h>
#include "../include/zipraf_bench_core.h"

static int check(int ok, const char *name) {
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", name);
        return 1;
    }
    printf("PASS %s\n", name);
    return 0;
}

int main(void) {
    int rc = 0;
    zh_u32 i;
    zh_u64 pre[ZH_BENCH_SAMPLE_COUNT];
    zh_u64 hot[ZH_BENCH_SAMPLE_COUNT];
    struct zh_bench_stats a;
    struct zh_bench_stats b;
    struct zh_bench_receipt_v1 receipt;
    struct zh_bench_receipt_v1 decoded;
    zh_u8 wire[ZH_BENCH_RECEIPT_V1_SIZE];
    zh_u32 seen;
    zh_u32 factors;
    zh_u32 quality;

    for (i = 0u; i < ZH_BENCH_SAMPLE_COUNT; ++i) {
        pre[i] = 130u - i;
        hot[i] = 110u - i;
    }

    rc |= check(
        zh_bench_analyze31(pre, ZH_BENCH_VALID_MASK, &a) == ZH_PASS &&
        a.n == 31u && a.min == 100u && a.p05 == 101u &&
        a.median == 115u && a.p95 == 129u && a.max == 130u,
        "prehot_stats"
    );
    rc |= check(
        zh_bench_analyze31(hot, ZH_BENCH_VALID_MASK, &b) == ZH_PASS &&
        b.median == 95u,
        "hot_stats"
    );
    rc |= check(
        zh_bench_compare(&a, &b) == ZH_BENCH_REL_HOT_FASTER,
        "prehot_hot_relation"
    );
    rc |= check(
        zh_bench_analyze31(pre, ZH_BENCH_VALID_MASK ^ 1u, &a) ==
            ZH_TOKEN_VAZIO &&
        a.state == ZH_TOKEN_VAZIO,
        "missing_sample_is_not_zero"
    );

    seen = 0u;
    for (i = 0u; i < 4u; ++i) {
        zh_u32 idx = zh_bench_order_index(0x52414631u, 7u, i, 4u);
        rc |= check(idx < 4u, "order_index_range");
        if (idx < 4u) seen |= 1u << idx;
    }
    rc |= check(seen == 0x0fu, "order_round_is_permutation");

    factors = zh_bench_factor_bit(ZH_FACTOR_COMPILER_OPT) |
              zh_bench_factor_bit(ZH_FACTOR_TURBO_BOOST) |
              zh_bench_factor_bit(ZH_FACTOR_TIMER_OVERHEAD);
    rc |= check(
        zh_bench_factor_layer(ZH_FACTOR_COMPILER_OPT) ==
            ZH_BENCH_LAYER_TOOLCHAIN &&
        zh_bench_factor_layer(ZH_FACTOR_TURBO_BOOST) ==
            ZH_BENCH_LAYER_POWER_THERMAL &&
        zh_bench_factor_layer(ZH_FACTOR_TIMER_OVERHEAD) ==
            ZH_BENCH_LAYER_OBSERVER_IO,
        "factor_layers"
    );
    rc |= check(
        zh_bench_factor_state(factors,
                              zh_bench_factor_bit(ZH_FACTOR_TURBO_BOOST),
                              ZH_FACTOR_TURBO_BOOST) == ZH_BENCH_FACTOR_ACTIVE &&
        zh_bench_factor_state(factors,
                              zh_bench_factor_bit(ZH_FACTOR_TURBO_BOOST),
                              ZH_FACTOR_COMPILER_OPT) == ZH_BENCH_FACTOR_INACTIVE &&
        zh_bench_factor_state(factors,
                              0u,
                              ZH_FACTOR_DVFS) == ZH_BENCH_FACTOR_UNKNOWN,
        "factor_tristate"
    );

    (void)zh_bench_analyze31(pre, ZH_BENCH_VALID_MASK, &a);
    (void)zh_bench_analyze31(hot, ZH_BENCH_VALID_MASK, &b);
    quality = (1u << ZH_QUALITY_SOURCE_IDENTITY) |
              (1u << ZH_QUALITY_ARTIFACT_IDENTITY) |
              (1u << ZH_QUALITY_CORRECTNESS_ORACLE) |
              (1u << ZH_QUALITY_SAMPLE_COMPLETE) |
              (1u << ZH_QUALITY_ORDER_SEED_RECORDED) |
              (1u << ZH_QUALITY_RAW_SAMPLES_PRESERVED);
    zh_bench_receipt_init(
        &receipt,
        0x52414631u,
        4u,
        ZH_BENCH_SPEED_ISA_COMPUTE,
        ZH_BENCH_UNIT_CYCLES,
        factors,
        zh_bench_factor_bit(ZH_FACTOR_TURBO_BOOST),
        quality,
        quality,
        &a,
        &b
    );
    rc |= check(
        receipt.relation == ZH_BENCH_REL_HOT_FASTER &&
        receipt.speed_ratio_num == 115u &&
        receipt.speed_ratio_den == 95u,
        "receipt_relation_ratio"
    );
    rc |= check(
        zh_bench_receipt_encode(&receipt, wire, sizeof(wire)) == 0 &&
        wire[0] == 'Z' && wire[1] == 'B' &&
        wire[2] == 'R' && wire[3] == '1',
        "receipt_encode"
    );
    rc |= check(
        zh_bench_receipt_decode(&decoded, wire, sizeof(wire)) == 0 &&
        decoded.seed == receipt.seed &&
        decoded.speed_group == ZH_BENCH_SPEED_ISA_COMPUTE &&
        decoded.unit == ZH_BENCH_UNIT_CYCLES &&
        decoded.prehot.median == 115u &&
        decoded.hot.median == 95u &&
        decoded.factor_active_mask ==
            zh_bench_factor_bit(ZH_FACTOR_TURBO_BOOST),
        "receipt_roundtrip"
    );
    rc |= check(
        zh_bench_receipt_decode(&decoded, wire,
                                ZH_BENCH_RECEIPT_V1_SIZE - 1u) == -1,
        "receipt_truncation_rejected"
    );

    return rc ? 1 : 0;
}
