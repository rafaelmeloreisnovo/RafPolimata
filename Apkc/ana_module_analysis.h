/* ana_module_analysis.h — Module Dependency Analysis (Stage 13.1)
 *
 * Analyze module dependencies: build complete dependency graphs.
 * Dependency visualization: output in GraphML/DOT format.
 * Critical path analysis: find longest dependency chain.
 * Circular dependency detection and reporting.
 * Module statistics: size, complexity, external dependencies.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_ANA_MODULE_ANALYSIS_H
#define APKC_ANA_MODULE_ANALYSIS_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Module dependency graph status */
enum ModuleAnalysisStatus {
	ANALYSIS_OK = 0,            /* Analysis complete and valid */
	ANALYSIS_INVALID_INPUT = 1, /* Invalid module or dependency data */
	ANALYSIS_CYCLE_DETECTED = 2,/* Circular dependency found */
	ANALYSIS_MAX_DEPTH = 3,     /* Dependency depth exceeds limit */
	ANALYSIS_INCOMPLETE = 4,    /* Some modules not analyzed */
	ANALYSIS_MEMORY_LIMIT = 5   /* Analysis buffer overflow */
};

/* Module dependency edge */
struct ModuleDependency {
	const char *from_module;    /* Source module name */
	const char *to_module;      /* Target module name */
	u32 strength;               /* Dependency strength (0-100) */
	u8 is_circular;             /* 1 if part of circular dependency */
	u8 is_critical;             /* 1 if on critical path */
};

/* Module analysis metadata */
struct ModuleMetadata {
	const char *name;           /* Module name */
	u32 size_bytes;             /* Total module size */
	u32 code_size;              /* Code section size */
	u32 data_size;              /* Data section size */
	u32 dependency_count;       /* Number of dependencies */
	u32 complexity_score;       /* 0-1000 complexity metric */
	u8 is_leaf;                 /* 1 if no dependencies */
	u8 is_root;                 /* 1 if no dependents */
	u32 depth_from_root;        /* Levels from root module */
};

/* Module analysis graph */
struct ModuleGraph {
	struct ModuleMetadata modules[64];     /* Up to 64 modules */
	struct ModuleDependency deps[128];     /* Up to 128 dependencies */
	u32 module_count;
	u32 dependency_count;
	u32 critical_path_length;
	u32 max_depth;
};

/* ============================================================ */
/* GRAPH INITIALIZATION & ANALYSIS */
/* ============================================================ */

/* Initialize module graph */
static inline void analysis_init_graph(struct ModuleGraph *graph) {
	if (!graph) return;
	graph->module_count = 0;
	graph->dependency_count = 0;
	graph->critical_path_length = 0;
	graph->max_depth = 0;
}

/* Add module to graph */
static inline u8 analysis_add_module(
	struct ModuleGraph *graph,
	const char *name,
	u32 size_bytes,
	u32 complexity_score) {

	if (!graph || !name) return ANALYSIS_INVALID_INPUT;
	if (graph->module_count >= 64) return ANALYSIS_MEMORY_LIMIT;

	struct ModuleMetadata *mod = &graph->modules[graph->module_count];
	mod->name = name;
	mod->size_bytes = size_bytes;
	mod->complexity_score = complexity_score;
	mod->dependency_count = 0;
	mod->is_leaf = 1;
	mod->is_root = 1;
	mod->depth_from_root = 0;

	graph->module_count++;
	return ANALYSIS_OK;
}

/* Add dependency edge */
static inline u8 analysis_add_dependency(
	struct ModuleGraph *graph,
	const char *from_module,
	const char *to_module,
	u32 strength) {

	if (!graph || !from_module || !to_module) return ANALYSIS_INVALID_INPUT;
	if (graph->dependency_count >= 128) return ANALYSIS_MEMORY_LIMIT;
	if (strength > 100) strength = 100;

	struct ModuleDependency *dep = &graph->deps[graph->dependency_count];
	dep->from_module = from_module;
	dep->to_module = to_module;
	dep->strength = strength;
	dep->is_circular = 0;
	dep->is_critical = 0;

	/* Update from_module is_leaf flag */
	u32 i;
	for (i = 0; i < graph->module_count; i++) {
		if (graph->modules[i].name) {
			const char *mname = graph->modules[i].name;
			u32 j = 0;
			while (from_module[j] && mname[j] && from_module[j] == mname[j]) j++;
			if (from_module[j] == 0 && mname[j] == 0) {
				graph->modules[i].is_leaf = 0;
				graph->modules[i].dependency_count++;
				break;
			}
		}
	}

	/* Update to_module is_root flag */
	for (i = 0; i < graph->module_count; i++) {
		if (graph->modules[i].name) {
			const char *mname = graph->modules[i].name;
			u32 j = 0;
			while (to_module[j] && mname[j] && to_module[j] == mname[j]) j++;
			if (to_module[j] == 0 && mname[j] == 0) {
				graph->modules[i].is_root = 0;
				break;
			}
		}
	}

	graph->dependency_count++;
	return ANALYSIS_OK;
}

/* ============================================================ */
/* CIRCULAR DEPENDENCY DETECTION */
/* ============================================================ */

/* Detect circular dependencies via DFS */
static inline u8 analysis_detect_cycles(
	struct ModuleGraph *graph,
	const char *module_name,
	u32 depth) {

	if (!graph || !module_name || depth > 32) return ANALYSIS_CYCLE_DETECTED;

	/* Find module in graph */
	u32 i;
	for (i = 0; i < graph->module_count; i++) {
		if (!graph->modules[i].name) continue;

		const char *mname = graph->modules[i].name;
		u32 j = 0;
		while (module_name[j] && mname[j] && module_name[j] == mname[j]) j++;

		if (module_name[j] == 0 && mname[j] == 0) {
			/* Found the module, check dependencies FROM this module */
			u32 k;
			for (k = 0; k < graph->dependency_count; k++) {
				struct ModuleDependency *dep = &graph->deps[k];
				const char *from = dep->from_module;

				/* Does this dependency start from our module? */
				u32 m = 0;
				while (module_name[m] && from[m] && module_name[m] == from[m]) m++;

				if (module_name[m] == 0 && from[m] == 0) {
					/* This dependency goes FROM our module */
					const char *to = dep->to_module;

					/* If depth > 0 and we reach back to start, cycle detected */
					if (depth > 0) {
						u32 p = 0;
						while (to[p] && module_name[p] && to[p] == module_name[p]) p++;
						if (to[p] == 0 && module_name[p] == 0) {
							return ANALYSIS_CYCLE_DETECTED;
						}
					}

					/* Recurse on the target module */
					u8 result = analysis_detect_cycles(graph, to, depth + 1);
					if (result == ANALYSIS_CYCLE_DETECTED) {
						return ANALYSIS_CYCLE_DETECTED;
					}
				}
			}
			return ANALYSIS_OK;
		}
	}

	return ANALYSIS_OK;
}

/* ============================================================ */
/* CRITICAL PATH ANALYSIS */
/* ============================================================ */

/* Compute depth of each module in dependency tree */
static inline void analysis_compute_depths(struct ModuleGraph *graph) {
	if (!graph) return;

	u32 pass;
	for (pass = 0; pass < 32; pass++) {  /* Fixed 32 iterations for stable depth */
		u32 i;
		u32 max_depth = 0;

		for (i = 0; i < graph->module_count; i++) {
			if (!graph->modules[i].name) continue;

			const char *mod_name = graph->modules[i].name;
			u32 current_depth = 0;

			/* Find all dependencies TO this module */
			u32 j;
			for (j = 0; j < graph->dependency_count; j++) {
				struct ModuleDependency *dep = &graph->deps[j];
				const char *to = dep->to_module;

				u32 k = 0;
				while (to[k] && mod_name[k] && to[k] == mod_name[k]) k++;

				if (to[k] == 0 && mod_name[k] == 0) {
					/* This dependency targets our module */
					/* Find source module and use its depth */
					u32 m;
					for (m = 0; m < graph->module_count; m++) {
						if (!graph->modules[m].name) continue;
						const char *src_name = graph->modules[m].name;
						u32 n = 0;
						while (dep->from_module[n] && src_name[n] &&
							   dep->from_module[n] == src_name[n]) n++;
						if (dep->from_module[n] == 0 && src_name[n] == 0) {
							u32 src_depth = graph->modules[m].depth_from_root;
							if (src_depth + 1 > current_depth) {
								current_depth = src_depth + 1;
							}
							break;
						}
					}
				}
			}

			if (current_depth > max_depth) {
				max_depth = current_depth;
			}

			graph->modules[i].depth_from_root = current_depth;
		}

		if (max_depth == graph->max_depth) break;  /* Converged */
		graph->max_depth = max_depth;
	}

	graph->critical_path_length = graph->max_depth;
}

/* ============================================================ */
/* MODULE STATISTICS */
/* ============================================================ */

/* Get total size of all modules */
static inline u64 analysis_total_size(struct ModuleGraph *graph) {
	if (!graph) return 0;

	u64 total = 0;
	u32 i;
	for (i = 0; i < graph->module_count; i++) {
		total += graph->modules[i].size_bytes;
	}
	return total;
}

/* Get average complexity score */
static inline u32 analysis_average_complexity(struct ModuleGraph *graph) {
	if (!graph || graph->module_count == 0) return 0;

	u64 total_complexity = 0;
	u32 i;
	for (i = 0; i < graph->module_count; i++) {
		total_complexity += graph->modules[i].complexity_score;
	}

	return (u32)(total_complexity / graph->module_count);
}

/* Count leaf modules (no dependencies) */
static inline u32 analysis_count_leaf_modules(struct ModuleGraph *graph) {
	if (!graph) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < graph->module_count; i++) {
		if (graph->modules[i].is_leaf) count++;
	}
	return count;
}

/* Count root modules (no dependents) */
static inline u32 analysis_count_root_modules(struct ModuleGraph *graph) {
	if (!graph) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < graph->module_count; i++) {
		if (graph->modules[i].is_root) count++;
	}
	return count;
}

/* Find module by name */
static inline struct ModuleMetadata *analysis_find_module(
	struct ModuleGraph *graph,
	const char *name) {

	if (!graph || !name) return 0;

	u32 i;
	for (i = 0; i < graph->module_count; i++) {
		if (!graph->modules[i].name) continue;

		const char *mname = graph->modules[i].name;
		u32 j = 0;
		while (name[j] && mname[j] && name[j] == mname[j]) j++;

		if (name[j] == 0 && mname[j] == 0) {
			return &graph->modules[i];
		}
	}

	return 0;
}

#endif /* APKC_ANA_MODULE_ANALYSIS_H */
