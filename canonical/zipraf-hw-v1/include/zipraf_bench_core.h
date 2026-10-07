#ifndef ZIPRAF_BENCH_CORE_H
#define ZIPRAF_BENCH_CORE_H

/*
 * ZIPRAF Pure Benchmark Core V1
 *
 * Governance boundary:
 * SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
 * CLOSURE_L12: physical/device measurements are caller-supplied evidence.
 *
 * This core does not read clocks, PMUs, files, environment variables, OS state
 * or provider APIs. It owns only deterministic scheduling, integer statistics,
 * interference/quality vocabulary and explicit wire serialization.
 */

#include "zipraf_hw.h"

#define ZH_BENCH_SAMPLE_COUNT 31u
#define ZH_BENCH_VALID_MASK 0x7fffffffu
#define ZH_BENCH_MAX_VARIANTS 8u
#define ZH_BENCH_RECEIPT_V1_SIZE 152u
#define ZH_BENCH_FACTOR_COUNT 32u
#define ZH_BENCH_QUALITY_COUNT 16u

enum zh_bench_phase {
    ZH_BENCH_PHASE_CONTROL = 0,
    ZH_BENCH_PHASE_PREHOT = 1,
    ZH_BENCH_PHASE_HOT = 2,
    ZH_BENCH_PHASE_POSTHOT = 3
};

enum zh_bench_relation {
    ZH_BENCH_REL_TOKEN_VAZIO = 0,
    ZH_BENCH_REL_HOT_FASTER = 1,
    ZH_BENCH_REL_EQUAL = 2,
    ZH_BENCH_REL_HOT_SLOWER = 3
};

enum zh_bench_factor_state {
    ZH_BENCH_FACTOR_UNKNOWN = 0,
    ZH_BENCH_FACTOR_INACTIVE = 1,
    ZH_BENCH_FACTOR_ACTIVE = 2
};

enum zh_bench_layer {
    ZH_BENCH_LAYER_MATH = 0,
    ZH_BENCH_LAYER_SOURCE = 1,
    ZH_BENCH_LAYER_TOOLCHAIN = 2,
    ZH_BENCH_LAYER_ISA = 3,
    ZH_BENCH_LAYER_MICROARCH = 4,
    ZH_BENCH_LAYER_MEMORY = 5,
    ZH_BENCH_LAYER_POWER_THERMAL = 6,
    ZH_BENCH_LAYER_OS_SCHEDULER = 7,
    ZH_BENCH_LAYER_VIRTUALIZATION = 8,
    ZH_BENCH_LAYER_OBSERVER_IO = 9,
    ZH_BENCH_LAYER_PHYSICAL_EXTERNAL = 10
};

enum zh_bench_speed_group {
    ZH_BENCH_SPEED_UNCLASSIFIED = 0,
    ZH_BENCH_SPEED_PURE_MATH = 1,
    ZH_BENCH_SPEED_ISA_COMPUTE = 2,
    ZH_BENCH_SPEED_CACHE_MEMORY = 3,
    ZH_BENCH_SPEED_OS_ABI = 4,
    ZH_BENCH_SPEED_VIRTUALIZATION_TRANSLATION = 5,
    ZH_BENCH_SPEED_IO_STORAGE = 6,
    ZH_BENCH_SPEED_POWER_THERMAL = 7,
    ZH_BENCH_SPEED_OBSERVER_INSTRUMENTATION = 8
};

enum zh_bench_unit {
    ZH_BENCH_UNIT_RAW_TICKS = 0,
    ZH_BENCH_UNIT_CYCLES = 1,
    ZH_BENCH_UNIT_NANOSECONDS = 2,
    ZH_BENCH_UNIT_BYTES_PER_SECOND = 3,
    ZH_BENCH_UNIT_OPERATIONS_PER_SECOND = 4
};

enum zh_bench_factor {
    ZH_FACTOR_COMPILER_OPT = 0,
    ZH_FACTOR_AUTOVECTORIZE = 1,
    ZH_FACTOR_LTO = 2,
    ZH_FACTOR_PGO = 3,
    ZH_FACTOR_ISA_SPECIALIZATION = 4,
    ZH_FACTOR_INSTRUCTION_ALIGNMENT = 5,
    ZH_FACTOR_BRANCH_PREDICTOR = 6,
    ZH_FACTOR_CACHE_WARMTH = 7,
    ZH_FACTOR_HW_PREFETCH = 8,
    ZH_FACTOR_MEMORY_ALIGNMENT = 9,
    ZH_FACTOR_MEMORY_BANDWIDTH = 10,
    ZH_FACTOR_NUMA_TOPOLOGY = 11,
    ZH_FACTOR_DVFS = 12,
    ZH_FACTOR_TURBO_BOOST = 13,
    ZH_FACTOR_THERMAL_THROTTLE = 14,
    ZH_FACTOR_POWER_GOVERNOR = 15,
    ZH_FACTOR_SCHED_PREEMPTION = 16,
    ZH_FACTOR_CORE_MIGRATION = 17,
    ZH_FACTOR_SMT_CONTENTION = 18,
    ZH_FACTOR_IRQ_NOISE = 19,
    ZH_FACTOR_PAGE_FAULT = 20,
    ZH_FACTOR_VIRTUALIZATION = 21,
    ZH_FACTOR_QEMU_TCG = 22,
    ZH_FACTOR_JIT_TRANSLATION = 23,
    ZH_FACTOR_TIMER_SOURCE = 24,
    ZH_FACTOR_TIMER_OVERHEAD = 25,
    ZH_FACTOR_PMU_OBSERVER = 26,
    ZH_FACTOR_LOGGING_IO = 27,
    ZH_FACTOR_FILESYSTEM_IO = 28,
    ZH_FACTOR_BACKGROUND_LOAD = 29,
    ZH_FACTOR_BATTERY_POWER_STATE = 30,
    ZH_FACTOR_UNKNOWN_EXTERNAL = 31
};

enum zh_bench_quality_gate {
    ZH_QUALITY_SOURCE_IDENTITY = 0,
    ZH_QUALITY_ARTIFACT_IDENTITY = 1,
    ZH_QUALITY_CORRECTNESS_ORACLE = 2,
    ZH_QUALITY_TIMER_VALID = 3,
    ZH_QUALITY_SAMPLE_COMPLETE = 4,
    ZH_QUALITY_FACTOR_STATE_RECORDED = 5,
    ZH_QUALITY_ORDER_SEED_RECORDED = 6,
    ZH_QUALITY_PHYSICAL_IDENTITY = 7,
    ZH_QUALITY_THERMAL_OBSERVED = 8,
    ZH_QUALITY_FREQUENCY_OBSERVED = 9,
    ZH_QUALITY_STABILITY_REPEATED = 10,
    ZH_QUALITY_INDEPENDENT_REPRO = 11,
    ZH_QUALITY_OPTIMIZER_STATE = 12,
    ZH_QUALITY_VIRTUALIZATION_STATE = 13,
    ZH_QUALITY_OS_STATE = 14,
    ZH_QUALITY_RAW_SAMPLES_PRESERVED = 15
};

struct zh_bench_stats {
    zh_u32 state;
    zh_u32 n;
    zh_u64 min;
    zh_u64 p05;
    zh_u64 median;
    zh_u64 p95;
    zh_u64 max;
};

struct zh_bench_receipt_v1 {
    zh_u32 seed;
    zh_u32 variant_count;
    zh_u32 speed_group;
    zh_u32 unit;
    zh_u32 factor_observed_mask;
    zh_u32 factor_active_mask;
    zh_u32 quality_observed_mask;
    zh_u32 quality_pass_mask;
    zh_u32 relation;
    struct zh_bench_stats prehot;
    struct zh_bench_stats hot;
    zh_u64 speed_ratio_num;
    zh_u64 speed_ratio_den;
};

zh_u32 zh_bench_factor_bit(zh_u32 factor_id);
zh_u32 zh_bench_factor_layer(zh_u32 factor_id);
zh_u32 zh_bench_factor_state(zh_u32 observed_mask,
                             zh_u32 active_mask,
                             zh_u32 factor_id);
zh_u32 zh_bench_order_index(zh_u32 seed,
                            zh_u32 round,
                            zh_u32 position,
                            zh_u32 variant_count);
zh_u32 zh_bench_analyze31(const zh_u64 samples[ZH_BENCH_SAMPLE_COUNT],
                          zh_u32 valid_mask,
                          struct zh_bench_stats *out);
zh_u32 zh_bench_compare(const struct zh_bench_stats *prehot,
                        const struct zh_bench_stats *hot);
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
                           const struct zh_bench_stats *hot);
zh_i32 zh_bench_receipt_encode(const struct zh_bench_receipt_v1 *r,
                               zh_u8 *out,
                               zh_u32 out_len);
zh_i32 zh_bench_receipt_decode(struct zh_bench_receipt_v1 *r,
                               const zh_u8 *in,
                               zh_u32 in_len);

#endif
