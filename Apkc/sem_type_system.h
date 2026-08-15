/* sem_type_system.h — Type System & Representation (Phase 21.1)
 *
 * Type representation: primitive, compound, and polymorphic types
 * Type operations: unification, substitution, equality, variance
 * Constraint collection: constraint equations & systems
 * Type variable management: scoped, tracked substitutions
 * Error reporting: type error classification & context
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEM_TYPE_SYSTEM_H
#define APKC_SEM_TYPE_SYSTEM_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed int s32;
typedef signed long long s64;

/* ============================================================ */
/* TYPE REPRESENTATION */
/* ============================================================ */

/* Type kind: classification of a type */
enum TypeKind {
	TYPE_UNKNOWN = 0,    /* Unknown/error type */
	TYPE_UNIT = 1,       /* () — unit/void */
	TYPE_BOOL = 2,       /* bool */
	TYPE_INT = 3,        /* int (platform-dependent width) */
	TYPE_INT8 = 4,       /* i8 */
	TYPE_INT16 = 5,      /* i16 */
	TYPE_INT32 = 6,      /* i32 */
	TYPE_INT64 = 7,      /* i64 */
	TYPE_UINT = 8,       /* uint */
	TYPE_UINT8 = 9,      /* u8 */
	TYPE_UINT16 = 10,    /* u16 */
	TYPE_UINT32 = 11,    /* u32 */
	TYPE_UINT64 = 12,    /* u64 */
	TYPE_FLOAT = 13,     /* f32 */
	TYPE_DOUBLE = 14,    /* f64 */
	TYPE_CHAR = 15,      /* char */
	TYPE_STR = 16,       /* &str, String */
	TYPE_VAR = 17,       /* Type variable (polymorphic) */
	TYPE_FUNC = 18,      /* Function type (param → return) */
	TYPE_ARRAY = 19,     /* Array type [T; n] */
	TYPE_SLICE = 20,     /* Slice type &[T] */
	TYPE_TUPLE = 21,     /* Tuple (T1, T2, ...) */
	TYPE_STRUCT = 22,    /* Struct type */
	TYPE_ENUM = 23,      /* Enum type */
	TYPE_PTR = 24,       /* Pointer/reference &T */
	TYPE_GENERIC = 25,   /* Generic type T<U, V> */
	TYPE_NEVER = 26,     /* Never type (!) - diverging */
	TYPE_CLASS = 27      /* Java/Kotlin class type */
};

/* Type variance: how a type parameter relates to its parent */
enum TypeVariance {
	VAR_INVARIANT = 0,   /* T appears in both input & output */
	VAR_COVARIANT = 1,   /* T appears only in output */
	VAR_CONTRAVARIANT = 2 /* T appears only in input */
};

/* Type variable: placeholder for unknown type */
struct TypeVar {
	u32 id;              /* Unique identifier (0-indexed) */
	const char *name;    /* Optional name for debugging */
	u8 variance;         /* VAR_INVARIANT, VAR_COVARIANT, VAR_CONTRAVARIANT */
	u8 bound_count;      /* Number of type bounds */
};

/* Type: algebraic type representation */
struct Type {
	u8 kind;             /* TypeKind enum */
	union {
		struct TypeVar var;        /* TYPE_VAR */
		struct {
			struct Type *param_type;
			struct Type *return_type;
		} func;                    /* TYPE_FUNC: param → return */
		struct {
			struct Type *element_type;
			u32 length;
		} array;                   /* TYPE_ARRAY: [T; n] */
		struct {
			struct Type *element_type;
		} slice;                   /* TYPE_SLICE: &[T] */
		struct {
			struct Type *elem_types[8];  /* Max 8-tuple elements */
			u32 arity;
		} tuple;                   /* TYPE_TUPLE: (T1, T2, ...) */
		struct {
			const char *struct_name;
			u32 field_count;
		} struct_type;             /* TYPE_STRUCT */
		struct {
			const char *enum_name;
			u32 variant_count;
		} enum_type;               /* TYPE_ENUM */
		struct {
			struct Type *pointee_type;
			u8 is_mutable;
		} ptr;                     /* TYPE_PTR: &T or &mut T */
		struct {
			const char *base_name;      /* e.g., "List" */
			struct Type *type_args[4];  /* Max 4 type arguments */
			u32 arg_count;
		} generic;                 /* TYPE_GENERIC: T<U, V> */
		struct {
			const char *class_name;
		} class_type;              /* TYPE_CLASS: Java/Kotlin */
	} data;
};

/* Type substitution: mapping from type variable to type */
struct Substitution {
	struct TypeVar var;
	struct Type ty;
};

/* Substitution map: collection of substitutions */
struct SubstitutionMap {
	struct Substitution entries[64];  /* Max 64 substitutions */
	u32 count;
};

/* Type constraint: equation T1 ~ T2 to be solved */
struct TypeConstraint {
	struct Type *left;
	struct Type *right;
	u32 source_line;  /* For error reporting */
	const char *context;  /* Description of constraint origin */
};

/* Constraint system: collection of constraints to solve */
struct ConstraintSystem {
	struct TypeConstraint constraints[128];  /* Max 128 constraints */
	u32 count;
};

/* Type error: classification of type mismatch */
struct TypeError {
	u8 code;          /* Error classification */
	const char *msg;  /* Human-readable error message */
	u32 line;         /* Source line where error occurred */
	struct Type *expected;
	struct Type *actual;
};

/* Type error classifications */
enum TypeErrorCode {
	TE_OK = 0,                   /* No error */
	TE_MISMATCH = 1,            /* Type mismatch */
	TE_UNBOUND_VAR = 2,         /* Unbound type variable */
	TE_INFINITE_TYPE = 3,       /* Occurs check failure (T ~ [T]) */
	TE_NO_UNIFIER = 4,          /* No common unifier exists */
	TE_RANK_RESTRICTION = 5,    /* Rank too high for inference */
	TE_UNRESOLVED = 6           /* Type remains unresolved after inference */
};

/* ============================================================ */
/* PRIMITIVE TYPE CONSTRUCTORS */
/* ============================================================ */

static inline struct Type type_unit(void) {
	struct Type t;
	t.kind = TYPE_UNIT;
	return t;
}

static inline struct Type type_bool(void) {
	struct Type t;
	t.kind = TYPE_BOOL;
	return t;
}

static inline struct Type type_int(void) {
	struct Type t;
	t.kind = TYPE_INT;
	return t;
}

static inline struct Type type_int32(void) {
	struct Type t;
	t.kind = TYPE_INT32;
	return t;
}

static inline struct Type type_int64(void) {
	struct Type t;
	t.kind = TYPE_INT64;
	return t;
}

static inline struct Type type_uint64(void) {
	struct Type t;
	t.kind = TYPE_UINT64;
	return t;
}

static inline struct Type type_float(void) {
	struct Type t;
	t.kind = TYPE_FLOAT;
	return t;
}

static inline struct Type type_double(void) {
	struct Type t;
	t.kind = TYPE_DOUBLE;
	return t;
}

static inline struct Type type_str(void) {
	struct Type t;
	t.kind = TYPE_STR;
	return t;
}

static inline struct Type type_never(void) {
	struct Type t;
	t.kind = TYPE_NEVER;
	return t;
}

/* ============================================================ */
/* COMPOUND TYPE CONSTRUCTORS */
/* ============================================================ */

static inline struct Type type_array(struct Type *elem_type, u32 length) {
	struct Type t;
	if (!elem_type) {
		t.kind = TYPE_UNKNOWN;
		return t;
	}
	t.kind = TYPE_ARRAY;
	t.data.array.element_type = elem_type;
	t.data.array.length = length;
	return t;
}

static inline struct Type type_slice(struct Type *elem_type) {
	struct Type t;
	if (!elem_type) {
		t.kind = TYPE_UNKNOWN;
		return t;
	}
	t.kind = TYPE_SLICE;
	t.data.slice.element_type = elem_type;
	return t;
}

static inline struct Type type_ptr(struct Type *pointee, u8 is_mutable) {
	struct Type t;
	if (!pointee) {
		t.kind = TYPE_UNKNOWN;
		return t;
	}
	t.kind = TYPE_PTR;
	t.data.ptr.pointee_type = pointee;
	t.data.ptr.is_mutable = is_mutable;
	return t;
}

static inline struct Type type_func(struct Type *param, struct Type *ret) {
	struct Type t;
	if (!param || !ret) {
		t.kind = TYPE_UNKNOWN;
		return t;
	}
	t.kind = TYPE_FUNC;
	t.data.func.param_type = param;
	t.data.func.return_type = ret;
	return t;
}

static inline struct Type type_tuple(struct Type **elems, u32 arity) {
	struct Type t;
	if (!elems || arity == 0 || arity > 8) {
		t.kind = TYPE_UNKNOWN;
		return t;
	}
	t.kind = TYPE_TUPLE;
	u32 i;
	for (i = 0; i < arity; i++) {
		t.data.tuple.elem_types[i] = elems[i];
	}
	t.data.tuple.arity = arity;
	return t;
}

/* ============================================================ */
/* TYPE VARIABLE MANAGEMENT */
/* ============================================================ */

static inline struct Type type_var(u32 id, const char *name) {
	struct Type t;
	t.kind = TYPE_VAR;
	t.data.var.id = id;
	t.data.var.name = name;
	t.data.var.variance = VAR_INVARIANT;
	t.data.var.bound_count = 0;
	return t;
}

/* ============================================================ */
/* SAFE TYPE CONSTRUCTORS FOR ERROR CASES */
/* ============================================================ */

static inline struct Type type_unknown_safe(void) {
	struct Type t;
	t.kind = TYPE_UNKNOWN;
	return t;
}

/* ============================================================ */
/* SUBSTITUTION OPERATIONS */
/* ============================================================ */

static inline void subst_map_init(struct SubstitutionMap *map) {
	if (!map) return;
	map->count = 0;
}

static inline u8 subst_map_add(
	struct SubstitutionMap *map,
	struct TypeVar var,
	struct Type ty) {

	if (!map || map->count >= 64) return 1;

	map->entries[map->count].var = var;
	map->entries[map->count].ty = ty;
	map->count++;
	return 0;
}

static inline struct Type *subst_map_lookup(
	struct SubstitutionMap *map,
	u32 var_id) {

	if (!map) return 0;

	u32 i;
	for (i = 0; i < map->count; i++) {
		if (map->entries[i].var.id == var_id) {
			return &map->entries[i].ty;
		}
	}
	return 0;
}

/* Apply substitution to a type */
static inline struct Type subst_apply(
	struct Type *ty,
	struct SubstitutionMap *subst) {

	if (!ty || !subst) return type_unknown_safe();

	if (ty->kind == TYPE_VAR) {
		struct Type *found = subst_map_lookup(subst, ty->data.var.id);
		return found ? *found : *ty;
	}
	/* Compound types: recursively apply substitution */
	return *ty;
}

/* ============================================================ */
/* TYPE EQUALITY & COMPARISON */
/* ============================================================ */

static inline u8 type_equal(struct Type *t1, struct Type *t2) {
	if (!t1 || !t2) return 0;
	if (t1->kind != t2->kind) return 0;

	switch (t1->kind) {
	case TYPE_VAR:
		return t1->data.var.id == t2->data.var.id;
	case TYPE_ARRAY:
		return t1->data.array.length == t2->data.array.length &&
		       type_equal(t1->data.array.element_type,
		                  t2->data.array.element_type);
	case TYPE_FUNC:
		return type_equal(t1->data.func.param_type,
		                  t2->data.func.param_type) &&
		       type_equal(t1->data.func.return_type,
		                  t2->data.func.return_type);
	case TYPE_TUPLE:
		if (t1->data.tuple.arity != t2->data.tuple.arity) return 0;
		u32 i;
		for (i = 0; i < t1->data.tuple.arity; i++) {
			if (!type_equal(t1->data.tuple.elem_types[i],
			                t2->data.tuple.elem_types[i]))
				return 0;
		}
		return 1;
	default:
		return t1->kind == t2->kind;
	}
}

/* ============================================================ */
/* CONSTRAINT SYSTEM OPERATIONS */
/* ============================================================ */

static inline void constraints_init(struct ConstraintSystem *cs) {
	if (!cs) return;
	cs->count = 0;
}

static inline u8 constraints_add(
	struct ConstraintSystem *cs,
	struct Type *left,
	struct Type *right,
	u32 line,
	const char *context) {

	if (!cs || !left || !right || cs->count >= 128) return 1;

	cs->constraints[cs->count].left = left;
	cs->constraints[cs->count].right = right;
	cs->constraints[cs->count].source_line = line;
	cs->constraints[cs->count].context = context;
	cs->count++;
	return 0;
}

/* ============================================================ */
/* TYPE CLASSIFICATION & INSPECTION */
/* ============================================================ */

static inline u8 type_is_numeric(struct Type *ty) {
	if (!ty) return 0;
	return ty->kind >= TYPE_INT && ty->kind <= TYPE_DOUBLE;
}

static inline u8 type_is_primitive(struct Type *ty) {
	if (!ty) return 0;
	return ty->kind >= TYPE_UNIT && ty->kind <= TYPE_CLASS;
}

static inline u8 type_is_compound(struct Type *ty) {
	if (!ty) return 0;
	return ty->kind == TYPE_FUNC || ty->kind == TYPE_ARRAY ||
	       ty->kind == TYPE_TUPLE || ty->kind == TYPE_STRUCT;
}

#endif /* APKC_SEM_TYPE_SYSTEM_H */
