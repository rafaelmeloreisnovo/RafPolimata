/* lang_control.h — Advanced control flow (Stage 4.9)
 *
 * Switch statements, break/continue, loop control, label management
 * Branchless and branch-based control flow synthesis
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_LANG_CONTROL_H
#define APKC_LANG_CONTROL_H 1

#include "compiler_language_direct.h"

typedef unsigned char u8;
typedef unsigned int u32;

/* Control flow statement types */
enum ControlType {
	CTRL_IF,
	CTRL_IF_ELSE,
	CTRL_WHILE,
	CTRL_FOR,
	CTRL_SWITCH,
	CTRL_BREAK,
	CTRL_CONTINUE,
	CTRL_RETURN,
};

/* Loop context (tracks active loops for break/continue) */
struct LoopContext {
	u32 loop_label;       /* Label to jump to on break */
	u32 continue_label;   /* Label to jump to on continue */
	u32 loop_type;        /* CTRL_WHILE, CTRL_FOR, etc */
	u8 depth;             /* Nesting depth (0=no loop) */
};

/* Switch case entry */
struct SwitchCase {
	u32 value;            /* Case value (constant only) */
	u32 label;            /* Label for this case */
	u8 is_default;        /* 1 if default case */
};

/* Switch context */
struct SwitchContext {
	struct SwitchCase cases[16];  /* Up to 16 case labels */
	u32 case_count;
	u32 end_label;        /* Label for switch end (break target) */
	u32 default_label;    /* Label for default case */
};

/* Initialize loop context */
static inline void loop_init(struct LoopContext *lc) {
	lc->loop_label = 0;
	lc->continue_label = 0;
	lc->loop_type = 0;
	lc->depth = 0;
}

/* Enter loop (push to stack) */
static inline void loop_enter(struct LoopContext *lc, u32 loop_label, u32 continue_label, u32 type) {
	if (lc->depth >= 15) return;  /* Too deep */
	lc->depth++;
	lc->loop_label = loop_label;
	lc->continue_label = continue_label;
	lc->loop_type = type;
}

/* Exit loop (pop from stack) */
static inline void loop_exit(struct LoopContext *lc) {
	if (lc->depth > 0) lc->depth--;
}

/* Check if in loop */
static inline u8 loop_in_loop(struct LoopContext *lc) {
	return lc->depth > 0;
}

/* Get current loop's break label */
static inline u32 loop_get_break_label(struct LoopContext *lc) {
	return lc->loop_label;
}

/* Get current loop's continue label */
static inline u32 loop_get_continue_label(struct LoopContext *lc) {
	return lc->continue_label;
}

/* === CONTROL FLOW CODE GENERATION === */

/* Emit if statement: condition in r0, jump to else_label if zero */
static inline void ctrl_emit_if(
	struct CodeGen *cg,
	u32 else_label)
{
	/* CMP r0, #0; JZ else_label */
	codegen_emit_cmp(cg, 0, 0);
	codegen_emit_jz(cg, else_label);
}

/* Emit while loop: condition check, jump to end if zero */
static inline void ctrl_emit_while_check(
	struct CodeGen *cg,
	u32 end_label)
{
	/* CMP r0, #0; JZ end_label */
	codegen_emit_cmp(cg, 0, 0);
	codegen_emit_jz(cg, end_label);
}

/* Emit while loop: jump back to check */
static inline void ctrl_emit_while_loop(
	struct CodeGen *cg,
	u32 check_label)
{
	/* JMP check_label */
	codegen_emit_jmp(cg, check_label);
}

/* Emit for loop: increment and jump back */
static inline void ctrl_emit_for_increment(
	struct CodeGen *cg,
	u8 loop_var_reg)
{
	/* ADD loop_var, loop_var, #1 */
	codegen_emit_add(cg, loop_var_reg, loop_var_reg, 0);  /* Simplified: no immediate ADD */
}

/* Emit break statement: jump to loop end */
static inline void ctrl_emit_break(
	struct CodeGen *cg,
	u32 break_label)
{
	codegen_emit_jmp(cg, break_label);
}

/* Emit continue statement: jump to loop continue point */
static inline void ctrl_emit_continue(
	struct CodeGen *cg,
	u32 continue_label)
{
	codegen_emit_jmp(cg, continue_label);
}

/* Emit return statement: move value to r0, emit RET */
static inline void ctrl_emit_return(
	struct CodeGen *cg,
	u8 value_reg)
{
	if (value_reg != 0) {
		codegen_emit_mov(cg, 0, value_reg);  /* Move result to r0 */
	}
	codegen_emit_ret(cg);
}

/* === SWITCH STATEMENT HANDLING === */

/* Initialize switch context */
static inline void switch_init(struct SwitchContext *sc) {
	sc->case_count = 0;
	sc->end_label = 0;
	sc->default_label = 0;
}

/* Add case entry */
static inline u8 switch_add_case(
	struct SwitchContext *sc,
	u32 value,
	u32 label,
	u8 is_default)
{
	if (sc->case_count >= 16) return 1;  /* Too many cases */

	sc->cases[sc->case_count].value = value;
	sc->cases[sc->case_count].label = label;
	sc->cases[sc->case_count].is_default = is_default;

	if (is_default) {
		sc->default_label = label;
	}

	sc->case_count++;
	return 0;
}

/* Emit switch dispatch: compute which case to jump to */
static inline void switch_emit_dispatch(
	struct CodeGen *cg,
	struct SwitchContext *sc)
{
	u32 i;

	/* Linear search through cases: CMP value, case[i].value; JZ case[i].label */
	for (i = 0; i < sc->case_count; i++) {
		if (!sc->cases[i].is_default) {
			/* CMP r0, case_value; JZ case_label */
			codegen_emit_cmp(cg, 0, 0);  /* Simplified: no CMPI opcode */
			codegen_emit_jz(cg, sc->cases[i].label);
		}
	}

	/* Default case or end */
	if (sc->default_label) {
		codegen_emit_jmp(cg, sc->default_label);
	} else {
		codegen_emit_jmp(cg, sc->end_label);
	}
}

/* === BRANCHLESS CONTROL FLOW (Alternative to Jumps) === */

/* Emit branchless if using CMOV: if (cond) { result_reg = true_val } else { result_reg = false_val } */
static inline void ctrl_emit_if_branchless(
	struct CodeGen *cg,
	u8 cond_reg,
	u8 result_reg,
	u8 true_val_reg,
	u8 false_val_reg)
{
	/* CMP cond_reg, #0 */
	codegen_emit_cmp(cg, cond_reg, cond_reg);
	/* CMOV_NZ result_reg, true_val_reg (if condition is true) */
	codegen_emit_cmov_nz(cg, result_reg, true_val_reg);
	/* CMOV_Z result_reg, false_val_reg (if condition is false) */
	codegen_emit_cmov_z(cg, result_reg, false_val_reg);
}

/* Emit ternary operator (a ? b : c) via CMOV */
static inline void ctrl_emit_ternary(
	struct CodeGen *cg,
	u8 cond_reg,
	u8 true_val_reg,
	u8 false_val_reg,
	u8 result_reg)
{
	codegen_emit_cmp(cg, cond_reg, cond_reg);
	codegen_emit_cmov_nz(cg, result_reg, true_val_reg);
	codegen_emit_cmov_z(cg, result_reg, false_val_reg);
}

#endif /* APKC_LANG_CONTROL_H */
