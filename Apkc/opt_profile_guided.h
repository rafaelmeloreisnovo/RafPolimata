/* opt_profile_guided.h — Profile-Guided Optimization (Phase 40)
 *
 * Phase 40: Profile-guided optimization integration
 * - Profile data collection infrastructure
 * - Hot path identification
 * - Code layout optimization
 * - Speculative optimization
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_PROFILE_GUIDED_H
#define APKC_OPT_PROFILE_GUIDED_H 1

#include "opt_link_time.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* PROFILE DATA COLLECTION */
/* ============================================================ */

struct ExecutionProfile {
	u32 func_id;
	u64 exec_count;
	u64 total_cycles;
	u32 caller_count;
	u32 instruction_count;
	u8 is_hot;
};

struct ProfileCollector {
	struct ExecutionProfile profiles[256];
	u32 profile_count;
	u64 total_executions;
	u32 hot_functions;
};

static inline void profile_collector_init(struct ProfileCollector *pc) {
	if (!pc) return;
	pc->profile_count = 0;
	pc->total_executions = 0;
	pc->hot_functions = 0;
}

static inline u8 record_profile(
	struct ProfileCollector *pc,
	u32 func_id,
	u64 exec_count) {

	if (!pc || pc->profile_count >= 256) return 1;

	struct ExecutionProfile *ep = &pc->profiles[pc->profile_count];
	ep->func_id = func_id;
	ep->exec_count = exec_count;
	ep->total_cycles = 0;
	ep->caller_count = 0;
	ep->instruction_count = 0;
	ep->is_hot = (exec_count > 1000);

	pc->profile_count++;
	pc->total_executions += exec_count;
	if (ep->is_hot) pc->hot_functions++;
	return 0;
}

/* ============================================================ */
/* HOT PATH IDENTIFICATION */
/* ============================================================ */

struct HotPath {
	u32 path_id;
	u32 function_sequence[32];
	u32 function_count;
	u64 execution_count;
	u64 total_cycles;
	u8 is_critical;
};

struct HotPathAnalyzer {
	struct HotPath paths[64];
	u32 path_count;
	u64 critical_path_cycles;
	u32 critical_path_count;
};

static inline void hot_path_analyzer_init(struct HotPathAnalyzer *hpa) {
	if (!hpa) return;
	hpa->path_count = 0;
	hpa->critical_path_cycles = 0;
	hpa->critical_path_count = 0;
}

static inline u8 identify_hot_path(
	struct HotPathAnalyzer *hpa,
	u32 path_id,
	u32 first_func,
	u64 exec_count) {

	if (!hpa || hpa->path_count >= 64) return 1;

	struct HotPath *hp = &hpa->paths[hpa->path_count];
	hp->path_id = path_id;
	hp->function_sequence[0] = first_func;
	hp->function_count = 1;
	hp->execution_count = exec_count;
	hp->total_cycles = 0;
	hp->is_critical = (exec_count > 10000);

	hpa->path_count++;
	if (hp->is_critical) {
		hpa->critical_path_count++;
		hpa->critical_path_cycles += hp->total_cycles;
	}
	return 0;
}

/* ============================================================ */
/* CODE LAYOUT OPTIMIZATION */
/* ============================================================ */

struct CodeBlock {
	u32 block_id;
	u32 offset;
	u32 size;
	u64 execution_count;
	u8 is_hot;
};

struct CodeLayoutOptimizer {
	struct CodeBlock blocks[256];
	u32 block_count;
	u32 cache_line_misses;
	u64 icache_misses_reduced;
};

static inline void code_layout_optimizer_init(struct CodeLayoutOptimizer *clo) {
	if (!clo) return;
	clo->block_count = 0;
	clo->cache_line_misses = 0;
	clo->icache_misses_reduced = 0;
}

static inline u8 add_code_block(
	struct CodeLayoutOptimizer *clo,
	u32 block_id,
	u32 size,
	u64 exec_count) {

	if (!clo || clo->block_count >= 256) return 1;

	struct CodeBlock *cb = &clo->blocks[clo->block_count];
	cb->block_id = block_id;
	cb->offset = 0;
	cb->size = size;
	cb->execution_count = exec_count;
	cb->is_hot = (exec_count > 1000);

	clo->block_count++;
	return 0;
}

/* ============================================================ */
/* SPECULATIVE OPTIMIZATION */
/* ============================================================ */

struct SpeculativeOptimization {
	u32 opt_id;
	const char *opt_name;
	u64 expected_benefit;
	u64 actual_benefit;
	u8 is_speculative;
	u8 validation_status;
};

struct SpeculativeOptimizer {
	struct SpeculativeOptimization optimizations[64];
	u32 opt_count;
	u32 validated_opts;
	u32 failed_speculations;
};

static inline void speculative_optimizer_init(struct SpeculativeOptimizer *so) {
	if (!so) return;
	so->opt_count = 0;
	so->validated_opts = 0;
	so->failed_speculations = 0;
}

static inline u8 add_speculative_optimization(
	struct SpeculativeOptimizer *so,
	u32 opt_id,
	const char *name,
	u64 expected_benefit) {

	if (!so || so->opt_count >= 64) return 1;

	struct SpeculativeOptimization *spec = &so->optimizations[so->opt_count];
	spec->opt_id = opt_id;
	spec->opt_name = name;
	spec->expected_benefit = expected_benefit;
	spec->actual_benefit = 0;
	spec->is_speculative = 1;
	spec->validation_status = 0;

	so->opt_count++;
	return 0;
}

/* ============================================================ */
/* POGO CONTEXT SENSITIVE OPTIMIZATION */
/* ============================================================ */

struct ContextProfile {
	u32 call_site;
	u32 target_func;
	u64 call_count;
	u8 is_frequent;
};

struct ContextSensitiveOptimizer {
	struct ContextProfile contexts[256];
	u32 context_count;
	u32 optimized_contexts;
	u64 context_specific_benefits;
};

static inline void context_sensitive_optimizer_init(struct ContextSensitiveOptimizer *cso) {
	if (!cso) return;
	cso->context_count = 0;
	cso->optimized_contexts = 0;
	cso->context_specific_benefits = 0;
}

static inline u8 track_call_context(
	struct ContextSensitiveOptimizer *cso,
	u32 call_site,
	u32 target_func,
	u64 call_count) {

	if (!cso || cso->context_count >= 256) return 1;

	struct ContextProfile *cp = &cso->contexts[cso->context_count];
	cp->call_site = call_site;
	cp->target_func = target_func;
	cp->call_count = call_count;
	cp->is_frequent = (call_count > 100);

	cso->context_count++;
	if (cp->is_frequent) cso->optimized_contexts++;
	return 0;
}

#endif /* APKC_OPT_PROFILE_GUIDED_H */
