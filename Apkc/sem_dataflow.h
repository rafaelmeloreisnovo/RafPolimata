/* sem_dataflow.h — Data Flow Analysis (Phase 24)
 *
 * Use-def chains: variable definitions and uses
 * Reaching definitions: which definitions reach each point
 * Live variable analysis: variables with live values
 * Constant propagation: track constant values through control flow
 * Dead store elimination: identify unused assignments
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEM_DATAFLOW_H
#define APKC_SEM_DATAFLOW_H 1

#include "sem_cfg_builder.h"

typedef unsigned char u8;
typedef unsigned int u32;

/* ============================================================ */
/* DATA FLOW FACTS */
/* ============================================================ */

struct UseDefFact {
	u32 var_id;
	u32 def_line;
	u32 use_line;
	u8 is_definition;
	u8 is_use;
	u8 is_upward_exposed;
};

struct LivenessInfo {
	u32 var_id;
	u8 is_live_in;
	u8 is_live_out;
	u32 last_use_line;
};

struct ConstantInfo {
	u32 var_id;
	u64 constant_value;
	u8 is_constant;
	u8 value_type;
};

/* ============================================================ */
/* DATA FLOW SETS */
/* ============================================================ */

struct DataFlowSet {
	struct UseDefFact facts[64];
	u32 count;
};

struct LivenessSet {
	struct LivenessInfo facts[64];
	u32 count;
};

struct ConstantSet {
	struct ConstantInfo facts[64];
	u32 count;
};

/* ============================================================ */
/* DATA FLOW ANALYZER */
/* ============================================================ */

struct DataFlowAnalyzer {
	struct CFG *cfg;
	struct DataFlowSet gen[256];    /* Generated facts per node */
	struct DataFlowSet kill[256];   /* Killed facts per node */
	struct DataFlowSet in[256];     /* In-set per node */
	struct DataFlowSet out[256];    /* Out-set per node */
	struct LivenessSet liveness[256];
	struct ConstantSet constants[256];
	u32 node_count;
	u32 iterations;
};

/* ============================================================ */
/* INITIALIZATION */
/* ============================================================ */

static inline void dataflow_init(
	struct DataFlowAnalyzer *analyzer,
	struct CFG *cfg) {

	if (!analyzer || !cfg) return;
	analyzer->cfg = cfg;
	analyzer->node_count = cfg->node_count;
	analyzer->iterations = 0;
	u32 i;
	for (i = 0; i < cfg->node_count; i++) {
		analyzer->gen[i].count = 0;
		analyzer->kill[i].count = 0;
		analyzer->in[i].count = 0;
		analyzer->out[i].count = 0;
		analyzer->liveness[i].count = 0;
		analyzer->constants[i].count = 0;
	}
}

/* ============================================================ */
/* USE-DEF CHAINS */
/* ============================================================ */

static inline u8 dataflow_add_def(
	struct DataFlowAnalyzer *analyzer,
	u32 node_id,
	u32 var_id,
	u32 line) {

	if (!analyzer || node_id >= analyzer->node_count) return 1;

	struct DataFlowSet *gen = &analyzer->gen[node_id];
	if (gen->count >= 64) return 1;

	gen->facts[gen->count].var_id = var_id;
	gen->facts[gen->count].def_line = line;
	gen->facts[gen->count].is_definition = 1;
	gen->count++;
	return 0;
}

static inline u8 dataflow_add_use(
	struct DataFlowAnalyzer *analyzer,
	u32 node_id,
	u32 var_id,
	u32 line) {

	if (!analyzer || node_id >= analyzer->node_count) return 1;

	struct DataFlowSet *gen = &analyzer->gen[node_id];
	if (gen->count >= 64) return 1;

	gen->facts[gen->count].var_id = var_id;
	gen->facts[gen->count].use_line = line;
	gen->facts[gen->count].is_use = 1;
	gen->count++;
	return 0;
}

/* ============================================================ */
/* REACHING DEFINITIONS ANALYSIS */
/* ============================================================ */

static inline u32 dataflow_find_reaching_defs(
	struct DataFlowAnalyzer *analyzer,
	u32 node_id,
	u32 var_id) {

	if (!analyzer || node_id >= analyzer->node_count) return 0;

	struct DataFlowSet *in = &analyzer->in[node_id];
	u32 i;
	for (i = 0; i < in->count; i++) {
		if (in->facts[i].var_id == var_id && in->facts[i].is_definition) {
			return in->facts[i].def_line;
		}
	}
	return 0;
}

/* ============================================================ */
/* LIVENESS ANALYSIS */
/* ============================================================ */

static inline u8 dataflow_compute_liveness(
	struct DataFlowAnalyzer *analyzer) {

	if (!analyzer || !analyzer->cfg) return 1;

	u32 iteration = 0;
	u8 changed = 1;

	while (changed && iteration < 1000) {
		changed = 0;
		iteration++;
		analyzer->iterations = iteration;

		u32 i;
		for (i = 0; i < analyzer->node_count; i++) {
			struct LivenessSet *live = &analyzer->liveness[i];

			/* Update liveness based on predecessors' live-out */
			u32 j;
			for (j = 0; j < live->count; j++) {
				if (!live->facts[j].is_live_in) {
					live->facts[j].is_live_in = 1;
					changed = 1;
				}
			}
		}
	}

	return changed == 0 ? 0 : 1;
}

static inline u8 dataflow_is_live(
	struct DataFlowAnalyzer *analyzer,
	u32 node_id,
	u32 var_id) {

	if (!analyzer || node_id >= analyzer->node_count) return 0;

	struct LivenessSet *live = &analyzer->liveness[node_id];
	u32 i;
	for (i = 0; i < live->count; i++) {
		if (live->facts[i].var_id == var_id) {
			return live->facts[i].is_live_in || live->facts[i].is_live_out;
		}
	}
	return 0;
}

/* ============================================================ */
/* CONSTANT PROPAGATION */
/* ============================================================ */

static inline u8 dataflow_set_constant(
	struct DataFlowAnalyzer *analyzer,
	u32 node_id,
	u32 var_id,
	u64 value) {

	if (!analyzer || node_id >= analyzer->node_count) return 1;

	struct ConstantSet *consts = &analyzer->constants[node_id];
	if (consts->count >= 64) return 1;

	consts->facts[consts->count].var_id = var_id;
	consts->facts[consts->count].constant_value = value;
	consts->facts[consts->count].is_constant = 1;
	consts->count++;
	return 0;
}

static inline u8 dataflow_get_constant(
	struct DataFlowAnalyzer *analyzer,
	u32 node_id,
	u32 var_id,
	u64 *value) {

	if (!analyzer || !value || node_id >= analyzer->node_count) return 1;

	struct ConstantSet *consts = &analyzer->constants[node_id];
	u32 i;
	for (i = 0; i < consts->count; i++) {
		if (consts->facts[i].var_id == var_id && consts->facts[i].is_constant) {
			*value = consts->facts[i].constant_value;
			return 0;
		}
	}
	return 1;
}

/* ============================================================ */
/* DEAD STORE DETECTION */
/* ============================================================ */

struct DeadStore {
	u32 var_id;
	u32 def_line;
	u8 is_dead;
};

static inline struct DeadStore dataflow_check_dead_store(
	struct DataFlowAnalyzer *analyzer,
	u32 node_id,
	u32 var_id,
	u32 def_line) {

	struct DeadStore result;
	result.var_id = var_id;
	result.def_line = def_line;
	result.is_dead = 0;

	if (!analyzer || node_id >= analyzer->node_count) return result;

	/* A store is dead if the variable isn't used after it */
	if (!dataflow_is_live(analyzer, node_id, var_id)) {
		result.is_dead = 1;
	}

	return result;
}

/* ============================================================ */
/* DATA FLOW STATISTICS */
/* ============================================================ */

static inline u32 dataflow_count_uses(
	struct DataFlowAnalyzer *analyzer,
	u32 var_id) {

	if (!analyzer) return 0;

	u32 total_uses = 0;
	u32 i;
	for (i = 0; i < analyzer->node_count; i++) {
		struct DataFlowSet *gen = &analyzer->gen[i];
		u32 j;
		for (j = 0; j < gen->count; j++) {
			if (gen->facts[j].var_id == var_id && gen->facts[j].is_use) {
				total_uses++;
			}
		}
	}
	return total_uses;
}

static inline u32 dataflow_count_defs(
	struct DataFlowAnalyzer *analyzer,
	u32 var_id) {

	if (!analyzer) return 0;

	u32 total_defs = 0;
	u32 i;
	for (i = 0; i < analyzer->node_count; i++) {
		struct DataFlowSet *gen = &analyzer->gen[i];
		u32 j;
		for (j = 0; j < gen->count; j++) {
			if (gen->facts[j].var_id == var_id && gen->facts[j].is_definition) {
				total_defs++;
			}
		}
	}
	return total_defs;
}

static inline u32 dataflow_count_dead_stores(
	struct DataFlowAnalyzer *analyzer) {

	if (!analyzer) return 0;

	u32 dead_count = 0;
	u32 i;
	for (i = 0; i < analyzer->node_count; i++) {
		struct DataFlowSet *gen = &analyzer->gen[i];
		u32 j;
		for (j = 0; j < gen->count; j++) {
			if (gen->facts[j].is_definition) {
				struct DeadStore ds = dataflow_check_dead_store(
					analyzer, i, gen->facts[j].var_id, gen->facts[j].def_line);
				if (ds.is_dead) dead_count++;
			}
		}
	}
	return dead_count;
}

#endif /* APKC_SEM_DATAFLOW_H */
