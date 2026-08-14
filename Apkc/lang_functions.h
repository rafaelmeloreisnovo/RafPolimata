/* lang_functions.h — Function definitions and parameter passing (Stage 4.10)
 *
 * Function parsing, parameter handling, calling conventions, symbol tables
 * Supports up to 32 function definitions per module
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_LANG_FUNCTIONS_H
#define APKC_LANG_FUNCTIONS_H 1

#include "lang_types.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Function parameter */
struct FuncParam {
	const u8 *name;
	u32 name_len;
	enum ValueType type;
	u8 reg;              /* Allocated register (0-2 for first 3 args) */
};

/* Function definition */
struct FuncDef {
	const u8 *name;
	u32 name_len;
	struct FuncParam params[8];  /* Up to 8 parameters */
	u32 param_count;
	enum ValueType return_type;
	u32 code_start;      /* Instruction offset where function starts */
	u32 code_len;        /* Number of instructions in function */
};

/* Function symbol table */
struct FuncTable {
	struct FuncDef funcs[32];   /* Up to 32 functions */
	u32 func_count;
	u32 current_func;    /* Index of function being compiled */
};

/* Calling convention constants */
#define ARG_REG_0 0
#define ARG_REG_1 1
#define ARG_REG_2 2
#define RET_REG   0
#define STACK_ARG_START 8  /* First stack argument offset from SP */

/* Initialize function table */
static inline void func_table_init(struct FuncTable *ft) {
	ft->func_count = 0;
	ft->current_func = 0;
}

/* Register function definition */
static inline u8 func_table_define(
	struct FuncTable *ft,
	const u8 *name, u32 name_len,
	enum ValueType return_type)
{
	if (ft->func_count >= 32) return 1;  /* Too many functions */

	struct FuncDef *fd = &ft->funcs[ft->func_count];
	fd->name = name;
	fd->name_len = name_len;
	fd->return_type = return_type;
	fd->param_count = 0;
	fd->code_start = 0;
	fd->code_len = 0;

	ft->current_func = ft->func_count;
	ft->func_count++;
	return 0;
}

/* Add parameter to current function */
static inline u8 func_add_param(
	struct FuncTable *ft,
	const u8 *param_name, u32 param_len,
	enum ValueType param_type)
{
	struct FuncDef *fd = &ft->funcs[ft->current_func];
	if (fd->param_count >= 8) return 1;  /* Too many parameters */

	struct FuncParam *p = &fd->params[fd->param_count];
	p->name = param_name;
	p->name_len = param_len;
	p->type = param_type;

	/* First 3 args in registers r0, r1, r2 */
	if (fd->param_count < 3) {
		p->reg = fd->param_count;
	} else {
		p->reg = 255;  /* Stack argument */
	}

	fd->param_count++;
	return 0;
}

/* Lookup function by name */
static inline struct FuncDef* func_table_lookup(
	struct FuncTable *ft,
	const u8 *name, u32 name_len)
{
	u32 i;
	for (i = 0; i < ft->func_count; i++) {
		if (ft->funcs[i].name_len != name_len) continue;

		u32 j;
		u8 match = 1;
		for (j = 0; j < name_len; j++) {
			if (ft->funcs[i].name[j] != name[j]) {
				match = 0;
				break;
			}
		}
		if (match) return &ft->funcs[i];
	}
	return NULL;
}

/* Get function by index */
static inline struct FuncDef* func_table_get(struct FuncTable *ft, u32 idx) {
	if (idx < ft->func_count) return &ft->funcs[idx];
	return NULL;
}

/* === PARAMETER PASSING === */

/* Map parameter index to register/stack location */
static inline u8 func_param_get_reg(u32 param_idx) {
	if (param_idx < 3) return (u8)param_idx;  /* r0, r1, r2 */
	return 255;  /* Stack */
}

/* Get stack offset for parameter (param_idx >= 3) */
static inline u32 func_param_get_stack_offset(u32 param_idx) {
	if (param_idx >= 3) {
		return STACK_ARG_START + (param_idx - 3) * 8;
	}
	return 0;
}

/* === FUNCTION CALL PREPARATION === */

/* Function call context */
struct FuncCallCtx {
	u32 arg_count;       /* Number of arguments */
	u8 arg_regs[3];      /* Registers for first 3 args */
	u32 stack_args[8];   /* Values for stack arguments (indices 3+) */
	u32 stack_arg_count;
};

/* Prepare function call */
static inline void func_call_prepare(struct FuncCallCtx *fcc) {
	fcc->arg_count = 0;
	fcc->stack_arg_count = 0;
}

/* Add argument to call */
static inline u8 func_call_add_arg(
	struct FuncCallCtx *fcc,
	u8 arg_reg)
{
	if (fcc->arg_count >= 3) {
		/* Push to stack arguments */
		if (fcc->stack_arg_count >= 8) return 1;  /* Too many */
		fcc->stack_args[fcc->stack_arg_count++] = arg_reg;
	} else {
		fcc->arg_regs[fcc->arg_count] = arg_reg;
	}
	fcc->arg_count++;
	return 0;
}

/* Allocate stack space for stack arguments */
static inline u32 func_call_stack_size(struct FuncCallCtx *fcc) {
	if (fcc->arg_count <= 3) return 0;
	return (fcc->arg_count - 3) * 8;  /* Each arg is 8 bytes */
}

/* === CODE GENERATION FOR FUNCTIONS === */

/* Emit function prologue */
static inline void func_emit_prologue(
	struct CodeGen *cg,
	u32 local_size)
{
	if (local_size > 0) {
		/* Adjust stack: sp -= local_size */
		codegen_emit_movi(cg, 14, local_size);  /* r14 = sp */
		codegen_emit_sub(cg, 14, 14, 14);       /* sp -= local_size */
	}
}

/* Emit function epilogue */
static inline void func_emit_epilogue(
	struct CodeGen *cg,
	u32 local_size)
{
	if (local_size > 0) {
		/* Restore stack: sp += local_size */
		codegen_emit_movi(cg, 1, local_size);
		codegen_emit_add(cg, 14, 14, 1);        /* sp += local_size */
	}
	codegen_emit_ret(cg);
}

/* Emit function call */
static inline void func_emit_call(
	struct CodeGen *cg,
	u32 func_address)
{
	codegen_emit_call(cg, func_address);
}

/* Prepare register state before function call (move args to r0-r2) */
static inline void func_emit_call_setup(
	struct CodeGen *cg,
	struct FuncCallCtx *fcc)
{
	u32 i;
	for (i = 0; i < fcc->arg_count && i < 3; i++) {
		if (fcc->arg_regs[i] != (u8)i) {
			/* Move arg from its register to r0/r1/r2 */
			codegen_emit_mov(cg, (u8)i, fcc->arg_regs[i]);
		}
	}
	/* Stack arguments would be pushed here in a real implementation */
}

/* === FUNCTION PARSING CONTEXT === */

struct FuncParseCtx {
	struct FuncTable func_table;
	struct CodeGen *codegen;
	u32 current_code_pos;  /* Code position at function start */
	enum ValueType current_return_type;
};

/* Initialize function parsing context */
static inline void func_parse_init(
	struct FuncParseCtx *fpc,
	struct CodeGen *cg)
{
	func_table_init(&fpc->func_table);
	fpc->codegen = cg;
	fpc->current_code_pos = 0;
	fpc->current_return_type = TYPE_VOID;
}

/* Mark function start */
static inline void func_parse_start(
	struct FuncParseCtx *fpc,
	u32 code_pos)
{
	fpc->current_code_pos = code_pos;
}

/* Mark function end and record its code length */
static inline void func_parse_end(
	struct FuncParseCtx *fpc,
	u32 end_pos)
{
	if (fpc->func_table.current_func < fpc->func_table.func_count) {
		struct FuncDef *fd = &fpc->func_table.funcs[fpc->func_table.current_func];
		fd->code_start = fpc->current_code_pos;
		fd->code_len = end_pos - fpc->current_code_pos;
	}
}

#endif /* APKC_LANG_FUNCTIONS_H */
