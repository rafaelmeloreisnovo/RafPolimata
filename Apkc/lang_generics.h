/* lang_generics.h — Generics & Parametric Polymorphism (Stage 5.2)
 *
 * Generic type parameters, monomorphization at compile time.
 * Supports function and struct generics with constraint checking.
 * Max 3 type parameters, single-level generics only.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_LANG_GENERICS_H
#define APKC_LANG_GENERICS_H 1

#include "lang_types.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Generic type parameter */
struct GenericParam {
	const u8 *name;        /* Parameter name (e.g., "T", "U") */
	u32 name_len;
	enum ValueType bound;  /* Constraint: TYPE_ANY, TYPE_INT, TYPE_PTR, etc. */
	u8 variance;           /* 0 = invariant, 1 = covariant, 2 = contravariant */
};

/* Generic specialization (concrete type instantiation) */
struct GenericSpecialization {
	enum ValueType type_args[3];  /* Concrete types for parameters */
	u32 arg_count;
	u32 code_start;       /* Code location for this specialization */
	u32 code_len;
};

/* Generic function definition */
struct GenericFunc {
	const u8 *name;
	u32 name_len;
	struct GenericParam params[3];  /* Up to 3 type parameters */
	u32 param_count;
	enum ValueType return_type;
	const u8 *body;       /* Pointer to generic function body */
	u32 body_len;
	struct GenericSpecialization specs[4];  /* Up to 4 specializations */
	u32 spec_count;
	u8 recursive;         /* 1 if generic recursively references itself */
};

/* Generic struct definition */
struct GenericStruct {
	const u8 *name;
	u32 name_len;
	struct GenericParam params[3];  /* Up to 3 type parameters */
	u32 param_count;
	struct {
		const u8 *field_name;
		u32 field_len;
		enum ValueType field_type;  /* May contain type parameter reference */
	} fields[8];
	u32 field_count;
	u32 struct_size;      /* Size with actual types filled in */
};

/* Generic context for parsing and compilation */
struct GenericCtx {
	struct GenericFunc generics[8];  /* Up to 8 generic functions */
	u32 generic_count;
	struct GenericStruct generic_structs[8];
	u32 struct_count;
	u32 current_generic;
	u32 specialization_depth;  /* Prevent infinite recursion */
};

/* Initialize generic context */
static inline void generic_ctx_init(struct GenericCtx *gc) {
	gc->generic_count = 0;
	gc->struct_count = 0;
	gc->current_generic = 0;
	gc->specialization_depth = 0;
}

/* Parse generic parameter list: <T, U, V> */
static inline u8 generic_parse_params(
	const u8 *src, u32 src_len,
	u32 *pos,
	struct GenericParam *params,
	u32 *param_count)
{
	/* Expect '<' at *pos */
	if (*pos >= src_len || src[*pos] != '<') return 1;
	(*pos)++;

	*param_count = 0;

	while (*pos < src_len && src[*pos] != '>') {
		if (src[*pos] == ' ' || src[*pos] == '\t' || src[*pos] == ',') {
			(*pos)++;
			continue;
		}
		if (*param_count >= 3) return 1;  /* Max 3 type parameters */

		/* Parse parameter name */
		u32 name_start = *pos;
		while (*pos < src_len && (src[*pos] >= 'A' && src[*pos] <= 'Z' ||
		       src[*pos] >= 'a' && src[*pos] <= 'z' ||
		       src[*pos] >= '0' && src[*pos] <= '9' || src[*pos] == '_')) {
			(*pos)++;
		}
		if (*pos > name_start) {
			params[*param_count].name = &src[name_start];
			params[*param_count].name_len = *pos - name_start;
			params[*param_count].bound = TYPE_ANY;
			params[*param_count].variance = 0;
			(*param_count)++;
		}
	}

	/* Expect '>' */
	if (*pos >= src_len || src[*pos] != '>') return 1;
	(*pos)++;

	return 0;
}

/* Register generic function definition */
static inline u8 generic_register_func(
	struct GenericCtx *gc,
	const u8 *name, u32 name_len,
	struct GenericParam *params, u32 param_count,
	enum ValueType return_type,
	const u8 *body, u32 body_len)
{
	if (gc->generic_count >= 8) return 1;  /* Too many generics */

	struct GenericFunc *gf = &gc->generics[gc->generic_count];
	gf->name = name;
	gf->name_len = name_len;
	gf->param_count = param_count;
	gf->return_type = return_type;
	gf->body = body;
	gf->body_len = body_len;
	gf->spec_count = 0;
	gf->recursive = 0;

	u32 i;
	for (i = 0; i < param_count && i < 3; i++) {
		gf->params[i] = params[i];
	}

	gc->current_generic = gc->generic_count;
	gc->generic_count++;
	return 0;
}

/* Lookup generic function by name */
static inline struct GenericFunc* generic_lookup_func(
	struct GenericCtx *gc,
	const u8 *name, u32 name_len)
{
	u32 i;
	for (i = 0; i < gc->generic_count; i++) {
		if (gc->generics[i].name_len != name_len) continue;

		u32 j;
		u8 match = 1;
		for (j = 0; j < name_len; j++) {
			if (gc->generics[i].name[j] != name[j]) {
				match = 0;
				break;
			}
		}
		if (match) return &gc->generics[i];
	}
	return NULL;
}

/* === MONOMORPHIZATION === */

/* Specialize generic function for concrete types */
static inline u8 generic_specialize(
	struct GenericCtx *gc,
	struct GenericFunc *gf,
	const enum ValueType *type_args, u32 arg_count,
	u32 code_start)
{
	if (gf->spec_count >= 4) return 1;  /* Max 4 specializations */
	if (arg_count != gf->param_count) return 1;  /* Arity mismatch */
	if (gc->specialization_depth >= 2) return 1;  /* Max nesting */

	struct GenericSpecialization *spec = &gf->specs[gf->spec_count];
	u32 i;
	for (i = 0; i < arg_count; i++) {
		spec->type_args[i] = type_args[i];
	}
	spec->arg_count = arg_count;
	spec->code_start = code_start;
	spec->code_len = 0;  /* Computed later during code generation */

	gf->spec_count++;
	return 0;
}

/* Substitute type parameters in generic body */
static inline u8 generic_substitute(
	const u8 *generic_body, u32 body_len,
	struct GenericParam *params, u32 param_count,
	const enum ValueType *type_args,
	u8 *output, u32 *output_len)
{
	/* Perform textual substitution: replace T with concrete type */
	/* For freestanding model, represent types as single-char codes: */
	/* 'i'=INT, 'p'=PTR, 'f'=FLOAT, 'v'=VOID */
	u32 i = 0;
	u32 out_pos = 0;
	u32 max_out = 1024;  /* Prevent buffer overflow */

	while (i < body_len && out_pos < max_out) {
		/* Check if current position starts a type parameter name */
		u32 j;
		u8 param_match = 0;
		for (j = 0; j < param_count; j++) {
			if (i + params[j].name_len <= body_len) {
				u32 k;
				u8 match = 1;
				for (k = 0; k < params[j].name_len; k++) {
					if (generic_body[i + k] != params[j].name[k]) {
						match = 0;
						break;
					}
				}
				if (match) {
					/* Replace with type code */
					output[out_pos++] = type_args[j] & 0xFF;
					i += params[j].name_len;
					param_match = 1;
					break;
				}
			}
		}
		if (!param_match) {
			/* Copy character as-is */
			output[out_pos++] = generic_body[i++];
		}
	}

	*output_len = out_pos;
	return 0;
}

/* Validate generic function against constraints */
static inline u8 generic_validate_constraints(
	struct GenericFunc *gf,
	const enum ValueType *type_args, u32 arg_count)
{
	u32 i;
	for (i = 0; i < arg_count && i < gf->param_count; i++) {
		enum ValueType bound = gf->params[i].bound;
		enum ValueType actual = type_args[i];

		/* Check constraint satisfaction */
		if (bound == TYPE_ANY) continue;  /* Any type satisfies */
		if (bound == TYPE_INT && actual != TYPE_INT) return 1;  /* Type mismatch */
		if (bound == TYPE_PTR && actual != TYPE_PTR) return 1;
		if (bound == TYPE_FLOAT && actual != TYPE_FLOAT) return 1;
	}
	return 0;
}

/* Get specialization for types (or create if needed) */
static inline struct GenericSpecialization* generic_get_specialization(
	struct GenericFunc *gf,
	const enum ValueType *type_args, u32 arg_count)
{
	u32 i;
	for (i = 0; i < gf->spec_count; i++) {
		struct GenericSpecialization *spec = &gf->specs[i];
		if (spec->arg_count != arg_count) continue;

		u32 j;
		u8 match = 1;
		for (j = 0; j < arg_count; j++) {
			if (spec->type_args[j] != type_args[j]) {
				match = 0;
				break;
			}
		}
		if (match) return spec;
	}
	return NULL;  /* No matching specialization */
}

/* === GENERIC STRUCTS === */

/* Register generic struct definition */
static inline u8 generic_register_struct(
	struct GenericCtx *gc,
	const u8 *name, u32 name_len,
	struct GenericParam *params, u32 param_count)
{
	if (gc->struct_count >= 8) return 1;  /* Too many generic structs */

	struct GenericStruct *gs = &gc->generic_structs[gc->struct_count];
	gs->name = name;
	gs->name_len = name_len;
	gs->param_count = param_count;
	gs->field_count = 0;
	gs->struct_size = 0;

	u32 i;
	for (i = 0; i < param_count && i < 3; i++) {
		gs->params[i] = params[i];
	}

	gc->struct_count++;
	return 0;
}

/* Add field to generic struct */
static inline u8 generic_struct_add_field(
	struct GenericCtx *gc,
	u32 struct_idx,
	const u8 *field_name, u32 field_len,
	enum ValueType field_type)
{
	if (struct_idx >= gc->struct_count) return 1;

	struct GenericStruct *gs = &gc->generic_structs[struct_idx];
	if (gs->field_count >= 8) return 1;  /* Too many fields */

	gs->fields[gs->field_count].field_name = field_name;
	gs->fields[gs->field_count].field_len = field_len;
	gs->fields[gs->field_count].field_type = field_type;
	gs->field_count++;

	return 0;
}

/* Compute struct size with concrete types */
static inline u32 generic_compute_struct_size(
	struct GenericStruct *gs,
	const enum ValueType *type_args, u32 arg_count)
{
	u32 size = 0;
	u32 i;
	for (i = 0; i < gs->field_count; i++) {
		enum ValueType ftype = gs->fields[i].field_type;
		/* Size depends on actual type: INT=8, PTR=8, FLOAT=8, etc. */
		size += 8;  /* All types are 64-bit in this model */
	}
	return size;
}

#endif /* APKC_LANG_GENERICS_H */
