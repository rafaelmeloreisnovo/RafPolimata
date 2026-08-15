/* opt_link_time.h — Link-Time Optimization (Phase 39)
 *
 * Phase 39: Link-time optimization framework
 * - Inter-module function inlining
 * - Visibility-based optimizations
 * - Symbol resolution at link time
 * - Link-time code generation
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_LINK_TIME_H
#define APKC_OPT_LINK_TIME_H 1

#include "opt_target_specific.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* LINK-TIME INLINING */
/* ============================================================ */

struct InlineCandidate {
	u32 func_id;
	u32 caller_id;
	u32 call_site;
	u32 func_size;
	u8 is_hot;
	u64 benefit_estimate;
};

struct LinkTimeInliner {
	struct InlineCandidate candidates[128];
	u32 candidate_count;
	u32 inlined_count;
	u64 total_benefit;
};

static inline void link_time_inliner_init(struct LinkTimeInliner *lti) {
	if (!lti) return;
	lti->candidate_count = 0;
	lti->inlined_count = 0;
	lti->total_benefit = 0;
}

static inline u8 add_inline_candidate(
	struct LinkTimeInliner *lti,
	u32 func_id,
	u32 caller_id,
	u32 func_size) {

	if (!lti || lti->candidate_count >= 128) return 1;

	struct InlineCandidate *ic = &lti->candidates[lti->candidate_count];
	ic->func_id = func_id;
	ic->caller_id = caller_id;
	ic->call_site = 0;
	ic->func_size = func_size;
	ic->is_hot = 0;
	ic->benefit_estimate = 0;

	lti->candidate_count++;
	return 0;
}

/* ============================================================ */
/* VISIBILITY-BASED OPTIMIZATION */
/* ============================================================ */

enum LinkSymbolVisibility {
	LVIS_INTERNAL = 0,
	LVIS_HIDDEN = 1,
	LVIS_PROTECTED = 2,
	LVIS_DEFAULT = 3
};

struct SymbolLinkInfo {
	u32 symbol_id;
	const char *symbol_name;
	enum LinkSymbolVisibility visibility;
	u8 is_weak;
	u8 can_be_removed;
	u32 ref_count;
};

struct VisibilityOptimizer {
	struct SymbolLinkInfo symbols[256];
	u32 symbol_count;
	u32 removable_symbols;
	u64 removable_bytes;
};

static inline void visibility_optimizer_init(struct VisibilityOptimizer *vo) {
	if (!vo) return;
	vo->symbol_count = 0;
	vo->removable_symbols = 0;
	vo->removable_bytes = 0;
}

static inline u8 add_symbol_link_info(
	struct VisibilityOptimizer *vo,
	u32 symbol_id,
	const char *name,
	enum LinkSymbolVisibility vis) {

	if (!vo || vo->symbol_count >= 256) return 1;

	struct SymbolLinkInfo *sli = &vo->symbols[vo->symbol_count];
	sli->symbol_id = symbol_id;
	sli->symbol_name = name;
	sli->visibility = vis;
	sli->is_weak = 0;
	sli->can_be_removed = (vis == LVIS_INTERNAL);
	sli->ref_count = 0;

	vo->symbol_count++;
	if (sli->can_be_removed) vo->removable_symbols++;
	return 0;
}

/* ============================================================ */
/* LINK-TIME SYMBOL RESOLUTION */
/* ============================================================ */

struct SymbolResolution {
	u32 symbol_id;
	u32 resolved_address;
	u8 is_resolved;
	u8 is_ambiguous;
	u32 candidates;
};

struct LinkTimeResolver {
	struct SymbolResolution resolutions[256];
	u32 resolution_count;
	u32 resolved_count;
	u32 unresolved_count;
};

static inline void link_time_resolver_init(struct LinkTimeResolver *ltr) {
	if (!ltr) return;
	ltr->resolution_count = 0;
	ltr->resolved_count = 0;
	ltr->unresolved_count = 0;
}

static inline u8 resolve_symbol(
	struct LinkTimeResolver *ltr,
	u32 symbol_id,
	u32 address) {

	if (!ltr || ltr->resolution_count >= 256) return 1;

	struct SymbolResolution *sr = &ltr->resolutions[ltr->resolution_count];
	sr->symbol_id = symbol_id;
	sr->resolved_address = address;
	sr->is_resolved = 1;
	sr->is_ambiguous = 0;
	sr->candidates = 1;

	ltr->resolution_count++;
	ltr->resolved_count++;
	return 0;
}

/* ============================================================ */
/* LINK-TIME CODE GENERATION */
/* ============================================================ */

struct GeneratedCode {
	u32 code_id;
	u32 size;
	const char *code_section;
	u8 is_hot_path;
	u32 execution_count;
};

struct LinkTimeCodeGen {
	struct GeneratedCode code_blocks[64];
	u32 code_count;
	u64 total_generated_bytes;
	u32 hot_code_count;
};

static inline void link_time_codegen_init(struct LinkTimeCodeGen *ltcg) {
	if (!ltcg) return;
	ltcg->code_count = 0;
	ltcg->total_generated_bytes = 0;
	ltcg->hot_code_count = 0;
}

static inline u8 generate_link_time_code(
	struct LinkTimeCodeGen *ltcg,
	u32 code_id,
	u32 size) {

	if (!ltcg || ltcg->code_count >= 64) return 1;

	struct GeneratedCode *gc = &ltcg->code_blocks[ltcg->code_count];
	gc->code_id = code_id;
	gc->size = size;
	gc->code_section = 0;
	gc->is_hot_path = 0;
	gc->execution_count = 0;

	ltcg->code_count++;
	ltcg->total_generated_bytes += size;
	return 0;
}

/* ============================================================ */
/* INTER-MODULE OPTIMIZATION */
/* ============================================================ */

struct ModuleInfo {
	u32 module_id;
	const char *module_name;
	u32 function_count;
	u32 exported_symbols;
	u64 module_size;
	u8 is_hot;
};

struct InterModuleOptimizer {
	struct ModuleInfo modules[32];
	u32 module_count;
	u32 cross_module_inlines;
	u64 cross_module_savings;
};

static inline void inter_module_optimizer_init(struct InterModuleOptimizer *imo) {
	if (!imo) return;
	imo->module_count = 0;
	imo->cross_module_inlines = 0;
	imo->cross_module_savings = 0;
}

static inline u8 register_module(
	struct InterModuleOptimizer *imo,
	u32 module_id,
	const char *name) {

	if (!imo || imo->module_count >= 32) return 1;

	struct ModuleInfo *mi = &imo->modules[imo->module_count];
	mi->module_id = module_id;
	mi->module_name = name;
	mi->function_count = 0;
	mi->exported_symbols = 0;
	mi->module_size = 0;
	mi->is_hot = 0;

	imo->module_count++;
	return 0;
}

#endif /* APKC_OPT_LINK_TIME_H */
