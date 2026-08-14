/* lang_types.h — Basic type system for multi-language compilation
 *
 * Unified type representation across all 7 languages
 * No complex type system - only essential types for compilation
 */

#ifndef APKC_LANG_TYPES_H
#define APKC_LANG_TYPES_H 1

typedef unsigned char u8;
typedef unsigned int u32;

/* Type enumeration */
enum ValueType {
	TYPE_VOID = 0,
	TYPE_I32,        /* 32-bit signed integer */
	TYPE_I64,        /* 64-bit signed integer */
	TYPE_U64,        /* 64-bit unsigned integer */
	TYPE_F64,        /* 64-bit floating point */
	TYPE_PTR,        /* pointer/reference */
	TYPE_UNKNOWN,    /* type inference needed */
};

/* Type descriptor */
struct TypeInfo {
	enum ValueType base_type;
	u8 is_ptr;              /* 1 if pointer */
	u8 is_const;            /* 1 if const/immutable */
	u32 size;               /* size in bytes */
};

/* Get type size in bytes */
static inline u32 type_size(enum ValueType t) {
	switch (t) {
	case TYPE_I32:
		return 4;
	case TYPE_I64:
	case TYPE_U64:
	case TYPE_F64:
	case TYPE_PTR:
		return 8;
	default:
		return 8;  /* default to 64-bit */
	}
}

/* Create type info */
static inline struct TypeInfo type_make(enum ValueType base, u8 is_ptr, u8 is_const) {
	struct TypeInfo ti;
	ti.base_type = base;
	ti.is_ptr = is_ptr;
	ti.is_const = is_const;
	ti.size = type_size(base);
	return ti;
}

/* Infer type from source code keyword/token */
static inline enum ValueType type_infer_keyword(const u8 *keyword, u32 len) {
	if (len == 3) {
		if (keyword[0] == 'i' && keyword[1] == '3' && keyword[2] == '2')
			return TYPE_I32;
		if (keyword[0] == 'u' && keyword[1] == '6' && keyword[2] == '4')
			return TYPE_U64;
		if (keyword[0] == 'i' && keyword[1] == '6' && keyword[2] == '4')
			return TYPE_I64;
	}
	if (len == 4) {
		if (keyword[0] == 'i' && keyword[1] == 'n' && keyword[2] == 't' && keyword[3] == 0)
			return TYPE_I32;
	}
	if (len == 5) {
		if (keyword[0] == 'f' && keyword[1] == 'l' && keyword[2] == 'o' &&
		    keyword[3] == 'a' && keyword[4] == 't')
			return TYPE_F64;
	}
	if (len == 6) {
		if (keyword[0] == 'd' && keyword[1] == 'o' && keyword[2] == 'u' &&
		    keyword[3] == 'b' && keyword[4] == 'l' && keyword[5] == 'e')
			return TYPE_F64;
	}
	return TYPE_UNKNOWN;
}

#endif /* APKC_LANG_TYPES_H */
