/* opt_interprocedural.h — Interprocedural Analysis (Phase 41)
 *
 * Phase 41: Interprocedural analysis
 * - Function parameter analysis
 * - Return value analysis
 * - Summary function computation
 * - Transitive property inference
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_INTERPROCEDURAL_H
#define APKC_OPT_INTERPROCEDURAL_H 1

#include "opt_profile_guided.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* INTERPROCEDURAL DATA FLOW */
/* ============================================================ */

struct FunctionSignature {
	u32 func_id;
	u32 param_count;
	u32 param_types[32];
	u32 return_type;
	u8 is_pure;
	u8 has_side_effects;
};

struct InterproceduralAnalyzer {
	struct FunctionSignature signatures[256];
	u32 signature_count;
	u32 pure_functions;
	u32 functions_with_side_effects;
};

static inline void interprocedural_analyzer_init(struct InterproceduralAnalyzer *ipa) {
	if (!ipa) return;
	ipa->signature_count = 0;
	ipa->pure_functions = 0;
	ipa->functions_with_side_effects = 0;
}

static inline u8 register_function_signature(
	struct InterproceduralAnalyzer *ipa,
	u32 func_id,
	u32 param_count,
	u32 return_type) {

	if (!ipa || ipa->signature_count >= 256) return 1;

	struct FunctionSignature *fs = &ipa->signatures[ipa->signature_count];
	fs->func_id = func_id;
	fs->param_count = param_count;
	fs->return_type = return_type;
	fs->is_pure = 1;
	fs->has_side_effects = 0;

	ipa->signature_count++;
	ipa->pure_functions++;
	return 0;
}

/* ============================================================ */
/* PARAMETER ANALYSIS */
/* ============================================================ */

struct ParameterProperty {
	u32 param_id;
	u8 is_readonly;
	u8 is_constant;
	u8 escapes_function;
	u32 usage_count;
};

struct ParameterAnalyzer {
	struct ParameterProperty properties[128];
	u32 property_count;
	u32 readonly_params;
	u32 constant_params;
};

static inline void parameter_analyzer_init(struct ParameterAnalyzer *pa) {
	if (!pa) return;
	pa->property_count = 0;
	pa->readonly_params = 0;
	pa->constant_params = 0;
}

static inline u8 analyze_parameter(
	struct ParameterAnalyzer *pa,
	u32 param_id,
	u8 is_const,
	u8 escapes) {

	if (!pa || pa->property_count >= 128) return 1;

	struct ParameterProperty *pp = &pa->properties[pa->property_count];
	pp->param_id = param_id;
	pp->is_readonly = !escapes;
	pp->is_constant = is_const;
	pp->escapes_function = escapes;
	pp->usage_count = 0;

	pa->property_count++;
	if (pp->is_readonly) pa->readonly_params++;
	if (pp->is_constant) pa->constant_params++;
	return 0;
}

/* ============================================================ */
/* SUMMARY FUNCTION COMPUTATION */
/* ============================================================ */

struct SummaryFunction {
	u32 func_id;
	u64 call_count;
	u32 param_modifications[32];
	u32 return_computation;
	u8 is_pure_summary;
};

struct SummaryFunctionComputer {
	struct SummaryFunction summaries[256];
	u32 summary_count;
	u32 summarized_functions;
	u64 summary_accuracy;
};

static inline void summary_function_computer_init(struct SummaryFunctionComputer *sfc) {
	if (!sfc) return;
	sfc->summary_count = 0;
	sfc->summarized_functions = 0;
	sfc->summary_accuracy = 0;
}

static inline u8 compute_summary_function(
	struct SummaryFunctionComputer *sfc,
	u32 func_id,
	u64 call_count) {

	if (!sfc || sfc->summary_count >= 256) return 1;

	struct SummaryFunction *sf = &sfc->summaries[sfc->summary_count];
	sf->func_id = func_id;
	sf->call_count = call_count;
	sf->return_computation = 0;
	sf->is_pure_summary = 1;

	sfc->summary_count++;
	sfc->summarized_functions++;
	return 0;
}

/* ============================================================ */
/* TRANSITIVE PROPERTY INFERENCE */
/* ============================================================ */

enum PropertyKind {
	PROP_PURE = 0,
	PROP_READONLY = 1,
	PROP_NO_THROW = 2,
	PROP_CONST_TIME = 3
};

struct TransitiveProperty {
	u32 func_id;
	enum PropertyKind property;
	u8 is_inferred;
	u8 inference_confidence;
};

struct PropertyInferencer {
	struct TransitiveProperty properties[256];
	u32 property_count;
	u32 inferred_properties;
	u32 verified_inferences;
};

static inline void property_inferencer_init(struct PropertyInferencer *pi) {
	if (!pi) return;
	pi->property_count = 0;
	pi->inferred_properties = 0;
	pi->verified_inferences = 0;
}

static inline u8 infer_property(
	struct PropertyInferencer *pi,
	u32 func_id,
	enum PropertyKind prop,
	u8 confidence) {

	if (!pi || pi->property_count >= 256) return 1;

	struct TransitiveProperty *tp = &pi->properties[pi->property_count];
	tp->func_id = func_id;
	tp->property = prop;
	tp->is_inferred = 1;
	tp->inference_confidence = confidence;

	pi->property_count++;
	pi->inferred_properties++;
	return 0;
}

#endif /* APKC_OPT_INTERPROCEDURAL_H */
