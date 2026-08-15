/* opt_whole_program.h — Whole-Program Analysis & Global Optimization (Phase 37)
 *
 * Phase 37: Whole-program analysis & global optimizations
 * - Global constant propagation across functions
 * - Interprocedural call graph analysis
 * - Global dead code elimination
 * - Function specialization opportunities
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_WHOLE_PROGRAM_H
#define APKC_OPT_WHOLE_PROGRAM_H 1

#include "opt_cross_phase.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* CALL GRAPH ANALYSIS */
/* ============================================================ */

struct FunctionNode {
	u32 func_id;
	const char *func_name;
	u32 call_count;
	u32 caller_count;
	u8 is_recursive;
	u8 is_exported;
};

struct CallEdge {
	u32 caller_id;
	u32 callee_id;
	u32 call_sites;
	u8 is_direct;
};

struct CallGraph {
	struct FunctionNode functions[128];
	u32 function_count;
	struct CallEdge edges[256];
	u32 edge_count;
	u32 recursive_functions;
};

static inline void call_graph_init(struct CallGraph *cg) {
	if (!cg) return;
	cg->function_count = 0;
	cg->edge_count = 0;
	cg->recursive_functions = 0;
}

static inline u8 add_function_node(
	struct CallGraph *cg,
	u32 func_id,
	const char *name,
	u8 is_exported) {

	if (!cg || cg->function_count >= 128) return 1;

	struct FunctionNode *fn = &cg->functions[cg->function_count];
	fn->func_id = func_id;
	fn->func_name = name;
	fn->call_count = 0;
	fn->caller_count = 0;
	fn->is_recursive = 0;
	fn->is_exported = is_exported;

	cg->function_count++;
	return 0;
}

static inline u8 add_call_edge(
	struct CallGraph *cg,
	u32 caller_id,
	u32 callee_id,
	u8 is_direct) {

	if (!cg || cg->edge_count >= 256) return 1;

	struct CallEdge *edge = &cg->edges[cg->edge_count];
	edge->caller_id = caller_id;
	edge->callee_id = callee_id;
	edge->call_sites = 1;
	edge->is_direct = is_direct;

	cg->edge_count++;
	return 0;
}

/* ============================================================ */
/* GLOBAL CONSTANT PROPAGATION */
/* ============================================================ */

struct GlobalConstant {
	u32 var_id;
	u64 const_value;
	u32 propagation_count;
	u8 is_proven;
};

struct GlobalConstantProp {
	struct GlobalConstant constants[256];
	u32 constant_count;
	u32 propagated_uses;
	u64 values_analyzed;
};

static inline void global_constant_prop_init(struct GlobalConstantProp *gcp) {
	if (!gcp) return;
	gcp->constant_count = 0;
	gcp->propagated_uses = 0;
	gcp->values_analyzed = 0;
}

static inline u8 register_global_constant(
	struct GlobalConstantProp *gcp,
	u32 var_id,
	u64 value) {

	if (!gcp || gcp->constant_count >= 256) return 1;

	struct GlobalConstant *gc = &gcp->constants[gcp->constant_count];
	gc->var_id = var_id;
	gc->const_value = value;
	gc->propagation_count = 0;
	gc->is_proven = 1;

	gcp->constant_count++;
	return 0;
}

/* ============================================================ */
/* GLOBAL DEAD CODE ELIMINATION */
/* ============================================================ */

struct UnreachableFunction {
	u32 func_id;
	const char *func_name;
	u32 code_size;
	u32 call_count;
	u8 is_internal;
};

struct GlobalDeadCodeAnalyzer {
	struct UnreachableFunction unreachable[64];
	u32 unreachable_count;
	u64 bytes_removable;
	u32 functions_analyzed;
};

static inline void global_dead_code_init(struct GlobalDeadCodeAnalyzer *gdca) {
	if (!gdca) return;
	gdca->unreachable_count = 0;
	gdca->bytes_removable = 0;
	gdca->functions_analyzed = 0;
}

static inline u8 mark_unreachable_function(
	struct GlobalDeadCodeAnalyzer *gdca,
	u32 func_id,
	const char *name,
	u32 size) {

	if (!gdca || gdca->unreachable_count >= 64) return 1;

	struct UnreachableFunction *uf = &gdca->unreachable[gdca->unreachable_count];
	uf->func_id = func_id;
	uf->func_name = name;
	uf->code_size = size;
	uf->call_count = 0;
	uf->is_internal = 1;

	gdca->unreachable_count++;
	gdca->bytes_removable += size;
	return 0;
}

/* ============================================================ */
/* FUNCTION SPECIALIZATION ANALYSIS */
/* ============================================================ */

struct SpecializationOpportunity {
	u32 func_id;
	u32 param_id;
	u64 constant_value;
	u32 specialization_count;
	u64 bytes_saved;
};

struct FunctionSpecializer {
	struct SpecializationOpportunity opportunities[128];
	u32 opportunity_count;
	u32 specialized_functions;
	u64 total_savings;
};

static inline void function_specializer_init(struct FunctionSpecializer *fs) {
	if (!fs) return;
	fs->opportunity_count = 0;
	fs->specialized_functions = 0;
	fs->total_savings = 0;
}

static inline u8 add_specialization_opportunity(
	struct FunctionSpecializer *fs,
	u32 func_id,
	u32 param_id,
	u64 const_val) {

	if (!fs || fs->opportunity_count >= 128) return 1;

	struct SpecializationOpportunity *so = &fs->opportunities[fs->opportunity_count];
	so->func_id = func_id;
	so->param_id = param_id;
	so->constant_value = const_val;
	so->specialization_count = 0;
	so->bytes_saved = 0;

	fs->opportunity_count++;
	return 0;
}

/* ============================================================ */
/* GLOBAL ALIAS ANALYSIS */
/* ============================================================ */

enum AliasKind {
	ALIAS_NO_ALIAS = 0,
	ALIAS_MAY_ALIAS = 1,
	ALIAS_MUST_ALIAS = 2
};

struct AliasSet {
	u32 var_id;
	u32 var_count;
	u32 alias_vars[16];
	u8 alias_kind;
};

struct AliasAnalyzer {
	struct AliasSet alias_sets[64];
	u32 set_count;
	u32 alias_queries;
	u32 no_alias_found;
};

static inline void alias_analyzer_init(struct AliasAnalyzer *aa) {
	if (!aa) return;
	aa->set_count = 0;
	aa->alias_queries = 0;
	aa->no_alias_found = 0;
}

static inline u8 add_alias_set(
	struct AliasAnalyzer *aa,
	u32 var_id,
	u8 alias_kind) {

	if (!aa || aa->set_count >= 64) return 1;

	struct AliasSet *as = &aa->alias_sets[aa->set_count];
	as->var_id = var_id;
	as->var_count = 1;
	as->alias_kind = alias_kind;

	aa->set_count++;
	return 0;
}

/* ============================================================ */
/* PROGRAM SLICING */
/* ============================================================ */

struct ProgramSlice {
	u32 slice_id;
	u32 sliced_statements;
	u32 data_dependence_count;
	u32 control_dependence_count;
	u64 bytes_in_slice;
};

struct ProgramSlicer {
	struct ProgramSlice slices[32];
	u32 slice_count;
	u64 total_slice_bytes;
	u32 total_dependencies;
};

static inline void program_slicer_init(struct ProgramSlicer *ps) {
	if (!ps) return;
	ps->slice_count = 0;
	ps->total_slice_bytes = 0;
	ps->total_dependencies = 0;
}

static inline u8 create_program_slice(
	struct ProgramSlicer *ps,
	u32 slice_id) {

	if (!ps || ps->slice_count >= 32) return 1;

	struct ProgramSlice *slice = &ps->slices[ps->slice_count];
	slice->slice_id = slice_id;
	slice->sliced_statements = 0;
	slice->data_dependence_count = 0;
	slice->control_dependence_count = 0;
	slice->bytes_in_slice = 0;

	ps->slice_count++;
	return 0;
}

/* ============================================================ */
/* WHOLE-PROGRAM STATISTICS */
/* ============================================================ */

struct WholeProgramStats {
	u32 total_functions;
	u32 total_call_sites;
	u32 recursive_functions;
	u32 dead_functions;
	u32 specialized_functions;
	u64 total_code_bytes;
	u64 removable_bytes;
	u32 global_constants;
};

static inline struct WholeProgramStats compute_whole_program_stats(
	struct CallGraph *cg,
	struct GlobalDeadCodeAnalyzer *gdca,
	struct FunctionSpecializer *fs) {

	struct WholeProgramStats stats;
	stats.total_functions = cg ? cg->function_count : 0;
	stats.total_call_sites = cg ? cg->edge_count : 0;
	stats.recursive_functions = cg ? cg->recursive_functions : 0;
	stats.dead_functions = gdca ? gdca->unreachable_count : 0;
	stats.specialized_functions = fs ? fs->specialized_functions : 0;
	stats.total_code_bytes = 0;
	stats.removable_bytes = gdca ? gdca->bytes_removable : 0;
	stats.global_constants = 0;

	return stats;
}

#endif /* APKC_OPT_WHOLE_PROGRAM_H */
