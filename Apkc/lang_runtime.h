/* lang_runtime.h — Runtime support for compiled programs
 *
 * Register allocation, stack frame management, calling conventions
 * Variable tracking via simple symbol table
 * All freestanding, no malloc, stack-only
 */

#ifndef APKC_LANG_RUNTIME_H
#define APKC_LANG_RUNTIME_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Symbol table entry: variable → register mapping */
struct SymbolEntry {
	const u8 *name;
	u32 name_len;
	u8 reg;              /* allocated register (0-13) */
	u8 is_active;        /* 1 if currently allocated */
};

/* Runtime context: tracks registers, variables, stack */
struct RuntimeCtx {
	struct SymbolEntry symbols[32];  /* up to 32 variables */
	u32 symbol_count;
	u8 reg_alloc[14];               /* r0-r13 allocation tracking */
	u32 stack_offset;               /* current stack pointer offset */
	u32 max_stack;                  /* max stack used in function */
};

/* Initialize runtime context */
static inline void runtime_init(struct RuntimeCtx *rt) {
	rt->symbol_count = 0;
	rt->stack_offset = 0;
	rt->max_stack = 0;
	u32 i;
	for (i = 0; i < 14; i++) rt->reg_alloc[i] = 0;
	for (i = 0; i < 32; i++) rt->symbols[i].is_active = 0;
}

/* Allocate register for variable */
static inline u8 runtime_alloc_var(struct RuntimeCtx *rt, const u8 *name, u32 name_len) {
	/* Find free register (r0-r13) */
	u32 i;
	for (i = 0; i < 14; i++) {
		if (!rt->reg_alloc[i]) {
			rt->reg_alloc[i] = 1;

			/* Add to symbol table */
			if (rt->symbol_count < 32) {
				rt->symbols[rt->symbol_count].name = name;
				rt->symbols[rt->symbol_count].name_len = name_len;
				rt->symbols[rt->symbol_count].reg = (u8)i;
				rt->symbols[rt->symbol_count].is_active = 1;
				rt->symbol_count++;
			}
			return (u8)i;
		}
	}

	/* Spill to stack if no free registers */
	rt->stack_offset += 8;
	if (rt->stack_offset > rt->max_stack) rt->max_stack = rt->stack_offset;
	return 255;  /* marker: spilled to stack */
}

/* Lookup variable register */
static inline u8 runtime_lookup_var(struct RuntimeCtx *rt, const u8 *name, u32 name_len) {
	u32 i;
	for (i = 0; i < rt->symbol_count; i++) {
		if (rt->symbols[i].name_len != name_len) continue;

		u32 j;
		u8 match = 1;
		for (j = 0; j < name_len; j++) {
			if (rt->symbols[i].name[j] != name[j]) {
				match = 0;
				break;
			}
		}
		if (match && rt->symbols[i].is_active) {
			return rt->symbols[i].reg;
		}
	}
	return 255;  /* not found */
}

/* Free register after variable goes out of scope */
static inline void runtime_free_var(struct RuntimeCtx *rt, const u8 *name, u32 name_len) {
	u32 i;
	for (i = 0; i < rt->symbol_count; i++) {
		if (rt->symbols[i].name_len != name_len) continue;

		u32 j;
		u8 match = 1;
		for (j = 0; j < name_len; j++) {
			if (rt->symbols[i].name[j] != name[j]) {
				match = 0;
				break;
			}
		}
		if (match) {
			rt->symbols[i].is_active = 0;
			rt->reg_alloc[rt->symbols[i].reg] = 0;
			return;
		}
	}
}

/* Emit function prologue (allocate locals on stack) */
static inline void runtime_emit_prologue(struct CodeGen *cg, u32 local_size) {
	if (local_size > 0) {
		/* Adjust stack pointer: sp -= local_size */
		codegen_emit_movi(cg, 14, local_size);  /* r14 = sp */
		codegen_emit_sub(cg, 14, 14, 1);        /* sp -= local_size */
	}
}

/* Emit function epilogue (deallocate locals, return) */
static inline void runtime_emit_epilogue(struct CodeGen *cg, u32 local_size) {
	if (local_size > 0) {
		/* Restore stack pointer: sp += local_size */
		codegen_emit_movi(cg, 1, local_size);
		codegen_emit_add(cg, 14, 14, 1);        /* sp += local_size */
	}
	codegen_emit_ret(cg);
}

#endif /* APKC_LANG_RUNTIME_H */
