/* debug_symbol_manager.h — Debug Symbol Management (Stage 16.1)
 *
 * Symbol table generation: map source locations to machine instructions.
 * DWARF metadata embedding: include debugging info in compiled binaries.
 * Source line mapping: precise file/line/column information for each opcode.
 * Symbol lookup: retrieve source location for any instruction address.
 * Debug info versioning: track debug format and compatibility.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_DEBUG_SYMBOL_MANAGER_H
#define APKC_DEBUG_SYMBOL_MANAGER_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Symbol type classification */
enum SymbolType {
	SYMBOL_FUNCTION = 0,        /* Function symbol */
	SYMBOL_VARIABLE = 1,        /* Variable (local or global) */
	SYMBOL_LABEL = 2,           /* Assembly label */
	SYMBOL_CONSTANT = 3,        /* Named constant */
	SYMBOL_TYPE = 4             /* Type definition */
};

/* Source location */
struct SourceLocation {
	const char *file_path;      /* Source file path */
	u32 line_number;            /* Line number (1-based) */
	u32 column_number;          /* Column number (1-based) */
	u32 end_line;               /* End line for ranges */
	u32 end_column;             /* End column for ranges */
};

/* Debug symbol entry */
struct DebugSymbol {
	const char *symbol_name;    /* Function/variable name */
	u8 symbol_type;             /* SymbolType */
	u64 address;                /* Memory address or code offset */
	u32 size;                   /* Size in bytes (for functions/data) */
	struct SourceLocation loc;  /* Where this symbol is defined */
	u32 scope_depth;            /* Nesting depth (0 = global, 1+ = local) */
	u8 is_optimized;            /* 1 if inlined or optimized */
};

/* Line mapping entry */
struct LineMapEntry {
	u64 instruction_offset;     /* Offset in compiled code */
	struct SourceLocation loc;  /* Source location for this instruction */
	u8 is_statement_start;      /* 1 if statement begins here */
	u8 is_basic_block_start;    /* 1 if basic block begins here */
};

/* Debug information manager */
struct DebugInfo {
	struct DebugSymbol symbols[128];    /* Up to 128 symbols */
	u32 symbol_count;
	struct LineMapEntry lines[512];    /* Up to 512 line mappings */
	u32 line_count;
	const char *source_dir;             /* Root source directory */
	const char *build_id;               /* Build identifier (hash) */
	u32 dwarf_version;                  /* DWARF version (3, 4, or 5) */
	u8 has_line_info;                   /* 1 if line table present */
	u8 has_abbrev_info;                 /* 1 if abbreviations present */
};

/* ============================================================ */
/* DEBUG INFO INITIALIZATION */
/* ============================================================ */

/* Initialize debug information manager */
static inline void debug_init(
	struct DebugInfo *info,
	const char *source_dir,
	const char *build_id,
	u32 dwarf_version) {

	if (!info) return;
	info->symbol_count = 0;
	info->line_count = 0;
	info->source_dir = source_dir;
	info->build_id = build_id;
	info->dwarf_version = (dwarf_version > 5) ? 5 : dwarf_version;
	info->has_line_info = 0;
	info->has_abbrev_info = 0;
}

/* ============================================================ */
/* SYMBOL REGISTRATION */
/* ============================================================ */

/* Register debug symbol */
static inline u8 debug_add_symbol(
	struct DebugInfo *info,
	const char *name,
	u8 symbol_type,
	u64 address,
	u32 size,
	const char *file,
	u32 line,
	u32 column) {

	if (!info || !name || !file) return 0;
	if (info->symbol_count >= 128) return 0;

	struct DebugSymbol *sym = &info->symbols[info->symbol_count];
	sym->symbol_name = name;
	sym->symbol_type = symbol_type;
	sym->address = address;
	sym->size = size;
	sym->loc.file_path = file;
	sym->loc.line_number = line;
	sym->loc.column_number = column;
	sym->loc.end_line = line;
	sym->loc.end_column = column + 1;
	sym->scope_depth = 0;
	sym->is_optimized = 0;

	info->symbol_count++;
	return 1;
}

/* Add local variable symbol */
static inline u8 debug_add_local_variable(
	struct DebugInfo *info,
	const char *var_name,
	u64 stack_offset,
	const char *file,
	u32 line,
	u32 scope_depth) {

	if (!info || !var_name || !file) return 0;
	if (info->symbol_count >= 128) return 0;

	struct DebugSymbol *sym = &info->symbols[info->symbol_count];
	sym->symbol_name = var_name;
	sym->symbol_type = SYMBOL_VARIABLE;
	sym->address = stack_offset;
	sym->size = 8;  /* Default to 64-bit */
	sym->loc.file_path = file;
	sym->loc.line_number = line;
	sym->loc.column_number = 0;
	sym->scope_depth = scope_depth;
	sym->is_optimized = 0;

	info->symbol_count++;
	return 1;
}

/* ============================================================ */
/* LINE MAPPING */
/* ============================================================ */

/* Add line mapping entry */
static inline u8 debug_add_line_map(
	struct DebugInfo *info,
	u64 instruction_offset,
	const char *file,
	u32 line,
	u32 column,
	u8 is_stmt_start) {

	if (!info || !file) return 0;
	if (info->line_count >= 512) return 0;

	struct LineMapEntry *entry = &info->lines[info->line_count];
	entry->instruction_offset = instruction_offset;
	entry->loc.file_path = file;
	entry->loc.line_number = line;
	entry->loc.column_number = column;
	entry->loc.end_line = line;
	entry->loc.end_column = column + 1;
	entry->is_statement_start = is_stmt_start;
	entry->is_basic_block_start = 0;

	info->line_count++;
	info->has_line_info = 1;

	return 1;
}

/* ============================================================ */
/* SYMBOL & SOURCE LOOKUP */
/* ============================================================ */

/* Lookup symbol by name */
static inline struct DebugSymbol *debug_lookup_symbol(
	struct DebugInfo *info,
	const char *symbol_name) {

	if (!info || !symbol_name) return 0;

	u32 i;
	for (i = 0; i < info->symbol_count; i++) {
		if (!info->symbols[i].symbol_name) continue;

		const char *sym_name = info->symbols[i].symbol_name;
		u32 j = 0;
		while (symbol_name[j] && sym_name[j] && symbol_name[j] == sym_name[j]) j++;

		if (symbol_name[j] == 0 && sym_name[j] == 0) {
			return &info->symbols[i];
		}
	}

	return 0;
}

/* Lookup symbol by address */
static inline struct DebugSymbol *debug_lookup_by_address(
	struct DebugInfo *info,
	u64 address) {

	if (!info) return 0;

	u32 i;
	for (i = 0; i < info->symbol_count; i++) {
		if (!info->symbols[i].symbol_name) continue;

		/* Check if address is within symbol's range */
		u64 sym_start = info->symbols[i].address;
		u64 sym_end = sym_start + info->symbols[i].size;

		if (address >= sym_start && address < sym_end) {
			return &info->symbols[i];
		}
	}

	return 0;
}

/* Lookup source location by instruction offset */
static inline struct SourceLocation *debug_lookup_source_by_offset(
	struct DebugInfo *info,
	u64 instruction_offset) {

	if (!info) return 0;

	struct LineMapEntry *best = 0;
	u64 best_offset = 0;

	u32 i;
	for (i = 0; i < info->line_count; i++) {
		if (info->lines[i].instruction_offset <= instruction_offset &&
			info->lines[i].instruction_offset > best_offset) {
			best = &info->lines[i];
			best_offset = info->lines[i].instruction_offset;
		}
	}

	return best ? &best->loc : 0;
}

/* ============================================================ */
/* DEBUG INFO STATISTICS */
/* ============================================================ */

/* Get symbol count by type */
static inline u32 debug_count_symbols_by_type(
	struct DebugInfo *info,
	u8 symbol_type) {

	if (!info) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < info->symbol_count; i++) {
		if (info->symbols[i].symbol_type == symbol_type) {
			count++;
		}
	}

	return count;
}

/* Get number of local variables at given scope */
static inline u32 debug_count_locals_at_scope(
	struct DebugInfo *info,
	u32 scope_depth) {

	if (!info) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < info->symbol_count; i++) {
		if (info->symbols[i].symbol_type == SYMBOL_VARIABLE &&
			info->symbols[i].scope_depth == scope_depth) {
			count++;
		}
	}

	return count;
}

/* Get coverage of source code (how many lines are mapped) */
static inline u32 debug_get_source_coverage_percent(struct DebugInfo *info) {
	if (!info || info->line_count == 0) return 0;

	/* Simplified: assume higher line_count means more comprehensive coverage */
	return (info->line_count > 256) ? 100 : (info->line_count * 100) / 256;
}

#endif /* APKC_DEBUG_SYMBOL_MANAGER_H */
