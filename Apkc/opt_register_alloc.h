/* opt_register_alloc.h — Register Allocation & Live Range Analysis (Stage 6.2)
 *
 * Live range computation, interference graph construction, graph coloring.
 * Register allocation with spilling to stack when necessary.
 * Max 14 available registers (r0-r13); r14=sp, r15=return.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_REGISTER_ALLOC_H
#define APKC_OPT_REGISTER_ALLOC_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Live range for a value (variable or temporary) */
struct LiveRange {
	u32 value_id;              /* Unique value identifier */
	u32 start_pos;             /* First instruction using this value */
	u32 end_pos;               /* Last instruction using this value */
	u8 assigned_reg;           /* Allocated register (0-13), or 255 for spilled */
	u32 stack_offset;          /* Stack offset if spilled */
};

/* Interference between two values */
struct Interference {
	u32 value1;
	u32 value2;
	u8 interferes;  /* 1 if values have overlapping live ranges */
};

/* Node in interference graph */
struct GraphNode {
	u32 value_id;
	u32 degree;                /* Number of neighbors in graph */
	u8 neighbors[16];          /* Neighbor node indices */
	u32 neighbor_count;
	u8 color;                  /* Assigned register (0-13) or 255 for uncolored */
	u8 spilled;                /* 1 if value spilled to stack */
};

/* Interference graph for register allocation */
struct InterferenceGraph {
	struct GraphNode nodes[32];  /* Up to 32 values (variables/temporaries) */
	u32 node_count;
	struct Interference edges[128];  /* Up to 128 interference edges */
	u32 edge_count;
	u32 num_registers;         /* Number of available registers (14) */
};

/* Register allocation context */
struct RegAllocCtx {
	struct LiveRange live_ranges[32];  /* Live ranges for all values */
	u32 range_count;
	struct InterferenceGraph graph;
	u32 spill_count;           /* Number of values spilled to stack */
	u32 stack_offset;          /* Current stack offset for spills */
};

/* Instruction structure (from machine_linear_branchless.h) */
struct Insn {
	u32 opcode;
	u8 rd;
	u8 rs1;
	u8 rs2;
	u32 imm;
};

/* Initialize register allocation context */
static inline void regalloc_ctx_init(struct RegAllocCtx *rc, u32 num_regs) {
	rc->range_count = 0;
	rc->spill_count = 0;
	rc->stack_offset = 0;
	rc->graph.node_count = 0;
	rc->graph.edge_count = 0;
	rc->graph.num_registers = num_regs;  /* Usually 14 */
}

/* === LIVE RANGE ANALYSIS === */

/* Compute live ranges for all values in instruction stream */
static inline void liveness_analyze(
	struct Insn *insns, u32 total,
	struct RegAllocCtx *rc)
{
	/* Forward pass: mark first use of each register */
	u32 i;
	u8 first_use[32] = {0};  /* Track if register already seen */

	for (i = 0; i < total; i++) {
		struct Insn *insn = &insns[i];

		/* Check source registers */
		if (insn->rs1 < 32 && !first_use[insn->rs1]) {
			/* Find or create live range for rs1 */
			u32 j;
			for (j = 0; j < rc->range_count; j++) {
				if (rc->live_ranges[j].value_id == insn->rs1) {
					rc->live_ranges[j].start_pos = i;
					first_use[insn->rs1] = 1;
					break;
				}
			}
			if (j == rc->range_count && rc->range_count < 32) {
				/* New live range */
				rc->live_ranges[rc->range_count].value_id = insn->rs1;
				rc->live_ranges[rc->range_count].start_pos = i;
				rc->live_ranges[rc->range_count].end_pos = i;
				rc->range_count++;
				first_use[insn->rs1] = 1;
			}
		}

		if (insn->rs2 < 32 && !first_use[insn->rs2]) {
			u32 j;
			for (j = 0; j < rc->range_count; j++) {
				if (rc->live_ranges[j].value_id == insn->rs2) {
					rc->live_ranges[j].start_pos = i;
					first_use[insn->rs2] = 1;
					break;
				}
			}
			if (j == rc->range_count && rc->range_count < 32) {
				rc->live_ranges[rc->range_count].value_id = insn->rs2;
				rc->live_ranges[rc->range_count].start_pos = i;
				rc->live_ranges[rc->range_count].end_pos = i;
				rc->range_count++;
				first_use[insn->rs2] = 1;
			}
		}

		/* Update end position for destination register */
		if (insn->rd < 32) {
			u32 j;
			for (j = 0; j < rc->range_count; j++) {
				if (rc->live_ranges[j].value_id == insn->rd) {
					rc->live_ranges[j].end_pos = i;
					break;
				}
			}
			if (j == rc->range_count && rc->range_count < 32) {
				rc->live_ranges[rc->range_count].value_id = insn->rd;
				rc->live_ranges[rc->range_count].start_pos = i;
				rc->live_ranges[rc->range_count].end_pos = i;
				rc->range_count++;
			}
		}
	}
}

/* === INTERFERENCE GRAPH CONSTRUCTION === */

/* Build interference graph from live ranges */
static inline void build_interference_graph(
	struct RegAllocCtx *rc)
{
	u32 i, j;

	/* Create graph node for each live range */
	for (i = 0; i < rc->range_count && rc->graph.node_count < 32; i++) {
		rc->graph.nodes[rc->graph.node_count].value_id = rc->live_ranges[i].value_id;
		rc->graph.nodes[rc->graph.node_count].color = 255;
		rc->graph.nodes[rc->graph.node_count].spilled = 0;
		rc->graph.nodes[rc->graph.node_count].neighbor_count = 0;
		rc->graph.nodes[rc->graph.node_count].degree = 0;
		rc->graph.node_count++;
	}

	/* Find interference edges: ranges with overlapping lifetimes */
	for (i = 0; i < rc->range_count; i++) {
		for (j = i + 1; j < rc->range_count; j++) {
			struct LiveRange *r1 = &rc->live_ranges[i];
			struct LiveRange *r2 = &rc->live_ranges[j];

			/* Check if ranges overlap */
			if (!(r1->end_pos < r2->start_pos || r2->end_pos < r1->start_pos)) {
				/* Ranges interfere: add edge to graph */
				if (rc->graph.edge_count < 128) {
					rc->graph.edges[rc->graph.edge_count].value1 = r1->value_id;
					rc->graph.edges[rc->graph.edge_count].value2 = r2->value_id;
					rc->graph.edges[rc->graph.edge_count].interferes = 1;
					rc->graph.edge_count++;

					/* Update node degrees */
					u32 n1, n2;
					for (n1 = 0; n1 < rc->graph.node_count; n1++) {
						if (rc->graph.nodes[n1].value_id == r1->value_id) {
							rc->graph.nodes[n1].degree++;
							break;
						}
					}
					for (n2 = 0; n2 < rc->graph.node_count; n2++) {
						if (rc->graph.nodes[n2].value_id == r2->value_id) {
							rc->graph.nodes[n2].degree++;
							break;
						}
					}
				}
			}
		}
	}
}

/* === GRAPH COLORING (GREEDY) === */

/* Greedy coloring of interference graph */
static inline u32 color_graph(
	struct RegAllocCtx *rc)
{
	u32 num_colors = rc->graph.num_registers;  /* 14 registers */
	u32 spills = 0;
	u32 i, j;

	/* Iteratively color nodes with lowest degree first */
	for (i = 0; i < rc->graph.node_count; i++) {
		struct GraphNode *node = &rc->graph.nodes[i];
		u8 used_colors[14] = {0};  /* Track which colors are used */

		/* Check colors of neighbors */
		for (j = 0; j < rc->graph.edge_count; j++) {
			struct Interference *edge = &rc->graph.edges[j];
			u32 neighbor_idx = 255;

			if (edge->value1 == node->value_id) {
				u32 k;
				for (k = 0; k < rc->graph.node_count; k++) {
					if (rc->graph.nodes[k].value_id == edge->value2) {
						neighbor_idx = k;
						break;
					}
				}
			} else if (edge->value2 == node->value_id) {
				u32 k;
				for (k = 0; k < rc->graph.node_count; k++) {
					if (rc->graph.nodes[k].value_id == edge->value1) {
						neighbor_idx = k;
						break;
					}
				}
			}

			if (neighbor_idx != 255 && rc->graph.nodes[neighbor_idx].color < 14) {
				used_colors[rc->graph.nodes[neighbor_idx].color] = 1;
			}
		}

		/* Find first available color */
		u8 color_found = 0;
		for (j = 0; j < num_colors; j++) {
			if (!used_colors[j]) {
				node->color = j;
				color_found = 1;
				break;
			}
		}

		if (!color_found) {
			/* No colors available: spill this value */
			node->spilled = 1;
			node->color = 255;
			node->stack_offset = rc->stack_offset;
			rc->stack_offset += 8;  /* Each value is 8 bytes */
			spills++;
			rc->spill_count++;
		}
	}

	return spills;
}

/* === REGISTER ALLOCATION === */

/* Allocate registers to values based on coloring */
static inline void alloc_registers(
	struct Insn *insns, u32 total,
	struct RegAllocCtx *rc)
{
	/* Rewrite instructions using allocated registers */
	u32 i;
	for (i = 0; i < total; i++) {
		struct Insn *insn = &insns[i];

		/* Map value IDs to allocated registers */
		if (insn->rd < 32) {
			u32 j;
			for (j = 0; j < rc->graph.node_count; j++) {
				if (rc->graph.nodes[j].value_id == insn->rd) {
					if (!rc->graph.nodes[j].spilled) {
						insn->rd = rc->graph.nodes[j].color;
					} else {
						/* Spilled: will be handled by stack store instruction */
						insn->rd = 0;  /* Temporary register */
					}
					break;
				}
			}
		}

		if (insn->rs1 < 32) {
			u32 j;
			for (j = 0; j < rc->graph.node_count; j++) {
				if (rc->graph.nodes[j].value_id == insn->rs1) {
					if (!rc->graph.nodes[j].spilled) {
						insn->rs1 = rc->graph.nodes[j].color;
					} else {
						insn->rs1 = 0;
					}
					break;
				}
			}
		}

		if (insn->rs2 < 32) {
			u32 j;
			for (j = 0; j < rc->graph.node_count; j++) {
				if (rc->graph.nodes[j].value_id == insn->rs2) {
					if (!rc->graph.nodes[j].spilled) {
						insn->rs2 = rc->graph.nodes[j].color;
					} else {
						insn->rs2 = 0;
					}
					break;
				}
			}
		}
	}
}

#endif /* APKC_OPT_REGISTER_ALLOC_H */
