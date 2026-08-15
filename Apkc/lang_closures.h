/* lang_closures.h — Closure/Lambda Support (Stage 5.1)
 *
 * Closure parsing, capture context, and invocation.
 * Supports lambda syntax across multiple languages.
 * Max 3 nested closures, no recursive closure definitions.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_LANG_CLOSURES_H
#define APKC_LANG_CLOSURES_H 1

#include "lang_types.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Captured variable binding */
struct ClosureCapture {
	const u8 *var_name;
	u32 var_name_len;
	u8 capture_reg;        /* Register containing captured value */
	u8 capture_by_ref;     /* 1 = by reference, 0 = by value */
	u32 stack_offset;      /* Stack offset if spilled */
};

/* Closure definition (lambda/anonymous function) */
struct ClosureDef {
	const u8 *params[8];   /* Parameter names */
	u32 param_lens[8];
	u32 param_count;
	struct ClosureCapture captures[8];  /* Up to 8 captured variables */
	u32 capture_count;
	u32 code_start;        /* Instruction offset for closure body */
	u32 code_len;          /* Number of instructions in closure */
	enum ValueType return_type;
	u8 nesting_depth;      /* 0 = top-level, 1+ = nested */
	u8 recursive;          /* 1 if closure references itself (forbidden) */
};

/* Closure context for parsing and compilation */
struct ClosureCtx {
	struct ClosureDef closures[3];  /* Up to 3 nested closures */
	u32 closure_count;
	u32 current_closure;   /* Index of closure being compiled */
	struct ClosureDef *parent_closure;  /* Parent closure if nested */
	u32 nesting_depth;     /* Current nesting level */
};

/* Initialize closure context */
static inline void closure_ctx_init(struct ClosureCtx *cc) {
	cc->closure_count = 0;
	cc->current_closure = 0;
	cc->parent_closure = NULL;
	cc->nesting_depth = 0;
}

/* Check if nesting depth exceeds limit (max 3) */
static inline u8 closure_check_nesting_limit(struct ClosureCtx *cc) {
	if (cc->nesting_depth >= 3) return 1;  /* Exceeds limit */
	return 0;
}

/* Begin closure definition */
static inline u8 closure_define_begin(
	struct ClosureCtx *cc,
	enum ValueType return_type)
{
	if (cc->closure_count >= 3) return 1;  /* Too many closures */
	if (closure_check_nesting_limit(cc)) return 1;  /* Nesting too deep */

	struct ClosureDef *cd = &cc->closures[cc->closure_count];
	cd->param_count = 0;
	cd->capture_count = 0;
	cd->code_start = 0;
	cd->code_len = 0;
	cd->return_type = return_type;
	cd->nesting_depth = cc->nesting_depth;
	cd->recursive = 0;

	cc->parent_closure = (cc->closure_count > 0) ?
		&cc->closures[cc->closure_count - 1] : NULL;
	cc->current_closure = cc->closure_count;
	cc->closure_count++;
	cc->nesting_depth++;

	return 0;
}

/* End closure definition */
static inline void closure_define_end(
	struct ClosureCtx *cc,
	u32 code_pos,
	u32 end_pos)
{
	if (cc->current_closure < cc->closure_count) {
		struct ClosureDef *cd = &cc->closures[cc->current_closure];
		cd->code_start = code_pos;
		cd->code_len = end_pos - code_pos;
	}
	if (cc->nesting_depth > 0) cc->nesting_depth--;
}

/* Add parameter to closure */
static inline u8 closure_add_param(
	struct ClosureCtx *cc,
	const u8 *param_name, u32 param_len)
{
	struct ClosureDef *cd = &cc->closures[cc->current_closure];
	if (cd->param_count >= 8) return 1;  /* Too many parameters */

	cd->params[cd->param_count] = param_name;
	cd->param_lens[cd->param_count] = param_len;
	cd->param_count++;
	return 0;
}

/* Capture variable from outer scope */
static inline u8 closure_capture_var(
	struct ClosureCtx *cc,
	const u8 *var_name, u32 var_name_len,
	u8 capture_reg,
	u8 by_reference)
{
	struct ClosureDef *cd = &cc->closures[cc->current_closure];
	if (cd->capture_count >= 8) return 1;  /* Too many captures */

	struct ClosureCapture *cap = &cd->captures[cd->capture_count];
	cap->var_name = var_name;
	cap->var_name_len = var_name_len;
	cap->capture_reg = capture_reg;
	cap->capture_by_ref = by_reference;
	cap->stack_offset = 0;

	cd->capture_count++;
	return 0;
}

/* Lookup captured variable by name */
static inline struct ClosureCapture* closure_find_capture(
	struct ClosureDef *cd,
	const u8 *var_name, u32 var_name_len)
{
	u32 i;
	for (i = 0; i < cd->capture_count; i++) {
		struct ClosureCapture *cap = &cd->captures[i];
		if (cap->var_name_len != var_name_len) continue;

		u32 j;
		u8 match = 1;
		for (j = 0; j < var_name_len; j++) {
			if (cap->var_name[j] != var_name[j]) {
				match = 0;
				break;
			}
		}
		if (match) return cap;
	}
	return NULL;
}

/* Get closure by index */
static inline struct ClosureDef* closure_get(struct ClosureCtx *cc, u32 idx) {
	if (idx < cc->closure_count) return &cc->closures[idx];
	return NULL;
}

/* === CLOSURE PARSING === */

/* Lambda syntax patterns per language:
 * JavaScript: (x, y) => x + y
 * Rust:       |x, y| x + y
 * Python:     lambda x, y: x + y
 * Go:         func(x, y int) int { return x + y }
 * C:          ({int x, y; x + y;})  (compound literal / statement expr)
 * Java:       (x, y) -> x + y  (functional interface)
 * Swift:      { (x, y) in x + y }
 */

/* Parse lambda header (parameters) */
struct ClosureLambdaHeader {
	const u8 *params[8];
	u32 param_lens[8];
	u32 param_count;
	u8 capture_by_ref;     /* For Rust closures: || vs |&| */
};

/* Parse JavaScript lambda: (x, y) => ... */
static inline u8 closure_parse_js_lambda(
	const u8 *src, u32 src_len,
	u32 *pos,
	struct ClosureLambdaHeader *header)
{
	/* Expect '(' at *pos */
	if (*pos >= src_len || src[*pos] != '(') return 1;
	(*pos)++;

	header->param_count = 0;
	header->capture_by_ref = 0;

	/* Parse parameters until ')' */
	while (*pos < src_len && src[*pos] != ')') {
		if (src[*pos] == ' ' || src[*pos] == '\t') {
			(*pos)++;
			continue;
		}
		/* Collect parameter name */
		u32 param_start = *pos;
		while (*pos < src_len && (src[*pos] >= 'a' && src[*pos] <= 'z' ||
		       src[*pos] >= 'A' && src[*pos] <= 'Z' ||
		       src[*pos] >= '0' && src[*pos] <= '9' || src[*pos] == '_')) {
			(*pos)++;
		}
		if (*pos > param_start) {
			header->params[header->param_count] = &src[param_start];
			header->param_lens[header->param_count] = *pos - param_start;
			header->param_count++;
		}
		if (header->param_count >= 8) return 1;

		/* Skip whitespace and comma */
		while (*pos < src_len && (src[*pos] == ' ' || src[*pos] == ',' || src[*pos] == '\t')) {
			(*pos)++;
		}
	}

	/* Expect ')' followed by '=>' */
	if (*pos >= src_len || src[*pos] != ')') return 1;
	(*pos)++;
	while (*pos < src_len && (src[*pos] == ' ' || src[*pos] == '\t')) (*pos)++;
	if (*pos + 1 >= src_len || src[*pos] != '=' || src[*pos + 1] != '>') return 1;
	(*pos) += 2;

	return 0;
}

/* Parse Rust lambda: |x, y| ... or |&x, &y| ... */
static inline u8 closure_parse_rust_lambda(
	const u8 *src, u32 src_len,
	u32 *pos,
	struct ClosureLambdaHeader *header)
{
	/* Expect '|' at *pos */
	if (*pos >= src_len || src[*pos] != '|') return 1;
	(*pos)++;

	header->param_count = 0;
	header->capture_by_ref = 0;

	/* Parse parameters until '|' */
	while (*pos < src_len && src[*pos] != '|') {
		if (src[*pos] == ' ' || src[*pos] == '\t') {
			(*pos)++;
			continue;
		}
		/* Check for & (reference) */
		if (src[*pos] == '&') {
			header->capture_by_ref = 1;
			(*pos)++;
		}
		/* Collect parameter name */
		u32 param_start = *pos;
		while (*pos < src_len && (src[*pos] >= 'a' && src[*pos] <= 'z' ||
		       src[*pos] >= 'A' && src[*pos] <= 'Z' ||
		       src[*pos] >= '0' && src[*pos] <= '9' || src[*pos] == '_')) {
			(*pos)++;
		}
		if (*pos > param_start) {
			header->params[header->param_count] = &src[param_start];
			header->param_lens[header->param_count] = *pos - param_start;
			header->param_count++;
		}
		if (header->param_count >= 8) return 1;

		/* Skip whitespace and comma */
		while (*pos < src_len && (src[*pos] == ' ' || src[*pos] == ',' || src[*pos] == '\t')) {
			(*pos)++;
		}
	}

	/* Expect closing '|' */
	if (*pos >= src_len || src[*pos] != '|') return 1;
	(*pos)++;

	return 0;
}

/* Parse Python lambda: lambda x, y: ... */
static inline u8 closure_parse_python_lambda(
	const u8 *src, u32 src_len,
	u32 *pos,
	struct ClosureLambdaHeader *header)
{
	/* Expect 'lambda' keyword */
	if (*pos + 6 >= src_len) return 1;
	u32 i;
	for (i = 0; i < 6; i++) {
		if (src[*pos + i] != "lambda"[i]) return 1;
	}
	*pos += 6;

	header->param_count = 0;
	header->capture_by_ref = 0;

	/* Skip whitespace */
	while (*pos < src_len && (src[*pos] == ' ' || src[*pos] == '\t')) (*pos)++;

	/* Parse parameters until ':' */
	while (*pos < src_len && src[*pos] != ':') {
		if (src[*pos] == ' ' || src[*pos] == '\t') {
			(*pos)++;
			continue;
		}
		/* Collect parameter name */
		u32 param_start = *pos;
		while (*pos < src_len && (src[*pos] >= 'a' && src[*pos] <= 'z' ||
		       src[*pos] >= 'A' && src[*pos] <= 'Z' ||
		       src[*pos] >= '0' && src[*pos] <= '9' || src[*pos] == '_')) {
			(*pos)++;
		}
		if (*pos > param_start) {
			header->params[header->param_count] = &src[param_start];
			header->param_lens[header->param_count] = *pos - param_start;
			header->param_count++;
		}
		if (header->param_count >= 8) return 1;

		/* Skip whitespace and comma */
		while (*pos < src_len && (src[*pos] == ' ' || src[*pos] == ',' || src[*pos] == '\t')) {
			(*pos)++;
		}
	}

	/* Expect ':' */
	if (*pos >= src_len || src[*pos] != ':') return 1;
	(*pos)++;

	return 0;
}

/* === CODE GENERATION FOR CLOSURES === */

/* Emit closure prologue (set up captured variables) */
static inline void closure_emit_prologue(
	struct CodeGen *cg,
	struct ClosureDef *cd)
{
	u32 i;
	/* Move captured variables to closure environment */
	for (i = 0; i < cd->capture_count; i++) {
		struct ClosureCapture *cap = &cd->captures[i];
		/* Emit MOV to preserve captured value in register or stack */
		if (cap->capture_by_ref) {
			/* By reference: store address */
			/* codegen_emit_mov(cg, cap->capture_reg, ...); */
		} else {
			/* By value: store value */
			/* Value already in capture_reg */
		}
	}
}

/* Emit closure invocation */
static inline void closure_emit_call(
	struct CodeGen *cg,
	u8 closure_reg,      /* Register containing closure pointer */
	const u8 *args,      /* Argument register indices */
	u32 arg_count)
{
	/* Move arguments to r0-r2 (first 3) or stack (4+) */
	u32 i;
	for (i = 0; i < arg_count && i < 3; i++) {
		if (args[i] != (u8)i) {
			/* codegen_emit_mov(cg, (u8)i, args[i]); */
		}
	}
	/* Emit indirect call through closure_reg */
	/* Closure pointer contains function address */
	/* codegen_emit_call_indirect(cg, closure_reg); */
}

/* Emit closure object creation (returns closure as value in r0) */
static inline void closure_emit_create(
	struct CodeGen *cg,
	struct ClosureDef *cd,
	u32 code_addr)
{
	/* Create closure object (function pointer + captured env) */
	/* For freestanding model: closure = {code_addr, captures[]} */
	/* Return as value in r0 */
	/* codegen_emit_movi(cg, 0, code_addr); */
}

#endif /* APKC_LANG_CLOSURES_H */
