/* opt_coordinator.h — Optimization Pass Coordinator (Phase 48)
 *
 * Phase 48: Manages application of optimization results to generated code
 * - Optimization pass scheduling and dependency management
 * - Apply optimizations to IR/code
 * - Trade-off resolution for competing optimization goals
 * - Performance tuning and profiling integration
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_COORDINATOR_H
#define APKC_OPT_COORDINATOR_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* OPTIMIZATION LEVELS */
/* ============================================================ */

enum OptimizationLevel {
	OPT_NONE = 0,
	OPT_SIZE = 1,
	OPT_SPEED = 2,
	OPT_BALANCED = 3,
	OPT_AGGRESSIVE = 4
};

enum OptimizationGoal {
	GOAL_CODE_SIZE = 0,
	GOAL_EXECUTION_SPEED = 1,
	GOAL_MEMORY_USAGE = 2,
	GOAL_ENERGY_EFFICIENCY = 3,
	GOAL_STARTUP_TIME = 4
};

/* ============================================================ */
/* OPTIMIZATION PASS */
/* ============================================================ */

struct OptimizationPass {
	u32 phase_id;
	const char *name;
	enum OptimizationLevel level;
	u32 improvement_percent;
	u64 execution_time_us;
	u8 is_enabled;
	u8 requires_profiling;
	u32 conflicts_with[8];
	u32 conflict_count;
};

static inline void optimization_pass_init(struct OptimizationPass *pass) {
	if (!pass) return;
	pass->phase_id = 0;
	pass->name = 0;
	pass->level = OPT_NONE;
	pass->improvement_percent = 0;
	pass->execution_time_us = 0;
	pass->is_enabled = 0;
	pass->requires_profiling = 0;
	pass->conflict_count = 0;
}

static inline u8 register_optimization_pass(
	struct OptimizationPass *pass,
	u32 phase_id,
	const char *name,
	enum OptimizationLevel level) {

	if (!pass || !name) return 1;
	pass->phase_id = phase_id;
	pass->name = name;
	pass->level = level;
	pass->is_enabled = 1;
	return 0;
}

static inline u8 add_conflict(
	struct OptimizationPass *pass,
	u32 conflicting_phase_id) {

	if (!pass || pass->conflict_count >= 8) return 1;
	pass->conflicts_with[pass->conflict_count] = conflicting_phase_id;
	pass->conflict_count++;
	return 0;
}

/* ============================================================ */
/* OPTIMIZATION SCHEDULE */
/* ============================================================ */

struct OptimizationSchedule {
	u32 pass_order[45];
	u32 pass_count;
	u8 pass_enabled[45];
	enum OptimizationLevel target_level;
	u64 total_estimated_time_us;
};

static inline void schedule_init(struct OptimizationSchedule *sched) {
	if (!sched) return;
	sched->pass_count = 0;
	sched->target_level = OPT_BALANCED;
	sched->total_estimated_time_us = 0;

	for (u32 i = 0; i < 45; i++) {
		sched->pass_enabled[i] = 0;
	}
}

static inline u8 add_pass_to_schedule(
	struct OptimizationSchedule *sched,
	u32 phase_id) {

	if (!sched || sched->pass_count >= 45) return 1;
	if (sched->pass_enabled[phase_id]) return 0;

	sched->pass_order[sched->pass_count] = phase_id;
	sched->pass_enabled[phase_id] = 1;
	sched->pass_count++;
	return 0;
}

static inline u8 disable_pass(
	struct OptimizationSchedule *sched,
	u32 phase_id) {

	if (!sched || phase_id >= 45) return 1;
	sched->pass_enabled[phase_id] = 0;
	return 0;
}

/* ============================================================ */
/* APPLIED OPTIMIZATIONS */
/* ============================================================ */

struct AppliedOptimizations {
	struct OptimizationPass passes[45];
	u32 pass_count;
	u32 total_improvement_percent;
	u32 size_reduction_percent;
	u32 speed_improvement_percent;
	u8 is_vectorized;
	u8 is_parallelized;
	u8 uses_profile_data;
	u64 total_time_us;
};

static inline void applied_opts_init(struct AppliedOptimizations *opts) {
	if (!opts) return;
	opts->pass_count = 0;
	opts->total_improvement_percent = 0;
	opts->size_reduction_percent = 0;
	opts->speed_improvement_percent = 0;
	opts->is_vectorized = 0;
	opts->is_parallelized = 0;
	opts->uses_profile_data = 0;
	opts->total_time_us = 0;
}

static inline u8 record_applied_pass(
	struct AppliedOptimizations *opts,
	struct OptimizationPass *pass) {

	if (!opts || !pass || opts->pass_count >= 45) return 1;

	opts->passes[opts->pass_count] = *pass;
	opts->pass_count++;
	opts->total_improvement_percent += pass->improvement_percent;
	opts->total_time_us += pass->execution_time_us;
	return 0;
}

/* ============================================================ */
/* TRADE-OFF RESOLUTION */
/* ============================================================ */

struct OptimizationTradeOff {
	enum OptimizationGoal primary_goal;
	enum OptimizationGoal secondary_goal;
	u32 primary_weight;
	u32 secondary_weight;
	u32 conflict_resolution_strategy;
};

static inline u8 resolve_trade_off(
	struct OptimizationTradeOff *tradeoff,
	enum OptimizationGoal goal1,
	enum OptimizationGoal goal2,
	u32 weight1,
	u32 weight2) {

	if (!tradeoff) return 1;
	tradeoff->primary_goal = goal1;
	tradeoff->secondary_goal = goal2;
	tradeoff->primary_weight = weight1;
	tradeoff->secondary_weight = weight2;
	return 0;
}

static inline enum OptimizationGoal get_primary_goal(struct OptimizationTradeOff *to) {
	if (!to) return GOAL_EXECUTION_SPEED;
	return to->primary_goal;
}

static inline enum OptimizationGoal get_secondary_goal(struct OptimizationTradeOff *to) {
	if (!to) return GOAL_CODE_SIZE;
	return to->secondary_goal;
}

/* ============================================================ */
/* OPTIMIZATION COORDINATOR */
/* ============================================================ */

struct OptimizationCoordinator {
	struct OptimizationSchedule schedule;
	struct AppliedOptimizations applied;
	struct OptimizationTradeOff tradeoff;
	enum OptimizationLevel current_level;
	u32 enabled_pass_count;
	u32 disabled_pass_count;
	u8 use_profile_data;
};

static inline void coordinator_init(struct OptimizationCoordinator *coord) {
	if (!coord) return;
	schedule_init(&coord->schedule);
	applied_opts_init(&coord->applied);
	coord->current_level = OPT_BALANCED;
	coord->enabled_pass_count = 0;
	coord->disabled_pass_count = 0;
	coord->use_profile_data = 0;
}

static inline u8 select_passes_for_level(
	struct OptimizationCoordinator *coord,
	enum OptimizationLevel level) {

	if (!coord) return 1;

	schedule_init(&coord->schedule);
	coord->schedule.target_level = level;
	coord->current_level = level;

	switch (level) {
	case OPT_NONE:
		break;
	case OPT_SIZE:
		add_pass_to_schedule(&coord->schedule, 37);
		add_pass_to_schedule(&coord->schedule, 25);
		break;
	case OPT_SPEED:
		add_pass_to_schedule(&coord->schedule, 36);
		add_pass_to_schedule(&coord->schedule, 38);
		add_pass_to_schedule(&coord->schedule, 43);
		add_pass_to_schedule(&coord->schedule, 45);
		break;
	case OPT_BALANCED:
		add_pass_to_schedule(&coord->schedule, 36);
		add_pass_to_schedule(&coord->schedule, 37);
		add_pass_to_schedule(&coord->schedule, 39);
		add_pass_to_schedule(&coord->schedule, 40);
		break;
	case OPT_AGGRESSIVE:
		for (u32 i = 36; i <= 45; i++) {
			add_pass_to_schedule(&coord->schedule, i);
		}
		break;
	}

	return 0;
}

static inline u8 should_enable_pass(
	struct OptimizationCoordinator *coord,
	u32 phase_id) {

	if (!coord || phase_id >= 45) return 0;
	return coord->schedule.pass_enabled[phase_id];
}

static inline u32 get_scheduled_pass_count(struct OptimizationCoordinator *coord) {
	if (!coord) return 0;
	return coord->schedule.pass_count;
}

static inline u32 get_total_optimization_benefit(struct OptimizationCoordinator *coord) {
	if (!coord) return 0;
	return coord->applied.total_improvement_percent;
}

/* ============================================================ */
/* CONSTRAINT RESOLUTION */
/* ============================================================ */

struct PassConstraint {
	u32 pass_id;
	u32 required_features;
	u32 hardware_requirements;
	u8 is_optional;
};

static inline u8 check_pass_constraints(
	struct PassConstraint *constraint,
	u32 available_features) {

	if (!constraint) return 0;
	return (constraint->required_features & available_features) == constraint->required_features;
}

#endif /* APKC_OPT_COORDINATOR_H */
