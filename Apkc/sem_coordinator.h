/* sem_coordinator.h — Semantic Analysis Phase Coordinator (Phase 46)
 *
 * Phase 46: Orchestrates execution of semantic analysis phases 21-45
 * - Sequential phase execution with error handling
 * - Shared execution context across all phases
 * - Phase dependency verification
 * - Error propagation and halt logic
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEM_COORDINATOR_H
#define APKC_SEM_COORDINATOR_H 1

#include "sem_type_system.h"
#include "sem_symbol_table.h"
#include "sem_cfg_builder.h"
#include "sem_dataflow.h"
#include "opt_semantic_fold.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* PHASE EXECUTION CONTEXT */
/* ============================================================ */

enum SemanticPhaseStatus {
	PHASE_NOT_STARTED = 0,
	PHASE_RUNNING = 1,
	PHASE_SUCCESS = 2,
	PHASE_FAILED = 3,
	PHASE_SKIPPED = 4
};

struct SemanticExecutionContext {
	void *types;
	void *symbols;
	void *cfg;
	void *dataflow;
	void *optimizer;

	u32 phase_status[45];
	u32 phase_count;
	u32 error_count;
	u32 warning_count;
	u32 completed_phases;
	u8 halt_on_error;
	u64 execution_time_us;
};

static inline void semantic_context_init(struct SemanticExecutionContext *ctx) {
	if (!ctx) return;

	ctx->types = 0;
	ctx->symbols = 0;
	ctx->cfg = 0;
	ctx->dataflow = 0;
	ctx->optimizer = 0;

	for (u32 i = 0; i < 45; i++) {
		ctx->phase_status[i] = PHASE_NOT_STARTED;
	}
	ctx->phase_count = 0;
	ctx->error_count = 0;
	ctx->warning_count = 0;
	ctx->completed_phases = 0;
	ctx->halt_on_error = 1;
	ctx->execution_time_us = 0;
}

/* ============================================================ */
/* PHASE REGISTRY */
/* ============================================================ */

struct PhaseMetadata {
	u32 phase_id;
	const char *name;
	const char *description;
	u32 prerequisites[5];
	u32 prerequisite_count;
	u8 is_optional;
	u32 estimated_time_us;
};

struct PhaseRegistry {
	struct PhaseMetadata phases[45];
	u32 phase_count;
};

static inline void phase_registry_init(struct PhaseRegistry *reg) {
	if (!reg) return;
	reg->phase_count = 0;
}

static inline u8 register_phase(
	struct PhaseRegistry *reg,
	u32 phase_id,
	const char *name) {

	if (!reg || reg->phase_count >= 45) return 1;

	struct PhaseMetadata *pm = &reg->phases[reg->phase_count];
	pm->phase_id = phase_id;
	pm->name = name;
	pm->description = 0;
	pm->prerequisite_count = 0;
	pm->is_optional = 0;
	pm->estimated_time_us = 0;

	reg->phase_count++;
	return 0;
}

static inline u8 phase_prerequisites_met(
	struct SemanticExecutionContext *ctx,
	struct PhaseMetadata *phase) {

	if (!ctx || !phase) return 0;

	for (u32 i = 0; i < phase->prerequisite_count; i++) {
		u32 prereq_id = phase->prerequisites[i];
		if (ctx->phase_status[prereq_id] != PHASE_SUCCESS) {
			return 0;
		}
	}
	return 1;
}

/* ============================================================ */
/* EXECUTION PLAN */
/* ============================================================ */

struct ExecutionPlan {
	u32 phase_order[45];
	u32 phase_count;
	u8 phase_enabled[45];
	u64 total_estimated_time_us;
};

static inline void execution_plan_init(struct ExecutionPlan *plan) {
	if (!plan) return;
	plan->phase_count = 0;
	plan->total_estimated_time_us = 0;

	for (u32 i = 0; i < 45; i++) {
		plan->phase_enabled[i] = 1;
	}
}

static inline u8 add_phase_to_plan(
	struct ExecutionPlan *plan,
	u32 phase_id,
	u64 estimated_time_us) {

	if (!plan || plan->phase_count >= 45) return 1;

	plan->phase_order[plan->phase_count] = phase_id;
	plan->phase_count++;
	plan->total_estimated_time_us += estimated_time_us;
	return 0;
}

static inline u8 disable_optimization_phase(struct ExecutionPlan *plan, u32 phase_id) {
	if (!plan || phase_id >= 45) return 1;
	plan->phase_enabled[phase_id] = 0;
	return 0;
}

/* ============================================================ */
/* PHASE EXECUTION */
/* ============================================================ */

enum ExecutionResult {
	EXEC_SUCCESS = 0,
	EXEC_TYPE_ERROR = 1,
	EXEC_SYMBOL_ERROR = 2,
	EXEC_CFG_ERROR = 3,
	EXEC_DATAFLOW_ERROR = 4,
	EXEC_OPTIMIZATION_ERROR = 5,
	EXEC_FATAL_ERROR = 6
};

struct PhaseResult {
	enum ExecutionResult status;
	const char *error_message;
	u32 error_code;
	u32 lines_processed;
	u64 execution_time_us;
};

static inline struct PhaseResult execute_phase(
	struct SemanticExecutionContext *ctx,
	u32 phase_id) {

	struct PhaseResult result;
	result.status = EXEC_SUCCESS;
	result.error_message = 0;
	result.error_code = 0;
	result.lines_processed = 0;
	result.execution_time_us = 0;

	if (!ctx || phase_id >= 45) {
		result.status = EXEC_FATAL_ERROR;
		return result;
	}

	ctx->phase_status[phase_id] = PHASE_RUNNING;
	return result;
}

static inline u8 record_phase_success(
	struct SemanticExecutionContext *ctx,
	u32 phase_id,
	u64 execution_time_us) {

	if (!ctx || phase_id >= 45) return 1;

	ctx->phase_status[phase_id] = PHASE_SUCCESS;
	ctx->completed_phases++;
	ctx->execution_time_us += execution_time_us;
	return 0;
}

static inline u8 record_phase_failure(
	struct SemanticExecutionContext *ctx,
	u32 phase_id,
	enum ExecutionResult status) {

	if (!ctx || phase_id >= 45) return 1;

	ctx->phase_status[phase_id] = PHASE_FAILED;
	ctx->error_count++;

	if (ctx->halt_on_error && status >= EXEC_FATAL_ERROR) {
		return 1;
	}
	return 0;
}

/* ============================================================ */
/* PIPELINE COORDINATOR */
/* ============================================================ */

struct PipelineCoordinator {
	struct SemanticExecutionContext *context;
	struct PhaseRegistry registry;
	struct ExecutionPlan plan;
	u32 current_phase;
	u8 is_executing;
};

static inline void pipeline_init(struct PipelineCoordinator *pipe) {
	if (!pipe) return;
	phase_registry_init(&pipe->registry);
	execution_plan_init(&pipe->plan);
	pipe->current_phase = 0;
	pipe->is_executing = 0;
}

static inline u8 start_pipeline_execution(struct PipelineCoordinator *pipe) {
	if (!pipe || pipe->is_executing) return 1;
	pipe->is_executing = 1;
	pipe->current_phase = 0;
	return 0;
}

static inline u8 end_pipeline_execution(struct PipelineCoordinator *pipe) {
	if (!pipe) return 1;
	pipe->is_executing = 0;
	return 0;
}

static inline u32 get_completed_phase_count(struct PipelineCoordinator *pipe) {
	if (!pipe || !pipe->context) return 0;
	return pipe->context->completed_phases;
}

static inline u32 get_total_error_count(struct PipelineCoordinator *pipe) {
	if (!pipe || !pipe->context) return 0;
	return pipe->context->error_count;
}

#endif /* APKC_SEM_COORDINATOR_H */
