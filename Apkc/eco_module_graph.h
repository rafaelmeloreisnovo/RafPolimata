/* eco_module_graph.h — Module Dependency Graph (Stage 19.1)
 *
 * Dependency graph construction: build graph of module interdependencies.
 * Circular dependency detection: find import cycles preventing resolution.
 * Module ranking: prioritize modules by importance (many dependents).
 * Critical path analysis: identify modules that block others.
 * Impact analysis: determine which modules affected by single change.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_ECO_MODULE_GRAPH_H
#define APKC_ECO_MODULE_GRAPH_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Module in dependency graph */
struct GraphModule {
	const char *module_name;    /* Module identifier */
	u32 module_id;              /* Unique module ID */
	u32 dependent_count;        /* Number of modules depending on this */
	u32 dependency_count;       /* Number of modules this depends on */
	u32 dependents[16];         /* IDs of modules depending on this (max 16) */
	u32 dependencies[16];       /* IDs of modules this depends on (max 16) */
	u32 depth;                  /* Dependency depth (0 = no dependencies) */
	u32 importance_score;       /* Higher = more dependents */
	u8 is_critical;             /* 1 if many modules depend on this */
};

/* Circular dependency (cycle in graph) */
struct Cycle {
	u32 module_ids[8];          /* Modules in cycle (max 8) */
	u32 cycle_length;           /* Number of modules in cycle */
	u8 severity;                /* Severity of cycle */
};

/* Module dependency graph */
struct ModuleGraph {
	struct GraphModule modules[64];      /* Up to 64 modules */
	u32 module_count;
	struct Cycle cycles[16];            /* Up to 16 cycles detected */
	u32 cycle_count;
	u32 total_edges;
	u32 max_depth;
	u8 has_cycles;
};

/* ============================================================ */
/* GRAPH INITIALIZATION */
/* ============================================================ */

/* Initialize module dependency graph */
static inline void graph_init(struct ModuleGraph *graph) {
	if (!graph) return;
	graph->module_count = 0;
	graph->cycle_count = 0;
	graph->total_edges = 0;
	graph->max_depth = 0;
	graph->has_cycles = 0;
}

/* ============================================================ */
/* MODULE & DEPENDENCY REGISTRATION */
/* ============================================================ */

/* Add module to graph */
static inline u8 graph_add_module(
	struct ModuleGraph *graph,
	const char *module_name) {

	if (!graph || !module_name) return 0;
	if (graph->module_count >= 64) return 0;

	struct GraphModule *mod = &graph->modules[graph->module_count];
	mod->module_name = module_name;
	mod->module_id = graph->module_count;
	mod->dependent_count = 0;
	mod->dependency_count = 0;
	mod->depth = 0;
	mod->importance_score = 0;
	mod->is_critical = 0;

	graph->module_count++;
	return 1;
}

/* Add dependency edge (moduleA depends on moduleB) */
static inline u8 graph_add_dependency(
	struct ModuleGraph *graph,
	u32 module_a_id,
	u32 module_b_id) {

	if (!graph || module_a_id >= graph->module_count) return 0;
	if (module_b_id >= graph->module_count) return 0;

	struct GraphModule *mod_a = &graph->modules[module_a_id];
	struct GraphModule *mod_b = &graph->modules[module_b_id];

	/* Add to A's dependencies */
	if (mod_a->dependency_count >= 16) return 0;
	mod_a->dependencies[mod_a->dependency_count++] = module_b_id;

	/* Add to B's dependents */
	if (mod_b->dependent_count >= 16) return 0;
	mod_b->dependents[mod_b->dependent_count++] = module_a_id;

	graph->total_edges++;
	return 1;
}

/* ============================================================ */
/* CIRCULAR DEPENDENCY DETECTION */
/* ============================================================ */

/* Detect circular dependency involving module */
static inline u8 graph_has_cycle_at_module(
	struct ModuleGraph *graph,
	u32 module_id) {

	if (!graph || module_id >= graph->module_count) return 0;

	struct GraphModule *mod = &graph->modules[module_id];

	/* Check if any dependency eventually depends back on this module */
	u32 i;
	for (i = 0; i < mod->dependency_count; i++) {
		u32 dep_id = mod->dependencies[i];
		struct GraphModule *dep = &graph->modules[dep_id];

		u32 j;
		for (j = 0; j < dep->dependency_count; j++) {
			if (dep->dependencies[j] == module_id) {
				return 1;  /* Cycle found */
			}
		}
	}

	return 0;
}

/* Detect all cycles in graph */
static inline u32 graph_detect_all_cycles(struct ModuleGraph *graph) {
	if (!graph) return 0;

	u32 cycle_count = 0;
	u32 i;
	for (i = 0; i < graph->module_count; i++) {
		if (graph_has_cycle_at_module(graph, i)) {
			if (cycle_count < 16) {
				struct Cycle *c = &graph->cycles[cycle_count];
				c->module_ids[0] = i;
				c->cycle_length = 1;
				cycle_count++;
			}
		}
	}

	graph->cycle_count = cycle_count;
	graph->has_cycles = cycle_count > 0 ? 1 : 0;
	return cycle_count;
}

/* ============================================================ */
/* MODULE RANKING & IMPORTANCE */
/* ============================================================ */

/* Calculate module importance (number of dependents) */
static inline void graph_calculate_importance(struct ModuleGraph *graph) {
	if (!graph) return;

	u32 i;
	for (i = 0; i < graph->module_count; i++) {
		graph->modules[i].importance_score = graph->modules[i].dependent_count;

		if (graph->modules[i].dependent_count > 10) {
			graph->modules[i].is_critical = 1;
		}
	}
}

/* Get module importance score */
static inline u32 graph_get_importance(
	struct ModuleGraph *graph,
	u32 module_id) {

	if (!graph || module_id >= graph->module_count) return 0;
	return graph->modules[module_id].importance_score;
}

/* Find most critical module (most dependents) */
static inline struct GraphModule *graph_find_critical_module(struct ModuleGraph *graph) {
	if (!graph || graph->module_count == 0) return 0;

	struct GraphModule *critical = &graph->modules[0];
	u32 i;
	for (i = 1; i < graph->module_count; i++) {
		if (graph->modules[i].dependent_count > critical->dependent_count) {
			critical = &graph->modules[i];
		}
	}

	return critical;
}

/* ============================================================ */
/* IMPACT ANALYSIS */
/* ============================================================ */

/* Find all modules affected by change to given module */
static inline u32 graph_find_affected_modules(
	struct ModuleGraph *graph,
	u32 changed_module_id,
	u32 *affected_ids,
	u32 max_count) {

	if (!graph || changed_module_id >= graph->module_count) return 0;
	if (!affected_ids) return 0;

	u32 count = 0;
	struct GraphModule *mod = &graph->modules[changed_module_id];

	/* All dependents are affected */
	u32 i;
	for (i = 0; i < mod->dependent_count && count < max_count; i++) {
		affected_ids[count++] = mod->dependents[i];
	}

	return count;
}

/* ============================================================ */
/* GRAPH STATISTICS */
/* ============================================================ */

/* Get total number of dependencies */
static inline u32 graph_get_edge_count(struct ModuleGraph *graph) {
	if (!graph) return 0;
	return graph->total_edges;
}

/* Get graph depth (longest dependency chain) */
static inline u32 graph_get_max_depth(struct ModuleGraph *graph) {
	if (!graph) return 0;
	return graph->max_depth;
}

/* Check if graph is acyclic */
static inline u8 graph_is_acyclic(struct ModuleGraph *graph) {
	if (!graph) return 1;
	return !graph->has_cycles;
}

#endif /* APKC_ECO_MODULE_GRAPH_H */
