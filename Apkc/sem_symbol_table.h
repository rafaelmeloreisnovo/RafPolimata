/* sem_symbol_table.h — Symbol Table & Registry (Phase 22.1)
 *
 * Symbol storage: identifiers with types and attributes
 * Symbol lookup: name-to-definition binding
 * Visibility control: public/private/internal symbols
 * Symbol attributes: kind, type, mutability, scope level
 * Namespace management: qualified names and module paths
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEM_SYMBOL_TABLE_H
#define APKC_SEM_SYMBOL_TABLE_H 1

#include "sem_type_system.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* SYMBOL KIND */
/* ============================================================ */

enum SymbolKind {
	SYM_UNKNOWN = 0,
	SYM_VARIABLE = 1,
	SYM_FUNCTION = 2,
	SYM_TYPE = 3,
	SYM_MODULE = 4,
	SYM_CONSTANT = 5,
	SYM_PARAMETER = 6,
	SYM_STRUCT_FIELD = 7,
	SYM_ENUM_VARIANT = 8,
	SYM_CLASS = 9,
	SYM_TRAIT = 10,
	SYM_IMPL = 11,
	SYM_GENERIC = 12,
	SYM_BUILTIN = 13
};

/* ============================================================ */
/* VISIBILITY & ATTRIBUTES */
/* ============================================================ */

enum Visibility {
	VIS_PRIVATE = 0,
	VIS_INTERNAL = 1,
	VIS_PUBLIC = 2
};

struct SymbolAttributes {
	u8 is_mutable;
	u8 is_exported;
	u8 is_imported;
	u8 is_generic;
	u8 visibility;
	u32 definition_line;
	const char *definition_file;
};

/* ============================================================ */
/* SYMBOL ENTRY */
/* ============================================================ */

struct Symbol {
	const char *name;
	u8 kind;
	struct Type *type;
	struct SymbolAttributes attrs;
	u32 scope_level;
	const char *qualified_name;  /* module::name */
	u32 id;  /* Unique symbol ID */
};

/* ============================================================ */
/* SYMBOL TABLE */
/* ============================================================ */

struct SymbolTable {
	struct Symbol entries[512];  /* Max 512 symbols */
	u32 count;
	u32 next_id;
};

/* ============================================================ */
/* SYMBOL TABLE OPERATIONS */
/* ============================================================ */

static inline void symbol_table_init(struct SymbolTable *table) {
	if (!table) return;
	table->count = 0;
	table->next_id = 0;
}

static inline u8 symbol_table_add(
	struct SymbolTable *table,
	const char *name,
	u8 kind,
	struct Type *type,
	u32 scope_level) {

	if (!table || !name || table->count >= 512) return 1;

	struct Symbol *sym = &table->entries[table->count];
	sym->name = name;
	sym->kind = kind;
	sym->type = type;
	sym->scope_level = scope_level;
	sym->id = table->next_id++;
	sym->attrs.is_mutable = 0;
	sym->attrs.is_exported = 0;
	sym->attrs.is_imported = 0;
	sym->attrs.is_generic = 0;
	sym->attrs.visibility = VIS_PRIVATE;
	sym->attrs.definition_line = 0;
	sym->attrs.definition_file = 0;
	sym->qualified_name = name;

	table->count++;
	return 0;
}

static inline struct Symbol *symbol_table_lookup(
	struct SymbolTable *table,
	const char *name) {

	if (!table || !name) return 0;

	u32 i;
	for (i = 0; i < table->count; i++) {
		const char *sym_name = table->entries[i].name;
		const char *lookup_name = name;
		u32 j = 0;
		while (sym_name[j] && lookup_name[j] && sym_name[j] == lookup_name[j]) j++;
		if (sym_name[j] == 0 && lookup_name[j] == 0) {
			return &table->entries[i];
		}
	}
	return 0;
}

static inline struct Symbol *symbol_table_lookup_by_id(
	struct SymbolTable *table,
	u32 id) {

	if (!table) return 0;

	u32 i;
	for (i = 0; i < table->count; i++) {
		if (table->entries[i].id == id) {
			return &table->entries[i];
		}
	}
	return 0;
}

static inline u8 symbol_table_set_visibility(
	struct SymbolTable *table,
	const char *name,
	u8 visibility) {

	struct Symbol *sym = symbol_table_lookup(table, name);
	if (!sym) return 1;
	sym->attrs.visibility = visibility;
	return 0;
}

static inline u8 symbol_table_set_mutable(
	struct SymbolTable *table,
	const char *name,
	u8 is_mutable) {

	struct Symbol *sym = symbol_table_lookup(table, name);
	if (!sym) return 1;
	sym->attrs.is_mutable = is_mutable;
	return 0;
}

/* ============================================================ */
/* SYMBOL LOOKUP BY KIND */
/* ============================================================ */

static inline u32 symbol_table_count_by_kind(
	struct SymbolTable *table,
	u8 kind) {

	if (!table) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < table->count; i++) {
		if (table->entries[i].kind == kind) {
			count++;
		}
	}
	return count;
}

static inline struct Symbol *symbol_table_lookup_by_kind(
	struct SymbolTable *table,
	const char *name,
	u8 kind) {

	if (!table || !name) return 0;

	u32 i;
	for (i = 0; i < table->count; i++) {
		if (table->entries[i].kind == kind) {
			const char *sym_name = table->entries[i].name;
			const char *lookup_name = name;
			u32 j = 0;
			while (sym_name[j] && lookup_name[j] && sym_name[j] == lookup_name[j]) j++;
			if (sym_name[j] == 0 && lookup_name[j] == 0) {
				return &table->entries[i];
			}
		}
	}
	return 0;
}

#endif /* APKC_SEM_SYMBOL_TABLE_H */
