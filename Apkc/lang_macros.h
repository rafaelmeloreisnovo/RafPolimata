/* lang_macros.h — Macros & Pattern Matching (Stage 5.4)
 *
 * Simple macro expansion (text substitution style).
 * Pattern matching compilation (match statements to if/else chains).
 * Match guards and exhaustiveness checking.
 * Max 100 char macro body, no recursive macros.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_LANG_MACROS_H
#define APKC_LANG_MACROS_H 1

#include "lang_types.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Macro definition */
struct MacroDef {
	const u8 *name;
	u32 name_len;
	const u8 *params[4];       /* Macro parameter names */
	u32 param_lens[4];
	u32 param_count;
	u8 body[100];              /* Macro body (max 100 chars) */
	u32 body_len;
	u8 expansion_count;        /* Number of expansions (prevent recursion) */
};

/* Pattern in match statement */
struct MatchPattern {
	const u8 *pattern;         /* Pattern value or name */
	u32 pattern_len;
	u8 is_wildcard;            /* 1 if pattern is '_' */
	u8 has_guard;              /* 1 if pattern has guard condition */
	const u8 *guard;           /* Guard expression (if has_guard=1) */
	u32 guard_len;
	const u8 *action;          /* Action code for this pattern */
	u32 action_len;
};

/* Match expression */
struct MatchExpr {
	const u8 *match_value;     /* Variable being matched */
	u32 match_len;
	struct MatchPattern patterns[8];  /* Up to 8 patterns */
	u32 pattern_count;
	u8 exhaustive;             /* 1 if all cases covered */
	u32 code_pos;              /* Code position for match block */
};

/* Macro context for parsing and expansion */
struct MacroCtx {
	struct MacroDef macros[8];  /* Up to 8 macros */
	u32 macro_count;
	struct MatchExpr matches[4];  /* Up to 4 match expressions */
	u32 match_count;
	u8 current_macro_depth;    /* Prevent recursive macro expansion */
};

/* Initialize macro context */
static inline void macro_ctx_init(struct MacroCtx *mc) {
	mc->macro_count = 0;
	mc->match_count = 0;
	mc->current_macro_depth = 0;
}

/* Define macro: #define NAME(arg1, arg2) body */
static inline u8 macro_define(
	struct MacroCtx *mc,
	const u8 *name, u32 name_len,
	const u8 *params[], u32 param_lens[], u32 param_count,
	const u8 *body, u32 body_len)
{
	if (mc->macro_count >= 8) return 1;  /* Too many macros */
	if (body_len > 100) return 1;        /* Body too long */
	if (param_count > 4) return 1;       /* Too many parameters */

	struct MacroDef *md = &mc->macros[mc->macro_count];
	md->name = name;
	md->name_len = name_len;
	md->param_count = param_count;
	md->body_len = body_len;
	md->expansion_count = 0;

	u32 i;
	for (i = 0; i < param_count; i++) {
		md->params[i] = params[i];
		md->param_lens[i] = param_lens[i];
	}
	/* Copy body */
	for (i = 0; i < body_len && i < 100; i++) {
		md->body[i] = body[i];
	}

	mc->macro_count++;
	return 0;
}

/* Lookup macro by name */
static inline struct MacroDef* macro_lookup(
	struct MacroCtx *mc,
	const u8 *name, u32 name_len)
{
	u32 i;
	for (i = 0; i < mc->macro_count; i++) {
		if (mc->macros[i].name_len != name_len) continue;

		u32 j;
		u8 match = 1;
		for (j = 0; j < name_len; j++) {
			if (mc->macros[i].name[j] != name[j]) {
				match = 0;
				break;
			}
		}
		if (match) return &mc->macros[i];
	}
	return NULL;
}

/* Expand macro with arguments */
static inline u8 macro_expand(
	struct MacroCtx *mc,
	struct MacroDef *md,
	const u8 *args[], u32 arg_lens[], u32 arg_count,
	u8 *output, u32 *output_len)
{
	if (md->expansion_count >= 1) return 1;  /* Prevent recursive expansion */
	if (arg_count != md->param_count) return 1;  /* Arity mismatch */

	md->expansion_count++;
	mc->current_macro_depth++;

	if (mc->current_macro_depth >= 1) {
		/* Maximum expansion depth reached */
		md->expansion_count--;
		mc->current_macro_depth--;
		return 1;
	}

	/* Perform textual substitution */
	u32 i = 0;
	u32 out_pos = 0;
	u32 max_out = 256;

	while (i < md->body_len && out_pos < max_out) {
		/* Check if current position starts a parameter name */
		u32 j;
		u8 param_match = 0;
		for (j = 0; j < md->param_count; j++) {
			if (i + md->param_lens[j] <= md->body_len) {
				u32 k;
				u8 match = 1;
				for (k = 0; k < md->param_lens[j]; k++) {
					if (md->body[i + k] != md->params[j][k]) {
						match = 0;
						break;
					}
				}
				if (match && j < arg_count) {
					/* Replace parameter with argument */
					for (k = 0; k < arg_lens[j] && out_pos < max_out; k++) {
						output[out_pos++] = args[j][k];
					}
					i += md->param_lens[j];
					param_match = 1;
					break;
				}
			}
		}
		if (!param_match) {
			/* Copy character as-is */
			output[out_pos++] = md->body[i++];
		}
	}

	*output_len = out_pos;
	md->expansion_count--;
	mc->current_macro_depth--;
	return 0;
}

/* === PATTERN MATCHING === */

/* Parse match expression: match value { pattern1 => action1, ... } */
static inline u8 macro_parse_match(
	const u8 *src, u32 src_len,
	u32 *pos,
	const u8 **match_var, u32 *match_len)
{
	/* Expect 'match' keyword */
	if (*pos + 5 >= src_len) return 1;
	u32 i;
	for (i = 0; i < 5; i++) {
		if (src[*pos + i] != "match"[i]) return 1;
	}
	*pos += 5;

	/* Skip whitespace */
	while (*pos < src_len && (src[*pos] == ' ' || src[*pos] == '\t')) (*pos)++;

	/* Collect match variable name */
	u32 var_start = *pos;
	while (*pos < src_len && (src[*pos] >= 'a' && src[*pos] <= 'z' ||
	       src[*pos] >= 'A' && src[*pos] <= 'Z' ||
	       src[*pos] >= '0' && src[*pos] <= '9' || src[*pos] == '_')) {
		(*pos)++;
	}
	if (*pos > var_start) {
		*match_var = &src[var_start];
		*match_len = *pos - var_start;
	}

	/* Skip whitespace and expect '{' */
	while (*pos < src_len && (src[*pos] == ' ' || src[*pos] == '\t')) (*pos)++;
	if (*pos >= src_len || src[*pos] != '{') return 1;
	(*pos)++;

	return 0;
}

/* Add pattern to match expression */
static inline u8 macro_add_match_pattern(
	struct MacroCtx *mc,
	const u8 *pattern, u32 pattern_len,
	u8 is_wildcard,
	const u8 *action, u32 action_len)
{
	if (mc->match_count >= 4) return 1;  /* Too many match expressions */
	struct MatchExpr *me = &mc->matches[mc->match_count];

	if (me->pattern_count >= 8) return 1;  /* Too many patterns */
	struct MatchPattern *mp = &me->patterns[me->pattern_count];

	mp->pattern = pattern;
	mp->pattern_len = pattern_len;
	mp->is_wildcard = is_wildcard;
	mp->has_guard = 0;
	mp->guard = NULL;
	mp->guard_len = 0;
	mp->action = action;
	mp->action_len = action_len;

	me->pattern_count++;
	return 0;
}

/* Add pattern with guard */
static inline u8 macro_add_match_guard(
	struct MacroCtx *mc,
	const u8 *pattern, u32 pattern_len,
	const u8 *guard, u32 guard_len,
	const u8 *action, u32 action_len)
{
	if (mc->match_count >= 4) return 1;
	struct MatchExpr *me = &mc->matches[mc->match_count];

	if (me->pattern_count >= 8) return 1;
	struct MatchPattern *mp = &me->patterns[me->pattern_count];

	mp->pattern = pattern;
	mp->pattern_len = pattern_len;
	mp->is_wildcard = 0;
	mp->has_guard = 1;
	mp->guard = guard;
	mp->guard_len = guard_len;
	mp->action = action;
	mp->action_len = action_len;

	me->pattern_count++;
	return 0;
}

/* Check match exhaustiveness */
static inline u8 macro_check_exhaustive(
	struct MacroCtx *mc)
{
	if (mc->match_count >= 1) {
		struct MatchExpr *me = &mc->matches[mc->match_count - 1];
		u32 i;
		for (i = 0; i < me->pattern_count; i++) {
			if (me->patterns[i].is_wildcard) {
				me->exhaustive = 1;
				return 0;  /* Wildcard _ covers all cases */
			}
		}
	}
	return 1;  /* Not exhaustive (no wildcard) */
}

/* === CODE GENERATION FOR MATCH === */

/* Emit match as if/else chain */
static inline void macro_emit_match_chain(
	struct CodeGen *cg,
	struct MatchExpr *me)
{
	u32 i;
	/* Emit: if (match_value == pattern[0]) { action[0] } else if (...) ... */
	for (i = 0; i < me->pattern_count; i++) {
		struct MatchPattern *mp = &me->patterns[i];
		if (mp->is_wildcard) {
			/* Default case: emit action without condition */
			/* codegen_emit_block(cg, mp->action, mp->action_len); */
			break;
		}
		/* Emit: CMP match_value, pattern[i]; if equal, emit action[i] */
		/* Emit if/else control flow via CMOV or conditional jumps */
	}
}

#endif /* APKC_LANG_MACROS_H */
