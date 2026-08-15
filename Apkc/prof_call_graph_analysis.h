/* prof_call_graph_analysis.h — Call Graph & Performance Analysis (Stage 16.2)
 *
 * Call graph construction: track function call relationships.
 * Call tree visualization: parent-child call hierarchies.
 * Cycle detection: identify recursive call chains.
 * Performance attribution: allocate execution time to call paths.
 * Hotspot identification: find most frequently called functions.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_PROF_CALL_GRAPH_ANALYSIS_H
#define APKC_PROF_CALL_GRAPH_ANALYSIS_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Call graph edge */
struct CallEdge {
	u32 from_func_idx;          /* Calling function index */
	u32 to_func_idx;            /* Called function index */
	u32 call_count;             /* Number of times this edge executes */
	u64 total_time_ns;          /* Total time spent through this call */
	u8 is_recursive;            /* 1 if this edge is part of recursion */
};

/* Function node in call graph */
struct FunctionNode {
	const char *function_name;  /* Function identifier */
	u32 call_count;             /* Total invocation count */
	u64 self_time_ns;           /* Time spent in function only */
	u64 total_time_ns;          /* Total time including callees */
	u32 callee_count;           /* Number of functions called */
	u32 callees[16];            /* Indices of called functions (max 16) */
	u32 caller_count;           /* Number of callers */
	u32 callers[8];             /* Indices of calling functions (max 8) */
	u8 is_recursive;            /* 1 if function calls itself */
	u8 is_leaf;                 /* 1 if no outgoing calls */
	u32 recursion_depth;        /* Max recursion nesting observed */
};

/* Call graph analyzer */
struct CallGraph {
	struct FunctionNode functions[64];   /* Up to 64 functions */
	u32 function_count;
	struct CallEdge edges[256];          /* Up to 256 call edges */
	u32 edge_count;
	u32 total_functions_called;         /* Total call count across all functions */
	u64 total_execution_time_ns;        /* Sum of all self times */
	u32 max_recursion_depth;            /* Maximum recursion depth observed */
};

/* ============================================================ */
/* CALL GRAPH INITIALIZATION */
/* ============================================================ */

/* Initialize call graph analyzer */
static inline void callgraph_init(struct CallGraph *cg) {
	if (!cg) return;
	cg->function_count = 0;
	cg->edge_count = 0;
	cg->total_functions_called = 0;
	cg->total_execution_time_ns = 0;
	cg->max_recursion_depth = 0;
}

/* ============================================================ */
/* FUNCTION & EDGE REGISTRATION */
/* ============================================================ */

/* Register function in call graph */
static inline u8 callgraph_add_function(
	struct CallGraph *cg,
	const char *function_name) {

	if (!cg || !function_name) return 0;
	if (cg->function_count >= 64) return 0;

	struct FunctionNode *func = &cg->functions[cg->function_count];
	func->function_name = function_name;
	func->call_count = 0;
	func->self_time_ns = 0;
	func->total_time_ns = 0;
	func->callee_count = 0;
	func->caller_count = 0;
	func->is_recursive = 0;
	func->is_leaf = 1;  /* Assume leaf until proven otherwise */
	func->recursion_depth = 0;

	cg->function_count++;
	return 1;
}

/* Add call edge between functions */
static inline u8 callgraph_add_call(
	struct CallGraph *cg,
	u32 from_idx,
	u32 to_idx,
	u32 call_count,
	u64 total_time) {

	if (!cg) return 0;
	if (from_idx >= cg->function_count || to_idx >= cg->function_count) return 0;
	if (cg->edge_count >= 256) return 0;

	struct CallEdge *edge = &cg->edges[cg->edge_count];
	edge->from_func_idx = from_idx;
	edge->to_func_idx = to_idx;
	edge->call_count = call_count;
	edge->total_time_ns = total_time;
	edge->is_recursive = (from_idx == to_idx) ? 1 : 0;

	cg->edge_count++;

	/* Update function metadata */
	struct FunctionNode *caller = &cg->functions[from_idx];
	struct FunctionNode *callee = &cg->functions[to_idx];

	/* Track callee */
	if (caller->callee_count < 16) {
		caller->callees[caller->callee_count] = to_idx;
		caller->callee_count++;
		caller->is_leaf = 0;  /* No longer a leaf */
	}

	/* Track caller */
	if (callee->caller_count < 8) {
		callee->callers[callee->caller_count] = from_idx;
		callee->caller_count++;
	}

	/* Mark recursion */
	if (edge->is_recursive) {
		callee->is_recursive = 1;
	}

	cg->total_functions_called += call_count;
	cg->total_execution_time_ns += total_time;

	return 1;
}

/* ============================================================ */
/* RECURSION & CYCLE DETECTION */
/* ============================================================ */

/* Check if function is part of a cycle (simplified check) */
static inline u8 callgraph_has_cycle_at_function(
	struct CallGraph *cg,
	u32 func_idx) {

	if (!cg || func_idx >= cg->function_count) return 0;

	struct FunctionNode *func = &cg->functions[func_idx];

	/* Check if function calls itself (direct recursion) */
	if (func->is_recursive) return 1;

	/* Check if any callee leads back to this function (simplified: depth 1 only) */
	u32 i;
	for (i = 0; i < func->callee_count; i++) {
		struct FunctionNode *callee = &cg->functions[func->callees[i]];
		u32 j;
		for (j = 0; j < callee->callee_count; j++) {
			if (callee->callees[j] == func_idx) {
				return 1;  /* Found cycle */
			}
		}
	}

	return 0;
}

/* ============================================================ */
/* HOTSPOT DETECTION */
/* ============================================================ */

/* Find function with highest call count */
static inline struct FunctionNode *callgraph_find_hottest_function(struct CallGraph *cg) {
	if (!cg || cg->function_count == 0) return 0;

	struct FunctionNode *hottest = &cg->functions[0];
	u32 i;
	for (i = 1; i < cg->function_count; i++) {
		if (cg->functions[i].call_count > hottest->call_count) {
			hottest = &cg->functions[i];
		}
	}

	return hottest;
}

/* Find function with highest total time */
static inline struct FunctionNode *callgraph_find_slowest_function(struct CallGraph *cg) {
	if (!cg || cg->function_count == 0) return 0;

	struct FunctionNode *slowest = &cg->functions[0];
	u32 i;
	for (i = 1; i < cg->function_count; i++) {
		if (cg->functions[i].total_time_ns > slowest->total_time_ns) {
			slowest = &cg->functions[i];
		}
	}

	return slowest;
}

/* ============================================================ */
/* CALL GRAPH STATISTICS */
/* ============================================================ */

/* Get leaf function count (functions that don't call others) */
static inline u32 callgraph_count_leaf_functions(struct CallGraph *cg) {
	if (!cg) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < cg->function_count; i++) {
		if (cg->functions[i].is_leaf) {
			count++;
		}
	}

	return count;
}

/* Get average call count per function */
static inline u32 callgraph_get_average_call_count(struct CallGraph *cg) {
	if (!cg || cg->function_count == 0) return 0;
	return cg->total_functions_called / cg->function_count;
}

/* Get average execution time per function */
static inline u64 callgraph_get_average_time_ns(struct CallGraph *cg) {
	if (!cg || cg->function_count == 0) return 0;
	return cg->total_execution_time_ns / cg->function_count;
}

/* Count recursive functions */
static inline u32 callgraph_count_recursive_functions(struct CallGraph *cg) {
	if (!cg) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < cg->function_count; i++) {
		if (cg->functions[i].is_recursive) {
			count++;
		}
	}

	return count;
}

#endif /* APKC_PROF_CALL_GRAPH_ANALYSIS_H */
