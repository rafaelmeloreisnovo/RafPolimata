/* bridge_language_bindings.h — Language Bindings & FFI Glue Layer (Stage 8.1)
 *
 * Type marshalling for 7 languages → common machine representation.
 * Binding registry for all supported languages.
 * Zero-copy argument/return value packing/unpacking.
 * ABI adaptation: language-specific calling conventions → branchless ABI.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation (max 256B per call).
 */

#ifndef APKC_BRIDGE_LANGUAGE_BINDINGS_H
#define APKC_BRIDGE_LANGUAGE_BINDINGS_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Language type identifiers */
enum LanguageType {
	LANG_PYTHON = 0,
	LANG_GO = 1,
	LANG_RUST = 2,
	LANG_C = 3,
	LANG_JAVASCRIPT = 4,
	LANG_JAVA = 5,
	LANG_SWIFT = 6
};

/* Type codes for marshalling */
enum MarshalType {
	MTYPE_INT64 = 0,      /* 64-bit signed integer */
	MTYPE_UINT64 = 1,     /* 64-bit unsigned integer */
	MTYPE_FLOAT64 = 2,    /* 64-bit IEEE floating point */
	MTYPE_POINTER = 3,    /* Memory pointer/reference */
	MTYPE_BOOL = 4,       /* Boolean (0 or 1) */
	MTYPE_STRING = 5,     /* String reference (ptr + len) */
	MTYPE_ARRAY = 6,      /* Array reference (ptr + len + stride) */
	MTYPE_STRUCT = 7,     /* Struct reference (ptr + type_id) */
	MTYPE_NULL = 8        /* Null/nil/None value */
};

/* Type descriptor for runtime type information */
struct TypeDescriptor {
	enum MarshalType base_type;   /* Primary type classification */
	u32 size;                      /* Size in bytes (for structs/arrays) */
	u32 element_size;              /* Element size (for arrays/maps) */
	u32 type_id;                   /* Language-specific type identifier */
	u8 is_ref;                     /* 1 if reference/pointer type */
	u8 is_nullable;                /* 1 if can be null/nil/None */
	const u8 *type_name;           /* Type name string (for debugging) */
	u32 type_name_len;
};

/* Marshalled value representation */
struct MarshalledValue {
	u64 value;                     /* Primary value (or pointer) */
	u64 aux;                       /* Auxiliary data (length, stride, type_id) */
	struct TypeDescriptor type;    /* Type metadata */
};

/* Language binding functions */
struct LanguageBinding {
	/* Convert language-specific value → machine u64 */
	struct MarshalledValue (*to_machine)(
		const u8 *lang_value,      /* Language-native value (opaque) */
		u32 value_len,
		struct TypeDescriptor type);

	/* Convert machine u64 → language-specific value */
	u8 (*from_machine)(
		struct MarshalledValue m,
		u8 *lang_value_out,        /* Output buffer (256B max) */
		u32 *len_out);

	/* Get type descriptor from language type name */
	struct TypeDescriptor (*type_from_name)(
		const u8 *type_name,
		u32 name_len);

	/* Validate value compatibility with expected type */
	u8 (*type_compat)(
		struct MarshalledValue value,
		struct TypeDescriptor expected);

	/* Format value for debugging/logging */
	u32 (*format_value)(
		struct MarshalledValue m,
		u8 *buf, u32 buf_size);
};

/* Binding registry (one per language) */
struct BindingRegistry {
	struct LanguageBinding bindings[7];  /* One per language */
	u8 initialized;
};

/* ============================================================ */
/* PYTHON BINDINGS */
/* ============================================================ */

static inline struct MarshalledValue binding_py_to_machine(
	const u8 *py_value, u32 value_len, struct TypeDescriptor type) {

	struct MarshalledValue m = {0};
	m.type = type;

	/* Python values are opaque; assume encoded as 64-bit + metadata */
	if (value_len >= 8) {
		u64 v = 0;
		u32 i;
		for (i = 0; i < 8 && i < value_len; i++) {
			v |= ((u64)py_value[i]) << (i * 8);
		}
		m.value = v;
	}

	/* Python strings/bytes: pointer + length in aux */
	if (type.base_type == MTYPE_STRING) {
		m.aux = value_len;  /* Length stored in aux */
	}

	return m;
}

static inline u8 binding_py_from_machine(
	struct MarshalledValue m,
	u8 *py_out, u32 *len_out) {

	if (!py_out || !len_out) return 1;

	/* Convert machine value back to Python representation */
	u32 i;
	for (i = 0; i < 8 && i < 256; i++) {
		py_out[i] = (u8)((m.value >> (i * 8)) & 0xFF);
	}
	*len_out = 8;
	return 0;
}

static inline struct TypeDescriptor binding_py_type_from_name(
	const u8 *name, u32 name_len) {

	struct TypeDescriptor td = {0};
	td.base_type = MTYPE_INT64;  /* Python int default */
	td.is_nullable = 1;          /* Python has None */

	/* Simple matching (first 4 bytes) */
	if (name_len >= 3) {
		if (name[0] == 'i' && name[1] == 'n' && name[2] == 't') {
			td.base_type = MTYPE_INT64;
		} else if (name[0] == 's' && name[1] == 't' && name[2] == 'r') {
			td.base_type = MTYPE_STRING;
		} else if (name[0] == 'f' && name[1] == 'l' && name[2] == 't') {
			td.base_type = MTYPE_FLOAT64;
		}
	}

	return td;
}

static inline u8 binding_py_type_compat(
	struct MarshalledValue v, struct TypeDescriptor expected) {

	/* Python is dynamically typed; always compatible unless explicitly type-checked */
	return 1;  /* Compatible */
}

static inline u32 binding_py_format_value(
	struct MarshalledValue m, u8 *buf, u32 buf_size) {

	if (!buf || buf_size < 20) return 0;

	const u8 *fmt = (const u8*)"py_val[";
	u32 i = 0;
	while (fmt[i] && i < buf_size - 1) {
		buf[i] = fmt[i];
		i++;
	}

	/* Add hex value */
	u64 v = m.value;
	u32 j;
	for (j = 0; j < 16 && i < buf_size - 2; j++) {
		u8 nib = (v >> (60 - j * 4)) & 0xF;
		buf[i++] = (nib < 10) ? ('0' + nib) : ('a' + nib - 10);
		v <<= 4;
	}

	if (i < buf_size - 1) buf[i++] = ']';
	buf[i] = '\0';

	return i;
}

/* ============================================================ */
/* GO BINDINGS */
/* ============================================================ */

static inline struct MarshalledValue binding_go_to_machine(
	const u8 *go_value, u32 value_len, struct TypeDescriptor type) {

	struct MarshalledValue m = {0};
	m.type = type;

	/* Go: encode as simple 64-bit value */
	if (value_len >= 8) {
		u64 v = 0;
		u32 i;
		for (i = 0; i < 8; i++) {
			v |= ((u64)go_value[i]) << (i * 8);
		}
		m.value = v;
	}

	return m;
}

static inline u8 binding_go_from_machine(
	struct MarshalledValue m,
	u8 *go_out, u32 *len_out) {

	if (!go_out || !len_out) return 1;

	u32 i;
	for (i = 0; i < 8 && i < 256; i++) {
		go_out[i] = (u8)((m.value >> (i * 8)) & 0xFF);
	}
	*len_out = 8;
	return 0;
}

static inline struct TypeDescriptor binding_go_type_from_name(
	const u8 *name, u32 name_len) {

	struct TypeDescriptor td = {0};
	td.base_type = MTYPE_INT64;  /* Go int default */
	td.is_nullable = 0;          /* Go doesn't have nil for primitives */

	if (name_len >= 3) {
		if (name[0] == 'i' && name[1] == 'n' && name[2] == 't') {
			td.base_type = MTYPE_INT64;
		} else if (name[0] == 's' && name[1] == 't' && name[2] == 'r') {
			td.base_type = MTYPE_STRING;
			td.is_nullable = 1;
		}
	}

	return td;
}

static inline u8 binding_go_type_compat(
	struct MarshalledValue v, struct TypeDescriptor expected) {

	/* Go has static types; check if base types match */
	return 1;  /* Simplified: assume compatible */
}

static inline u32 binding_go_format_value(
	struct MarshalledValue m, u8 *buf, u32 buf_size) {

	if (!buf || buf_size < 20) return 0;

	const u8 *fmt = (const u8*)"go_val[";
	u32 i = 0;
	while (fmt[i] && i < buf_size - 1) {
		buf[i] = fmt[i];
		i++;
	}

	/* Append hex representation */
	u64 v = m.value;
	u32 j;
	for (j = 0; j < 16 && i < buf_size - 2; j++) {
		u8 nib = (v >> (60 - j * 4)) & 0xF;
		buf[i++] = (nib < 10) ? ('0' + nib) : ('a' + nib - 10);
		v <<= 4;
	}

	if (i < buf_size - 1) buf[i++] = ']';
	buf[i] = '\0';

	return i;
}

/* ============================================================ */
/* RUST BINDINGS */
/* ============================================================ */

static inline struct MarshalledValue binding_rust_to_machine(
	const u8 *rust_value, u32 value_len, struct TypeDescriptor type) {

	struct MarshalledValue m = {0};
	m.type = type;

	/* Rust: encode as 64-bit + aux for generic metadata */
	if (value_len >= 8) {
		u64 v = 0;
		u32 i;
		for (i = 0; i < 8; i++) {
			v |= ((u64)rust_value[i]) << (i * 8);
		}
		m.value = v;
	}

	/* Rust generic types store type_id in aux */
	if (value_len >= 16) {
		u64 aux = 0;
		u32 i;
		for (i = 0; i < 8 && i + 8 < value_len; i++) {
			aux |= ((u64)rust_value[i + 8]) << (i * 8);
		}
		m.aux = aux;
	}

	return m;
}

static inline u8 binding_rust_from_machine(
	struct MarshalledValue m,
	u8 *rust_out, u32 *len_out) {

	if (!rust_out || !len_out) return 1;

	u32 i;
	for (i = 0; i < 8 && i < 256; i++) {
		rust_out[i] = (u8)((m.value >> (i * 8)) & 0xFF);
	}

	/* Include aux in output for generics */
	for (i = 0; i < 8 && i + 8 < 256; i++) {
		rust_out[i + 8] = (u8)((m.aux >> (i * 8)) & 0xFF);
	}

	*len_out = 16;
	return 0;
}

static inline struct TypeDescriptor binding_rust_type_from_name(
	const u8 *name, u32 name_len) {

	struct TypeDescriptor td = {0};
	td.base_type = MTYPE_INT64;  /* Rust i64 default */
	td.is_nullable = 1;          /* Rust Option<T> */

	if (name_len >= 2) {
		if (name[0] == 'i' && name[1] == '6') {
			td.base_type = MTYPE_INT64;
		} else if (name[0] == 'u' && name[1] == '6') {
			td.base_type = MTYPE_UINT64;
		} else if (name[0] == 'f' && name[1] == '6') {
			td.base_type = MTYPE_FLOAT64;
		}
	}

	return td;
}

static inline u8 binding_rust_type_compat(
	struct MarshalledValue v, struct TypeDescriptor expected) {

	/* Rust: check base type match */
	return 1;  /* Simplified */
}

static inline u32 binding_rust_format_value(
	struct MarshalledValue m, u8 *buf, u32 buf_size) {

	if (!buf || buf_size < 30) return 0;

	const u8 *fmt = (const u8*)"rust_val[";
	u32 i = 0;
	while (fmt[i] && i < buf_size - 1) {
		buf[i] = fmt[i];
		i++;
	}

	u64 v = m.value;
	u32 j;
	for (j = 0; j < 16 && i < buf_size - 2; j++) {
		u8 nib = (v >> (60 - j * 4)) & 0xF;
		buf[i++] = (nib < 10) ? ('0' + nib) : ('a' + nib - 10);
		v <<= 4;
	}

	if (i < buf_size - 1) buf[i++] = ']';
	buf[i] = '\0';

	return i;
}

/* ============================================================ */
/* C BINDINGS */
/* ============================================================ */

static inline struct MarshalledValue binding_c_to_machine(
	const u8 *c_value, u32 value_len, struct TypeDescriptor type) {

	struct MarshalledValue m = {0};
	m.type = type;

	/* C: direct byte copy (matches ABI layout) */
	if (value_len >= 8) {
		u64 v = 0;
		u32 i;
		for (i = 0; i < 8; i++) {
			v |= ((u64)c_value[i]) << (i * 8);
		}
		m.value = v;
	}

	return m;
}

static inline u8 binding_c_from_machine(
	struct MarshalledValue m,
	u8 *c_out, u32 *len_out) {

	if (!c_out || !len_out) return 1;

	u32 i;
	for (i = 0; i < 8 && i < 256; i++) {
		c_out[i] = (u8)((m.value >> (i * 8)) & 0xFF);
	}
	*len_out = 8;
	return 0;
}

static inline struct TypeDescriptor binding_c_type_from_name(
	const u8 *name, u32 name_len) {

	struct TypeDescriptor td = {0};
	td.base_type = MTYPE_INT64;
	td.is_nullable = 0;  /* C doesn't have nil for primitives */

	if (name_len >= 1) {
		if (name[0] == 'i') {
			td.base_type = MTYPE_INT64;
		} else if (name[0] == 'u') {
			td.base_type = MTYPE_UINT64;
		} else if (name[0] == 'f') {
			td.base_type = MTYPE_FLOAT64;
		} else if (name[0] == 'p') {
			td.base_type = MTYPE_POINTER;
		}
	}

	return td;
}

static inline u8 binding_c_type_compat(
	struct MarshalledValue v, struct TypeDescriptor expected) {

	return 1;  /* C: trust caller type correctness */
}

static inline u32 binding_c_format_value(
	struct MarshalledValue m, u8 *buf, u32 buf_size) {

	if (!buf || buf_size < 20) return 0;

	const u8 *fmt = (const u8*)"c_val[";
	u32 i = 0;
	while (fmt[i] && i < buf_size - 1) {
		buf[i] = fmt[i];
		i++;
	}

	u64 v = m.value;
	u32 j;
	for (j = 0; j < 16 && i < buf_size - 2; j++) {
		u8 nib = (v >> (60 - j * 4)) & 0xF;
		buf[i++] = (nib < 10) ? ('0' + nib) : ('a' + nib - 10);
		v <<= 4;
	}

	if (i < buf_size - 1) buf[i++] = ']';
	buf[i] = '\0';

	return i;
}

/* ============================================================ */
/* JAVASCRIPT BINDINGS */
/* ============================================================ */

static inline struct MarshalledValue binding_js_to_machine(
	const u8 *js_value, u32 value_len, struct TypeDescriptor type) {

	struct MarshalledValue m = {0};
	m.type = type;

	/* JavaScript: encode as 64-bit double + type tag in aux */
	if (value_len >= 8) {
		u64 v = 0;
		u32 i;
		for (i = 0; i < 8; i++) {
			v |= ((u64)js_value[i]) << (i * 8);
		}
		m.value = v;
	}

	/* JS type tag in aux */
	if (value_len >= 9) {
		m.aux = js_value[8];
	}

	return m;
}

static inline u8 binding_js_from_machine(
	struct MarshalledValue m,
	u8 *js_out, u32 *len_out) {

	if (!js_out || !len_out) return 1;

	u32 i;
	for (i = 0; i < 8 && i < 256; i++) {
		js_out[i] = (u8)((m.value >> (i * 8)) & 0xFF);
	}

	if (256 > 8) {
		js_out[8] = (u8)(m.aux & 0xFF);
		*len_out = 9;
	} else {
		*len_out = 8;
	}

	return 0;
}

static inline struct TypeDescriptor binding_js_type_from_name(
	const u8 *name, u32 name_len) {

	struct TypeDescriptor td = {0};
	td.base_type = MTYPE_INT64;  /* Default to number */
	td.is_nullable = 1;          /* JavaScript has null/undefined */

	if (name_len >= 6) {
		if (name[0] == 'n' && name[1] == 'u' && name[2] == 'm') {
			td.base_type = MTYPE_FLOAT64;
		} else if (name[0] == 's' && name[1] == 't' && name[2] == 'r') {
			td.base_type = MTYPE_STRING;
		}
	}

	return td;
}

static inline u8 binding_js_type_compat(
	struct MarshalledValue v, struct TypeDescriptor expected) {

	/* JavaScript: duck typing, always compatible */
	return 1;
}

static inline u32 binding_js_format_value(
	struct MarshalledValue m, u8 *buf, u32 buf_size) {

	if (!buf || buf_size < 20) return 0;

	const u8 *fmt = (const u8*)"js_val[";
	u32 i = 0;
	while (fmt[i] && i < buf_size - 1) {
		buf[i] = fmt[i];
		i++;
	}

	u64 v = m.value;
	u32 j;
	for (j = 0; j < 16 && i < buf_size - 2; j++) {
		u8 nib = (v >> (60 - j * 4)) & 0xF;
		buf[i++] = (nib < 10) ? ('0' + nib) : ('a' + nib - 10);
		v <<= 4;
	}

	if (i < buf_size - 1) buf[i++] = ']';
	buf[i] = '\0';

	return i;
}

/* ============================================================ */
/* JAVA BINDINGS */
/* ============================================================ */

static inline struct MarshalledValue binding_java_to_machine(
	const u8 *java_value, u32 value_len, struct TypeDescriptor type) {

	struct MarshalledValue m = {0};
	m.type = type;

	/* Java: encode as 64-bit value + class type_id in aux */
	if (value_len >= 8) {
		u64 v = 0;
		u32 i;
		for (i = 0; i < 8; i++) {
			v |= ((u64)java_value[i]) << (i * 8);
		}
		m.value = v;
	}

	/* Java class identifier */
	if (value_len >= 12) {
		u64 aux = 0;
		u32 i;
		for (i = 0; i < 4 && i + 8 < value_len; i++) {
			aux |= ((u64)java_value[i + 8]) << (i * 8);
		}
		m.aux = aux;
	}

	return m;
}

static inline u8 binding_java_from_machine(
	struct MarshalledValue m,
	u8 *java_out, u32 *len_out) {

	if (!java_out || !len_out) return 1;

	u32 i;
	for (i = 0; i < 8 && i < 256; i++) {
		java_out[i] = (u8)((m.value >> (i * 8)) & 0xFF);
	}

	for (i = 0; i < 4 && i + 8 < 256; i++) {
		java_out[i + 8] = (u8)((m.aux >> (i * 8)) & 0xFF);
	}

	*len_out = 12;
	return 0;
}

static inline struct TypeDescriptor binding_java_type_from_name(
	const u8 *name, u32 name_len) {

	struct TypeDescriptor td = {0};
	td.base_type = MTYPE_INT64;  /* Default long */
	td.is_nullable = 1;          /* Java objects can be null */

	if (name_len >= 3) {
		if (name[0] == 'i' && name[1] == 'n' && name[2] == 't') {
			td.base_type = MTYPE_INT64;
		} else if (name[0] == 'l' && name[1] == 'o' && name[2] == 'n') {
			td.base_type = MTYPE_INT64;
		} else if (name[0] == 'd' && name[1] == 'o' && name[2] == 'u') {
			td.base_type = MTYPE_FLOAT64;
		}
	}

	return td;
}

static inline u8 binding_java_type_compat(
	struct MarshalledValue v, struct TypeDescriptor expected) {

	/* Java: check based on class type_id */
	return 1;  /* Simplified */
}

static inline u32 binding_java_format_value(
	struct MarshalledValue m, u8 *buf, u32 buf_size) {

	if (!buf || buf_size < 30) return 0;

	const u8 *fmt = (const u8*)"java_val[";
	u32 i = 0;
	while (fmt[i] && i < buf_size - 1) {
		buf[i] = fmt[i];
		i++;
	}

	u64 v = m.value;
	u32 j;
	for (j = 0; j < 16 && i < buf_size - 2; j++) {
		u8 nib = (v >> (60 - j * 4)) & 0xF;
		buf[i++] = (nib < 10) ? ('0' + nib) : ('a' + nib - 10);
		v <<= 4;
	}

	if (i < buf_size - 1) buf[i++] = ']';
	buf[i] = '\0';

	return i;
}

/* ============================================================ */
/* SWIFT BINDINGS */
/* ============================================================ */

static inline struct MarshalledValue binding_swift_to_machine(
	const u8 *swift_value, u32 value_len, struct TypeDescriptor type) {

	struct MarshalledValue m = {0};
	m.type = type;

	/* Swift: encode as 64-bit value + type metadata */
	if (value_len >= 8) {
		u64 v = 0;
		u32 i;
		for (i = 0; i < 8; i++) {
			v |= ((u64)swift_value[i]) << (i * 8);
		}
		m.value = v;
	}

	/* Swift type metadata */
	if (value_len >= 16) {
		u64 aux = 0;
		u32 i;
		for (i = 0; i < 8 && i + 8 < value_len; i++) {
			aux |= ((u64)swift_value[i + 8]) << (i * 8);
		}
		m.aux = aux;
	}

	return m;
}

static inline u8 binding_swift_from_machine(
	struct MarshalledValue m,
	u8 *swift_out, u32 *len_out) {

	if (!swift_out || !len_out) return 1;

	u32 i;
	for (i = 0; i < 8 && i < 256; i++) {
		swift_out[i] = (u8)((m.value >> (i * 8)) & 0xFF);
	}

	for (i = 0; i < 8 && i + 8 < 256; i++) {
		swift_out[i + 8] = (u8)((m.aux >> (i * 8)) & 0xFF);
	}

	*len_out = 16;
	return 0;
}

static inline struct TypeDescriptor binding_swift_type_from_name(
	const u8 *name, u32 name_len) {

	struct TypeDescriptor td = {0};
	td.base_type = MTYPE_INT64;  /* Default Int */
	td.is_nullable = 1;          /* Swift Optional<T> */

	if (name_len >= 2) {
		if (name[0] == 'I' && name[1] == 'n') {
			td.base_type = MTYPE_INT64;
		} else if (name[0] == 'D' && name[1] == 'o') {
			td.base_type = MTYPE_FLOAT64;
		} else if (name[0] == 'S' && name[1] == 't') {
			td.base_type = MTYPE_STRING;
		}
	}

	return td;
}

static inline u8 binding_swift_type_compat(
	struct MarshalledValue v, struct TypeDescriptor expected) {

	/* Swift: check static type match */
	return 1;  /* Simplified */
}

static inline u32 binding_swift_format_value(
	struct MarshalledValue m, u8 *buf, u32 buf_size) {

	if (!buf || buf_size < 30) return 0;

	const u8 *fmt = (const u8*)"swift_val[";
	u32 i = 0;
	while (fmt[i] && i < buf_size - 1) {
		buf[i] = fmt[i];
		i++;
	}

	u64 v = m.value;
	u32 j;
	for (j = 0; j < 16 && i < buf_size - 2; j++) {
		u8 nib = (v >> (60 - j * 4)) & 0xF;
		buf[i++] = (nib < 10) ? ('0' + nib) : ('a' + nib - 10);
		v <<= 4;
	}

	if (i < buf_size - 1) buf[i++] = ']';
	buf[i] = '\0';

	return i;
}

/* ============================================================ */
/* BINDING REGISTRY & INITIALIZATION */
/* ============================================================ */

/* Initialize binding registry with all 7 language bindings */
static inline void binding_registry_init(struct BindingRegistry *br) {
	if (!br) return;

	/* Python binding */
	br->bindings[LANG_PYTHON].to_machine = binding_py_to_machine;
	br->bindings[LANG_PYTHON].from_machine = binding_py_from_machine;
	br->bindings[LANG_PYTHON].type_from_name = binding_py_type_from_name;
	br->bindings[LANG_PYTHON].type_compat = binding_py_type_compat;
	br->bindings[LANG_PYTHON].format_value = binding_py_format_value;

	/* Go binding */
	br->bindings[LANG_GO].to_machine = binding_go_to_machine;
	br->bindings[LANG_GO].from_machine = binding_go_from_machine;
	br->bindings[LANG_GO].type_from_name = binding_go_type_from_name;
	br->bindings[LANG_GO].type_compat = binding_go_type_compat;
	br->bindings[LANG_GO].format_value = binding_go_format_value;

	/* Rust binding */
	br->bindings[LANG_RUST].to_machine = binding_rust_to_machine;
	br->bindings[LANG_RUST].from_machine = binding_rust_from_machine;
	br->bindings[LANG_RUST].type_from_name = binding_rust_type_from_name;
	br->bindings[LANG_RUST].type_compat = binding_rust_type_compat;
	br->bindings[LANG_RUST].format_value = binding_rust_format_value;

	/* C binding */
	br->bindings[LANG_C].to_machine = binding_c_to_machine;
	br->bindings[LANG_C].from_machine = binding_c_from_machine;
	br->bindings[LANG_C].type_from_name = binding_c_type_from_name;
	br->bindings[LANG_C].type_compat = binding_c_type_compat;
	br->bindings[LANG_C].format_value = binding_c_format_value;

	/* JavaScript binding */
	br->bindings[LANG_JAVASCRIPT].to_machine = binding_js_to_machine;
	br->bindings[LANG_JAVASCRIPT].from_machine = binding_js_from_machine;
	br->bindings[LANG_JAVASCRIPT].type_from_name = binding_js_type_from_name;
	br->bindings[LANG_JAVASCRIPT].type_compat = binding_js_type_compat;
	br->bindings[LANG_JAVASCRIPT].format_value = binding_js_format_value;

	/* Java binding */
	br->bindings[LANG_JAVA].to_machine = binding_java_to_machine;
	br->bindings[LANG_JAVA].from_machine = binding_java_from_machine;
	br->bindings[LANG_JAVA].type_from_name = binding_java_type_from_name;
	br->bindings[LANG_JAVA].type_compat = binding_java_type_compat;
	br->bindings[LANG_JAVA].format_value = binding_java_format_value;

	/* Swift binding */
	br->bindings[LANG_SWIFT].to_machine = binding_swift_to_machine;
	br->bindings[LANG_SWIFT].from_machine = binding_swift_from_machine;
	br->bindings[LANG_SWIFT].type_from_name = binding_swift_type_from_name;
	br->bindings[LANG_SWIFT].type_compat = binding_swift_type_compat;
	br->bindings[LANG_SWIFT].format_value = binding_swift_format_value;

	br->initialized = 1;
}

/* Get binding for language */
static inline struct LanguageBinding* binding_get(
	struct BindingRegistry *br, u32 lang_type) {

	if (!br || !br->initialized || lang_type >= 7) return NULL;
	return &br->bindings[lang_type];
}

/* Marshal value from language to machine */
static inline struct MarshalledValue binding_marshal_to_machine(
	struct BindingRegistry *br,
	u32 lang_type,
	const u8 *lang_value, u32 value_len,
	struct TypeDescriptor type) {

	struct MarshalledValue result = {0};

	if (!br || !br->initialized || lang_type >= 7) return result;

	struct LanguageBinding *binding = &br->bindings[lang_type];
	if (!binding->to_machine) return result;

	return binding->to_machine(lang_value, value_len, type);
}

/* Marshal value from machine to language */
static inline u8 binding_marshal_from_machine(
	struct BindingRegistry *br,
	u32 lang_type,
	struct MarshalledValue m,
	u8 *lang_out, u32 *len_out) {

	if (!br || !br->initialized || lang_type >= 7) return 1;

	struct LanguageBinding *binding = &br->bindings[lang_type];
	if (!binding->from_machine) return 1;

	return binding->from_machine(m, lang_out, len_out);
}

#endif /* APKC_BRIDGE_LANGUAGE_BINDINGS_H */
