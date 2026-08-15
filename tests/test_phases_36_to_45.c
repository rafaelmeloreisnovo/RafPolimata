/* test_phases_36_to_45.c — Comprehensive Tests for Phases 36-45
 *
 * 100+ tests covering all advanced optimization phases:
 * Phase 36: Cross-phase integration & dependency analysis
 * Phase 37: Whole-program analysis & global optimizations
 * Phase 38: Target-specific optimizations (ARM64 SIMD)
 * Phase 39: Link-time optimization (LTO)
 * Phase 40: Profile-guided optimization (PGO)
 * Phase 41: Interprocedural analysis (IPA)
 * Phase 42: Speculative optimization & devirtualization
 * Phase 43: Parallelization & auto-vectorization
 * Phase 44: Cache hierarchy optimization
 * Phase 45: Runtime specialization & JIT compilation
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#include <stdio.h>
#include <string.h>

#include "Apkc/opt_cross_phase.h"
#include "Apkc/opt_whole_program.h"
#include "Apkc/opt_target_specific.h"
#include "Apkc/opt_link_time.h"
#include "Apkc/opt_profile_guided.h"
#include "Apkc/opt_interprocedural.h"
#include "Apkc/opt_speculative.h"
#include "Apkc/opt_parallelization.h"
#include "Apkc/opt_cache_hierarchy.h"
#include "Apkc/opt_runtime_specialization.h"

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
/* PHASE 36: CROSS-PHASE INTEGRATION TESTS */
/* ============================================================ */

static void test_cross_phase_graph_init(void) {
	struct CrossPhaseGraph graph;
	cross_phase_graph_init(&graph);
	test_assert(graph.dependency_count == 0, "Cross phase graph init");
	test_assert(graph.phases_total == 0, "Graph phases cleared");
	test_assert(graph.critical_deps == 0, "Critical deps cleared");
}

static void test_add_phase_dependency(void) {
	struct CrossPhaseGraph graph;
	cross_phase_graph_init(&graph);
	u8 result = add_phase_dependency(&graph, 21, 22, 1);
	test_assert(result == 0, "Add phase dependency succeeds");
	test_assert(graph.dependency_count == 1, "Dependency count incremented");
	test_assert(graph.critical_deps == 1, "Critical dep tracked");
}

static void test_inter_phase_dataflow_init(void) {
	struct InterPhaseDataFlow df;
	inter_phase_dataflow_init(&df);
	test_assert(df.value_count == 0, "Dataflow init clears values");
	test_assert(df.transfers == 0, "Transfers zeroed");
	test_assert(df.dead_transfers == 0, "Dead transfers zeroed");
}

static void test_track_inter_phase_value(void) {
	struct InterPhaseDataFlow df;
	inter_phase_dataflow_init(&df);
	u8 result = track_inter_phase_value(&df, 1, 21, 22);
	test_assert(result == 0, "Track inter-phase value succeeds");
	test_assert(df.value_count == 1, "Value count incremented");
	test_assert(df.transfers == 1, "Transfer count incremented");
}

static void test_phase_coordinator_init(void) {
	struct PhaseCoordinator coord;
	phase_coordinator_init(&coord);
	test_assert(coord.strategy == COORD_SEQUENTIAL, "Default strategy sequential");
	test_assert(coord.phase_count == 0, "Phase count zeroed");
	test_assert(coord.abort_on_error == 1, "Abort on error enabled");
}

static void test_add_phase_to_order(void) {
	struct PhaseCoordinator coord;
	phase_coordinator_init(&coord);
	u8 result = add_phase_to_order(&coord, 21);
	test_assert(result == 0, "Add phase to order succeeds");
	test_assert(coord.phase_count == 1, "Phase count incremented");
	test_assert(coord.phase_order[0] == 21, "Phase ID stored correctly");
}

static void test_invariant_validator_init(void) {
	struct InvariantValidator val;
	invariant_validator_init(&val);
	test_assert(val.invariant_count == 0, "Invariants initialized");
	test_assert(val.total_checks == 0, "Checks zeroed");
	test_assert(val.failed_checks == 0, "Failed checks zeroed");
}

static void test_register_phase_invariant(void) {
	struct InvariantValidator val;
	invariant_validator_init(&val);
	u8 result = register_phase_invariant(&val, 21, "Type safety", 1);
	test_assert(result == 0, "Register invariant succeeds");
	test_assert(val.invariant_count == 1, "Invariant count incremented");
}

static void test_optimization_coordinator_init(void) {
	struct OptimizationCoordinator opt;
	optimization_coordinator_init(&opt);
	test_assert(opt.optimization_count == 0, "Opt count zeroed");
	test_assert(opt.total_savings == 0, "Savings zeroed");
}

static void test_register_cross_phase_optimization(void) {
	struct OptimizationCoordinator opt;
	optimization_coordinator_init(&opt);
	u8 result = register_cross_phase_optimization(&opt, 21, 25, "Inlining");
	test_assert(result == 0, "Register cross-phase opt succeeds");
	test_assert(opt.optimization_count == 1, "Opt count incremented");
}

/* ============================================================ */
/* PHASE 37: WHOLE-PROGRAM ANALYSIS TESTS */
/* ============================================================ */

static void test_call_graph_init(void) {
	struct CallGraph cg;
	call_graph_init(&cg);
	test_assert(cg.function_count == 0, "Call graph init");
	test_assert(cg.edge_count == 0, "Edges cleared");
	test_assert(cg.recursive_functions == 0, "Recursion count cleared");
}

static void test_add_function_node(void) {
	struct CallGraph cg;
	call_graph_init(&cg);
	u8 result = add_function_node(&cg, 1, "main", 0);
	test_assert(result == 0, "Add function node succeeds");
	test_assert(cg.function_count == 1, "Function count incremented");
}

static void test_add_call_edge(void) {
	struct CallGraph cg;
	call_graph_init(&cg);
	u8 result = add_call_edge(&cg, 1, 2, 1);
	test_assert(result == 0, "Add call edge succeeds");
	test_assert(cg.edge_count == 1, "Edge count incremented");
}

static void test_global_constant_prop_init(void) {
	struct GlobalConstantProp gcp;
	global_constant_prop_init(&gcp);
	test_assert(gcp.constant_count == 0, "Global const prop init");
	test_assert(gcp.propagated_uses == 0, "Uses zeroed");
	test_assert(gcp.values_analyzed == 0, "Values zeroed");
}

static void test_register_global_constant(void) {
	struct GlobalConstantProp gcp;
	global_constant_prop_init(&gcp);
	u8 result = register_global_constant(&gcp, 1, 42);
	test_assert(result == 0, "Register global const succeeds");
	test_assert(gcp.constant_count == 1, "Constant count incremented");
}

static void test_global_dead_code_init(void) {
	struct GlobalDeadCodeAnalyzer gdca;
	global_dead_code_init(&gdca);
	test_assert(gdca.unreachable_count == 0, "Dead code analyzer init");
	test_assert(gdca.bytes_removable == 0, "Bytes zeroed");
}

static void test_mark_unreachable_function(void) {
	struct GlobalDeadCodeAnalyzer gdca;
	global_dead_code_init(&gdca);
	u8 result = mark_unreachable_function(&gdca, 1, "unused", 256);
	test_assert(result == 0, "Mark unreachable succeeds");
	test_assert(gdca.unreachable_count == 1, "Count incremented");
	test_assert(gdca.bytes_removable == 256, "Bytes tracked");
}

static void test_function_specializer_init(void) {
	struct FunctionSpecializer fs;
	function_specializer_init(&fs);
	test_assert(fs.opportunity_count == 0, "Specializer init");
	test_assert(fs.specialized_functions == 0, "Specialized cleared");
	test_assert(fs.total_savings == 0, "Savings zeroed");
}

static void test_add_specialization_opportunity(void) {
	struct FunctionSpecializer fs;
	function_specializer_init(&fs);
	u8 result = add_specialization_opportunity(&fs, 1, 0, 42);
	test_assert(result == 0, "Add specialization succeeds");
	test_assert(fs.opportunity_count == 1, "Opportunity count incremented");
}

static void test_alias_analyzer_init(void) {
	struct AliasAnalyzer aa;
	alias_analyzer_init(&aa);
	test_assert(aa.set_count == 0, "Alias analyzer init");
	test_assert(aa.alias_queries == 0, "Queries zeroed");
	test_assert(aa.no_alias_found == 0, "No-alias count zeroed");
}

static void test_add_alias_set(void) {
	struct AliasAnalyzer aa;
	alias_analyzer_init(&aa);
	u8 result = add_alias_set(&aa, 1, ALIAS_NO_ALIAS);
	test_assert(result == 0, "Add alias set succeeds");
	test_assert(aa.set_count == 1, "Set count incremented");
}

static void test_program_slicer_init(void) {
	struct ProgramSlicer ps;
	program_slicer_init(&ps);
	test_assert(ps.slice_count == 0, "Program slicer init");
	test_assert(ps.total_slice_bytes == 0, "Bytes zeroed");
	test_assert(ps.total_dependencies == 0, "Dependencies zeroed");
}

static void test_create_program_slice(void) {
	struct ProgramSlicer ps;
	program_slicer_init(&ps);
	u8 result = create_program_slice(&ps, 1);
	test_assert(result == 0, "Create program slice succeeds");
	test_assert(ps.slice_count == 1, "Slice count incremented");
}

/* ============================================================ */
/* PHASE 38: TARGET-SPECIFIC OPTIMIZATION TESTS */
/* ============================================================ */

static void test_simd_vectorizer_init(void) {
	struct SIMDVectorizer sv;
	simd_vectorizer_init(&sv);
	test_assert(sv.loop_count == 0, "SIMD vectorizer init");
	test_assert(sv.successful_vectorizations == 0, "Vectorizations zeroed");
	test_assert(sv.total_cycles_saved == 0, "Cycles zeroed");
}

static void test_add_vectorized_loop(void) {
	struct SIMDVectorizer sv;
	simd_vectorizer_init(&sv);
	u8 result = add_vectorized_loop(&sv, 1, SIMD_VADD, 4);
	test_assert(result == 0, "Add vectorized loop succeeds");
	test_assert(sv.loop_count == 1, "Loop count incremented");
	test_assert(sv.successful_vectorizations == 1, "Vectorization count incremented");
}

static void test_neon_generator_init(void) {
	struct NEONGenerator ng;
	neon_generator_init(&ng);
	test_assert(ng.intrinsic_count == 0, "NEON generator init");
	test_assert(ng.generated_instructions == 0, "Instructions zeroed");
	test_assert(ng.total_registers_used == 0, "Registers zeroed");
}

static void test_register_neon_intrinsic(void) {
	struct NEONGenerator ng;
	neon_generator_init(&ng);
	u8 result = register_neon_intrinsic(&ng, 1, "vaddq_s32", 2);
	test_assert(result == 0, "Register NEON intrinsic succeeds");
	test_assert(ng.intrinsic_count == 1, "Intrinsic count incremented");
}

static void test_cache_optimizer_init(void) {
	struct CacheOptimizer co;
	cache_optimizer_init(&co);
	test_assert(co.opt_count == 0, "Cache optimizer init");
	test_assert(co.total_misses_reduced == 0, "Misses reduced zeroed");
	test_assert(co.reuse_distance_improved == 0, "Reuse zeroed");
}

static void test_add_cache_optimization(void) {
	struct CacheOptimizer co;
	cache_optimizer_init(&co);
	u8 result = add_cache_optimization(&co, 1, CACHE_L1, 64);
	test_assert(result == 0, "Add cache opt succeeds");
	test_assert(co.opt_count == 1, "Opt count incremented");
}

static void test_branch_predictor_init(void) {
	struct BranchPredictor bp;
	branch_predictor_init(&bp);
	test_assert(bp.branch_count == 0, "Branch predictor init");
	test_assert(bp.correct_predictions == 0, "Predictions zeroed");
	test_assert(bp.mispredictions == 0, "Mispredictions zeroed");
}

static void test_add_branch_hint(void) {
	struct BranchPredictor bp;
	branch_predictor_init(&bp);
	u8 result = add_branch_hint(&bp, 1, 100, BRANCH_LIKELY);
	test_assert(result == 0, "Add branch hint succeeds");
	test_assert(bp.branch_count == 1, "Branch count incremented");
}

static void test_instruction_scheduler_init(void) {
	struct InstructionScheduler is;
	instruction_scheduler_init(&is);
	test_assert(is.insn_count == 0, "Instruction scheduler init");
	test_assert(is.critical_path_length == 0, "Path zeroed");
	test_assert(is.register_pressure == 0, "Pressure zeroed");
}

static void test_add_scheduled_instruction(void) {
	struct InstructionScheduler is;
	instruction_scheduler_init(&is);
	u8 result = add_scheduled_instruction(&is, 1, 0, 2);
	test_assert(result == 0, "Add scheduled insn succeeds");
	test_assert(is.insn_count == 1, "Insn count incremented");
}

/* ============================================================ */
/* PHASE 39: LINK-TIME OPTIMIZATION TESTS */
/* ============================================================ */

static void test_link_time_inliner_init(void) {
	struct LinkTimeInliner lti;
	link_time_inliner_init(&lti);
	test_assert(lti.candidate_count == 0, "Link-time inliner init");
	test_assert(lti.inlined_count == 0, "Inlined cleared");
	test_assert(lti.total_benefit == 0, "Benefit zeroed");
}

static void test_add_inline_candidate(void) {
	struct LinkTimeInliner lti;
	link_time_inliner_init(&lti);
	u8 result = add_inline_candidate(&lti, 1, 2, 128);
	test_assert(result == 0, "Add inline candidate succeeds");
	test_assert(lti.candidate_count == 1, "Candidate count incremented");
}

static void test_visibility_optimizer_init(void) {
	struct VisibilityOptimizer vo;
	visibility_optimizer_init(&vo);
	test_assert(vo.symbol_count == 0, "Visibility optimizer init");
	test_assert(vo.removable_symbols == 0, "Removable cleared");
	test_assert(vo.removable_bytes == 0, "Bytes zeroed");
}

static void test_add_symbol_link_info(void) {
	struct VisibilityOptimizer vo;
	visibility_optimizer_init(&vo);
	u8 result = add_symbol_link_info(&vo, 1, "unused", LVIS_INTERNAL);
	test_assert(result == 0, "Add symbol link info succeeds");
	test_assert(vo.symbol_count == 1, "Symbol count incremented");
	test_assert(vo.removable_symbols == 1, "Removable incremented");
}

static void test_link_time_resolver_init(void) {
	struct LinkTimeResolver ltr;
	link_time_resolver_init(&ltr);
	test_assert(ltr.resolution_count == 0, "Link-time resolver init");
	test_assert(ltr.resolved_count == 0, "Resolved cleared");
	test_assert(ltr.unresolved_count == 0, "Unresolved cleared");
}

static void test_resolve_symbol(void) {
	struct LinkTimeResolver ltr;
	link_time_resolver_init(&ltr);
	u8 result = resolve_symbol(&ltr, 1, 0x1000);
	test_assert(result == 0, "Resolve symbol succeeds");
	test_assert(ltr.resolution_count == 1, "Resolution count incremented");
	test_assert(ltr.resolved_count == 1, "Resolved count incremented");
}

static void test_link_time_codegen_init(void) {
	struct LinkTimeCodeGen ltcg;
	link_time_codegen_init(&ltcg);
	test_assert(ltcg.code_count == 0, "Link-time codegen init");
	test_assert(ltcg.total_generated_bytes == 0, "Bytes zeroed");
	test_assert(ltcg.hot_code_count == 0, "Hot code zeroed");
}

static void test_generate_link_time_code(void) {
	struct LinkTimeCodeGen ltcg;
	link_time_codegen_init(&ltcg);
	u8 result = generate_link_time_code(&ltcg, 1, 256);
	test_assert(result == 0, "Generate link-time code succeeds");
	test_assert(ltcg.code_count == 1, "Code count incremented");
	test_assert(ltcg.total_generated_bytes == 256, "Bytes tracked");
}

static void test_inter_module_optimizer_init(void) {
	struct InterModuleOptimizer imo;
	inter_module_optimizer_init(&imo);
	test_assert(imo.module_count == 0, "Inter-module optimizer init");
	test_assert(imo.cross_module_inlines == 0, "Inlines zeroed");
	test_assert(imo.cross_module_savings == 0, "Savings zeroed");
}

static void test_register_module(void) {
	struct InterModuleOptimizer imo;
	inter_module_optimizer_init(&imo);
	u8 result = register_module(&imo, 1, "libmath");
	test_assert(result == 0, "Register module succeeds");
	test_assert(imo.module_count == 1, "Module count incremented");
}

/* ============================================================ */
/* PHASE 40: PROFILE-GUIDED OPTIMIZATION TESTS */
/* ============================================================ */

static void test_profile_collector_init(void) {
	struct ProfileCollector pc;
	profile_collector_init(&pc);
	test_assert(pc.profile_count == 0, "Profile collector init");
	test_assert(pc.total_executions == 0, "Executions zeroed");
	test_assert(pc.hot_functions == 0, "Hot functions zeroed");
}

static void test_record_profile(void) {
	struct ProfileCollector pc;
	profile_collector_init(&pc);
	u8 result = record_profile(&pc, 1, 5000);
	test_assert(result == 0, "Record profile succeeds");
	test_assert(pc.profile_count == 1, "Profile count incremented");
	test_assert(pc.hot_functions == 1, "Hot function identified");
}

static void test_hot_path_analyzer_init(void) {
	struct HotPathAnalyzer hpa;
	hot_path_analyzer_init(&hpa);
	test_assert(hpa.path_count == 0, "Hot path analyzer init");
	test_assert(hpa.critical_path_cycles == 0, "Path cycles zeroed");
	test_assert(hpa.critical_path_count == 0, "Path count zeroed");
}

static void test_identify_hot_path(void) {
	struct HotPathAnalyzer hpa;
	hot_path_analyzer_init(&hpa);
	u8 result = identify_hot_path(&hpa, 1, 1, 20000);
	test_assert(result == 0, "Identify hot path succeeds");
	test_assert(hpa.path_count == 1, "Path count incremented");
	test_assert(hpa.critical_path_count == 1, "Critical path identified");
}

static void test_code_layout_optimizer_init(void) {
	struct CodeLayoutOptimizer clo;
	code_layout_optimizer_init(&clo);
	test_assert(clo.block_count == 0, "Code layout optimizer init");
	test_assert(clo.cache_line_misses == 0, "Misses zeroed");
	test_assert(clo.icache_misses_reduced == 0, "Reduction zeroed");
}

static void test_add_code_block(void) {
	struct CodeLayoutOptimizer clo;
	code_layout_optimizer_init(&clo);
	u8 result = add_code_block(&clo, 1, 256, 3000);
	test_assert(result == 0, "Add code block succeeds");
	test_assert(clo.block_count == 1, "Block count incremented");
}

static void test_speculative_optimizer_init(void) {
	struct SpeculativeOptimizer so;
	speculative_optimizer_init(&so);
	test_assert(so.opt_count == 0, "Speculative optimizer init");
	test_assert(so.validated_opts == 0, "Validated cleared");
	test_assert(so.failed_speculations == 0, "Failed cleared");
}

static void test_add_speculative_optimization(void) {
	struct SpeculativeOptimizer so;
	speculative_optimizer_init(&so);
	u8 result = add_speculative_optimization(&so, 1, "inlining", 100);
	test_assert(result == 0, "Add speculative opt succeeds");
	test_assert(so.opt_count == 1, "Opt count incremented");
}

static void test_context_sensitive_optimizer_init(void) {
	struct ContextSensitiveOptimizer cso;
	context_sensitive_optimizer_init(&cso);
	test_assert(cso.context_count == 0, "Context optimizer init");
	test_assert(cso.optimized_contexts == 0, "Optimized cleared");
	test_assert(cso.context_specific_benefits == 0, "Benefits zeroed");
}

static void test_track_call_context(void) {
	struct ContextSensitiveOptimizer cso;
	context_sensitive_optimizer_init(&cso);
	u8 result = track_call_context(&cso, 100, 2, 500);
	test_assert(result == 0, "Track call context succeeds");
	test_assert(cso.context_count == 1, "Context count incremented");
	test_assert(cso.optimized_contexts == 1, "Optimized context identified");
}

/* ============================================================ */
/* PHASE 41: INTERPROCEDURAL ANALYSIS TESTS */
/* ============================================================ */

static void test_interprocedural_analyzer_init(void) {
	struct InterproceduralAnalyzer ipa;
	interprocedural_analyzer_init(&ipa);
	test_assert(ipa.signature_count == 0, "Interprocedural analyzer init");
	test_assert(ipa.pure_functions == 0, "Pure functions cleared");
	test_assert(ipa.functions_with_side_effects == 0, "Side effects cleared");
}

static void test_register_function_signature(void) {
	struct InterproceduralAnalyzer ipa;
	interprocedural_analyzer_init(&ipa);
	u8 result = register_function_signature(&ipa, 1, 2, 0);
	test_assert(result == 0, "Register signature succeeds");
	test_assert(ipa.signature_count == 1, "Signature count incremented");
	test_assert(ipa.pure_functions == 1, "Pure function identified");
}

static void test_parameter_analyzer_init(void) {
	struct ParameterAnalyzer pa;
	parameter_analyzer_init(&pa);
	test_assert(pa.property_count == 0, "Parameter analyzer init");
	test_assert(pa.readonly_params == 0, "Readonly cleared");
	test_assert(pa.constant_params == 0, "Constant cleared");
}

static void test_analyze_parameter(void) {
	struct ParameterAnalyzer pa;
	parameter_analyzer_init(&pa);
	u8 result = analyze_parameter(&pa, 1, 1, 0);
	test_assert(result == 0, "Analyze parameter succeeds");
	test_assert(pa.property_count == 1, "Property count incremented");
	test_assert(pa.constant_params == 1, "Constant param tracked");
}

static void test_summary_function_computer_init(void) {
	struct SummaryFunctionComputer sfc;
	summary_function_computer_init(&sfc);
	test_assert(sfc.summary_count == 0, "Summary computer init");
	test_assert(sfc.summarized_functions == 0, "Summarized cleared");
	test_assert(sfc.summary_accuracy == 0, "Accuracy zeroed");
}

static void test_compute_summary_function(void) {
	struct SummaryFunctionComputer sfc;
	summary_function_computer_init(&sfc);
	u8 result = compute_summary_function(&sfc, 1, 1000);
	test_assert(result == 0, "Compute summary succeeds");
	test_assert(sfc.summary_count == 1, "Summary count incremented");
	test_assert(sfc.summarized_functions == 1, "Summarized incremented");
}

static void test_property_inferencer_init(void) {
	struct PropertyInferencer pi;
	property_inferencer_init(&pi);
	test_assert(pi.property_count == 0, "Property inferencer init");
	test_assert(pi.inferred_properties == 0, "Inferred cleared");
	test_assert(pi.verified_inferences == 0, "Verified cleared");
}

static void test_infer_property(void) {
	struct PropertyInferencer pi;
	property_inferencer_init(&pi);
	u8 result = infer_property(&pi, 1, PROP_PURE, 95);
	test_assert(result == 0, "Infer property succeeds");
	test_assert(pi.property_count == 1, "Property count incremented");
	test_assert(pi.inferred_properties == 1, "Inferred incremented");
}

/* ============================================================ */
/* PHASE 42: SPECULATIVE OPTIMIZATION TESTS */
/* ============================================================ */

static void test_devirtualization_optimizer_init(void) {
	struct DevirtualizationOptimizer dopt;
	devirtualization_optimizer_init(&dopt);
	test_assert(dopt.call_site_count == 0, "Devirtualization optimizer init");
	test_assert(dopt.devirtualized_calls == 0, "Devirtualized cleared");
	test_assert(dopt.virtual_call_overhead_removed == 0, "Overhead zeroed");
}

static void test_add_virtual_call_site(void) {
	struct DevirtualizationOptimizer dopt;
	devirtualization_optimizer_init(&dopt);
	u8 result = add_virtual_call_site(&dopt, 1, 0, 1, 2);
	test_assert(result == 0, "Add virtual call site succeeds");
	test_assert(dopt.call_site_count == 1, "Call site count incremented");
	test_assert(dopt.devirtualized_calls == 1, "Devirtualized incremented");
}

static void test_type_speculator_init(void) {
	struct TypeSpeculator ts;
	type_speculator_init(&ts);
	test_assert(ts.speculation_count == 0, "Type speculator init");
	test_assert(ts.valid_speculations == 0, "Valid cleared");
	test_assert(ts.failed_speculations == 0, "Failed cleared");
}

static void test_add_type_speculation(void) {
	struct TypeSpeculator ts;
	type_speculator_init(&ts);
	u8 result = add_type_speculation(&ts, 1, 0);
	test_assert(result == 0, "Add type speculation succeeds");
	test_assert(ts.speculation_count == 1, "Speculation count incremented");
	test_assert(ts.valid_speculations == 1, "Valid incremented");
}

static void test_assumption_tracker_init(void) {
	struct AssumptionTracker at;
	assumption_tracker_init(&at);
	test_assert(at.assumption_count == 0, "Assumption tracker init");
	test_assert(at.violated_assumptions == 0, "Violated cleared");
	test_assert(at.optimization_rollbacks == 0, "Rollbacks cleared");
}

static void test_track_assumption(void) {
	struct AssumptionTracker at;
	assumption_tracker_init(&at);
	u8 result = track_assumption(&at, 1, "type stable", 1);
	test_assert(result == 0, "Track assumption succeeds");
	test_assert(at.assumption_count == 1, "Assumption count incremented");
}

static void test_guard_insertion_engine_init(void) {
	struct GuardInsertionEngine gie;
	guard_insertion_engine_init(&gie);
	test_assert(gie.guard_count == 0, "Guard insertion engine init");
	test_assert(gie.inserted_guards == 0, "Guards cleared");
	test_assert(gie.total_guard_overhead == 0, "Overhead zeroed");
}

static void test_insert_guard(void) {
	struct GuardInsertionEngine gie;
	guard_insertion_engine_init(&gie);
	u8 result = insert_guard(&gie, 1, 1, 10);
	test_assert(result == 0, "Insert guard succeeds");
	test_assert(gie.guard_count == 1, "Guard count incremented");
	test_assert(gie.inserted_guards == 1, "Inserted incremented");
	test_assert(gie.total_guard_overhead == 10, "Overhead tracked");
}

/* ============================================================ */
/* PHASE 43: PARALLELIZATION TESTS */
/* ============================================================ */

static void test_loop_parallelizer_init(void) {
	struct LoopParallelizer lp;
	loop_parallelizer_init(&lp);
	test_assert(lp.loop_count == 0, "Loop parallelizer init");
	test_assert(lp.parallelized_loops == 0, "Parallelized cleared");
	test_assert(lp.total_expected_speedup == 0, "Speedup zeroed");
}

static void test_analyze_loop_parallelism(void) {
	struct LoopParallelizer lp;
	loop_parallelizer_init(&lp);
	u8 result = analyze_loop_parallelism(&lp, 1, PARALLEL_OUTER_LOOP, 4);
	test_assert(result == 0, "Analyze loop parallelism succeeds");
	test_assert(lp.loop_count == 1, "Loop count incremented");
	test_assert(lp.parallelized_loops == 1, "Parallelized incremented");
	test_assert(lp.total_expected_speedup == 4, "Speedup tracked");
}

static void test_auto_vectorizer_init(void) {
	struct AutoVectorizer av;
	auto_vectorizer_init(&av);
	test_assert(av.op_count == 0, "Auto vectorizer init");
	test_assert(av.vectorized_ops == 0, "Vectorized cleared");
	test_assert(av.total_throughput_gain == 0, "Throughput zeroed");
}

static void test_auto_vectorize_operation(void) {
	struct AutoVectorizer av;
	auto_vectorizer_init(&av);
	u8 result = auto_vectorize_operation(&av, 1, 0, 4);
	test_assert(result == 0, "Auto vectorize succeeds");
	test_assert(av.op_count == 1, "Op count incremented");
	test_assert(av.vectorized_ops == 1, "Vectorized incremented");
}

static void test_dependence_analyzer_init(void) {
	struct DependenceAnalyzer da;
	dependence_analyzer_init(&da);
	test_assert(da.dep_count == 0, "Dependence analyzer init");
	test_assert(da.parallelizable_deps == 0, "Parallelizable cleared");
	test_assert(da.loop_carried_deps == 0, "Loop deps cleared");
}

static void test_add_dependence(void) {
	struct DependenceAnalyzer da;
	dependence_analyzer_init(&da);
	u8 result = add_dependence(&da, 1, 2, DEP_INPUT);
	test_assert(result == 0, "Add dependence succeeds");
	test_assert(da.dep_count == 1, "Dep count incremented");
	test_assert(da.parallelizable_deps == 1, "Parallelizable incremented");
}

static void test_thread_parallelism_extractor_init(void) {
	struct ThreadParallelismExtractor tpe;
	thread_parallelism_extractor_init(&tpe);
	test_assert(tpe.task_count == 0, "Thread parallelism extractor init");
	test_assert(tpe.total_parallelism_level == 0, "Parallelism level zeroed");
}

static void test_extract_task_region(void) {
	struct ThreadParallelismExtractor tpe;
	thread_parallelism_extractor_init(&tpe);
	u8 result = extract_task_region(&tpe, 1, 100, 200, 4);
	test_assert(result == 0, "Extract task region succeeds");
	test_assert(tpe.task_count == 1, "Task count incremented");
	test_assert(tpe.total_parallelism_level == 4, "Parallelism level tracked");
}

/* ============================================================ */
/* PHASE 44: CACHE HIERARCHY TESTS */
/* ============================================================ */

static void test_cache_oblivious_optimizer_init(void) {
	struct CacheObliviousOptimizer coo;
	cache_oblivious_optimizer_init(&coo);
	test_assert(coo.algo_count == 0, "Cache oblivious init");
	test_assert(coo.optimized_algos == 0, "Optimized cleared");
	test_assert(coo.total_cache_misses_reduced == 0, "Misses zeroed");
}

static void test_register_cache_oblivious_algorithm(void) {
	struct CacheObliviousOptimizer coo;
	cache_oblivious_optimizer_init(&coo);
	u8 result = register_cache_oblivious_algorithm(&coo, 1, "FFT");
	test_assert(result == 0, "Register algorithm succeeds");
	test_assert(coo.algo_count == 1, "Algo count incremented");
	test_assert(coo.optimized_algos == 1, "Optimized incremented");
}

static void test_data_layout_optimizer_init(void) {
	struct DataLayoutOptimizer dlo;
	data_layout_optimizer_init(&dlo);
	test_assert(dlo.opt_count == 0, "Data layout optimizer init");
	test_assert(dlo.improved_layouts == 0, "Improved cleared");
	test_assert(dlo.total_efficiency_gain == 0, "Gain zeroed");
}

static void test_optimize_data_layout(void) {
	struct DataLayoutOptimizer dlo;
	data_layout_optimizer_init(&dlo);
	u8 result = optimize_data_layout(&dlo, 1, LAYOUT_ARRAY_OF_STRUCT, LAYOUT_STRUCT_OF_ARRAY);
	test_assert(result == 0, "Optimize data layout succeeds");
	test_assert(dlo.opt_count == 1, "Opt count incremented");
	test_assert(dlo.improved_layouts == 1, "Improved incremented");
}

static void test_prefetch_inserter_init(void) {
	struct PrefetchInserter pi;
	prefetch_inserter_init(&pi);
	test_assert(pi.prefetch_count == 0, "Prefetch inserter init");
	test_assert(pi.inserted_prefetches == 0, "Inserted cleared");
	test_assert(pi.total_latency_hidden == 0, "Latency zeroed");
}

static void test_insert_prefetch(void) {
	struct PrefetchInserter pi;
	prefetch_inserter_init(&pi);
	u8 result = insert_prefetch(&pi, 1, 0x1000, 8);
	test_assert(result == 0, "Insert prefetch succeeds");
	test_assert(pi.prefetch_count == 1, "Prefetch count incremented");
	test_assert(pi.inserted_prefetches == 1, "Inserted incremented");
}

static void test_cache_aware_memory_manager_init(void) {
	struct CacheAwareMemoryManager camm;
	cache_aware_memory_manager_init(&camm);
	test_assert(camm.region_count == 0, "Memory manager init");
	test_assert(camm.total_cache_utilization == 0, "Utilization zeroed");
	test_assert(camm.regions_optimized == 0, "Regions cleared");
}

static void test_allocate_memory_region(void) {
	struct CacheAwareMemoryManager camm;
	cache_aware_memory_manager_init(&camm);
	u8 result = allocate_memory_region(&camm, 1, 4096, CACHE_L1);
	test_assert(result == 0, "Allocate memory region succeeds");
	test_assert(camm.region_count == 1, "Region count incremented");
	test_assert(camm.regions_optimized == 1, "Regions optimized incremented");
}

/* ============================================================ */
/* PHASE 45: RUNTIME SPECIALIZATION TESTS */
/* ============================================================ */

static void test_runtime_hot_code_id_init(void) {
	struct RuntimeHotCodeIdentifier rhci;
	runtime_hot_code_id_init(&rhci);
	test_assert(rhci.segment_count == 0, "Runtime hot code init");
	test_assert(rhci.hot_segments == 0, "Hot segments cleared");
	test_assert(rhci.critical_segments == 0, "Critical cleared");
}

static void test_identify_hot_segment(void) {
	struct RuntimeHotCodeIdentifier rhci;
	runtime_hot_code_id_init(&rhci);
	u8 result = identify_hot_segment(&rhci, 1, 0x1000, 0x2000);
	test_assert(result == 0, "Identify hot segment succeeds");
	test_assert(rhci.segment_count == 1, "Segment count incremented");
}

static void test_dynamic_specializer_init(void) {
	struct DynamicSpecializer ds;
	dynamic_specializer_init(&ds);
	test_assert(ds.version_count == 0, "Dynamic specializer init");
	test_assert(ds.active_specializations == 0, "Active cleared");
	test_assert(ds.specialization_benefit == 0, "Benefit zeroed");
}

static void test_create_specialized_version(void) {
	struct DynamicSpecializer ds;
	dynamic_specializer_init(&ds);
	u8 result = create_specialized_version(&ds, 1, 0);
	test_assert(result == 0, "Create specialized version succeeds");
	test_assert(ds.version_count == 1, "Version count incremented");
	test_assert(ds.active_specializations == 1, "Active incremented");
}

static void test_osr_executor_init(void) {
	struct OSRExecutor osr;
	osr_executor_init(&osr);
	test_assert(osr.osr_point_count == 0, "OSR executor init");
	test_assert(osr.executed_osrs == 0, "Executed cleared");
	test_assert(osr.osrs_deferred == 0, "Deferred zeroed");
}

static void test_add_osr_point(void) {
	struct OSRExecutor osr;
	osr_executor_init(&osr);
	u8 result = add_osr_point(&osr, 1, 100, 2);
	test_assert(result == 0, "Add OSR point succeeds");
	test_assert(osr.osr_point_count == 1, "OSR point count incremented");
}

static void test_jit_compiler_init(void) {
	struct JITCompiler jc;
	jit_compiler_init(&jc);
	test_assert(jc.cache_size == 0, "JIT compiler init");
	test_assert(jc.compiled_functions == 0, "Compiled cleared");
	test_assert(jc.total_compilation_time == 0, "Time zeroed");
}

static void test_compile_with_jit(void) {
	struct JITCompiler jc;
	jit_compiler_init(&jc);
	u8 result = compile_with_jit(&jc, 1, TIER_OPTIMIZED, 5000);
	test_assert(result == 0, "Compile with JIT succeeds");
	test_assert(jc.cache_size == 1, "Cache size incremented");
	test_assert(jc.compiled_functions == 1, "Compiled functions incremented");
	test_assert(jc.total_compilation_time == 5000, "Time tracked");
}

static void test_adaptive_optimizer_init(void) {
	struct AdaptiveOptimizer ao;
	adaptive_optimizer_init(&ao);
	test_assert(ao.current_policy == POLICY_BALANCED, "Adaptive optimizer init");
	test_assert(ao.policy_switches == 0, "Switches cleared");
	test_assert(ao.total_adaptive_benefit == 0, "Benefit zeroed");
}

/* ============================================================ */
/* MAIN TEST RUNNER */
/* ============================================================ */

int main(void) {
	printf("=== Phase 36-45 Optimization Tests ===\n\n");

	printf("--- Phase 36: Cross-Phase Integration ---\n");
	test_cross_phase_graph_init();
	test_add_phase_dependency();
	test_inter_phase_dataflow_init();
	test_track_inter_phase_value();
	test_phase_coordinator_init();
	test_add_phase_to_order();
	test_invariant_validator_init();
	test_register_phase_invariant();
	test_optimization_coordinator_init();
	test_register_cross_phase_optimization();

	printf("\n--- Phase 37: Whole-Program Analysis ---\n");
	test_call_graph_init();
	test_add_function_node();
	test_add_call_edge();
	test_global_constant_prop_init();
	test_register_global_constant();
	test_global_dead_code_init();
	test_mark_unreachable_function();
	test_function_specializer_init();
	test_add_specialization_opportunity();
	test_alias_analyzer_init();
	test_add_alias_set();
	test_program_slicer_init();
	test_create_program_slice();

	printf("\n--- Phase 38: Target-Specific Optimization ---\n");
	test_simd_vectorizer_init();
	test_add_vectorized_loop();
	test_neon_generator_init();
	test_register_neon_intrinsic();
	test_cache_optimizer_init();
	test_add_cache_optimization();
	test_branch_predictor_init();
	test_add_branch_hint();
	test_instruction_scheduler_init();
	test_add_scheduled_instruction();

	printf("\n--- Phase 39: Link-Time Optimization ---\n");
	test_link_time_inliner_init();
	test_add_inline_candidate();
	test_visibility_optimizer_init();
	test_add_symbol_link_info();
	test_link_time_resolver_init();
	test_resolve_symbol();
	test_link_time_codegen_init();
	test_generate_link_time_code();
	test_inter_module_optimizer_init();
	test_register_module();

	printf("\n--- Phase 40: Profile-Guided Optimization ---\n");
	test_profile_collector_init();
	test_record_profile();
	test_hot_path_analyzer_init();
	test_identify_hot_path();
	test_code_layout_optimizer_init();
	test_add_code_block();
	test_speculative_optimizer_init();
	test_add_speculative_optimization();
	test_context_sensitive_optimizer_init();
	test_track_call_context();

	printf("\n--- Phase 41: Interprocedural Analysis ---\n");
	test_interprocedural_analyzer_init();
	test_register_function_signature();
	test_parameter_analyzer_init();
	test_analyze_parameter();
	test_summary_function_computer_init();
	test_compute_summary_function();
	test_property_inferencer_init();
	test_infer_property();

	printf("\n--- Phase 42: Speculative Optimization ---\n");
	test_devirtualization_optimizer_init();
	test_add_virtual_call_site();
	test_type_speculator_init();
	test_add_type_speculation();
	test_assumption_tracker_init();
	test_track_assumption();
	test_guard_insertion_engine_init();
	test_insert_guard();

	printf("\n--- Phase 43: Parallelization ---\n");
	test_loop_parallelizer_init();
	test_analyze_loop_parallelism();
	test_auto_vectorizer_init();
	test_auto_vectorize_operation();
	test_dependence_analyzer_init();
	test_add_dependence();
	test_thread_parallelism_extractor_init();
	test_extract_task_region();

	printf("\n--- Phase 44: Cache Hierarchy Optimization ---\n");
	test_cache_oblivious_optimizer_init();
	test_register_cache_oblivious_algorithm();
	test_data_layout_optimizer_init();
	test_optimize_data_layout();
	test_prefetch_inserter_init();
	test_insert_prefetch();
	test_cache_aware_memory_manager_init();
	test_allocate_memory_region();

	printf("\n--- Phase 45: Runtime Specialization & JIT ---\n");
	test_runtime_hot_code_id_init();
	test_identify_hot_segment();
	test_dynamic_specializer_init();
	test_create_specialized_version();
	test_osr_executor_init();
	test_add_osr_point();
	test_jit_compiler_init();
	test_compile_with_jit();
	test_adaptive_optimizer_init();

	printf("\n=== Summary ===\n");
	printf("Total: %d | Passed: %d | Failed: %d\n", total_tests, passed_tests, failed_tests);

	return failed_tests > 0 ? 1 : 0;
}
