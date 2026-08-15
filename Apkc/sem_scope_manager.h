/* sem_scope_manager.h — Scope Chain Management (Phase 22.2)
 *
 * Scope stack: nested scopes with parent pointers
 * Scope entering: push new scope on declaration
 * Scope exiting: pop scope and validate references
 * Variable shadowing: detect and track hidden symbols
 * Scope depth tracking: nesting level monitoring
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEM_SCOPE_MANAGER_H
#define APKC_SEM_SCOPE_MANAGER_H 1

#include "sem_symbol_table.h"

typedef unsigned char u8;
typedef unsigned int u32;

/* ============================================================ */
/* SCOPE ENTRY */
/* ============================================================ */

struct Scope {
	struct SymbolTable symbols;
	u32 level;
	u32 parent_index;  /* Index in scope stack, or -1 for root */
	const char *scope_name;  /* e.g., "function_foo", "block_3" */
	u8 is_function_scope;
	u8 is_class_scope;
	u8 is_module_scope;
};

/* ============================================================ */
/* SCOPE MANAGER */
/* ============================================================ */

struct ScopeManager {
	struct Scope scopes[32];  /* Max 32 nested scopes */
	u32 scope_count;
	u32 current_scope_index;
};

/* ============================================================ */
/* INITIALIZATION */
/* ============================================================ */

static inline void scope_manager_init(struct ScopeManager *mgr) {
	if (!mgr) return;
	mgr->scope_count = 0;
	mgr->current_scope_index = 0;

	/* Create root scope */
	struct Scope *root = &mgr->scopes[0];
	symbol_table_init(&root->symbols);
	root->level = 0;
	root->parent_index = 0xFFFFFFFFU;  /* No parent */
	root->scope_name = "global";
	root->is_function_scope = 0;
	root->is_class_scope = 0;
	root->is_module_scope = 1;

	mgr->scope_count = 1;
}

/* ============================================================ */
/* SCOPE OPERATIONS */
/* ============================================================ */

static inline u8 scope_push(
	struct ScopeManager *mgr,
	const char *scope_name,
	u8 is_function) {

	if (!mgr || mgr->scope_count >= 32) return 1;

	u32 current = mgr->current_scope_index;
	u32 new_index = mgr->scope_count;

	struct Scope *new_scope = &mgr->scopes[new_index];
	symbol_table_init(&new_scope->symbols);
	new_scope->level = mgr->scopes[current].level + 1;
	new_scope->parent_index = current;
	new_scope->scope_name = scope_name;
	new_scope->is_function_scope = is_function;
	new_scope->is_class_scope = 0;
	new_scope->is_module_scope = 0;

	mgr->scope_count++;
	mgr->current_scope_index = new_index;
	return 0;
}

static inline u8 scope_pop(struct ScopeManager *mgr) {
	if (!mgr || mgr->current_scope_index == 0) return 1;

	u32 current = mgr->current_scope_index;
	struct Scope *current_scope = &mgr->scopes[current];
	mgr->current_scope_index = current_scope->parent_index;
	return 0;
}

static inline u32 scope_get_level(struct ScopeManager *mgr) {
	if (!mgr || mgr->current_scope_index >= mgr->scope_count) return 0;
	return mgr->scopes[mgr->current_scope_index].level;
}

static inline const char *scope_get_name(struct ScopeManager *mgr) {
	if (!mgr || mgr->current_scope_index >= mgr->scope_count) return "?";
	return mgr->scopes[mgr->current_scope_index].scope_name;
}

/* ============================================================ */
/* SYMBOL LOOKUP IN SCOPE CHAIN */
/* ============================================================ */

static inline struct Symbol *scope_lookup_local(
	struct ScopeManager *mgr,
	const char *name) {

	if (!mgr || !name) return 0;

	struct Scope *scope = &mgr->scopes[mgr->current_scope_index];
	return symbol_table_lookup(&scope->symbols, name);
}

static inline struct Symbol *scope_lookup_recursive(
	struct ScopeManager *mgr,
	const char *name) {

	if (!mgr || !name) return 0;

	u32 current = mgr->current_scope_index;

	while (1) {
		struct Scope *scope = &mgr->scopes[current];
		struct Symbol *sym = symbol_table_lookup(&scope->symbols, name);
		if (sym) return sym;

		if (scope->parent_index == 0xFFFFFFFFU) {
			/* Reached root scope */
			break;
		}
		current = scope->parent_index;
	}

	return 0;
}

/* ============================================================ */
/* SYMBOL DECLARATION IN CURRENT SCOPE */
/* ============================================================ */

static inline u8 scope_declare(
	struct ScopeManager *mgr,
	const char *name,
	u8 kind,
	struct Type *type) {

	if (!mgr || !name) return 1;

	/* Check for redeclaration in current scope */
	struct Symbol *existing = scope_lookup_local(mgr, name);
	if (existing) return 1;  /* Already declared in this scope */

	struct Scope *scope = &mgr->scopes[mgr->current_scope_index];
	return symbol_table_add(&scope->symbols, name, kind, type, scope->level);
}

/* ============================================================ */
/* SHADOWING DETECTION */
/* ============================================================ */

struct ShadowInfo {
	u8 is_shadowed;
	u32 outer_scope_level;
	const char *outer_symbol_name;
};

static inline struct ShadowInfo scope_check_shadowing(
	struct ScopeManager *mgr,
	const char *name) {

	struct ShadowInfo info;
	info.is_shadowed = 0;
	info.outer_scope_level = 0;
	info.outer_symbol_name = 0;

	if (!mgr || !name) return info;

	u32 current = mgr->current_scope_index;
	u32 first_scope_level = mgr->scopes[current].level;

	/* Check parent scopes */
	while (1) {
		struct Scope *scope = &mgr->scopes[current];

		if (scope->level < first_scope_level) {
			struct Symbol *outer_sym = symbol_table_lookup(&scope->symbols, name);
			if (outer_sym) {
				info.is_shadowed = 1;
				info.outer_scope_level = scope->level;
				info.outer_symbol_name = outer_sym->name;
				return info;
			}
		}

		if (scope->parent_index == 0xFFFFFFFFU) break;
		current = scope->parent_index;
	}

	return info;
}

/* ============================================================ */
/* SCOPE STATISTICS */
/* ============================================================ */

static inline u32 scope_count_symbols(struct ScopeManager *mgr) {
	if (!mgr) return 0;
	struct Scope *scope = &mgr->scopes[mgr->current_scope_index];
	return scope->symbols.count;
}

static inline u32 scope_count_symbols_recursive(struct ScopeManager *mgr) {
	if (!mgr) return 0;

	u32 total = 0;
	u32 i;
	for (i = 0; i < mgr->scope_count; i++) {
		total += mgr->scopes[i].symbols.count;
	}
	return total;
}

#endif /* APKC_SEM_SCOPE_MANAGER_H */
