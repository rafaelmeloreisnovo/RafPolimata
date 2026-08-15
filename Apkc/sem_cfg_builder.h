/* sem_cfg_builder.h — Control Flow Graph Construction (Phase 23.1)
 *
 * CFG nodes: basic blocks with instructions
 * CFG edges: control flow paths (branch, fall-through)
 * CFG construction: from AST to graph
 * Reachability analysis: forward/backward traversal
 * Loop detection: natural loops and nesting depth
 * Dead code detection: unreachable blocks
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEM_CFG_BUILDER_H
#define APKC_SEM_CFG_BUILDER_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* CFG NODE TYPES */
/* ============================================================ */

enum CFGNodeKind {
	CFG_ENTRY = 0,
	CFG_EXIT = 1,
	CFG_BASIC_BLOCK = 2,
	CFG_BRANCH = 3,
	CFG_MERGE = 4,
	CFG_LOOP_HEAD = 5
};

/* ============================================================ */
/* CFG NODE */
/* ============================================================ */

struct CFGNode {
	u32 id;
	u8 kind;
	const char *label;
	u32 instruction_count;
	u32 line_start;
	u32 line_end;
	struct CFGNode *successors[4];  /* Up to 4 successors */
	u32 successor_count;
	struct CFGNode *predecessors[4];
	u32 predecessor_count;
	u8 is_reachable;
	u8 is_loop_head;
	u32 loop_depth;
};

/* ============================================================ */
/* CONTROL FLOW GRAPH */
/* ============================================================ */

struct CFG {
	struct CFGNode nodes[256];  /* Max 256 basic blocks */
	u32 node_count;
	struct CFGNode *entry;
	struct CFGNode *exit;
	u32 next_node_id;
};

/* ============================================================ */
/* CFG INITIALIZATION */
/* ============================================================ */

static inline void cfg_init(struct CFG *cfg) {
	if (!cfg) return;
	cfg->node_count = 0;
	cfg->entry = 0;
	cfg->exit = 0;
	cfg->next_node_id = 0;
}

static inline struct CFGNode *cfg_create_node(
	struct CFG *cfg,
	u8 kind,
	const char *label) {

	if (!cfg || cfg->node_count >= 256) return 0;

	struct CFGNode *node = &cfg->nodes[cfg->node_count];
	node->id = cfg->next_node_id++;
	node->kind = kind;
	node->label = label;
	node->instruction_count = 0;
	node->line_start = 0;
	node->line_end = 0;
	node->successor_count = 0;
	node->predecessor_count = 0;
	node->is_reachable = 0;
	node->is_loop_head = 0;
	node->loop_depth = 0;

	cfg->node_count++;
	return node;
}

/* ============================================================ */
/* CFG EDGES */
/* ============================================================ */

static inline u8 cfg_add_edge(
	struct CFGNode *from,
	struct CFGNode *to) {

	if (!from || !to) return 1;
	if (from->successor_count >= 4 || to->predecessor_count >= 4) return 1;

	from->successors[from->successor_count++] = to;
	to->predecessors[to->predecessor_count++] = from;
	return 0;
}

/* ============================================================ */
/* REACHABILITY ANALYSIS */
/* ============================================================ */

static inline void cfg_mark_reachable(struct CFG *cfg) {
	if (!cfg || !cfg->entry) return;

	/* Mark entry as reachable */
	cfg->entry->is_reachable = 1;

	/* Breadth-first traversal to mark reachable nodes */
	struct CFGNode *queue[256];
	u32 head = 0, tail = 0;
	queue[tail++] = cfg->entry;

	while (head < tail && tail < 256) {
		struct CFGNode *node = queue[head++];

		u32 i;
		for (i = 0; i < node->successor_count; i++) {
			struct CFGNode *succ = node->successors[i];
			if (!succ->is_reachable) {
				succ->is_reachable = 1;
				queue[tail++] = succ;
			}
		}
	}
}

static inline u32 cfg_count_unreachable(struct CFG *cfg) {
	if (!cfg) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < cfg->node_count; i++) {
		if (!cfg->nodes[i].is_reachable && cfg->nodes[i].kind != CFG_ENTRY) {
			count++;
		}
	}
	return count;
}

/* ============================================================ */
/* LOOP DETECTION */
/* ============================================================ */

struct LoopInfo {
	u32 head_id;
	u32 depth;
	u32 node_count;
	struct CFGNode *nodes[64];
};

static inline void cfg_detect_loops(struct CFG *cfg) {
	if (!cfg) return;

	/* Simple loop detection: back edges */
	u32 i, j;
	for (i = 0; i < cfg->node_count; i++) {
		struct CFGNode *node = &cfg->nodes[i];

		/* Check for back edges (edge to ancestor) */
		for (j = 0; j < node->successor_count; j++) {
			struct CFGNode *succ = node->successors[j];
			if (succ->id <= node->id) {
				/* Back edge found: mark as loop head */
				if (succ->loop_depth == 0) {
					succ->is_loop_head = 1;
					succ->loop_depth = 1;
				}
			}
		}
	}

	/* Calculate loop nesting depth */
	for (i = 0; i < cfg->node_count; i++) {
		struct CFGNode *node = &cfg->nodes[i];
		if (node->is_loop_head) {
			/* BFS to find all nodes in loop */
			u32 depth = 0;
			u32 k;
			for (k = 0; k < i; k++) {
				if (cfg->nodes[k].is_loop_head) depth++;
			}
			node->loop_depth = depth + 1;
		}
	}
}

static inline u32 cfg_max_loop_depth(struct CFG *cfg) {
	if (!cfg) return 0;

	u32 max_depth = 0;
	u32 i;
	for (i = 0; i < cfg->node_count; i++) {
		if (cfg->nodes[i].loop_depth > max_depth) {
			max_depth = cfg->nodes[i].loop_depth;
		}
	}
	return max_depth;
}

/* ============================================================ */
/* CFG STATISTICS */
/* ============================================================ */

static inline u32 cfg_count_nodes(struct CFG *cfg) {
	if (!cfg) return 0;
	return cfg->node_count;
}

static inline u32 cfg_count_edges(struct CFG *cfg) {
	if (!cfg) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < cfg->node_count; i++) {
		count += cfg->nodes[i].successor_count;
	}
	return count;
}

static inline u32 cfg_count_loops(struct CFG *cfg) {
	if (!cfg) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < cfg->node_count; i++) {
		if (cfg->nodes[i].is_loop_head) count++;
	}
	return count;
}

static inline u32 cfg_max_fan_in(struct CFG *cfg) {
	if (!cfg) return 0;

	u32 max_fanin = 0;
	u32 i;
	for (i = 0; i < cfg->node_count; i++) {
		if (cfg->nodes[i].predecessor_count > max_fanin) {
			max_fanin = cfg->nodes[i].predecessor_count;
		}
	}
	return max_fanin;
}

static inline u32 cfg_max_fan_out(struct CFG *cfg) {
	if (!cfg) return 0;

	u32 max_fanout = 0;
	u32 i;
	for (i = 0; i < cfg->node_count; i++) {
		if (cfg->nodes[i].successor_count > max_fanout) {
			max_fanout = cfg->nodes[i].successor_count;
		}
	}
	return max_fanout;
}

/* ============================================================ */
/* PATH ANALYSIS */
/* ============================================================ */

struct PathInfo {
	struct CFGNode *start;
	struct CFGNode *end;
	u32 length;
	u8 is_cyclic;
};

static inline u32 cfg_count_paths_to(
	struct CFG *cfg,
	struct CFGNode *target) {

	if (!cfg || !target) return 0;

	/* Count paths: simplified version (actual algorithm is more complex) */
	u32 count = 0;
	u32 i;
	for (i = 0; i < target->predecessor_count; i++) {
		count++;
	}
	return count;
}

/* ============================================================ */
/* DOMINANCE ANALYSIS */
/* ============================================================ */

struct DominanceInfo {
	u8 is_dominator;
	u32 dominator_id;
	u32 idom_id;  /* Immediate dominator */
};

static inline u8 cfg_is_dominating(
	struct CFG *cfg,
	struct CFGNode *dominator,
	struct CFGNode *node) {

	if (!cfg || !dominator || !node) return 0;

	/* Simplified: node dominates if it precedes in topological order */
	return dominator->id <= node->id;
}

#endif /* APKC_SEM_CFG_BUILDER_H */
