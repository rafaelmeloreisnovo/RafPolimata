/* opt_speculative.h — Speculative Optimization & Devirtualization (Phase 42)
 *
 * Phase 42: Speculative optimization and devirtualization
 * - Virtual call devirtualization
 * - Type speculation
 * - Assumption tracking
 * - Guard insertion and validation
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_SPECULATIVE_H
#define APKC_OPT_SPECULATIVE_H 1

#include "opt_interprocedural.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* VIRTUAL CALL DEVIRTUALIZATION */
/* ============================================================ */

struct VirtualCallSite {
	u32 call_site_id;
	u32 base_type;
	u32 virtual_function;
	u32 devirtual_target;
	u64 call_frequency;
	u8 is_monomorphic;
};

struct DevirtualizationOptimizer {
	struct VirtualCallSite call_sites[128];
	u32 call_site_count;
	u32 devirtualized_calls;
	u64 virtual_call_overhead_removed;
};

static inline void devirtualization_optimizer_init(struct DevirtualizationOptimizer *dopt) {
	if (!dopt) return;
	dopt->call_site_count = 0;
	dopt->devirtualized_calls = 0;
	dopt->virtual_call_overhead_removed = 0;
}

static inline u8 add_virtual_call_site(
	struct DevirtualizationOptimizer *dopt,
	u32 call_site_id,
	u32 base_type,
	u32 vfunc,
	u32 target) {

	if (!dopt || dopt->call_site_count >= 128) return 1;

	struct VirtualCallSite *vcs = &dopt->call_sites[dopt->call_site_count];
	vcs->call_site_id = call_site_id;
	vcs->base_type = base_type;
	vcs->virtual_function = vfunc;
	vcs->devirtual_target = target;
	vcs->call_frequency = 0;
	vcs->is_monomorphic = 1;

	dopt->call_site_count++;
	dopt->devirtualized_calls++;
	return 0;
}

/* ============================================================ */
/* TYPE SPECULATION */
/* ============================================================ */

struct TypeSpeculation {
	u32 var_id;
	u32 speculated_type;
	u32 actual_type;
	u64 speculation_count;
	u8 is_valid;
};

struct TypeSpeculator {
	struct TypeSpeculation speculations[256];
	u32 speculation_count;
	u32 valid_speculations;
	u32 failed_speculations;
};

static inline void type_speculator_init(struct TypeSpeculator *ts) {
	if (!ts) return;
	ts->speculation_count = 0;
	ts->valid_speculations = 0;
	ts->failed_speculations = 0;
}

static inline u8 add_type_speculation(
	struct TypeSpeculator *ts,
	u32 var_id,
	u32 spec_type) {

	if (!ts || ts->speculation_count >= 256) return 1;

	struct TypeSpeculation *tspec = &ts->speculations[ts->speculation_count];
	tspec->var_id = var_id;
	tspec->speculated_type = spec_type;
	tspec->actual_type = spec_type;
	tspec->speculation_count = 0;
	tspec->is_valid = 1;

	ts->speculation_count++;
	ts->valid_speculations++;
	return 0;
}

/* ============================================================ */
/* ASSUMPTION TRACKING */
/* ============================================================ */

struct Assumption {
	u32 assumption_id;
	const char *description;
	u32 dependent_opts;
	u8 is_critical;
	u8 is_violated;
};

struct AssumptionTracker {
	struct Assumption assumptions[128];
	u32 assumption_count;
	u32 violated_assumptions;
	u32 optimization_rollbacks;
};

static inline void assumption_tracker_init(struct AssumptionTracker *at) {
	if (!at) return;
	at->assumption_count = 0;
	at->violated_assumptions = 0;
	at->optimization_rollbacks = 0;
}

static inline u8 track_assumption(
	struct AssumptionTracker *at,
	u32 assumption_id,
	const char *desc,
	u8 is_critical) {

	if (!at || at->assumption_count >= 128) return 1;

	struct Assumption *assump = &at->assumptions[at->assumption_count];
	assump->assumption_id = assumption_id;
	assump->description = desc;
	assump->dependent_opts = 0;
	assump->is_critical = is_critical;
	assump->is_violated = 0;

	at->assumption_count++;
	return 0;
}

/* ============================================================ */
/* GUARD INSERTION & VALIDATION */
/* ============================================================ */

struct Guard {
	u32 guard_id;
	u32 guarded_assumption;
	u32 check_instruction;
	u32 failure_path;
	u64 guard_cost;
	u8 is_hoistable;
};

struct GuardInsertionEngine {
	struct Guard guards[256];
	u32 guard_count;
	u32 inserted_guards;
	u64 total_guard_overhead;
};

static inline void guard_insertion_engine_init(struct GuardInsertionEngine *gie) {
	if (!gie) return;
	gie->guard_count = 0;
	gie->inserted_guards = 0;
	gie->total_guard_overhead = 0;
}

static inline u8 insert_guard(
	struct GuardInsertionEngine *gie,
	u32 guard_id,
	u32 assumption,
	u64 guard_cost) {

	if (!gie || gie->guard_count >= 256) return 1;

	struct Guard *g = &gie->guards[gie->guard_count];
	g->guard_id = guard_id;
	g->guarded_assumption = assumption;
	g->check_instruction = 0;
	g->failure_path = 0;
	g->guard_cost = guard_cost;
	g->is_hoistable = 1;

	gie->guard_count++;
	gie->inserted_guards++;
	gie->total_guard_overhead += guard_cost;
	return 0;
}

#endif /* APKC_OPT_SPECULATIVE_H */
