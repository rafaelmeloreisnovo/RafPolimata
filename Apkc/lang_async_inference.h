/* lang_async_inference.h — Async/Await & Type Inference (Stage 5.3)
 *
 * Async/await syntax parsing, type inference from expressions.
 * Simplified execution: await treated as sequential call.
 * Type inference via expression analysis and return statement tracking.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_LANG_ASYNC_INFERENCE_H
#define APKC_LANG_ASYNC_INFERENCE_H 1

#include "lang_types.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Type inference state for variable */
struct TypeInference {
	const u8 *var_name;
	u32 var_name_len;
	enum ValueType inferred_type;  /* TYPE_ANY initially, narrowed via usage */
	u8 type_locked;                /* 1 = type is final, 0 = still inferring */
	u32 last_usage_pos;            /* Position of last usage for tracking */
};

/* Async function state */
struct AsyncFunc {
	const u8 *name;
	u32 name_len;
	enum ValueType inferred_return_type;
	u32 code_start;
	u32 code_len;
	u8 has_await;                  /* 1 if function contains await */
	u8 recursive;                  /* 1 if async function calls itself */
};

/* Type inference context */
struct InferenceCtx {
	struct TypeInference vars[16];  /* Up to 16 variables being tracked */
	u32 var_count;
	struct AsyncFunc async_funcs[4];  /* Up to 4 async functions */
	u32 async_count;
	enum ValueType current_func_return_type;  /* Inferred return type */
	u8 function_type_locked;       /* 1 = return type finalized */
};

/* Initialize type inference context */
static inline void inference_ctx_init(struct InferenceCtx *ic) {
	ic->var_count = 0;
	ic->async_count = 0;
	ic->current_func_return_type = TYPE_VOID;
	ic->function_type_locked = 0;
}

/* Infer type from binary operation */
static inline enum ValueType inference_type_from_binop(
	enum ValueType left_type,
	enum ValueType right_type,
	u8 operator)
{
	/* Arithmetic operations: INT op INT = INT */
	if (operator == '+' || operator == '-' || operator == '*' ||
	    operator == '/' || operator == '%') {
		if (left_type == TYPE_INT && right_type == TYPE_INT) return TYPE_INT;
		if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) return TYPE_FLOAT;
	}
	/* Bitwise operations: INT op INT = INT */
	if (operator == '&' || operator == '|' || operator == '^') {
		if (left_type == TYPE_INT && right_type == TYPE_INT) return TYPE_INT;
	}
	/* Shift operations: INT shift INT = INT */
	if (operator == 's') {  /* shift indicator */
		if (left_type == TYPE_INT && right_type == TYPE_INT) return TYPE_INT;
	}
	/* Comparison: any op any = BOOL */
	if (operator == '<' || operator == '>' || operator == '=' ||
	    operator == '!' || operator == ':') {  /* :=, == handled as symbolic */
		return TYPE_BOOL;
	}
	/* Default: propagate left type */
	return left_type;
}

/* Infer type from literal value */
static inline enum ValueType inference_type_from_literal(
	const u8 *lit, u32 lit_len)
{
	if (lit_len == 0) return TYPE_VOID;

	/* Check for numeric literal: "123", "0x1A", etc. */
	u32 i = 0;
	if (lit[0] == '0' && lit_len > 1 && lit[1] == 'x') {
		/* Hexadecimal: assume INT */
		return TYPE_INT;
	}
	/* Decimal: check all digits */
	u8 is_number = 1;
	for (i = 0; i < lit_len; i++) {
		if (!(lit[i] >= '0' && lit[i] <= '9')) {
			is_number = 0;
			break;
		}
	}
	if (is_number) return TYPE_INT;

	/* String literal: "text" or 'c' */
	if ((lit[0] == '"' && lit[lit_len - 1] == '"') ||
	    (lit[0] == '\'' && lit[lit_len - 1] == '\'')) {
		return TYPE_PTR;  /* String as pointer to char array */
	}

	/* Boolean: true/false */
	if (lit_len == 4 && lit[0] == 't' && lit[1] == 'r' &&
	    lit[2] == 'u' && lit[3] == 'e') return TYPE_BOOL;
	if (lit_len == 5 && lit[0] == 'f' && lit[1] == 'a' &&
	    lit[2] == 'l' && lit[3] == 's' && lit[4] == 'e') return TYPE_BOOL;

	return TYPE_ANY;
}

/* Track variable type from assignment */
static inline u8 inference_track_var(
	struct InferenceCtx *ic,
	const u8 *var_name, u32 var_name_len,
	enum ValueType assigned_type)
{
	u32 i;
	/* Check if variable already tracked */
	for (i = 0; i < ic->var_count; i++) {
		if (ic->vars[i].var_name_len != var_name_len) continue;
		u32 j;
		u8 match = 1;
		for (j = 0; j < var_name_len; j++) {
			if (ic->vars[i].var_name[j] != var_name[j]) {
				match = 0;
				break;
			}
		}
		if (match) {
			/* Variable exists: narrow type */
			if (ic->vars[i].inferred_type == TYPE_ANY) {
				ic->vars[i].inferred_type = assigned_type;
			} else if (ic->vars[i].inferred_type != assigned_type) {
				ic->vars[i].type_locked = 1;  /* Type conflict: lock */
				return 1;  /* Type mismatch */
			}
			return 0;
		}
	}
	/* New variable: add to tracking */
	if (ic->var_count >= 16) return 1;  /* Too many variables */
	ic->vars[ic->var_count].var_name = var_name;
	ic->vars[ic->var_count].var_name_len = var_name_len;
	ic->vars[ic->var_count].inferred_type = assigned_type;
	ic->vars[ic->var_count].type_locked = 0;
	ic->var_count++;
	return 0;
}

/* Get inferred type of variable */
static inline enum ValueType inference_lookup_var(
	struct InferenceCtx *ic,
	const u8 *var_name, u32 var_name_len)
{
	u32 i;
	for (i = 0; i < ic->var_count; i++) {
		if (ic->vars[i].var_name_len != var_name_len) continue;
		u32 j;
		u8 match = 1;
		for (j = 0; j < var_name_len; j++) {
			if (ic->vars[i].var_name[j] != var_name[j]) {
				match = 0;
				break;
			}
		}
		if (match) return ic->vars[i].inferred_type;
	}
	return TYPE_ANY;  /* Unknown variable */
}

/* === ASYNC/AWAIT PARSING === */

/* Parse await expression: await fn(...) */
static inline u8 inference_parse_await(
	const u8 *src, u32 src_len,
	u32 *pos,
	const u8 **func_name, u32 *func_len)
{
	/* Expect 'await' keyword */
	if (*pos + 5 >= src_len) return 1;
	u32 i;
	for (i = 0; i < 5; i++) {
		if (src[*pos + i] != "await"[i]) return 1;
	}
	*pos += 5;

	/* Skip whitespace */
	while (*pos < src_len && (src[*pos] == ' ' || src[*pos] == '\t')) (*pos)++;

	/* Collect function name */
	u32 name_start = *pos;
	while (*pos < src_len && (src[*pos] >= 'a' && src[*pos] <= 'z' ||
	       src[*pos] >= 'A' && src[*pos] <= 'Z' ||
	       src[*pos] >= '0' && src[*pos] <= '9' || src[*pos] == '_')) {
		(*pos)++;
	}
	if (*pos > name_start) {
		*func_name = &src[name_start];
		*func_len = *pos - name_start;
	}

	return 0;
}

/* Register async function */
static inline u8 inference_register_async(
	struct InferenceCtx *ic,
	const u8 *name, u32 name_len,
	u32 code_start)
{
	if (ic->async_count >= 4) return 1;  /* Max 4 async functions */

	struct AsyncFunc *af = &ic->async_funcs[ic->async_count];
	af->name = name;
	af->name_len = name_len;
	af->inferred_return_type = TYPE_ANY;
	af->code_start = code_start;
	af->code_len = 0;
	af->has_await = 0;
	af->recursive = 0;

	ic->async_count++;
	return 0;
}

/* Mark async function as containing await */
static inline void inference_mark_await_used(
	struct InferenceCtx *ic)
{
	if (ic->async_count > 0) {
		ic->async_funcs[ic->async_count - 1].has_await = 1;
	}
}

/* === TYPE NARROWING FROM RETURN STATEMENTS === */

/* Infer function return type from return statement */
static inline u8 inference_infer_return_type(
	struct InferenceCtx *ic,
	enum ValueType return_expr_type)
{
	if (ic->function_type_locked) {
		/* Return type already finalized */
		if (ic->current_func_return_type != return_expr_type) {
			/* Type mismatch on return */
			return 1;
		}
	} else {
		/* First return: set type */
		if (ic->current_func_return_type == TYPE_VOID) {
			ic->current_func_return_type = return_expr_type;
		} else if (ic->current_func_return_type != return_expr_type) {
			/* Multiple return statements with different types */
			/* Use least specific common type (INT | FLOAT = FLOAT) */
			if ((ic->current_func_return_type == TYPE_INT && return_expr_type == TYPE_FLOAT) ||
			    (ic->current_func_return_type == TYPE_FLOAT && return_expr_type == TYPE_INT)) {
				ic->current_func_return_type = TYPE_FLOAT;
			}
		}
	}
	return 0;
}

/* Finalize function return type */
static inline enum ValueType inference_finalize_return_type(
	struct InferenceCtx *ic)
{
	ic->function_type_locked = 1;
	return ic->current_func_return_type;
}

/* === TYPE MISMATCH DETECTION === */

/* Detect and report type mismatch */
static inline u8 inference_check_type_compat(
	enum ValueType expected,
	enum ValueType actual)
{
	if (expected == TYPE_ANY || actual == TYPE_ANY) return 0;  /* Any matches anything */
	if (expected == actual) return 0;  /* Exact match */
	if (expected == TYPE_INT && actual == TYPE_BOOL) return 0;  /* Bool->Int implicit */
	if (expected == TYPE_FLOAT && actual == TYPE_INT) return 0;  /* Int->Float implicit */
	return 1;  /* Type mismatch */
}

#endif /* APKC_LANG_ASYNC_INFERENCE_H */
