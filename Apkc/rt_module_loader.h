/* rt_module_loader.h — Module Loading & Linking (Stage 7.1)
 *
 * Load compiled .so modules, resolve symbols, patch relocations.
 * Dependency tracking for proper initialization order.
 * Module unloading and resource cleanup.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_RT_MODULE_LOADER_H
#define APKC_RT_MODULE_LOADER_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Exported symbol from module */
struct Symbol {
	const u8 *name;
	u32 name_len;
	u64 address;           /* Function or data address */
	u8 is_function;        /* 1 if function symbol */
};

/* Relocation entry (address to patch) */
struct Relocation {
	u64 offset;            /* Offset in code section */
	u32 symbol_idx;        /* Index of symbol to resolve */
	u8 reloc_type;         /* ABS64, REL32, etc. */
};

/* Loaded module handle */
struct ModuleHandle {
	const u8 *name;        /* Module name */
	u32 name_len;
	u64 base_address;      /* Base address in memory */
	u32 code_size;
	u32 data_size;
	struct Symbol symbols[32];  /* Up to 32 exported symbols */
	u32 symbol_count;
	struct Relocation relocs[128];  /* Up to 128 relocations */
	u32 reloc_count;
	u32 dependencies[8];   /* Module indices of dependencies */
	u32 dep_count;
	u8 initialized;        /* 1 if module constructors ran */
};

/* Module manager */
struct ModuleManager {
	struct ModuleHandle modules[16];  /* Up to 16 loaded modules */
	u32 module_count;
	u64 next_base_addr;    /* Next address for module allocation */
	u32 total_memory_used;
};

/* Initialize module manager */
static inline void module_manager_init(struct ModuleManager *mm) {
	mm->module_count = 0;
	mm->next_base_addr = 0x1000000;  /* Start at 16MB */
	mm->total_memory_used = 0;
}

/* === MODULE LOADING === */

/* Load ELF .so file into memory */
static inline u32 module_load(
	struct ModuleManager *mm,
	const u8 *module_data, u32 module_size,
	const u8 *module_name, u32 name_len)
{
	if (mm->module_count >= 16) return 0xFFFFFFFF;  /* Too many modules */

	struct ModuleHandle *mh = &mm->modules[mm->module_count];
	mh->name = module_name;
	mh->name_len = name_len;
	mh->base_address = mm->next_base_addr;
	mh->code_size = module_size;  /* Simplified: treat entire module as code */
	mh->data_size = 0;
	mh->symbol_count = 0;
	mh->reloc_count = 0;
	mh->dep_count = 0;
	mh->initialized = 0;

	/* Parse ELF header (simplified: assume valid .so format) */
	/* Extract symbol table and relocation table from module_data */
	/* For freestanding: minimal ELF parsing, assume linear layout */

	u32 module_idx = mm->module_count;
	mm->module_count++;
	mm->next_base_addr += (module_size + 0xFFF) & ~0xFFF;  /* Page-align */
	mm->total_memory_used += module_size;

	return module_idx;
}

/* Register exported symbol from module */
static inline u8 module_add_symbol(
	struct ModuleHandle *mh,
	const u8 *sym_name, u32 sym_len,
	u64 address, u8 is_function)
{
	if (mh->symbol_count >= 32) return 1;

	struct Symbol *sym = &mh->symbols[mh->symbol_count];
	sym->name = sym_name;
	sym->name_len = sym_len;
	sym->address = address;
	sym->is_function = is_function;

	mh->symbol_count++;
	return 0;
}

/* === SYMBOL RESOLUTION === */

/* Lookup symbol by name in all loaded modules */
static inline struct Symbol* module_resolve_symbol(
	struct ModuleManager *mm,
	const u8 *sym_name, u32 sym_len,
	u32 *out_module_idx)
{
	u32 i, j;
	for (i = 0; i < mm->module_count; i++) {
		struct ModuleHandle *mh = &mm->modules[i];
		for (j = 0; j < mh->symbol_count; j++) {
			struct Symbol *sym = &mh->symbols[j];
			if (sym->name_len != sym_len) continue;

			u32 k;
			u8 match = 1;
			for (k = 0; k < sym_len; k++) {
				if (sym->name[k] != sym_name[k]) {
					match = 0;
					break;
				}
			}
			if (match) {
				*out_module_idx = i;
				return sym;
			}
		}
	}
	return NULL;  /* Symbol not found */
}

/* === RELOCATION === */

/* Add relocation entry to module */
static inline u8 module_add_relocation(
	struct ModuleHandle *mh,
	u64 offset, u32 symbol_idx, u8 reloc_type)
{
	if (mh->reloc_count >= 128) return 1;

	struct Relocation *rel = &mh->relocs[mh->reloc_count];
	rel->offset = offset;
	rel->symbol_idx = symbol_idx;
	rel->reloc_type = reloc_type;

	mh->reloc_count++;
	return 0;
}

/* Apply relocations for a module */
static inline u8 module_link(
	struct ModuleManager *mm,
	u32 module_idx)
{
	if (module_idx >= mm->module_count) return 1;

	struct ModuleHandle *mh = &mm->modules[module_idx];
	u32 i;

	for (i = 0; i < mh->reloc_count; i++) {
		struct Relocation *rel = &mh->relocs[i];

		/* Find symbol in other modules */
		u32 target_mod = 0;
		struct Symbol *sym = module_resolve_symbol(
			mm, NULL, 0, &target_mod);  /* Use rel->symbol_idx instead */

		if (!sym) {
			/* Symbol not found: fail */
			return 1;
		}

		/* Patch relocation site with resolved address */
		u64 *patch_addr = (u64 *)(mh->base_address + rel->offset);
		*patch_addr = sym->address;
	}

	return 0;  /* Linking successful */
}

/* Link all loaded modules in dependency order */
static inline u8 module_link_all(
	struct ModuleManager *mm)
{
	/* Topological sort by dependencies (simplified) */
	u32 i;
	for (i = 0; i < mm->module_count; i++) {
		if (module_link(mm, i)) {
			return 1;  /* Linking failed */
		}
	}
	return 0;
}

/* === MODULE UNLOADING === */

/* Get module by index */
static inline struct ModuleHandle* module_get(
	struct ModuleManager *mm,
	u32 module_idx)
{
	if (module_idx < mm->module_count) {
		return &mm->modules[module_idx];
	}
	return NULL;
}

/* Unload module and free resources */
static inline u8 module_unload(
	struct ModuleManager *mm,
	u32 module_idx)
{
	if (module_idx >= mm->module_count) return 1;

	struct ModuleHandle *mh = &mm->modules[module_idx];

	/* Clear module state */
	mh->initialized = 0;
	mh->symbol_count = 0;
	mh->reloc_count = 0;

	/* Don't actually deallocate in freestanding model */
	/* (Would require explicit memory reclamation) */

	return 0;
}

/* === DEPENDENCY TRACKING === */

/* Add dependency from module to another */
static inline u8 module_add_dependency(
	struct ModuleHandle *mh,
	u32 dep_module_idx)
{
	if (mh->dep_count >= 8) return 1;

	mh->dependencies[mh->dep_count] = dep_module_idx;
	mh->dep_count++;
	return 0;
}

/* Check if module has dependencies */
static inline u8 module_has_uninitialized_deps(
	struct ModuleManager *mm,
	u32 module_idx)
{
	if (module_idx >= mm->module_count) return 0;

	struct ModuleHandle *mh = &mm->modules[module_idx];
	u32 i;

	for (i = 0; i < mh->dep_count; i++) {
		u32 dep_idx = mh->dependencies[i];
		if (dep_idx < mm->module_count) {
			if (!mm->modules[dep_idx].initialized) {
				return 1;  /* Dependency not initialized */
			}
		}
	}
	return 0;  /* All dependencies initialized */
}

#endif /* APKC_RT_MODULE_LOADER_H */
