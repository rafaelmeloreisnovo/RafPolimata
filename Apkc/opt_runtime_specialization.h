/* opt_runtime_specialization.h — Runtime Specialization & JIT Compilation (Phase 45)
 *
 * Phase 45: Runtime specialization and JIT compilation
 * - Hot code identification at runtime
 * - Dynamic specialization of functions
 * - On-stack replacement (OSR)
 * - JIT compilation framework
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_RUNTIME_SPECIALIZATION_H
#define APKC_OPT_RUNTIME_SPECIALIZATION_H 1

#include "opt_cache_hierarchy.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* HOT CODE IDENTIFICATION */
/* ============================================================ */

enum HotCodeStatus {
	HOT_COLD = 0,
	HOT_WARM = 1,
	HOT_HOT = 2,
	HOT_CRITICAL = 3
};

struct HotCodeSegment {
	u32 segment_id;
	u32 start_addr;
	u32 end_addr;
	u64 exec_count;
	enum HotCodeStatus status;
	u64 compilation_time_invested;
};

struct RuntimeHotCodeIdentifier {
	struct HotCodeSegment segments[256];
	u32 segment_count;
	u32 hot_segments;
	u32 critical_segments;
};

static inline void runtime_hot_code_id_init(struct RuntimeHotCodeIdentifier *rhci) {
	if (!rhci) return;
	rhci->segment_count = 0;
	rhci->hot_segments = 0;
	rhci->critical_segments = 0;
}

static inline u8 identify_hot_segment(
	struct RuntimeHotCodeIdentifier *rhci,
	u32 segment_id,
	u32 start_addr,
	u32 end_addr) {

	if (!rhci || rhci->segment_count >= 256) return 1;

	struct HotCodeSegment *hcs = &rhci->segments[rhci->segment_count];
	hcs->segment_id = segment_id;
	hcs->start_addr = start_addr;
	hcs->end_addr = end_addr;
	hcs->exec_count = 0;
	hcs->status = HOT_COLD;
	hcs->compilation_time_invested = 0;

	rhci->segment_count++;
	return 0;
}

/* ============================================================ */
/* DYNAMIC FUNCTION SPECIALIZATION */
/* ============================================================ */

struct SpecializedVersion {
	u32 version_id;
	u32 base_function;
	u32 specialization_constraints[16];
	u32 constraint_count;
	u64 exec_count;
	u8 is_active;
};

struct DynamicSpecializer {
	struct SpecializedVersion versions[128];
	u32 version_count;
	u32 active_specializations;
	u64 specialization_benefit;
};

static inline void dynamic_specializer_init(struct DynamicSpecializer *ds) {
	if (!ds) return;
	ds->version_count = 0;
	ds->active_specializations = 0;
	ds->specialization_benefit = 0;
}

static inline u8 create_specialized_version(
	struct DynamicSpecializer *ds,
	u32 version_id,
	u32 base_func) {

	if (!ds || ds->version_count >= 128) return 1;

	struct SpecializedVersion *sv = &ds->versions[ds->version_count];
	sv->version_id = version_id;
	sv->base_function = base_func;
	sv->constraint_count = 0;
	sv->exec_count = 0;
	sv->is_active = 1;

	ds->version_count++;
	ds->active_specializations++;
	return 0;
}

/* ============================================================ */
/* ON-STACK REPLACEMENT (OSR) */
/* ============================================================ */

struct OSRPoint {
	u32 osr_id;
	u32 instruction_offset;
	u32 loop_nesting_depth;
	u32 live_registers;
	u64 osr_threshold;
	u8 is_active;
};

struct OSRExecutor {
	struct OSRPoint osr_points[64];
	u32 osr_point_count;
	u32 executed_osrs;
	u64 osrs_deferred;
};

static inline void osr_executor_init(struct OSRExecutor *osr) {
	if (!osr) return;
	osr->osr_point_count = 0;
	osr->executed_osrs = 0;
	osr->osrs_deferred = 0;
}

static inline u8 add_osr_point(
	struct OSRExecutor *osr,
	u32 osr_id,
	u32 insn_offset,
	u32 loop_depth) {

	if (!osr || osr->osr_point_count >= 64) return 1;

	struct OSRPoint *op = &osr->osr_points[osr->osr_point_count];
	op->osr_id = osr_id;
	op->instruction_offset = insn_offset;
	op->loop_nesting_depth = loop_depth;
	op->live_registers = 0;
	op->osr_threshold = 100000;
	op->is_active = 1;

	osr->osr_point_count++;
	return 0;
}

/* ============================================================ */
/* JIT COMPILATION FRAMEWORK */
/* ============================================================ */

enum JITTier {
	TIER_INTERPRETER = 0,
	TIER_BASELINE = 1,
	TIER_OPTIMIZED = 2,
	TIER_FULL_PROFILE = 3
};

struct JITCompiledCode {
	u32 code_id;
	enum JITTier tier;
	u64 compilation_time_us;
	u32 code_size;
	u64 exec_count;
	u8 is_valid;
};

struct JITCompiler {
	struct JITCompiledCode code_cache[256];
	u32 cache_size;
	u32 compiled_functions;
	u64 total_compilation_time;
};

static inline void jit_compiler_init(struct JITCompiler *jc) {
	if (!jc) return;
	jc->cache_size = 0;
	jc->compiled_functions = 0;
	jc->total_compilation_time = 0;
}

static inline u8 compile_with_jit(
	struct JITCompiler *jc,
	u32 code_id,
	enum JITTier tier,
	u64 comp_time) {

	if (!jc || jc->cache_size >= 256) return 1;

	struct JITCompiledCode *jcc = &jc->code_cache[jc->cache_size];
	jcc->code_id = code_id;
	jcc->tier = tier;
	jcc->compilation_time_us = comp_time;
	jcc->code_size = 0;
	jcc->exec_count = 0;
	jcc->is_valid = 1;

	jc->cache_size++;
	jc->compiled_functions++;
	jc->total_compilation_time += comp_time;
	return 0;
}

/* ============================================================ */
/* RUNTIME OPTIMIZATION STATISTICS */
/* ============================================================ */

struct RuntimeOptimizationStats {
	u32 hot_segments_identified;
	u32 dynamic_specializations;
	u32 osr_executions;
	u32 jit_compilations;
	u64 runtime_optimization_time;
	u64 total_speedup_achieved;
};

static inline struct RuntimeOptimizationStats compute_runtime_stats(
	struct RuntimeHotCodeIdentifier *rhci,
	struct DynamicSpecializer *ds,
	struct OSRExecutor *osr,
	struct JITCompiler *jc) {

	struct RuntimeOptimizationStats stats;
	stats.hot_segments_identified = rhci ? rhci->hot_segments : 0;
	stats.dynamic_specializations = ds ? ds->active_specializations : 0;
	stats.osr_executions = osr ? osr->executed_osrs : 0;
	stats.jit_compilations = jc ? jc->compiled_functions : 0;
	stats.runtime_optimization_time = jc ? jc->total_compilation_time : 0;
	stats.total_speedup_achieved = 0;

	return stats;
}

/* ============================================================ */
/* ADAPTIVE OPTIMIZATION POLICY */
/* ============================================================ */

enum OptimizationPolicy {
	POLICY_AGGRESSIVE = 0,
	POLICY_BALANCED = 1,
	POLICY_CONSERVATIVE = 2,
	POLICY_THROUGHPUT = 3
};

struct AdaptiveOptimizer {
	enum OptimizationPolicy current_policy;
	u32 policy_switches;
	u64 total_adaptive_benefit;
	u32 workload_type;
};

static inline void adaptive_optimizer_init(struct AdaptiveOptimizer *ao) {
	if (!ao) return;
	ao->current_policy = POLICY_BALANCED;
	ao->policy_switches = 0;
	ao->total_adaptive_benefit = 0;
	ao->workload_type = 0;
}

#endif /* APKC_OPT_RUNTIME_SPECIALIZATION_H */
