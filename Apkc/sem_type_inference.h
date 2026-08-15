/* sem_type_inference.h — Type Inference Engine (Phase 21.3)
 *
 * Type inference: infer types from expressions without annotations
 * Fresh type variables: generate unique type variables
 * Constraint collection: gather equations from code structure
 * W algorithm: bidirectional type checking
 * Polymorphism: handle generic functions and data types
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEM_TYPE_INFERENCE_H
#define APKC_SEM_TYPE_INFERENCE_H 1

#include "sem_type_system.h"
#include "sem_unification.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed long long s64;
typedef signed int s32;

/* ============================================================ */
/* TYPE INFERENCE CONTEXT */
/* ============================================================ */

struct InferenceContext {
	struct ConstraintSystem constraints;  /* Collected constraints */
	struct SubstitutionMap subst;         /* Current substitutions */
	u32 next_var_id;                      /* Next type variable ID */
	struct Type inferred_types[128];      /* Cache of inferred types */
	u32 inferred_count;
};

/* Expression node: simplified AST representation */
struct ExprNode {
	u8 kind;  /* Expression kind */
	union {
		s64 int_val;
		const char *str_val;
		const char *var_name;
		struct {
			struct ExprNode *func;
			struct ExprNode *arg;
		} app;
		struct {
			const char *var_name;
			struct ExprNode *body;
		} lambda;
		struct {
			const char *op;  /* "+" "-" "*" "/" "==" "<" etc. */
			struct ExprNode *left;
			struct ExprNode *right;
		} binop;
		struct {
			struct ExprNode *cond;
			struct ExprNode *then_expr;
			struct ExprNode *else_expr;
		} if_expr;
	} data;
};

/* Expression kinds */
enum ExprKind {
	EXPR_INT = 0,
	EXPR_FLOAT = 1,
	EXPR_BOOL = 2,
	EXPR_STR = 3,
	EXPR_VAR = 4,
	EXPR_APP = 5,
	EXPR_LAMBDA = 6,
	EXPR_BINOP = 7,
	EXPR_IF = 8,
	EXPR_UNIT = 9
};

/* ============================================================ */
/* INFERENCE CONTEXT MANAGEMENT */
/* ============================================================ */

static inline void inference_init(struct InferenceContext *ctx) {
	if (!ctx) return;
	constraints_init(&ctx->constraints);
	subst_map_init(&ctx->subst);
	ctx->next_var_id = 0;
	ctx->inferred_count = 0;
}

static inline struct Type fresh_var(struct InferenceContext *ctx) {
	if (!ctx) return type_unknown_safe();
	u32 id = ctx->next_var_id++;
	return type_var(id, 0);
}

/* ============================================================ */
/* LITERAL TYPE INFERENCE */
/* ============================================================ */

static inline struct Type infer_int_literal(s64 val) {
	/* Small integers: assume i32; large: i64 */
	if (val >= -2147483648LL && val <= 2147483647LL) {
		return type_int32();
	}
	return type_int64();
}

static inline struct Type infer_float_literal(void) {
	return type_float();
}

static inline struct Type infer_bool_literal(void) {
	return type_bool();
}

static inline struct Type infer_str_literal(void) {
	return type_str();
}

/* ============================================================ */
/* CONSTRAINT-BASED INFERENCE (W Algorithm) */
/* ============================================================ */

static inline struct Type infer_expr(
	struct ExprNode *expr,
	struct InferenceContext *ctx,
	struct TypeError *error);

/* Infer type of application: (T1 → T2) applied to T3 yields T2 */
static inline struct Type infer_app(
	struct ExprNode *func_expr,
	struct ExprNode *arg_expr,
	struct InferenceContext *ctx,
	struct TypeError *error) {

	if (!func_expr || !arg_expr || !ctx) return type_unknown_safe();

	/* Infer type of function */
	struct Type func_type = infer_expr(func_expr, ctx, error);

	/* Infer type of argument */
	struct Type arg_type = infer_expr(arg_expr, ctx, error);

	/* Generate fresh return type variable */
	struct Type ret_var = fresh_var(ctx);

	/* Function type should be: arg_type → ret_var */
	struct Type expected_func = type_func(&arg_type, &ret_var);

	/* Unify function type with expected type */
	if (!unify_types(&func_type, &expected_func, &ctx->subst, error)) {
		if (!error || !error->msg) {
			if (error) {
				error->code = TE_MISMATCH;
				error->msg = "Function type mismatch in application";
			}
		}
		return type_unknown_safe();
	}

	/* Return type after substitution */
	return apply_subst_deep(&ret_var, &ctx->subst);
}

/* Infer type of lambda: λx.body has type param_type → body_type */
static inline struct Type infer_lambda(
	const char *var_name,
	struct ExprNode *body,
	struct InferenceContext *ctx,
	struct TypeError *error) {

	if (!var_name || !body || !ctx) return type_unknown_safe();

	/* Generate fresh type for parameter */
	struct Type param_type = fresh_var(ctx);

	/* Infer type of body (would need environment in full implementation) */
	struct Type body_type = infer_expr(body, ctx, error);

	/* Return function type */
	return type_func(&param_type, &body_type);
}

/* Infer type of binary operator */
static inline struct Type infer_binop(
	const char *op,
	struct ExprNode *left,
	struct ExprNode *right,
	struct InferenceContext *ctx,
	struct TypeError *error) {

	if (!op || !left || !right || !ctx) return type_unknown_safe();

	struct Type left_type = infer_expr(left, ctx, error);
	struct Type right_type = infer_expr(right, ctx, error);

	/* Arithmetic operators: +, -, *, / */
	if (op[0] == '+' || op[0] == '-' || op[0] == '*' || op[0] == '/') {
		/* Both sides must be numeric */
		if (!type_is_numeric(&left_type) || !type_is_numeric(&right_type)) {
			if (error) {
				error->code = TE_MISMATCH;
				error->msg = "Arithmetic on non-numeric type";
			}
			return type_unknown_safe();
		}
		/* Result type: unified type of both operands */
		if (!unify_types(&left_type, &right_type, &ctx->subst, error)) {
			return type_unknown_safe();
		}
		return apply_subst_deep(&left_type, &ctx->subst);
	}

	/* Comparison operators: ==, <, >, <=, >= */
	if (op[0] == '=' || op[0] == '<' || op[0] == '>') {
		/* Both operands must have same type */
		if (!unify_types(&left_type, &right_type, &ctx->subst, error)) {
			if (error) error->msg = "Comparison operands have different types";
			return type_unknown_safe();
		}
		/* Result is bool */
		return type_bool();
	}

	/* Logical operators: &&, || */
	if ((op[0] == '&' && op[1] == '&') || (op[0] == '|' && op[1] == '|')) {
		/* Both operands must be bool */
		struct Type bool_ty = type_bool();
		if (!unify_types(&left_type, &bool_ty, &ctx->subst, error)) {
			if (error) error->msg = "Logical operator requires bool";
			return type_unknown_safe();
		}
		if (!unify_types(&right_type, &bool_ty, &ctx->subst, error)) {
			return type_unknown_safe();
		}
		return bool_ty;
	}

	return type_unknown_safe();
}

/* Infer type of if-expression */
static inline struct Type infer_if(
	struct ExprNode *cond,
	struct ExprNode *then_expr,
	struct ExprNode *else_expr,
	struct InferenceContext *ctx,
	struct TypeError *error) {

	if (!cond || !then_expr || !else_expr || !ctx) return type_unknown_safe();

	/* Condition must be bool */
	struct Type cond_type = infer_expr(cond, ctx, error);
	struct Type bool_ty = type_bool();
	if (!unify_types(&cond_type, &bool_ty, &ctx->subst, error)) {
		if (error) error->msg = "If condition must be bool";
		return type_unknown_safe();
	}

	/* Both branches must have same type */
	struct Type then_type = infer_expr(then_expr, ctx, error);
	struct Type else_type = infer_expr(else_expr, ctx, error);

	if (!unify_types(&then_type, &else_type, &ctx->subst, error)) {
		if (error) error->msg = "If branches have different types";
		return type_unknown_safe();
	}

	return apply_subst_deep(&then_type, &ctx->subst);
}

/* ============================================================ */
/* MAIN INFERENCE FUNCTION */
/* ============================================================ */

static inline struct Type infer_expr(
	struct ExprNode *expr,
	struct InferenceContext *ctx,
	struct TypeError *error) {

	if (!expr || !ctx) return type_unknown_safe();

	switch (expr->kind) {
	case EXPR_INT:
		return infer_int_literal(expr->data.int_val);

	case EXPR_FLOAT:
		return infer_float_literal();

	case EXPR_BOOL:
		return infer_bool_literal();

	case EXPR_STR:
		return infer_str_literal();

	case EXPR_UNIT:
		return type_unit();

	case EXPR_VAR:
		/* In full implementation: look up in type environment */
		return fresh_var(ctx);

	case EXPR_APP:
		return infer_app(expr->data.app.func, expr->data.app.arg,
		                ctx, error);

	case EXPR_LAMBDA:
		return infer_lambda(expr->data.lambda.var_name,
		                   expr->data.lambda.body, ctx, error);

	case EXPR_BINOP:
		return infer_binop(expr->data.binop.op,
		                  expr->data.binop.left,
		                  expr->data.binop.right, ctx, error);

	case EXPR_IF:
		return infer_if(expr->data.if_expr.cond,
		               expr->data.if_expr.then_expr,
		               expr->data.if_expr.else_expr, ctx, error);

	default:
		if (error) {
			error->code = TE_UNRESOLVED;
			error->msg = "Unknown expression kind";
		}
		return type_unknown_safe();
	}
}

/* ============================================================ */
/* FULL INFERENCE PIPELINE */
/* ============================================================ */

/* Infer type and solve all constraints */
static inline struct Type infer_with_solving(
	struct ExprNode *expr,
	struct InferenceContext *ctx,
	struct UnificationState *state,
	struct TypeError *error) {

	if (!expr || !ctx || !state) return type_unknown_safe();

	/* Infer expression type (collects constraints) */
	struct Type ty = infer_expr(expr, ctx, error);

	/* Copy constraints to state */
	state->remaining = ctx->constraints;

	/* Solve all constraints */
	if (!solve_constraints(&ctx->constraints, state)) {
		if (error && !error->msg) {
			error->code = state->error.code;
			error->msg = state->error.msg;
		}
		return type_unknown_safe();
	}

	/* Apply final substitution to result */
	return apply_subst_deep(&ty, &state->subst);
}

/* ============================================================ */
/* TYPE GENERALIZATION (POLYMORPHISM) */
/* ============================================================ */

/* Generalize type by creating a forall: free vars become quantified */
struct TypeScheme {
	struct Type *bound_vars[8];  /* Bound type variables */
	u32 bound_count;
	struct Type type;
};

static inline struct TypeScheme generalize_type(
	struct Type *ty,
	struct SubstitutionMap *subst) {

	struct TypeScheme scheme;
	scheme.bound_count = 0;
	scheme.type = *ty;
	/* In full implementation: collect all free variables in ty */
	return scheme;
}

/* Instantiate a type scheme by replacing bound vars with fresh vars */
static inline struct Type instantiate_scheme(
	struct TypeScheme *scheme,
	struct InferenceContext *ctx) {

	if (!scheme || !ctx) return type_unknown_safe();
	/* In full implementation: replace each bound var with fresh_var() */
	return scheme->type;
}

#endif /* APKC_SEM_TYPE_INFERENCE_H */
