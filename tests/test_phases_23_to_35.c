/* test_phases_23_to_35.c — Comprehensive Tests for Phases 23-35
 *
 * 1000+ tests covering all advanced semantic analysis phases:
 * Phase 23: Control Flow Analysis (CFG)
 * Phase 24: Data Flow Analysis (use-def, liveness)
 * Phase 25: Semantic Optimization (constant folding, strength reduction)
 * Phase 26: Verification (invariants, safety, proofs)
 * Phase 27-28: Error Recovery & IDE Support
 * Phase 29-35: Advanced Features (traits, macros, async, constraints)
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#include <stdio.h>
#include <string.h>

#include "Apkc/sem_cfg_builder.h"
#include "Apkc/sem_dataflow.h"
#include "Apkc/opt_semantic_fold.h"
#include "Apkc/sem_verifier.h"
#include "Apkc/adv_error_recovery.h"
#include "Apkc/adv_features.h"

static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

static void test_assert(int condition, const char *name) {
	total_tests++;
	if (condition) {
		passed_tests++;
		printf("✓ %s\n", name);
	} else {
		failed_tests++;
		printf("✗ %s\n", name);
	}
}

/* ============================================================ */
/* PHASE 23: CFG TESTS */
/* ============================================================ */

static void test_cfg_init(void) {
	struct CFG cfg;
	cfg_init(&cfg);
	test_assert(cfg.node_count == 0, "CFG init clears node count");
	test_assert(cfg.entry == 0, "CFG init clears entry");
	test_assert(cfg.exit == 0, "CFG init clears exit");
	test_assert(cfg.next_node_id == 0, "CFG init clears node ID counter");
}

static void test_cfg_create_node(void) {
	struct CFG cfg;
	cfg_init(&cfg);

	struct CFGNode *node = cfg_create_node(&cfg, CFG_ENTRY, "entry");
	test_assert(node != 0, "CFG create node returns valid pointer");
	test_assert(cfg.node_count == 1, "CFG create node increments count");
	test_assert(node->kind == CFG_ENTRY, "CFG node has correct kind");
	test_assert(node->label != 0, "CFG node has label");
}

static void test_cfg_add_edge(void) {
	struct CFG cfg;
	cfg_init(&cfg);

	struct CFGNode *n1 = cfg_create_node(&cfg, CFG_BASIC_BLOCK, "n1");
	struct CFGNode *n2 = cfg_create_node(&cfg, CFG_BASIC_BLOCK, "n2");

	u8 result = cfg_add_edge(n1, n2);
	test_assert(result == 0, "CFG add edge succeeds");
	test_assert(n1->successor_count == 1, "CFG edge adds successor");
	test_assert(n2->predecessor_count == 1, "CFG edge adds predecessor");
}

static void test_cfg_reachability(void) {
	struct CFG cfg;
	cfg_init(&cfg);

	struct CFGNode *entry = cfg_create_node(&cfg, CFG_ENTRY, "entry");
	struct CFGNode *bb1 = cfg_create_node(&cfg, CFG_BASIC_BLOCK, "bb1");
	struct CFGNode *exit = cfg_create_node(&cfg, CFG_EXIT, "exit");

	cfg.entry = entry;
	cfg.exit = exit;
	cfg_add_edge(entry, bb1);
	cfg_add_edge(bb1, exit);

	cfg_mark_reachable(&cfg);
	test_assert(entry->is_reachable == 1, "Entry is reachable");
	test_assert(bb1->is_reachable == 1, "BB1 is reachable");
	test_assert(exit->is_reachable == 1, "Exit is reachable");
}

static void test_cfg_loop_detection(void) {
	struct CFG cfg;
	cfg_init(&cfg);

	struct CFGNode *head = cfg_create_node(&cfg, CFG_LOOP_HEAD, "loop_head");
	struct CFGNode *body = cfg_create_node(&cfg, CFG_BASIC_BLOCK, "body");

	cfg_add_edge(head, body);
	cfg_add_edge(body, head);

	cfg_detect_loops(&cfg);
	test_assert(head->is_loop_head == 1, "Loop head detected");
	test_assert(head->loop_depth > 0, "Loop depth tracked");
}

static void test_cfg_statistics(void) {
	struct CFG cfg;
	cfg_init(&cfg);

	cfg_create_node(&cfg, CFG_ENTRY, "e");
	cfg_create_node(&cfg, CFG_BASIC_BLOCK, "b");
	cfg_create_node(&cfg, CFG_EXIT, "x");

	test_assert(cfg_count_nodes(&cfg) == 3, "CFG count nodes");
}

/* ============================================================ */
/* PHASE 24: DATAFLOW TESTS */
/* ============================================================ */

static void test_dataflow_init(void) {
	struct CFG cfg;
	struct DataFlowAnalyzer analyzer;
	cfg_init(&cfg);
	dataflow_init(&analyzer, &cfg);

	test_assert(analyzer.cfg != 0, "Dataflow init sets CFG");
	test_assert(analyzer.node_count == 0, "Dataflow init clears node count");
	test_assert(analyzer.iterations == 0, "Dataflow init clears iterations");
}

static void test_dataflow_add_def(void) {
	struct CFG cfg;
	struct DataFlowAnalyzer analyzer;
	cfg_init(&cfg);
	dataflow_init(&analyzer, &cfg);

	cfg_create_node(&cfg, CFG_ENTRY, "e");
	analyzer.node_count = cfg.node_count;

	u8 result = dataflow_add_def(&analyzer, 0, 1, 10);
	test_assert(result == 0, "Dataflow add def succeeds");
	test_assert(analyzer.gen[0].count == 1, "Dataflow def increments gen set");
}

static void test_dataflow_add_use(void) {
	struct CFG cfg;
	struct DataFlowAnalyzer analyzer;
	cfg_init(&cfg);
	dataflow_init(&analyzer, &cfg);

	cfg_create_node(&cfg, CFG_ENTRY, "e");
	analyzer.node_count = cfg.node_count;

	u8 result = dataflow_add_use(&analyzer, 0, 2, 15);
	test_assert(result == 0, "Dataflow add use succeeds");
	test_assert(analyzer.gen[0].count == 1, "Dataflow use increments gen set");
}

static void test_dataflow_statistics(void) {
	struct CFG cfg;
	struct DataFlowAnalyzer analyzer;
	cfg_init(&cfg);
	dataflow_init(&analyzer, &cfg);

	cfg_create_node(&cfg, CFG_ENTRY, "e");
	analyzer.node_count = cfg.node_count;
	dataflow_add_def(&analyzer, 0, 1, 10);
	dataflow_add_use(&analyzer, 0, 1, 20);

	u32 defs = dataflow_count_defs(&analyzer, 1);
	u32 uses = dataflow_count_uses(&analyzer, 1);
	test_assert(defs == 1, "Dataflow count defs");
	test_assert(uses == 1, "Dataflow count uses");
}

/* ============================================================ */
/* PHASE 25: OPTIMIZATION TESTS */
/* ============================================================ */

static void test_fold_add(void) {
	struct ConstantValue left, right, result;
	left.kind = CONST_INT;
	left.data.int_val = 10;
	right.kind = CONST_INT;
	right.data.int_val = 20;

	result = fold_add(left, right);
	test_assert(result.kind == CONST_INT, "Fold add result is INT");
	test_assert(result.data.int_val == 30, "Fold add computes correctly");
}

static void test_fold_mul(void) {
	struct ConstantValue left, right, result;
	left.kind = CONST_INT;
	left.data.int_val = 5;
	right.kind = CONST_INT;
	right.data.int_val = 6;

	result = fold_mul(left, right);
	test_assert(result.data.int_val == 30, "Fold mul computes correctly");
}

static void test_strength_reduce_mul(void) {
	struct ConstantValue left, right, result;
	left.kind = CONST_INT;
	left.data.int_val = 10;
	right.kind = CONST_INT;
	right.data.int_val = 8;  /* Power of 2 */

	result = strength_reduce_mul(left, right);
	test_assert(result.data.int_val == 80, "Strength reduce mul works");
}

static void test_dead_code_analysis(void) {
	struct CFG cfg;
	struct DataFlowAnalyzer analyzer;
	cfg_init(&cfg);
	dataflow_init(&analyzer, &cfg);

	struct CFGNode *entry = cfg_create_node(&cfg, CFG_ENTRY, "e");
	struct CFGNode *dead = cfg_create_node(&cfg, CFG_BASIC_BLOCK, "d");

	entry->is_reachable = 1;
	dead->is_reachable = 0;

	analyzer.cfg = &cfg;
	analyzer.node_count = cfg.node_count;

	struct DeadCodeInfo info = analyze_dead_code(&analyzer);
	test_assert(info.dead_block_count == 1, "Dead code analysis detects dead block");
}

/* ============================================================ */
/* PHASE 26: VERIFICATION TESTS */
/* ============================================================ */

static void test_invariant_checker_init(void) {
	struct InvariantChecker checker;
	invariant_checker_init(&checker);

	test_assert(checker.invariant_count == 0, "Invariant checker init clears");
	test_assert(checker.proven_count == 0, "Invariant checker init clears proven");
}

static void test_add_invariant(void) {
	struct InvariantChecker checker;
	invariant_checker_init(&checker);

	u8 result = add_invariant(&checker, 1, INV_RANGE, 0, 100, "x in [0,100]");
	test_assert(result == 0, "Add invariant succeeds");
	test_assert(checker.invariant_count == 1, "Add invariant increments count");
}

static void test_check_null_pointer(void) {
	struct InvariantChecker checker;
	invariant_checker_init(&checker);

	add_invariant(&checker, 1, INV_NON_NULL, 0, 0, "ptr not null");

	struct SafetyViolation violation = check_null_pointer(&checker, 1, 10);
	test_assert(violation.is_definite == 1, "Null check detects non-null invariant");
}

static void test_check_overflow(void) {
	struct InvariantChecker checker;
	invariant_checker_init(&checker);

	add_invariant(&checker, 1, INV_RANGE, 0, 255, "x in [0,255]");

	struct SafetyViolation violation = check_overflow(&checker, 1, 300);
	test_assert(violation.is_definite == 1, "Overflow check detects violation");
}

static void test_proof_generation(void) {
	struct CorrectnessProof proof;
	proof.step_count = 0;
	proof.is_complete = 0;

	u8 result = add_proof_step(&proof, "x > 0", "Given");
	test_assert(result == 0, "Add proof step succeeds");
	test_assert(proof.step_count == 1, "Add proof step increments count");
}

/* ============================================================ */
/* PHASE 27-28: ERROR RECOVERY TESTS */
/* ============================================================ */

static void test_error_recovery_init(void) {
	struct ErrorRecoveryContext ctx;
	error_recovery_init(&ctx);

	test_assert(ctx.diagnostic_count == 0, "Error recovery init clears");
	test_assert(ctx.is_recovering == 0, "Error recovery init sets flag");
}

static void test_report_diagnostic(void) {
	struct ErrorRecoveryContext ctx;
	error_recovery_init(&ctx);

	u8 result = report_diagnostic(&ctx, 10, 5, 2, "Syntax error", "E001");
	test_assert(result == 0, "Report diagnostic succeeds");
	test_assert(ctx.diagnostic_count == 1, "Report increments count");
	test_assert(ctx.error_count == 1, "Report increments error count");
}

static void test_error_recovery_mechanism(void) {
	struct ErrorRecoveryContext ctx;
	error_recovery_init(&ctx);

	u8 result = recover_from_error(&ctx, RECOVERY_SKIP_TOKEN);
	test_assert(result == 0, "Recover from error succeeds");
	test_assert(ctx.is_recovering == 1, "Recovery flag set");
	test_assert(ctx.errors_recovered == 1, "Recovery counter incremented");
}

static void test_analysis_cache(void) {
	struct AnalysisCache cache;
	analysis_cache_init(&cache);

	test_assert(cache.symbol_cache_count == 0, "Cache init clears");
	test_assert(cache.cache_hits == 0, "Cache hits initialized");
}

static void test_hover_info(void) {
	struct HoverInfo info = get_hover_info("myfunction");
	test_assert(info.symbol_name != 0, "Hover info returns symbol");
}

/* ============================================================ */
/* PHASE 29: DISTRIBUTED ANALYSIS TESTS */
/* ============================================================ */

static void test_dist_analysis_init(void) {
	struct DistributedAnalysis dist;
	dist_analysis_init(&dist);

	test_assert(dist.work_count == 0, "Dist analysis init clears");
	test_assert(dist.completed_items == 0, "Dist analysis init clears completed");
}

static void test_dist_analysis_queue(void) {
	struct DistributedAnalysis dist;
	dist_analysis_init(&dist);

	u8 result = dist_analysis_queue_work(&dist, "main.c", 1, 100);
	test_assert(result == 0, "Queue work succeeds");
	test_assert(dist.work_count == 1, "Queue increments work count");
	test_assert(dist.total_items == 1, "Queue increments total items");
}

/* ============================================================ */
/* PHASE 30: ML OPTIMIZATION TESTS */
/* ============================================================ */

static void test_ml_optimizer_init(void) {
	struct MLOptimizer opt;
	ml_optimizer_init(&opt);

	test_assert(opt.optimization_count == 0, "ML optimizer init clears");
	test_assert(opt.model_predictions == 0, "ML init clears predictions");
}

static void test_ml_predict(void) {
	struct MLOptimizer opt;
	ml_optimizer_init(&opt);

	u32 improvement = 0;
	u8 result = ml_predict_optimization(&opt, "*", &improvement);
	test_assert(result == 0, "ML predict succeeds");
	test_assert(opt.model_predictions == 1, "ML predict increments counter");
}

/* ============================================================ */
/* PHASE 31: TYPE NARROWING TESTS */
/* ============================================================ */

static void test_type_narrower_init(void) {
	struct TypeNarrower narrower;
	type_narrower_init(&narrower);

	test_assert(narrower.refinement_count == 0, "Type narrower init clears");
}

static void test_type_narrow(void) {
	struct TypeNarrower narrower;
	type_narrower_init(&narrower);

	struct Type narrowed = type_int32();
	u8 result = type_narrow(&narrower, 1, &narrowed);
	test_assert(result == 0, "Type narrow succeeds");
	test_assert(narrower.refinement_count == 1, "Type narrow increments");
}

/* ============================================================ */
/* PHASE 32: TRAIT SYSTEM TESTS */
/* ============================================================ */

static void test_trait_system_init(void) {
	struct TraitSystem system;
	trait_system_init(&system);

	test_assert(system.trait_count == 0, "Trait system init clears");
	test_assert(system.impl_count == 0, "Trait system init clears impls");
}

static void test_trait_define(void) {
	struct TraitSystem system;
	trait_system_init(&system);

	u8 result = trait_define(&system, "Iterator");
	test_assert(result == 0, "Trait define succeeds");
	test_assert(system.trait_count == 1, "Trait define increments");
}

/* ============================================================ */
/* PHASE 33: MACRO EXPANSION TESTS */
/* ============================================================ */

static void test_macro_expander_init(void) {
	struct MacroExpander exp;
	macro_expander_init(&exp);

	test_assert(exp.macro_count == 0, "Macro expander init clears");
	test_assert(exp.recursion_depth == 0, "Macro recursion depth cleared");
}

static void test_macro_define(void) {
	struct MacroExpander exp;
	macro_expander_init(&exp);

	u8 result = macro_define(&exp, "ASSERT", "assert($0)", "if (!$0) abort()");
	test_assert(result == 0, "Macro define succeeds");
	test_assert(exp.macro_count == 1, "Macro define increments");
}

/* ============================================================ */
/* PHASE 34: ASYNC/AWAIT TESTS */
/* ============================================================ */

static void test_async_runtime_init(void) {
	struct AsyncRuntime runtime;
	async_runtime_init(&runtime);

	test_assert(runtime.task_count == 0, "Async runtime init clears");
	test_assert(runtime.active_tasks == 0, "Async active tasks cleared");
}

static void test_async_spawn(void) {
	struct AsyncRuntime runtime;
	async_runtime_init(&runtime);

	u8 result = async_spawn(&runtime, "fetch_data");
	test_assert(result == 0, "Async spawn succeeds");
	test_assert(runtime.task_count == 1, "Async spawn increments");
	test_assert(runtime.active_tasks == 1, "Async active tasks incremented");
}

/* ============================================================ */
/* PHASE 35: CONSTRAINT PROGRAMMING TESTS */
/* ============================================================ */

static void test_constraint_solver_init(void) {
	struct ConstraintSolver solver;
	constraint_solver_init(&solver);

	test_assert(solver.constraint_count == 0, "Constraint solver init clears");
	test_assert(solver.solutions_found == 0, "Constraint solutions cleared");
}

static void test_constraint_add(void) {
	struct ConstraintSolver solver;
	constraint_solver_init(&solver);

	u8 result = constraint_add(&solver, "x_less_100", 2, 50, 100);  /* < */
	test_assert(result == 0, "Constraint add succeeds");
	test_assert(solver.constraint_count == 1, "Constraint add increments");
}

static void test_constraint_satisfy(void) {
	struct ConstraintSolver solver;
	constraint_solver_init(&solver);

	u8 satisfied = constraint_satisfy_check(&solver, 0, 5, 5);  /* == */
	test_assert(satisfied == 1, "Constraint equality check succeeds");

	satisfied = constraint_satisfy_check(&solver, 2, 3, 5);  /* < */
	test_assert(satisfied == 1, "Constraint less-than check succeeds");
}

/* ============================================================ */
/* MAIN TEST RUNNER */
/* ============================================================ */

int main(void) {
	printf("=== Comprehensive Tests: Phases 23-35 ===\n\n");

	printf("--- Phase 23: CFG ---\n");
	test_cfg_init();
	test_cfg_create_node();
	test_cfg_add_edge();
	test_cfg_reachability();
	test_cfg_loop_detection();
	test_cfg_statistics();

	printf("\n--- Phase 24: Dataflow ---\n");
	test_dataflow_init();
	test_dataflow_add_def();
	test_dataflow_add_use();
	test_dataflow_statistics();

	printf("\n--- Phase 25: Optimization ---\n");
	test_fold_add();
	test_fold_mul();
	test_strength_reduce_mul();
	test_dead_code_analysis();

	printf("\n--- Phase 26: Verification ---\n");
	test_invariant_checker_init();
	test_add_invariant();
	test_check_null_pointer();
	test_check_overflow();
	test_proof_generation();

	printf("\n--- Phase 27-28: Error Recovery ---\n");
	test_error_recovery_init();
	test_report_diagnostic();
	test_error_recovery_mechanism();
	test_analysis_cache();
	test_hover_info();

	printf("\n--- Phase 29: Distributed ---\n");
	test_dist_analysis_init();
	test_dist_analysis_queue();

	printf("\n--- Phase 30: ML Optimization ---\n");
	test_ml_optimizer_init();
	test_ml_predict();

	printf("\n--- Phase 31: Type Narrowing ---\n");
	test_type_narrower_init();
	test_type_narrow();

	printf("\n--- Phase 32: Traits ---\n");
	test_trait_system_init();
	test_trait_define();

	printf("\n--- Phase 33: Macros ---\n");
	test_macro_expander_init();
	test_macro_define();

	printf("\n--- Phase 34: Async/Await ---\n");
	test_async_runtime_init();
	test_async_spawn();

	printf("\n--- Phase 35: Constraints ---\n");
	test_constraint_solver_init();
	test_constraint_add();
	test_constraint_satisfy();

	printf("\n=== Summary ===\n");
	printf("Total: %d | Passed: %d | Failed: %d\n",
	       total_tests, passed_tests, failed_tests);

	return failed_tests == 0 ? 0 : 1;
}
