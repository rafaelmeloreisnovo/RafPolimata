/* opt_cross_phase.h — Cross-Phase Integration & Dependency Analysis (Phase 36)
 *
 * Phase 36: Cross-phase integration & dependency analysis
 * - Build inter-phase dependency graphs
 * - Track data flow across phases
 * - Coordinate optimizations across phases
 * - Validate phase invariants
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_CROSS_PHASE_H
#define APKC_OPT_CROSS_PHASE_H 1

#include "adv_features.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* PHASE DEPENDENCY GRAPH */
/* ============================================================ */

enum PhaseKind {
	PHASE_LEXER = 0,
	PHASE_PARSER = 1,
	PHASE_TYPE_CHECKER = 21,
	PHASE_SYMBOL_RESOLVER = 22,
	PHASE_CFG_BUILDER = 23,
	PHASE_DATAFLOW = 24,
	PHASE_OPTIMIZER = 25,
	PHASE_VERIFIER = 26,
	PHASE_CODEGEN = 40
};

struct PhaseDependency {
	u8 from_phase;
	u8 to_phase;
	u8 is_critical;
	u32 data_transferred;
};

struct CrossPhaseGraph {
	struct PhaseDependency dependencies[64];
	u32 dependency_count;
	u32 phases_total;
	u32 critical_deps;
};

static inline void cross_phase_graph_init(struct CrossPhaseGraph *graph) {
	if (!graph) return;
	graph->dependency_count = 0;
	graph->phases_total = 0;
	graph->critical_deps = 0;
}

static inline u8 add_phase_dependency(
	struct CrossPhaseGraph *graph,
	u8 from_phase,
	u8 to_phase,
	u8 is_critical) {

	if (!graph || graph->dependency_count >= 64) return 1;

	struct PhaseDependency *dep = &graph->dependencies[graph->dependency_count];
	dep->from_phase = from_phase;
	dep->to_phase = to_phase;
	dep->is_critical = is_critical;
	dep->data_transferred = 0;

	graph->dependency_count++;
	if (is_critical) graph->critical_deps++;
	return 0;
}

/* ============================================================ */
/* DATA FLOW ACROSS PHASES */
/* ============================================================ */

struct InterPhaseValue {
	u32 value_id;
	u8 source_phase;
	u8 dest_phase;
	u32 line;
	u8 is_live;
};

struct InterPhaseDataFlow {
	struct InterPhaseValue values[128];
	u32 value_count;
	u32 transfers;
	u32 dead_transfers;
};

static inline void inter_phase_dataflow_init(struct InterPhaseDataFlow *df) {
	if (!df) return;
	df->value_count = 0;
	df->transfers = 0;
	df->dead_transfers = 0;
}

static inline u8 track_inter_phase_value(
	struct InterPhaseDataFlow *df,
	u32 value_id,
	u8 src_phase,
	u8 dst_phase) {

	if (!df || df->value_count >= 128) return 1;

	struct InterPhaseValue *val = &df->values[df->value_count];
	val->value_id = value_id;
	val->source_phase = src_phase;
	val->dest_phase = dst_phase;
	val->line = 0;
	val->is_live = 1;

	df->value_count++;
	df->transfers++;
	return 0;
}

/* ============================================================ */
/* PHASE COORDINATION */
/* ============================================================ */

enum CoordinationStrategy {
	COORD_SEQUENTIAL = 0,
	COORD_LAZY = 1,
	COORD_EAGER = 2,
	COORD_INCREMENTAL = 3
};

struct PhaseCoordinator {
	u8 strategy;
	u32 phase_order[45];
	u32 phase_count;
	u32 coordination_points;
	u8 abort_on_error;
};

static inline void phase_coordinator_init(struct PhaseCoordinator *coord) {
	if (!coord) return;
	coord->strategy = COORD_SEQUENTIAL;
	coord->phase_count = 0;
	coord->coordination_points = 0;
	coord->abort_on_error = 1;
}

static inline u8 add_phase_to_order(
	struct PhaseCoordinator *coord,
	u32 phase_id) {

	if (!coord || coord->phase_count >= 45) return 1;

	coord->phase_order[coord->phase_count] = phase_id;
	coord->phase_count++;
	return 0;
}

/* ============================================================ */
/* PHASE INVARIANT TRACKING */
/* ============================================================ */

struct PhaseInvariant {
	u8 phase_id;
	const char *description;
	u32 checks_passed;
	u32 checks_failed;
	u8 is_critical;
};

struct InvariantValidator {
	struct PhaseInvariant invariants[64];
	u32 invariant_count;
	u32 total_checks;
	u32 failed_checks;
};

static inline void invariant_validator_init(struct InvariantValidator *val) {
	if (!val) return;
	val->invariant_count = 0;
	val->total_checks = 0;
	val->failed_checks = 0;
}

static inline u8 register_phase_invariant(
	struct InvariantValidator *val,
	u8 phase_id,
	const char *desc,
	u8 is_critical) {

	if (!val || val->invariant_count >= 64) return 1;

	struct PhaseInvariant *inv = &val->invariants[val->invariant_count];
	inv->phase_id = phase_id;
	inv->description = desc;
	inv->checks_passed = 0;
	inv->checks_failed = 0;
	inv->is_critical = is_critical;

	val->invariant_count++;
	return 0;
}

static inline u8 validate_phase_invariant(
	struct InvariantValidator *val,
	u8 phase_id,
	u8 condition) {

	if (!val) return 1;

	u32 i;
	for (i = 0; i < val->invariant_count; i++) {
		if (val->invariants[i].phase_id == phase_id) {
			val->total_checks++;
			if (condition) {
				val->invariants[i].checks_passed++;
			} else {
				val->invariants[i].checks_failed++;
				val->failed_checks++;
				if (val->invariants[i].is_critical) {
					return 1;
				}
			}
			return 0;
		}
	}
	return 1;
}

/* ============================================================ */
/* OPTIMIZATION COORDINATION */
/* ============================================================ */

struct CrossPhaseOptimization {
	u8 from_phase;
	u8 to_phase;
	const char *optimization_name;
	u32 opportunity_count;
	u32 applied_count;
	u64 bytes_saved;
};

struct OptimizationCoordinator {
	struct CrossPhaseOptimization optimizations[32];
	u32 optimization_count;
	u64 total_savings;
};

static inline void optimization_coordinator_init(struct OptimizationCoordinator *opt) {
	if (!opt) return;
	opt->optimization_count = 0;
	opt->total_savings = 0;
}

static inline u8 register_cross_phase_optimization(
	struct OptimizationCoordinator *opt,
	u8 from_phase,
	u8 to_phase,
	const char *name) {

	if (!opt || opt->optimization_count >= 32) return 1;

	struct CrossPhaseOptimization *co = &opt->optimizations[opt->optimization_count];
	co->from_phase = from_phase;
	co->to_phase = to_phase;
	co->optimization_name = name;
	co->opportunity_count = 0;
	co->applied_count = 0;
	co->bytes_saved = 0;

	opt->optimization_count++;
	return 0;
}

/* ============================================================ */
/* PHASE STATISTICS & METRICS */
/* ============================================================ */

struct PhaseMetrics {
	u8 phase_id;
	u32 input_size;
	u32 output_size;
	u64 compilation_time_us;
	u32 memory_peak_bytes;
	u32 errors_caught;
	u8 phase_success;
};

struct CrossPhaseMetrics {
	struct PhaseMetrics metrics[45];
	u32 total_phases;
	u64 total_time_us;
	u32 total_memory_bytes;
	u32 total_errors;
};

static inline void cross_phase_metrics_init(struct CrossPhaseMetrics *metrics) {
	if (!metrics) return;
	metrics->total_phases = 0;
	metrics->total_time_us = 0;
	metrics->total_memory_bytes = 0;
	metrics->total_errors = 0;
}

static inline u8 record_phase_metrics(
	struct CrossPhaseMetrics *metrics,
	u8 phase_id,
	u32 in_size,
	u32 out_size,
	u64 time_us) {

	if (!metrics || metrics->total_phases >= 45) return 1;

	struct PhaseMetrics *pm = &metrics->metrics[metrics->total_phases];
	pm->phase_id = phase_id;
	pm->input_size = in_size;
	pm->output_size = out_size;
	pm->compilation_time_us = time_us;
	pm->memory_peak_bytes = 0;
	pm->errors_caught = 0;
	pm->phase_success = 1;

	metrics->total_phases++;
	metrics->total_time_us += time_us;
	return 0;
}

#endif /* APKC_OPT_CROSS_PHASE_H */
