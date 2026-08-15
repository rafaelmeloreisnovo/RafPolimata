/* opt_semantic_fold.h — Semantic Optimization (Phase 25)
 *
 * Constant folding: compile-time evaluation
 * Dead code elimination: remove unreachable code
 * Copy propagation: replace copies with originals
 * Strength reduction: replace expensive with cheaper ops
 * Common subexpression elimination: CSE
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_SEMANTIC_FOLD_H
#define APKC_OPT_SEMANTIC_FOLD_H 1

#include "sem_dataflow.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed long long s64;

/* ============================================================ */
/* CONSTANT VALUE REPRESENTATION */
/* ============================================================ */

enum ConstantKind {
	CONST_INT = 0,
	CONST_FLOAT = 1,
	CONST_BOOL = 2,
	CONST_STR = 3
};

struct ConstantValue {
	u8 kind;
	union {
		s64 int_val;
		const char *str_val;
		u8 bool_val;
	} data;
};

/* ============================================================ */
/* CONSTANT FOLDING */
/* ============================================================ */

static inline struct ConstantValue fold_add(
	struct ConstantValue left,
	struct ConstantValue right) {

	struct ConstantValue result;
	result.kind = CONST_INT;
	if (left.kind == CONST_INT && right.kind == CONST_INT) {
		result.data.int_val = left.data.int_val + right.data.int_val;
	} else {
		result.kind = CONST_INT;
		result.data.int_val = 0;
	}
	return result;
}

static inline struct ConstantValue fold_sub(
	struct ConstantValue left,
	struct ConstantValue right) {

	struct ConstantValue result;
	result.kind = CONST_INT;
	if (left.kind == CONST_INT && right.kind == CONST_INT) {
		result.data.int_val = left.data.int_val - right.data.int_val;
	} else {
		result.data.int_val = 0;
	}
	return result;
}

static inline struct ConstantValue fold_mul(
	struct ConstantValue left,
	struct ConstantValue right) {

	struct ConstantValue result;
	result.kind = CONST_INT;
	if (left.kind == CONST_INT && right.kind == CONST_INT) {
		result.data.int_val = left.data.int_val * right.data.int_val;
	} else {
		result.data.int_val = 0;
	}
	return result;
}

static inline struct ConstantValue fold_div(
	struct ConstantValue left,
	struct ConstantValue right) {

	struct ConstantValue result;
	result.kind = CONST_INT;
	if (left.kind == CONST_INT && right.kind == CONST_INT &&
	    right.data.int_val != 0) {
		result.data.int_val = left.data.int_val / right.data.int_val;
	} else {
		result.data.int_val = 0;
	}
	return result;
}

static inline struct ConstantValue fold_mod(
	struct ConstantValue left,
	struct ConstantValue right) {

	struct ConstantValue result;
	result.kind = CONST_INT;
	if (left.kind == CONST_INT && right.kind == CONST_INT &&
	    right.data.int_val != 0) {
		result.data.int_val = left.data.int_val % right.data.int_val;
	} else {
		result.data.int_val = 0;
	}
	return result;
}

static inline struct ConstantValue fold_and(
	struct ConstantValue left,
	struct ConstantValue right) {

	struct ConstantValue result;
	result.kind = CONST_INT;
	if (left.kind == CONST_INT && right.kind == CONST_INT) {
		result.data.int_val = left.data.int_val & right.data.int_val;
	} else {
		result.data.int_val = 0;
	}
	return result;
}

static inline struct ConstantValue fold_or(
	struct ConstantValue left,
	struct ConstantValue right) {

	struct ConstantValue result;
	result.kind = CONST_INT;
	if (left.kind == CONST_INT && right.kind == CONST_INT) {
		result.data.int_val = left.data.int_val | right.data.int_val;
	} else {
		result.data.int_val = 0;
	}
	return result;
}

static inline struct ConstantValue fold_xor(
	struct ConstantValue left,
	struct ConstantValue right) {

	struct ConstantValue result;
	result.kind = CONST_INT;
	if (left.kind == CONST_INT && right.kind == CONST_INT) {
		result.data.int_val = left.data.int_val ^ right.data.int_val;
	} else {
		result.data.int_val = 0;
	}
	return result;
}

/* ============================================================ */
/* STRENGTH REDUCTION */
/* ============================================================ */

static inline struct ConstantValue strength_reduce_mul(
	struct ConstantValue left,
	struct ConstantValue right) {

	/* Replace multiplication by power of 2 with shift */
	if (left.kind == CONST_INT && right.kind == CONST_INT) {
		s64 multiplier = right.data.int_val;
		/* Check if multiplier is power of 2 */
		if (multiplier > 0 && (multiplier & (multiplier - 1)) == 0) {
			/* Count trailing zeros = shift amount */
			u32 shift = 0;
			s64 temp = multiplier;
			while ((temp & 1) == 0) {
				shift++;
				temp >>= 1;
			}
			/* Equivalent to multiply by multiplier, but cheaper */
			struct ConstantValue result;
			result.kind = CONST_INT;
			result.data.int_val = left.data.int_val << shift;
			return result;
		}
	}

	return fold_mul(left, right);
}

static inline struct ConstantValue strength_reduce_div(
	struct ConstantValue left,
	struct ConstantValue right) {

	/* Replace division by power of 2 with shift */
	if (left.kind == CONST_INT && right.kind == CONST_INT) {
		s64 divisor = right.data.int_val;
		if (divisor > 0 && (divisor & (divisor - 1)) == 0) {
			u32 shift = 0;
			s64 temp = divisor;
			while ((temp & 1) == 0) {
				shift++;
				temp >>= 1;
			}
			struct ConstantValue result;
			result.kind = CONST_INT;
			result.data.int_val = left.data.int_val >> shift;
			return result;
		}
	}

	return fold_div(left, right);
}

/* ============================================================ */
/* DEAD CODE ELIMINATION */
/* ============================================================ */

struct DeadCodeInfo {
	u32 instruction_count;
	u32 dead_instruction_count;
	u32 dead_block_count;
};

static inline struct DeadCodeInfo analyze_dead_code(
	struct DataFlowAnalyzer *analyzer) {

	struct DeadCodeInfo info;
	info.instruction_count = 0;
	info.dead_instruction_count = 0;
	info.dead_block_count = 0;

	if (!analyzer || !analyzer->cfg) return info;

	/* Count unreachable blocks */
	u32 i;
	for (i = 0; i < analyzer->cfg->node_count; i++) {
		if (!analyzer->cfg->nodes[i].is_reachable) {
			info.dead_block_count++;
			info.dead_instruction_count +=
				analyzer->cfg->nodes[i].instruction_count;
		}
		info.instruction_count += analyzer->cfg->nodes[i].instruction_count;
	}

	return info;
}

/* ============================================================ */
/* COPY PROPAGATION */
/* ============================================================ */

struct CopyAssignment {
	u32 dest_var;
	u32 src_var;
	u32 assignment_line;
};

static inline struct CopyAssignment track_copy_assignment(
	u32 var_id,
	u32 source_var) {

	struct CopyAssignment copy;
	copy.dest_var = var_id;
	copy.src_var = source_var;
	copy.assignment_line = 0;
	return copy;
}

static inline u32 resolve_copy_propagation(
	u32 var_id,
	struct CopyAssignment *assignments,
	u32 assignment_count) {

	if (!assignments) return var_id;

	u32 i;
	for (i = 0; i < assignment_count; i++) {
		if (assignments[i].dest_var == var_id) {
			return assignments[i].src_var;
		}
	}
	return var_id;
}

/* ============================================================ */
/* COMMON SUBEXPRESSION ELIMINATION */
/* ============================================================ */

struct Expression {
	u8 op;
	u32 left_var;
	u32 right_var;
	u32 result_var;
	u32 line;
};

struct CSEEntry {
	struct Expression expr;
	u32 first_occurrence_line;
	u32 elimination_count;
};

static inline u8 expressions_equal(
	struct Expression *e1,
	struct Expression *e2) {

	if (!e1 || !e2) return 0;
	return e1->op == e2->op &&
	       e1->left_var == e2->left_var &&
	       e1->right_var == e2->right_var;
}

/* ============================================================ */
/* OPTIMIZATION STATISTICS */
/* ============================================================ */

struct OptimizationStats {
	u32 constants_folded;
	u32 dead_stores_eliminated;
	u32 dead_blocks_eliminated;
	u32 strength_reductions;
	u32 copies_propagated;
	u32 cse_eliminations;
	u32 total_instructions_eliminated;
};

#endif /* APKC_OPT_SEMANTIC_FOLD_H */
