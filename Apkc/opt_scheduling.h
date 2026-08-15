/* opt_scheduling.h — Instruction Scheduling & Constant Folding (Stage 6.3)
 *
 * Dependency analysis, instruction reordering for pipeline efficiency.
 * Constant folding for compile-time expression evaluation.
 * Loop invariant code motion and strength reduction.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_SCHEDULING_H
#define APKC_OPT_SCHEDULING_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Instruction dependency */
struct DepEdge {
	u32 from_idx;              /* Source instruction index */
	u32 to_idx;                /* Destination instruction index */
	u8 dep_type;               /* 0=data, 1=control, 2=memory */
};

/* Dependency graph for instruction scheduling */
struct DepGraph {
	struct DepEdge edges[256];  /* Up to 256 dependencies */
	u32 edge_count;
	u32 num_insns;
};

/* Instruction structure */
struct Insn {
	u32 opcode;
	u8 rd;
	u8 rs1;
	u8 rs2;
	u32 imm;
};

/* Scheduling context */
struct ScheduleCtx {
	struct DepGraph dep_graph;
	u32 optimizations_applied;
	u8 enable_aggressive;  /* 1 = enable loop invariant motion, etc. */
};

/* Constant value */
struct ConstValue {
	u8 rd;
	u64 value;
	u32 opcode;             /* Instruction that produces value */
};

/* Initialize scheduling context */
static inline void schedule_ctx_init(struct ScheduleCtx *sc) {
	sc->dep_graph.edge_count = 0;
	sc->dep_graph.num_insns = 0;
	sc->optimizations_applied = 0;
	sc->enable_aggressive = 0;
}

/* === DEPENDENCY ANALYSIS === */

/* Check if two instructions have data dependency */
static inline u8 check_data_dependency(
	struct Insn *i1, struct Insn *i2)
{
	/* i1 writes to register, i2 reads it */
	if (i1->rd != 255 && (i1->rd == i2->rs1 || i1->rd == i2->rs2)) {
		return 1;  /* True dependency */
	}
	/* i1 and i2 both write to same register: anti-dependency */
	if (i1->rd != 255 && i1->rd == i2->rd) {
		return 1;  /* Write-after-write */
	}
	/* i2 reads what i1 writes, then i1 reads what i2 writes */
	if (i2->rd != 255 && (i2->rd == i1->rs1 || i2->rd == i1->rs2)) {
		return 1;  /* Anti-dependency */
	}
	return 0;
}

/* Check if two instructions have control dependency */
static inline u8 check_control_dependency(
	struct Insn *i1, struct Insn *i2)
{
	/* Assume OP_JMP=0x11, OP_JZ=0x12, OP_JNZ=0x13, OP_CALL=0x0E, OP_RET=0x0F */
	u32 branch_ops[] = {0x11, 0x12, 0x13, 0x0E, 0x0F};
	u32 i;

	/* Check if i1 is a branch */
	for (i = 0; i < 5; i++) {
		if (i1->opcode == branch_ops[i]) {
			return 1;  /* i2 is control-dependent on branch i1 */
		}
	}
	return 0;
}

/* Check if two instructions have memory dependency */
static inline u8 check_memory_dependency(
	struct Insn *i1, struct Insn *i2)
{
	/* Assume OP_LOAD=0x0C, OP_STORE=0x0D */
	u8 is_mem_op1 = (i1->opcode == 0x0C || i1->opcode == 0x0D);
	u8 is_mem_op2 = (i2->opcode == 0x0C || i2->opcode == 0x0D);

	if (is_mem_op1 && is_mem_op2) {
		/* Conservative: assume all memory ops depend on each other */
		return 1;
	}
	return 0;
}

/* Build dependency graph from instruction stream */
static inline void build_dep_graph(
	struct Insn *insns, u32 total,
	struct ScheduleCtx *sc)
{
	sc->dep_graph.num_insns = total;
	u32 i, j;

	for (i = 0; i < total; i++) {
		for (j = i + 1; j < total; j++) {
			u8 has_dep = 0;
			u8 dep_type = 0;

			if (check_data_dependency(insns + i, insns + j)) {
				has_dep = 1;
				dep_type = 0;
			} else if (check_control_dependency(insns + i, insns + j)) {
				has_dep = 1;
				dep_type = 1;
			} else if (check_memory_dependency(insns + i, insns + j)) {
				has_dep = 1;
				dep_type = 2;
			}

			if (has_dep && sc->dep_graph.edge_count < 256) {
				sc->dep_graph.edges[sc->dep_graph.edge_count].from_idx = i;
				sc->dep_graph.edges[sc->dep_graph.edge_count].to_idx = j;
				sc->dep_graph.edges[sc->dep_graph.edge_count].dep_type = dep_type;
				sc->dep_graph.edge_count++;
			}
		}
	}
}

/* === INSTRUCTION REORDERING === */

/* Check if instruction i2 can be moved before i1 */
static inline u8 can_move_before(
	struct ScheduleCtx *sc, u32 i1, u32 i2)
{
	if (i2 >= i1) return 0;  /* i2 is after i1 */

	u32 i;
	for (i = 0; i < sc->dep_graph.edge_count; i++) {
		struct DepEdge *edge = &sc->dep_graph.edges[i];
		/* Check for dependency from i2 to i1 */
		if (edge->from_idx == i2 && edge->to_idx == i1) {
			return 0;  /* Dependency: cannot reorder */
		}
	}
	return 1;  /* No dependency: can move */
}

/* Reorder instructions respecting dependencies */
static inline void schedule_instructions(
	struct Insn *insns, u32 total,
	struct ScheduleCtx *sc)
{
	/* Simple pass: move independent instructions forward */
	u32 i = 0;
	while (i < total - 1) {
		if (can_move_before(sc, i, i + 1)) {
			/* Move i+1 before i */
			struct Insn temp = insns[i + 1];
			insns[i + 1] = insns[i];
			insns[i] = temp;
			sc->optimizations_applied++;
			/* Don't advance i: check position again for more moves */
		} else {
			i++;
		}
	}
}

/* === CONSTANT FOLDING === */

/* Evaluate binary operation at compile time */
static inline u64 fold_binop(
	u8 opcode, u64 left, u64 right)
{
	/* Assume opcodes: 0x03=ADD, 0x04=SUB, 0x05=AND, 0x06=OR, 0x07=XOR, etc. */
	switch (opcode) {
		case 0x03: return left + right;  /* ADD */
		case 0x04: return left - right;  /* SUB */
		case 0x05: return left & right;  /* AND */
		case 0x06: return left | right;  /* OR */
		case 0x07: return left ^ right;  /* XOR */
		case 0x08: return left * right;  /* MUL */
		case 0x09: return (right > 0) ? left / right : 0;  /* DIV */
		case 0x0A: return (right > 0) ? left % right : 0;  /* MOD */
		case 0x0B: return left << (right & 63);  /* SHL */
		default: return 0;
	}
}

/* Fold constant expressions in instruction stream */
static inline u32 fold_constants(
	struct Insn *insns, u32 total,
	struct ScheduleCtx *sc)
{
	u32 i = 0;
	u32 folded = 0;
	struct ConstValue const_map[32];  /* Track constant values in registers */
	u32 const_count = 0;

	while (i < total) {
		struct Insn *insn = &insns[i];

		/* Track MOVI (load immediate) as constant */
		if (insn->opcode == 0x01) {  /* OP_MOVI */
			u32 j;
			for (j = 0; j < const_count; j++) {
				if (const_map[j].rd == insn->rd) {
					const_map[j].value = insn->imm;
					break;
				}
			}
			if (j == const_count && const_count < 32) {
				const_map[const_count].rd = insn->rd;
				const_map[const_count].value = insn->imm;
				const_map[const_count].opcode = 0x01;
				const_count++;
			}
			i++;
			continue;
		}

		/* Try to fold binary operations with constant operands */
		u8 is_binop = (insn->opcode >= 0x03 && insn->opcode <= 0x0B);
		if (is_binop) {
			u64 left_val = 0, right_val = 0;
			u8 left_const = 0, right_const = 0;

			/* Check if rs1 is a constant */
			u32 j;
			for (j = 0; j < const_count; j++) {
				if (const_map[j].rd == insn->rs1) {
					left_val = const_map[j].value;
					left_const = 1;
					break;
				}
			}

			/* Check if rs2 is a constant */
			for (j = 0; j < const_count; j++) {
				if (const_map[j].rd == insn->rs2) {
					right_val = const_map[j].value;
					right_const = 1;
					break;
				}
			}

			if (left_const && right_const) {
				/* Both operands are constants: fold at compile time */
				u64 result = fold_binop(insn->opcode, left_val, right_val);

				/* Replace with MOVI rd, result */
				insn->opcode = 0x01;  /* OP_MOVI */
				insn->imm = result & 0xFFFFFFFF;
				insn->rs1 = 0;
				insn->rs2 = 0;

				/* Update constant map */
				for (j = 0; j < const_count; j++) {
					if (const_map[j].rd == insn->rd) {
						const_map[j].value = result;
						break;
					}
				}

				folded++;
				sc->optimizations_applied++;
			}
		}

		i++;
	}

	return folded;
}

#endif /* APKC_OPT_SCHEDULING_H */
