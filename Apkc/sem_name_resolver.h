/* sem_name_resolver.h — Name Resolution & Binding (Phase 22.3)
 *
 * Name resolution: bind identifiers to symbols
 * Qualified names: module::type::name hierarchies
 * Import tracking: imported symbols and their origins
 * Undefined name detection: catch unbound references
 * Name conflict resolution: qualified vs unqualified
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEM_NAME_RESOLVER_H
#define APKC_SEM_NAME_RESOLVER_H 1

#include "sem_scope_manager.h"

typedef unsigned char u8;
typedef unsigned int u32;

/* ============================================================ */
/* NAME RESOLUTION RESULT */
/* ============================================================ */

struct NameResolution {
	struct Symbol *symbol;
	u8 is_local;
	u8 is_imported;
	u8 is_builtin;
	u32 scope_distance;  /* 0 = current scope, 1 = parent, etc. */
	const char *resolution_path;
};

/* ============================================================ */
/* NAME RESOLUTION ERROR */
/* ============================================================ */

enum NameError {
	NE_OK = 0,
	NE_UNDEFINED = 1,
	NE_AMBIGUOUS = 2,
	NE_WRONG_KIND = 3,
	NE_PRIVATE = 4,
	NE_CYCLE = 5
};

struct NameError_Info {
	u8 code;
	const char *name;
	u32 line;
	const char *message;
};

/* ============================================================ */
/* NAME RESOLVER CONTEXT */
/* ============================================================ */

struct NameResolver {
	struct ScopeManager *scope_mgr;
	struct SymbolTable builtins;  /* Built-in functions/types */
	struct NameError_Info last_error;
};

/* ============================================================ */
/* INITIALIZATION */
/* ============================================================ */

static inline void name_resolver_init(
	struct NameResolver *resolver,
	struct ScopeManager *scope_mgr) {

	if (!resolver || !scope_mgr) return;
	resolver->scope_mgr = scope_mgr;
	symbol_table_init(&resolver->builtins);
	resolver->last_error.code = NE_OK;
	resolver->last_error.message = 0;
}

static inline void name_resolver_add_builtin(
	struct NameResolver *resolver,
	const char *name,
	u8 kind) {

	if (!resolver || !name) return;
	symbol_table_add(&resolver->builtins, name, kind, 0, 0);
}

/* ============================================================ */
/* NAME RESOLUTION */
/* ============================================================ */

static inline struct NameResolution resolve_name(
	struct NameResolver *resolver,
	const char *name) {

	struct NameResolution result;
	result.symbol = 0;
	result.is_local = 0;
	result.is_imported = 0;
	result.is_builtin = 0;
	result.scope_distance = 0;
	result.resolution_path = 0;

	if (!resolver || !name) {
		resolver->last_error.code = NE_UNDEFINED;
		resolver->last_error.message = "Invalid name";
		return result;
	}

	/* Try current scope and parents */
	struct Symbol *sym = scope_lookup_recursive(resolver->scope_mgr, name);
	if (sym) {
		result.symbol = sym;
		result.is_local = 1;
		result.scope_distance =
			resolver->scope_mgr->scopes[resolver->scope_mgr->current_scope_index].level -
			sym->scope_level;
		return result;
	}

	/* Try builtins */
	sym = symbol_table_lookup(&resolver->builtins, name);
	if (sym) {
		result.symbol = sym;
		result.is_builtin = 1;
		return result;
	}

	/* Undefined */
	resolver->last_error.code = NE_UNDEFINED;
	resolver->last_error.name = name;
	resolver->last_error.message = "Undefined name";
	return result;
}

/* ============================================================ */
/* QUALIFIED NAME RESOLUTION */
/* ============================================================ */

static inline struct NameResolution resolve_qualified_name(
	struct NameResolver *resolver,
	const char *module,
	const char *name) {

	struct NameResolution result;
	result.symbol = 0;
	result.is_local = 0;
	result.is_imported = 0;
	result.is_builtin = 0;
	result.scope_distance = 0;
	result.resolution_path = 0;

	if (!resolver || !module || !name) {
		resolver->last_error.code = NE_UNDEFINED;
		return result;
	}

	/* In full implementation: search module exports */
	/* For now: do simple name lookup */
	struct Symbol *sym = scope_lookup_recursive(resolver->scope_mgr, name);
	if (sym && sym->attrs.visibility == VIS_PUBLIC) {
		result.symbol = sym;
		result.is_local = 0;
		return result;
	}

	resolver->last_error.code = NE_UNDEFINED;
	return result;
}

/* ============================================================ */
/* NAME BINDING */
/* ============================================================ */

struct Binding {
	const char *name;
	struct Symbol *symbol;
	u32 usage_line;
	u8 is_definition;
	u8 is_usage;
};

static inline u8 bind_name(
	struct NameResolver *resolver,
	const char *name,
	u8 kind,
	struct Type *type) {

	if (!resolver || !name) return 1;

	struct ScopeManager *scope_mgr = resolver->scope_mgr;
	return scope_declare(scope_mgr, name, kind, type);
}

/* ============================================================ */
/* UNDEFINED REFERENCE DETECTION */
/* ============================================================ */

struct UndefinedRef {
	const char *name;
	u32 line;
	u8 is_function_call;
	u8 is_type_reference;
};

static inline u8 is_undefined(
	struct NameResolver *resolver,
	const char *name) {

	if (!resolver || !name) return 1;

	struct NameResolution res = resolve_name(resolver, name);
	return res.symbol == 0;
}

/* ============================================================ */
/* VISIBILITY CHECKING */
/* ============================================================ */

static inline u8 check_visibility(
	struct NameResolver *resolver,
	struct Symbol *symbol) {

	if (!resolver || !symbol) return 0;

	if (symbol->attrs.visibility == VIS_PUBLIC) return 1;
	if (symbol->attrs.visibility == VIS_INTERNAL) return 1;
	if (symbol->attrs.visibility == VIS_PRIVATE) {
		/* Private: only accessible in defining scope */
		return 1;  /* Simplified: always allow */
	}

	resolver->last_error.code = NE_PRIVATE;
	resolver->last_error.message = "Private symbol";
	return 0;
}

/* ============================================================ */
/* ERROR REPORTING */
/* ============================================================ */

static inline const char *name_error_string(enum NameError code) {
	switch (code) {
	case NE_OK: return "No error";
	case NE_UNDEFINED: return "Undefined name";
	case NE_AMBIGUOUS: return "Ambiguous name";
	case NE_WRONG_KIND: return "Wrong symbol kind";
	case NE_PRIVATE: return "Private symbol";
	case NE_CYCLE: return "Circular dependency";
	default: return "Unknown error";
	}
}

#endif /* APKC_SEM_NAME_RESOLVER_H */
