#include <stdio.h>
#include <stdlib.h>
#include "../include/zipraf_bench_core.h"

/*
 * Hosted known-vector adapter.
 * This file is not part of the freestanding claim. It only materializes the
 * deterministic test receipt into a file and a human-readable view.
 */

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : "zipraf-bench-known-vector-v1.zipraf";
    zh_u64 pre[ZH_BENCH_SAMPLE_COUNT];
    zh_u64 hot[ZH_BENCH_SAMPLE_COUNT];
    struct zh_bench_stats a;
    struct zh_bench_stats b;
    struct zh_bench_receipt_v1 receipt;
    zh_u8 wire[ZH_BENCH_RECEIPT_V1_SIZE];
    zh_u32 i;
    zh_u32 factors;
    zh_u32 quality;
    FILE *f;

    for (i = 0u; i < ZH_BENCH_SAMPLE_COUNT; ++i) {
        pre[i] = 130u - i;
        hot[i] = 110u - i;
    }
    if (zh_bench_analyze31(pre, ZH_BENCH_VALID_MASK, &a) != ZH_PASS)
        return 10;
    if (zh_bench_analyze31(hot, ZH_BENCH_VALID_MASK, &b) != ZH_PASS)
        return 11;

    factors = zh_bench_factor_bit(ZH_FACTOR_COMPILER_OPT) |
              zh_bench_factor_bit(ZH_FACTOR_AUTOVECTORIZE);

    quality = (1u << ZH_QUALITY_CORRECTNESS_ORACLE) |
              (1u << ZH_QUALITY_SAMPLE_COMPLETE) |
              (1u << ZH_QUALITY_ORDER_SEED_RECORDED) |
              (1u << ZH_QUALITY_OPTIMIZER_STATE) |
              (1u << ZH_QUALITY_RAW_SAMPLES_PRESERVED);

    zh_bench_receipt_init(
        &receipt,
        0x52414631u,
        4u,
        ZH_BENCH_SPEED_ISA_COMPUTE,
        ZH_BENCH_UNIT_CYCLES,
        factors,
        0u,
        quality,
        quality,
        &a,
        &b
    );
    if (zh_bench_receipt_encode(&receipt, wire, sizeof(wire)) != 0)
        return 12;

    f = fopen(path, "wb");
    if (!f) return 13;
    if (fwrite(wire, 1u, sizeof(wire), f) != sizeof(wire)) {
        fclose(f);
        return 14;
    }
    if (fclose(f) != 0) return 15;

    printf("schema=ZIPRAF-BENCH-RECEIPT-V1\n");
    printf("status=TEST_VECTOR_NOT_PHYSICAL_EVIDENCE\n");
    printf("wire_bytes=%u\n", (unsigned)ZH_BENCH_RECEIPT_V1_SIZE);
    printf("speed_group=ISA_COMPUTE\n");
    printf("unit=CYCLES\n");
    printf("prehot_n=%u prehot_min=%llu prehot_p05=%llu prehot_median=%llu prehot_p95=%llu prehot_max=%llu\n",
           (unsigned)a.n,
           (unsigned long long)a.min,
           (unsigned long long)a.p05,
           (unsigned long long)a.median,
           (unsigned long long)a.p95,
           (unsigned long long)a.max);
    printf("hot_n=%u hot_min=%llu hot_p05=%llu hot_median=%llu hot_p95=%llu hot_max=%llu\n",
           (unsigned)b.n,
           (unsigned long long)b.min,
           (unsigned long long)b.p05,
           (unsigned long long)b.median,
           (unsigned long long)b.p95,
           (unsigned long long)b.max);
    printf("relation=HOT_FASTER\n");
    printf("speed_ratio=%llu/%llu\n",
           (unsigned long long)receipt.speed_ratio_num,
           (unsigned long long)receipt.speed_ratio_den);
    printf("factor_observed_mask=0x%08x factor_active_mask=0x%08x\n",
           (unsigned)receipt.factor_observed_mask,
           (unsigned)receipt.factor_active_mask);
    printf("quality_observed_mask=0x%08x quality_pass_mask=0x%08x\n",
           (unsigned)receipt.quality_observed_mask,
           (unsigned)receipt.quality_pass_mask);
    printf("claim_allowed=false\n");
    return 0;
}
