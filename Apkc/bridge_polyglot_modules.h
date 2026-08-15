/* bridge_polyglot_modules.h — Polyglot Module System (Stage 8.3)
 *
 * Multi-language module metadata tracking.
 * Cross-module symbol resolution (find symbols across language boundaries).
 * Language-aware loading order (topological sort by dependencies).
 * Polyglot module lifetime management (init/finalize with language-specific callbacks).
 * Max 64 modules, max 256 cross-language imports.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_BRIDGE_POLYGLOT_MODULES_H
#define APKC_BRIDGE_POLYGLOT_MODULES_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Module export (symbol from module) */
struct PolyglotExport {
	const u8 *name;            /* Symbol name */
	u32 name_len;
	u64 address;               /* Function/data address */
	u8 is_function;            /* 1 if function, 0 if data */
	u8 lang_specific;          /* 1 if language-specific (not portable) */
};

/* Module import (symbol imported from another module) */
struct PolyglotImport {
	const u8 *name;            /* Imported symbol name */
	u32 name_len;
	u32 source_module;         /* Index of exporting module */
	u64 resolved_addr;         /* Resolved address (0 if unresolved) */
	u8 resolved;               /* 1 if address resolved */
};

/* Polyglot module metadata */
struct PolyglotModule {
	const u8 *name;            /* Module name */
	u32 name_len;
	u64 base_address;          /* Base address in memory */
	u32 code_size;
	u8 lang_type;              /* Language (0=Python, 1=Go, ..., 6=Swift) */
	struct PolyglotExport exports[16];  /* Up to 16 exported symbols */
	u32 export_count;
	struct PolyglotImport imports[16];  /* Up to 16 imported symbols */
	u32 import_count;
	u32 dependencies[8];       /* Indices of dependency modules */
	u32 dep_count;
	u8 initialized;            /* 1 if module constructors ran */
	void (*init_func)(void);   /* Language-specific init function */
	void (*finalize_func)(void);  /* Language-specific finalize function */
};

/* Cross-language import entry (for tracking cross-module dependencies) */
struct CrossLangImport {
	u32 importer_module;       /* Module doing the import */
	u32 exporter_module;       /* Module providing the export */
	const u8 *symbol_name;     /* Symbol being imported */
	u32 symbol_len;
	u8 importer_lang;          /* Language of importer */
	u8 exporter_lang;          /* Language of exporter */
};

/* Polyglot module manager */
struct PolyglotModuleManager {
	struct PolyglotModule modules[64];  /* Up to 64 modules */
	u32 module_count;
	struct CrossLangImport cross_imports[256];  /* Up to 256 cross-lang imports */
	u32 cross_import_count;
	u32 load_order[64];        /* Module load order (topologically sorted) */
	u32 load_order_count;
	u8 dependency_graph[64 * 8];  /* Adjacency list for dependency DAG */
	u8 initialized;
};

/* ============================================================ */
/* MODULE REGISTRATION & MANAGEMENT */
/* ============================================================ */

/* Initialize polyglot module manager */
static inline void polyglot_manager_init(struct PolyglotModuleManager *pm) {
	if (!pm) return;

	pm->module_count = 0;
	pm->cross_import_count = 0;
	pm->load_order_count = 0;
	pm->initialized = 1;
}

/* Register a polyglot module */
static inline u32 polyglot_module_register(
	struct PolyglotModuleManager *pm,
	const u8 *module_name, u32 name_len,
	u64 base_addr, u32 code_size,
	u8 lang_type) {

	if (!pm || pm->module_count >= 64) return 0xFFFFFFFF;

	struct PolyglotModule *mod = &pm->modules[pm->module_count];
	mod->name = module_name;
	mod->name_len = name_len;
	mod->base_address = base_addr;
	mod->code_size = code_size;
	mod->lang_type = lang_type;
	mod->export_count = 0;
	mod->import_count = 0;
	mod->dep_count = 0;
	mod->initialized = 0;
	mod->init_func = NULL;
	mod->finalize_func = NULL;

	u32 module_idx = pm->module_count;
	pm->module_count++;
	return module_idx;
}

/* Add export to module */
static inline u8 polyglot_module_add_export(
	struct PolyglotModuleManager *pm,
	u32 module_idx,
	const u8 *sym_name, u32 sym_len,
	u64 address, u8 is_function) {

	if (!pm || module_idx >= pm->module_count) return 1;

	struct PolyglotModule *mod = &pm->modules[module_idx];
	if (mod->export_count >= 16) return 1;

	struct PolyglotExport *exp = &mod->exports[mod->export_count];
	exp->name = sym_name;
	exp->name_len = sym_len;
	exp->address = address;
	exp->is_function = is_function;
	exp->lang_specific = 0;

	mod->export_count++;
	return 0;
}

/* Add import to module */
static inline u8 polyglot_module_add_import(
	struct PolyglotModuleManager *pm,
	u32 module_idx,
	const u8 *sym_name, u32 sym_len) {

	if (!pm || module_idx >= pm->module_count) return 1;

	struct PolyglotModule *mod = &pm->modules[module_idx];
	if (mod->import_count >= 16) return 1;

	struct PolyglotImport *imp = &mod->imports[mod->import_count];
	imp->name = sym_name;
	imp->name_len = sym_len;
	imp->source_module = 0xFFFFFFFF;  /* Unresolved */
	imp->resolved_addr = 0;
	imp->resolved = 0;

	mod->import_count++;
	return 0;
}

/* Add dependency from one module to another */
static inline u8 polyglot_module_add_dependency(
	struct PolyglotModuleManager *pm,
	u32 dependent_idx, u32 dependency_idx) {

	if (!pm || dependent_idx >= pm->module_count) return 1;
	if (dependency_idx >= pm->module_count) return 1;

	struct PolyglotModule *mod = &pm->modules[dependent_idx];
	if (mod->dep_count >= 8) return 1;

	mod->dependencies[mod->dep_count] = dependency_idx;
	mod->dep_count++;
	return 0;
}

/* ============================================================ */
/* SYMBOL RESOLUTION */
/* ============================================================ */

/* Resolve symbol within a module */
static inline struct PolyglotExport* polyglot_module_find_export(
	struct PolyglotModule *mod,
	const u8 *sym_name, u32 sym_len) {

	if (!mod) return NULL;

	u32 i;
	for (i = 0; i < mod->export_count; i++) {
		struct PolyglotExport *exp = &mod->exports[i];
		if (exp->name_len != sym_len) continue;

		/* Compare names byte-by-byte */
		u32 j;
		u8 match = 1;
		for (j = 0; j < sym_len; j++) {
			if (exp->name[j] != sym_name[j]) {
				match = 0;
				break;
			}
		}

		if (match) return exp;
	}

	return NULL;
}

/* Resolve symbol across all modules (global resolution) */
static inline struct PolyglotExport* polyglot_symbol_resolve_global(
	struct PolyglotModuleManager *pm,
	const u8 *sym_name, u32 sym_len,
	u32 *out_module_idx) {

	if (!pm) return NULL;

	u32 i;
	for (i = 0; i < pm->module_count; i++) {
		struct PolyglotModule *mod = &pm->modules[i];
		struct PolyglotExport *exp = polyglot_module_find_export(mod, sym_name, sym_len);
		if (exp) {
			if (out_module_idx) *out_module_idx = i;
			return exp;
		}
	}

	return NULL;
}

/* Resolve imports for a module (find and link exported symbols) */
static inline u8 polyglot_module_resolve_imports(
	struct PolyglotModuleManager *pm,
	u32 module_idx) {

	if (!pm || module_idx >= pm->module_count) return 1;

	struct PolyglotModule *mod = &pm->modules[module_idx];

	u32 i;
	for (i = 0; i < mod->import_count; i++) {
		struct PolyglotImport *imp = &mod->imports[i];

		/* Find export in all modules */
		u32 src_idx = 0;
		struct PolyglotExport *exp = polyglot_symbol_resolve_global(
			pm, imp->name, imp->name_len, &src_idx);

		if (!exp) {
			return 1;  /* Symbol not found */
		}

		/* Link import to export */
		imp->source_module = src_idx;
		imp->resolved_addr = exp->address;
		imp->resolved = 1;
	}

	return 0;
}

/* ============================================================ */
/* DEPENDENCY SORTING & INITIALIZATION ORDER */
/* ============================================================ */

/* Topological sort: compute module load order from dependencies */
static inline u8 polyglot_dependency_sort(
	struct PolyglotModuleManager *pm) {

	if (!pm || pm->module_count == 0) return 0;

	/* Simple topological sort (Kahn's algorithm) */
	u32 in_degree[64];
	u32 i, j;

	/* Compute in-degrees */
	for (i = 0; i < pm->module_count; i++) {
		in_degree[i] = 0;
	}

	/* Count incoming edges */
	for (i = 0; i < pm->module_count; i++) {
		struct PolyglotModule *mod = &pm->modules[i];
		for (j = 0; j < mod->dep_count; j++) {
			u32 dep_idx = mod->dependencies[j];
			if (dep_idx < pm->module_count) {
				in_degree[i]++;
			}
		}
	}

	/* Extract vertices with in-degree 0 */
	pm->load_order_count = 0;
	u32 processed = 0;

	while (processed < pm->module_count && pm->load_order_count < 64) {
		/* Find module with in-degree 0 */
		u32 candidate = 0xFFFFFFFF;
		for (i = 0; i < pm->module_count; i++) {
			if (in_degree[i] == 0) {
				/* Check if already added */
				u32 found = 0;
				for (j = 0; j < pm->load_order_count; j++) {
					if (pm->load_order[j] == i) {
						found = 1;
						break;
					}
				}
				if (!found) {
					candidate = i;
					break;
				}
			}
		}

		if (candidate == 0xFFFFFFFF) break;  /* Cycle or done */

		/* Add to load order */
		pm->load_order[pm->load_order_count++] = candidate;
		processed++;
	}

	/* Check for cycles */
	if (processed < pm->module_count) {
		return 1;  /* Cycle detected */
	}

	return 0;
}

/* ============================================================ */
/* MODULE INITIALIZATION & FINALIZATION */
/* ============================================================ */

/* Initialize module (call init function if present) */
static inline u8 polyglot_module_init(
	struct PolyglotModuleManager *pm,
	u32 module_idx) {

	if (!pm || module_idx >= pm->module_count) return 1;

	struct PolyglotModule *mod = &pm->modules[module_idx];

	/* Check dependencies are initialized */
	u32 i;
	for (i = 0; i < mod->dep_count; i++) {
		u32 dep_idx = mod->dependencies[i];
		if (dep_idx < pm->module_count) {
			if (!pm->modules[dep_idx].initialized) {
				return 1;  /* Dependency not initialized */
			}
		}
	}

	/* Call init function if present */
	if (mod->init_func) {
		mod->init_func();
	}

	mod->initialized = 1;
	return 0;
}

/* Initialize all modules in dependency order */
static inline u8 polyglot_modules_init_all(
	struct PolyglotModuleManager *pm) {

	if (!pm || pm->load_order_count == 0) {
		/* No load order computed yet; compute it */
		if (polyglot_dependency_sort(pm)) {
			return 1;  /* Cycle or error */
		}
	}

	u32 i;
	for (i = 0; i < pm->load_order_count; i++) {
		u32 idx = pm->load_order[i];
		if (polyglot_module_init(pm, idx)) {
			return 1;  /* Initialization failed */
		}
	}

	return 0;
}

/* Finalize module (call finalize function if present) */
static inline void polyglot_module_finalize(
	struct PolyglotModuleManager *pm,
	u32 module_idx) {

	if (!pm || module_idx >= pm->module_count) return;

	struct PolyglotModule *mod = &pm->modules[module_idx];

	if (mod->finalize_func) {
		mod->finalize_func();
	}

	mod->initialized = 0;
}

/* Finalize all modules (reverse order of initialization) */
static inline void polyglot_modules_finalize_all(
	struct PolyglotModuleManager *pm) {

	if (!pm || pm->load_order_count == 0) return;

	/* Finalize in reverse order */
	u32 i;
	for (i = pm->load_order_count; i > 0; i--) {
		u32 idx = pm->load_order[i - 1];
		polyglot_module_finalize(pm, idx);
	}
}

/* ============================================================ */
/* CROSS-LANGUAGE IMPORT TRACKING */
/* ============================================================ */

/* Register cross-language import */
static inline u8 polyglot_cross_import_register(
	struct PolyglotModuleManager *pm,
	u32 importer_idx, u32 exporter_idx,
	const u8 *sym_name, u32 sym_len) {

	if (!pm || pm->cross_import_count >= 256) return 1;

	struct CrossLangImport *imp = &pm->cross_imports[pm->cross_import_count];
	imp->importer_module = importer_idx;
	imp->exporter_module = exporter_idx;
	imp->symbol_name = sym_name;
	imp->symbol_len = sym_len;

	if (importer_idx < pm->module_count) {
		imp->importer_lang = pm->modules[importer_idx].lang_type;
	}
	if (exporter_idx < pm->module_count) {
		imp->exporter_lang = pm->modules[exporter_idx].lang_type;
	}

	pm->cross_import_count++;
	return 0;
}

/* Get module by index */
static inline struct PolyglotModule* polyglot_module_get(
	struct PolyglotModuleManager *pm,
	u32 module_idx) {

	if (!pm || module_idx >= pm->module_count) return NULL;
	return &pm->modules[module_idx];
}

#endif /* APKC_BRIDGE_POLYGLOT_MODULES_H */
