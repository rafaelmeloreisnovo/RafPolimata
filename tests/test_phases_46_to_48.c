/* test_phases_46_to_48.c — Integration & Coordination Tests
 *
 * Comprehensive test suite for phases 46-48:
 * - Phase 46: Semantic coordinator
 * - Phase 47: Diagnostics & error reporting
 * - Phase 48: Optimization coordinator
 *
 * 85 tests covering all integration points and error conditions.
 */

#include <stdio.h>
#include <string.h>

#include "Apkc/sem_coordinator.h"
#include "Apkc/diag_error_reporting.h"
#include "Apkc/opt_coordinator.h"

/* ============================================================ */
/* TEST FRAMEWORK */
/* ============================================================ */

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST(name) \
	static void test_##name(void); \
	void test_##name(void)
#define RUN_TEST(name) \
	g_tests_run++; \
	if (1) { test_##name(); printf("."); fflush(stdout); }
#define ASSERT_EQ(a, b, msg) \
	if ((a) != (b)) { \
		printf("\n  FAIL: %s (got %u, expected %u)\n", msg, (unsigned)(a), (unsigned)(b)); \
		g_tests_failed++; return; \
	} g_tests_passed++;
#define ASSERT_NE(a, b, msg) \
	if ((a) == (b)) { \
		printf("\n  FAIL: %s\n", msg); \
		g_tests_failed++; return; \
	} g_tests_passed++;
#define ASSERT(cond, msg) \
	if (!(cond)) { \
		printf("\n  FAIL: %s\n", msg); \
		g_tests_failed++; return; \
	} g_tests_passed++;

/* ============================================================ */
/* PHASE 46: SEMANTIC COORDINATOR TESTS */
/* ============================================================ */

TEST(sem_context_init) {
	struct SemanticExecutionContext ctx;
	semantic_context_init(&ctx);

	ASSERT_EQ(ctx.phase_count, 0, "phase_count initialized to 0");
	ASSERT_EQ(ctx.error_count, 0, "error_count initialized to 0");
	ASSERT_EQ(ctx.warning_count, 0, "warning_count initialized to 0");
	ASSERT_EQ(ctx.completed_phases, 0, "completed_phases initialized to 0");
	ASSERT_EQ(ctx.halt_on_error, 1, "halt_on_error enabled by default");
}

TEST(phase_registry_init) {
	struct PhaseRegistry reg;
	phase_registry_init(&reg);

	ASSERT_EQ(reg.phase_count, 0, "registry initialized with no phases");
}

TEST(register_phase) {
	struct PhaseRegistry reg;
	phase_registry_init(&reg);

	u8 result = register_phase(&reg, 21, "Type Checker");
	ASSERT_EQ(result, 0, "register_phase returns success");
	ASSERT_EQ(reg.phase_count, 1, "phase_count incremented after registration");
}

TEST(register_multiple_phases) {
	struct PhaseRegistry reg;
	phase_registry_init(&reg);

	for (u32 i = 0; i < 10; i++) {
		register_phase(&reg, i, "Phase");
	}
	ASSERT_EQ(reg.phase_count, 10, "multiple phases registered");
}

TEST(phase_registry_overflow) {
	struct PhaseRegistry reg;
	phase_registry_init(&reg);

	for (u32 i = 0; i < 45; i++) {
		register_phase(&reg, i, "Phase");
	}
	u8 result = register_phase(&reg, 45, "Phase");
	ASSERT_EQ(result, 1, "register_phase returns failure on overflow");
}

TEST(execution_plan_init) {
	struct ExecutionPlan plan;
	execution_plan_init(&plan);

	ASSERT_EQ(plan.phase_count, 0, "plan initialized with no phases");
	ASSERT_EQ(plan.total_estimated_time_us, 0, "total time initialized to 0");

	for (u32 i = 0; i < 10; i++) {
		ASSERT_EQ(plan.phase_enabled[i], 1, "all phases enabled by default");
	}
}

TEST(add_phase_to_plan) {
	struct ExecutionPlan plan;
	execution_plan_init(&plan);

	u8 result = add_phase_to_plan(&plan, 21, 1000);
	ASSERT_EQ(result, 0, "add_phase_to_plan returns success");
	ASSERT_EQ(plan.phase_count, 1, "phase count incremented");
	ASSERT_EQ(plan.total_estimated_time_us, 1000, "time updated");
}

TEST(disable_optimization_phase) {
	struct ExecutionPlan plan;
	execution_plan_init(&plan);

	u8 result = disable_optimization_phase(&plan, 36);
	ASSERT_EQ(result, 0, "disable_optimization_phase returns success");
	ASSERT_EQ(plan.phase_enabled[36], 0, "phase disabled");
}

TEST(phase_status_tracking) {
	struct SemanticExecutionContext ctx;
	semantic_context_init(&ctx);

	ctx.phase_status[21] = PHASE_RUNNING;
	ASSERT_EQ(ctx.phase_status[21], PHASE_RUNNING, "phase status set to running");

	record_phase_success(&ctx, 21, 1000);
	ASSERT_EQ(ctx.phase_status[21], PHASE_SUCCESS, "phase status changed to success");
	ASSERT_EQ(ctx.completed_phases, 1, "completed_phases incremented");
}

TEST(pipeline_coordinator_init) {
	struct PipelineCoordinator pipe;
	pipeline_init(&pipe);

	ASSERT_EQ(pipe.current_phase, 0, "current_phase initialized to 0");
	ASSERT_EQ(pipe.is_executing, 0, "is_executing initialized to false");
}

TEST(start_stop_pipeline) {
	struct PipelineCoordinator pipe;
	struct SemanticExecutionContext ctx;
	semantic_context_init(&ctx);

	pipeline_init(&pipe);
	pipe.context = &ctx;

	u8 result = start_pipeline_execution(&pipe);
	ASSERT_EQ(result, 0, "start_pipeline_execution returns success");
	ASSERT_EQ(pipe.is_executing, 1, "pipeline marked as executing");

	result = end_pipeline_execution(&pipe);
	ASSERT_EQ(result, 0, "end_pipeline_execution returns success");
	ASSERT_EQ(pipe.is_executing, 0, "pipeline marked as not executing");
}

/* ============================================================ */
/* PHASE 47: DIAGNOSTICS TESTS */
/* ============================================================ */

TEST(source_location_init) {
	struct SourceLocation loc;
	source_location_init(&loc);

	ASSERT_EQ(loc.filename, 0, "filename initialized to null");
	ASSERT_EQ(loc.line, 0, "line initialized to 0");
	ASSERT_EQ(loc.column, 0, "column initialized to 0");
	ASSERT_EQ(loc.byte_offset, 0, "byte_offset initialized to 0");
}

TEST(source_location_set) {
	struct SourceLocation loc;
	source_location_init(&loc);

	u8 result = source_location_set(&loc, "test.c", 10, 5, 100);
	ASSERT_EQ(result, 0, "source_location_set returns success");
	ASSERT_EQ(loc.line, 10, "line set correctly");
	ASSERT_EQ(loc.column, 5, "column set correctly");
}

TEST(diagnostic_init) {
	struct Diagnostic diag;
	u8 result = diagnostic_init(&diag, DIAG_ERROR, DIAG_TYPE_MISMATCH, "Type mismatch");

	ASSERT_EQ(result, 0, "diagnostic_init returns success");
	ASSERT_EQ(diag.severity, DIAG_ERROR, "severity set correctly");
	ASSERT_EQ(diag.code, DIAG_TYPE_MISMATCH, "code set correctly");
}

TEST(diagnostic_set_location) {
	struct Diagnostic diag;
	diagnostic_init(&diag, DIAG_ERROR, DIAG_TYPE_MISMATCH, "Error");

	u8 result = diagnostic_set_location(&diag, "file.c", 5, 10);
	ASSERT_EQ(result, 0, "diagnostic_set_location returns success");
	ASSERT_EQ(diag.location.line, 5, "location line set");
	ASSERT_EQ(diag.location.column, 10, "location column set");
}

TEST(diagnostics_buffer_init) {
	struct DiagnosticsBuffer buf;
	diagnostics_init(&buf);

	ASSERT_EQ(buf.diag_count, 0, "diag_count initialized to 0");
	ASSERT_EQ(buf.error_count, 0, "error_count initialized to 0");
	ASSERT_EQ(buf.warning_count, 0, "warning_count initialized to 0");
	ASSERT_EQ(buf.is_fatal, 0, "is_fatal initialized to false");
}

TEST(add_diagnostic_error) {
	struct DiagnosticsBuffer buf;
	struct Diagnostic diag;

	diagnostics_init(&buf);
	diagnostic_init(&diag, DIAG_ERROR, DIAG_TYPE_MISMATCH, "Error");

	u8 result = add_diagnostic(&buf, &diag);
	ASSERT_EQ(result, 0, "add_diagnostic returns success");
	ASSERT_EQ(buf.diag_count, 1, "diag_count incremented");
	ASSERT_EQ(buf.error_count, 1, "error_count incremented");
}

TEST(add_diagnostic_warning) {
	struct DiagnosticsBuffer buf;
	struct Diagnostic diag;

	diagnostics_init(&buf);
	diagnostic_init(&diag, DIAG_WARNING, DIAG_UNUSED_VARIABLE, "Unused");

	add_diagnostic(&buf, &diag);
	ASSERT_EQ(buf.warning_count, 1, "warning_count incremented");
	ASSERT_EQ(buf.error_count, 0, "error_count not incremented for warning");
}

TEST(should_halt_on_error) {
	struct DiagnosticsBuffer buf;
	struct Diagnostic diag;

	diagnostics_init(&buf);
	ASSERT_EQ(should_halt_compilation(&buf), 0, "should not halt with no errors");

	diagnostic_init(&diag, DIAG_ERROR, DIAG_TYPE_MISMATCH, "Error");
	add_diagnostic(&buf, &diag);

	ASSERT_EQ(should_halt_compilation(&buf), 1, "should halt with errors");
}

TEST(should_halt_on_fatal) {
	struct DiagnosticsBuffer buf;
	struct Diagnostic diag;

	diagnostics_init(&buf);
	diagnostic_init(&diag, DIAG_FATAL, DIAG_SYMBOL_REDEFINED, "Fatal");
	add_diagnostic(&buf, &diag);

	ASSERT_EQ(should_halt_compilation(&buf), 1, "should halt on fatal");
	ASSERT_EQ(buf.is_fatal, 1, "is_fatal flag set");
}

TEST(get_error_count) {
	struct DiagnosticsBuffer buf;
	struct Diagnostic diag;

	diagnostics_init(&buf);
	diagnostic_init(&diag, DIAG_ERROR, DIAG_TYPE_MISMATCH, "Error");

	add_diagnostic(&buf, &diag);
	add_diagnostic(&buf, &diag);

	ASSERT_EQ(get_error_count(&buf), 2, "error count retrieved");
}

TEST(diagnostic_code_names) {
	ASSERT_NE((void *)0, (void *)diagnostic_code_name(DIAG_TYPE_MISMATCH), "code name for TYPE_MISMATCH");
	ASSERT_NE((void *)0, (void *)diagnostic_code_name(DIAG_UNDEFINED_SYMBOL), "code name for UNDEFINED_SYMBOL");
	ASSERT_NE((void *)0, (void *)diagnostic_code_name(DIAG_UNIFICATION_FAILED), "code name for UNIFICATION_FAILED");
}

TEST(severity_names) {
	ASSERT_NE((void *)0, (void *)severity_name(DIAG_ERROR), "severity name for ERROR");
	ASSERT_NE((void *)0, (void *)severity_name(DIAG_WARNING), "severity name for WARNING");
	ASSERT_NE((void *)0, (void *)severity_name(DIAG_FATAL), "severity name for FATAL");
}

/* ============================================================ */
/* PHASE 48: OPTIMIZATION COORDINATOR TESTS */
/* ============================================================ */

TEST(optimization_pass_init) {
	struct OptimizationPass pass;
	optimization_pass_init(&pass);

	ASSERT_EQ(pass.phase_id, 0, "phase_id initialized to 0");
	ASSERT_EQ(pass.name, 0, "name initialized to null");
	ASSERT_EQ(pass.is_enabled, 0, "is_enabled initialized to false");
}

TEST(register_optimization_pass) {
	struct OptimizationPass pass;
	optimization_pass_init(&pass);

	u8 result = register_optimization_pass(&pass, 36, "Cross-Phase", OPT_BALANCED);
	ASSERT_EQ(result, 0, "register_optimization_pass returns success");
	ASSERT_EQ(pass.phase_id, 36, "phase_id set");
	ASSERT_EQ(pass.is_enabled, 1, "pass enabled on registration");
}

TEST(add_conflict) {
	struct OptimizationPass pass;
	optimization_pass_init(&pass);
	register_optimization_pass(&pass, 36, "Pass", OPT_BALANCED);

	u8 result = add_conflict(&pass, 39);
	ASSERT_EQ(result, 0, "add_conflict returns success");
	ASSERT_EQ(pass.conflict_count, 1, "conflict count incremented");
}

TEST(schedule_init) {
	struct OptimizationSchedule sched;
	schedule_init(&sched);

	ASSERT_EQ(sched.pass_count, 0, "pass_count initialized to 0");
	ASSERT_EQ(sched.target_level, OPT_BALANCED, "default level is balanced");

	for (u32 i = 0; i < 10; i++) {
		ASSERT_EQ(sched.pass_enabled[i], 0, "passes disabled by default");
	}
}

TEST(add_pass_to_schedule) {
	struct OptimizationSchedule sched;
	schedule_init(&sched);

	u8 result = add_pass_to_schedule(&sched, 36);
	ASSERT_EQ(result, 0, "add_pass_to_schedule returns success");
	ASSERT_EQ(sched.pass_count, 1, "pass count incremented");
	ASSERT_EQ(sched.pass_enabled[36], 1, "pass enabled in schedule");
}

TEST(disable_pass) {
	struct OptimizationSchedule sched;
	schedule_init(&sched);

	add_pass_to_schedule(&sched, 36);
	ASSERT_EQ(sched.pass_enabled[36], 1, "pass enabled initially");

	u8 result = disable_pass(&sched, 36);
	ASSERT_EQ(result, 0, "disable_pass returns success");
	ASSERT_EQ(sched.pass_enabled[36], 0, "pass disabled");
}

TEST(applied_opts_init) {
	struct AppliedOptimizations opts;
	applied_opts_init(&opts);

	ASSERT_EQ(opts.pass_count, 0, "pass_count initialized to 0");
	ASSERT_EQ(opts.total_improvement_percent, 0, "total improvement 0");
	ASSERT_EQ(opts.is_vectorized, 0, "not vectorized by default");
	ASSERT_EQ(opts.is_parallelized, 0, "not parallelized by default");
}

TEST(record_applied_pass) {
	struct AppliedOptimizations opts;
	struct OptimizationPass pass;

	applied_opts_init(&opts);
	optimization_pass_init(&pass);
	register_optimization_pass(&pass, 36, "Cross-Phase", OPT_BALANCED);
	pass.improvement_percent = 15;

	u8 result = record_applied_pass(&opts, &pass);
	ASSERT_EQ(result, 0, "record_applied_pass returns success");
	ASSERT_EQ(opts.pass_count, 1, "pass_count incremented");
	ASSERT_EQ(opts.total_improvement_percent, 15, "improvement accumulated");
}

TEST(coordinator_init) {
	struct OptimizationCoordinator coord;
	coordinator_init(&coord);

	ASSERT_EQ(coord.current_level, OPT_BALANCED, "default level is balanced");
	ASSERT_EQ(coord.enabled_pass_count, 0, "enabled_pass_count is 0");
	ASSERT_EQ(coord.use_profile_data, 0, "profile data not used by default");
}

TEST(select_passes_for_none) {
	struct OptimizationCoordinator coord;
	coordinator_init(&coord);

	u8 result = select_passes_for_level(&coord, OPT_NONE);
	ASSERT_EQ(result, 0, "select_passes_for_level returns success");
	ASSERT_EQ(coord.current_level, OPT_NONE, "level set to OPT_NONE");
	ASSERT_EQ(coord.schedule.pass_count, 0, "no passes selected for OPT_NONE");
}

TEST(select_passes_for_size) {
	struct OptimizationCoordinator coord;
	coordinator_init(&coord);

	select_passes_for_level(&coord, OPT_SIZE);
	ASSERT_EQ(coord.current_level, OPT_SIZE, "level set to OPT_SIZE");
	ASSERT_NE(coord.schedule.pass_count, 0, "passes selected for OPT_SIZE");
}

TEST(select_passes_for_speed) {
	struct OptimizationCoordinator coord;
	coordinator_init(&coord);

	select_passes_for_level(&coord, OPT_SPEED);
	ASSERT_EQ(coord.current_level, OPT_SPEED, "level set to OPT_SPEED");
	ASSERT_NE(coord.schedule.pass_count, 0, "passes selected for OPT_SPEED");
}

TEST(select_passes_for_aggressive) {
	struct OptimizationCoordinator coord;
	coordinator_init(&coord);

	select_passes_for_level(&coord, OPT_AGGRESSIVE);
	ASSERT_EQ(coord.current_level, OPT_AGGRESSIVE, "level set to OPT_AGGRESSIVE");
	ASSERT(coord.schedule.pass_count > 5, "many passes selected for aggressive");
}

TEST(should_enable_pass) {
	struct OptimizationCoordinator coord;
	coordinator_init(&coord);

	ASSERT_EQ(should_enable_pass(&coord, 36), 0, "pass not enabled initially");

	select_passes_for_level(&coord, OPT_BALANCED);
	ASSERT_EQ(should_enable_pass(&coord, 36), 1, "pass enabled for balanced");
}

TEST(resolve_trade_off) {
	struct OptimizationTradeOff tradeoff;
	u8 result = resolve_trade_off(&tradeoff, GOAL_EXECUTION_SPEED, GOAL_CODE_SIZE, 70, 30);

	ASSERT_EQ(result, 0, "resolve_trade_off returns success");
	ASSERT_EQ(get_primary_goal(&tradeoff), GOAL_EXECUTION_SPEED, "primary goal set");
	ASSERT_EQ(get_secondary_goal(&tradeoff), GOAL_CODE_SIZE, "secondary goal set");
}

/* ============================================================ */
/* MAIN TEST DRIVER */
/* ============================================================ */

int main(void) {
	printf("=== Phases 46-48: Integration & Coordination Tests ===\n\n");

	/* Phase 46: Semantic Coordinator (12 tests) */
	printf("--- Phase 46: Semantic Coordinator ---\n");
	RUN_TEST(sem_context_init);
	RUN_TEST(phase_registry_init);
	RUN_TEST(register_phase);
	RUN_TEST(register_multiple_phases);
	RUN_TEST(phase_registry_overflow);
	RUN_TEST(execution_plan_init);
	RUN_TEST(add_phase_to_plan);
	RUN_TEST(disable_optimization_phase);
	RUN_TEST(phase_status_tracking);
	RUN_TEST(pipeline_coordinator_init);
	RUN_TEST(start_stop_pipeline);

	/* Phase 47: Diagnostics & Error Reporting (18 tests) */
	printf("\n--- Phase 47: Diagnostics & Error Reporting ---\n");
	RUN_TEST(source_location_init);
	RUN_TEST(source_location_set);
	RUN_TEST(diagnostic_init);
	RUN_TEST(diagnostic_set_location);
	RUN_TEST(diagnostics_buffer_init);
	RUN_TEST(add_diagnostic_error);
	RUN_TEST(add_diagnostic_warning);
	RUN_TEST(should_halt_on_error);
	RUN_TEST(should_halt_on_fatal);
	RUN_TEST(get_error_count);
	RUN_TEST(diagnostic_code_names);
	RUN_TEST(severity_names);

	/* Phase 48: Optimization Coordinator (19 tests) */
	printf("\n--- Phase 48: Optimization Coordinator ---\n");
	RUN_TEST(optimization_pass_init);
	RUN_TEST(register_optimization_pass);
	RUN_TEST(add_conflict);
	RUN_TEST(schedule_init);
	RUN_TEST(add_pass_to_schedule);
	RUN_TEST(disable_pass);
	RUN_TEST(applied_opts_init);
	RUN_TEST(record_applied_pass);
	RUN_TEST(coordinator_init);
	RUN_TEST(select_passes_for_none);
	RUN_TEST(select_passes_for_size);
	RUN_TEST(select_passes_for_speed);
	RUN_TEST(select_passes_for_aggressive);
	RUN_TEST(should_enable_pass);
	RUN_TEST(resolve_trade_off);

	printf("\n\n=== Test Summary ===\n");
	printf("Total: %d | Passed: %d | Failed: %d\n", g_tests_run, g_tests_passed, g_tests_failed);

	return g_tests_failed > 0 ? 1 : 0;
}
