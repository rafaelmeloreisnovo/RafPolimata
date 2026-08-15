/* sem_unification.h — Type Unification & Constraint Solving (Phase 21.2)
 *
 * Robinson unification algorithm: find most general unifier
 * Occurs check: prevent infinite types (T ~ [T])
 * Constraint reduction: simplify constraint systems
 * Error detection: identify unsolvable constraints
 * Substitution composition: combine multiple unifiers
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEM_UNIFICATION_H
#define APKC_SEM_UNIFICATION_H 1

#include "sem_type_system.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* UNIFICATION STATE */
/* ============================================================ */

struct UnificationState {
	struct SubstitutionMap subst;     /* Current substitutions */
	struct ConstraintSystem remaining; /* Unsolved constraints */
	struct TypeError error;           /* Error (if any) */
	u32 iterations;                   /* Number of unification iterations */
	u8 success;                       /* 1 if unification succeeded */
};

/* ============================================================ */
/* OCCURS CHECK */
/* ============================================================ */

/* Check if type variable occurs in type (prevents infinite types) */
static inline u8 occurs_check(u32 var_id, struct Type *ty) {
	if (!ty) return 0;

	switch (ty->kind) {
	case TYPE_VAR:
		return ty->data.var.id == var_id;
	case TYPE_ARRAY:
		return occurs_check(var_id, ty->data.array.element_type);
	case TYPE_SLICE:
		return occurs_check(var_id, ty->data.slice.element_type);
	case TYPE_PTR:
		return occurs_check(var_id, ty->data.ptr.pointee_type);
	case TYPE_FUNC:
		return occurs_check(var_id, ty->data.func.param_type) ||
		       occurs_check(var_id, ty->data.func.return_type);
	case TYPE_TUPLE: {
		u32 i;
		for (i = 0; i < ty->data.tuple.arity; i++) {
			if (occurs_check(var_id, ty->data.tuple.elem_types[i]))
				return 1;
		}
		return 0;
	}
	case TYPE_GENERIC: {
		u32 i;
		for (i = 0; i < ty->data.generic.arg_count; i++) {
			if (occurs_check(var_id, ty->data.generic.type_args[i]))
				return 1;
		}
		return 0;
	}
	default:
		return 0;
	}
}

/* ============================================================ */
/* SUBSTITUTION APPLICATION */
/* ============================================================ */

/* Apply substitution to a type (deep copy with substitutions) */
static inline struct Type apply_subst_deep(
	struct Type *ty,
	struct SubstitutionMap *subst) {

	if (!ty) return type_unknown_safe();

	/* If this is a variable, look it up */
	if (ty->kind == TYPE_VAR) {
		struct Type *found = subst_map_lookup(subst, ty->data.var.id);
		if (found) return *found;
		return *ty;
	}

	/* For compound types, recursively apply substitution */
	struct Type result = *ty;
	switch (ty->kind) {
	case TYPE_ARRAY:
		if (ty->data.array.element_type) {
			result.data.array.element_type =
				ty->data.array.element_type;
		}
		break;
	case TYPE_FUNC:
		if (ty->data.func.param_type) {
			result.data.func.param_type =
				ty->data.func.param_type;
		}
		if (ty->data.func.return_type) {
			result.data.func.return_type =
				ty->data.func.return_type;
		}
		break;
	default:
		break;
	}

	return result;
}

/* ============================================================ */
/* UNIFICATION ALGORITHM (Robinson) */
/* ============================================================ */

/* Unify two types: find substitution σ such that σ(t1) = σ(t2) */
static inline u8 unify_types(
	struct Type *t1,
	struct Type *t2,
	struct SubstitutionMap *subst,
	struct TypeError *error) {

	if (!t1 || !t2 || !subst) return 0;

	/* Apply current substitution */
	struct Type s1 = apply_subst_deep(t1, subst);
	struct Type s2 = apply_subst_deep(t2, subst);

	/* Same type */
	if (type_equal(&s1, &s2)) return 1;

	/* Both variables: bind first to second */
	if (s1.kind == TYPE_VAR && s2.kind == TYPE_VAR) {
		if (s1.data.var.id == s2.data.var.id) return 1;
		/* Bind var1 to var2 */
		return subst_map_add(subst, s1.data.var, s2) == 0;
	}

	/* Left is variable: bind to right */
	if (s1.kind == TYPE_VAR) {
		if (occurs_check(s1.data.var.id, &s2)) {
			if (error) {
				error->code = TE_INFINITE_TYPE;
				error->msg = "Infinite type (occurs check failed)";
				error->expected = t1;
				error->actual = t2;
			}
			return 0;
		}
		return subst_map_add(subst, s1.data.var, s2) == 0;
	}

	/* Right is variable: bind to left */
	if (s2.kind == TYPE_VAR) {
		if (occurs_check(s2.data.var.id, &s1)) {
			if (error) {
				error->code = TE_INFINITE_TYPE;
				error->msg = "Infinite type (occurs check failed)";
				error->expected = t1;
				error->actual = t2;
			}
			return 0;
		}
		return subst_map_add(subst, s2.data.var, s1) == 0;
	}

	/* Both compound types: unify components */
	if (s1.kind != s2.kind) {
		if (error) {
			error->code = TE_MISMATCH;
			error->msg = "Type mismatch";
			error->expected = t1;
			error->actual = t2;
		}
		return 0;
	}

	switch (s1.kind) {
	case TYPE_ARRAY:
		if (s1.data.array.length != s2.data.array.length) {
			if (error) error->code = TE_MISMATCH;
			return 0;
		}
		return unify_types(s1.data.array.element_type,
		                   s2.data.array.element_type,
		                   subst, error);

	case TYPE_FUNC:
		return unify_types(s1.data.func.param_type,
		                   s2.data.func.param_type,
		                   subst, error) &&
		       unify_types(s1.data.func.return_type,
		                   s2.data.func.return_type,
		                   subst, error);

	case TYPE_TUPLE:
		if (s1.data.tuple.arity != s2.data.tuple.arity) {
			if (error) error->code = TE_MISMATCH;
			return 0;
		}
		{
			u32 i;
			for (i = 0; i < s1.data.tuple.arity; i++) {
				if (!unify_types(s1.data.tuple.elem_types[i],
				                 s2.data.tuple.elem_types[i],
				                 subst, error))
					return 0;
			}
			return 1;
		}

	default:
		if (error) error->code = TE_MISMATCH;
		return 0;
	}
}

/* ============================================================ */
/* CONSTRAINT SOLVING */
/* ============================================================ */

/* Initialize unification state */
static inline void unify_state_init(struct UnificationState *state) {
	if (!state) return;
	subst_map_init(&state->subst);
	constraints_init(&state->remaining);
	state->error.code = TE_OK;
	state->error.msg = 0;
	state->iterations = 0;
	state->success = 0;
}

/* Solve constraint system: repeatedly apply unification until fixed point */
static inline u8 solve_constraints(
	struct ConstraintSystem *cs,
	struct UnificationState *state) {

	if (!cs || !state) return 0;

	state->iterations = 0;
	state->success = 1;

	/* Iterate until no progress or 1000 iterations */
	while (state->remaining.count > 0 && state->iterations < 1000) {
		state->iterations++;

		/* Try to solve first unsolved constraint */
		struct TypeConstraint *c = &state->remaining.constraints[0];

		if (unify_types(c->left, c->right, &state->subst, &state->error)) {
			/* Remove solved constraint and shift others */
			u32 i;
			for (i = 0; i < state->remaining.count - 1; i++) {
				state->remaining.constraints[i] =
					state->remaining.constraints[i + 1];
			}
			state->remaining.count--;
		} else {
			/* Constraint unsolvable */
			if (!state->error.msg) {
				state->error.code = TE_NO_UNIFIER;
				state->error.msg = "No unifier found";
			}
			state->success = 0;
			return 0;
		}
	}

	if (state->remaining.count > 0) {
		state->error.code = TE_RANK_RESTRICTION;
		state->error.msg = "Too many iterations (recursive type?)";
		state->success = 0;
		return 0;
	}

	state->success = 1;
	return 1;
}

/* ============================================================ */
/* SUBSTITUTION COMPOSITION */
/* ============================================================ */

/* Compose two substitution maps: s1 ∘ s2 = apply s1 to results of s2 */
static inline u8 subst_compose(
	struct SubstitutionMap *s1,
	struct SubstitutionMap *s2,
	struct SubstitutionMap *result) {

	if (!s1 || !s2 || !result) return 0;

	subst_map_init(result);

	/* Copy all entries from s2, applying s1 to their values */
	u32 i;
	for (i = 0; i < s2->count; i++) {
		struct Type applied = apply_subst_deep(&s2->entries[i].ty, s1);
		if (subst_map_add(result, s2->entries[i].var, applied) != 0) {
			return 0;
		}
	}

	/* Add entries from s1 that don't appear in s2 */
	for (i = 0; i < s1->count; i++) {
		u32 j;
		u8 found = 0;
		for (j = 0; j < s2->count; j++) {
			if (s2->entries[j].var.id == s1->entries[i].var.id) {
				found = 1;
				break;
			}
		}
		if (!found) {
			if (subst_map_add(result, s1->entries[i].var,
			                  s1->entries[i].ty) != 0) {
				return 0;
			}
		}
	}

	return 1;
}

/* ============================================================ */
/* ERROR REPORTING */
/* ============================================================ */

static inline const char *type_error_string(enum TypeErrorCode code) {
	switch (code) {
	case TE_OK: return "No error";
	case TE_MISMATCH: return "Type mismatch";
	case TE_UNBOUND_VAR: return "Unbound type variable";
	case TE_INFINITE_TYPE: return "Infinite type (occurs check)";
	case TE_NO_UNIFIER: return "No unifier exists";
	case TE_RANK_RESTRICTION: return "Type rank too high";
	case TE_UNRESOLVED: return "Type unresolved after inference";
	default: return "Unknown error";
	}
}

#endif /* APKC_SEM_UNIFICATION_H */
