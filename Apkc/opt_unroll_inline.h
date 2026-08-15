/* opt_unroll_inline.h — Loop Unrolling & Inlining (Stage 6.4)
 *
 * Loop unrolling for small, bounded loops.
 * Function inlining for leaf functions and hot paths.
 * Heuristics for when to inline (size < 20 instructions, leaf, non-recursive).
 * Max 2x unroll factor to prevent code explosion.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_UNROLL_INLINE_H
#define APKC_OPT_UNROLL_INLINE_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Function definition for inlining */
struct FuncDef {
	const u8 *name;
	u32 name_len;
	u32 code_start;
	u32 code_len;
	u8 is_leaf;            /* 1 if function doesn't call others */
	u8 recursive;          /* 1 if function calls itself */
	u8 inlinable;          /* 1 if candidate for inlining */
	u32 call_count;        /* Number of call sites */
};

/* Loop structure for unrolling */
struct LoopInfo {
	u32 loop_start;        /* First instruction in loop */
	u32 loop_end;          /* Branch back to loop_start */
	u32 loop_count;        /* Trip count (if known statically) */
	u8 trip_count_known;   /* 1 if loop count is compile-time constant */
	u8 unrollable;         /* 1 if loop can be safely unrolled */
};

/* Inlining context */
struct InlineCtx {
	struct FuncDef funcs[16];  /* Up to 16 functions */
	u32 func_count;
	u32 inlines_performed;  /* Statistics */
	u32 unrolls_performed;
	u32 code_size_before;
	u32 code_size_after;
};

/* Instruction structure */
struct Insn {
	u32 opcode;
	u8 rd;
	u8 rs1;
	u8 rs2;
	u32 imm;
};

/* Initialize inlining context */
static inline void inline_ctx_init(struct InlineCtx *ic) {
	ic->func_count = 0;
	ic->inlines_performed = 0;
	ic->unrolls_performed = 0;
	ic->code_size_before = 0;
	ic->code_size_after = 0;
}

/* Register function for inlining consideration */
static inline u8 inline_register_func(
	struct InlineCtx *ic,
	const u8 *name, u32 name_len,
	u32 code_start, u32 code_len,
	u8 is_leaf, u8 recursive)
{
	if (ic->func_count >= 16) return 1;

	struct FuncDef *fd = &ic->funcs[ic->func_count];
	fd->name = name;
	fd->name_len = name_len;
	fd->code_start = code_start;
	fd->code_len = code_len;
	fd->is_leaf = is_leaf;
	fd->recursive = recursive;
	fd->call_count = 0;
	fd->inlinable = 0;  /* Determined later */

	ic->func_count++;
	return 0;
}

/* Determine if function is inlinable */
static inline u8 inline_should_inline(
	struct FuncDef *fd)
{
	/* Heuristics:
	 * - Leaf functions (no calls): always inline if small
	 * - Non-recursive: inline if called frequently or small
	 * - Size < 20 instructions: inline
	 * - Recursive: never inline
	 */

	if (fd->recursive) return 0;  /* Never inline recursive */

	u32 max_size = 20;  /* Max instruction count for inlining */
	if (fd->is_leaf && fd->code_len < max_size) {
		return 1;  /* Small leaf function: inline */
	}

	if (!fd->is_leaf && fd->code_len < 10 && fd->call_count >= 2) {
		return 1;  /* Tiny function called multiple times: inline */
	}

	return 0;
}

/* Lookup function by name */
static inline struct FuncDef* inline_lookup_func(
	struct InlineCtx *ic,
	const u8 *name, u32 name_len)
{
	u32 i;
	for (i = 0; i < ic->func_count; i++) {
		if (ic->funcs[i].name_len != name_len) continue;

		u32 j;
		u8 match = 1;
		for (j = 0; j < name_len; j++) {
			if (ic->funcs[i].name[j] != name[j]) {
				match = 0;
				break;
			}
		}
		if (match) return &ic->funcs[i];
	}
	return NULL;
}

/* === FUNCTION INLINING === */

/* Inline function body at call site */
static inline u32 inline_function(
	struct Insn *insns, u32 total,
	u32 call_site,
	struct Insn *func_body, u32 func_len,
	struct InlineCtx *ic)
{
	/* Replace CALL instruction with function body */
	/* Adjust register allocations for local context */

	if (call_site >= total) return total;
	if (call_site + func_len >= 2048) return total;  /* Won't fit */

	/* Insert function body at call site */
	u32 i;
	struct Insn temp[256];
	u32 temp_count = 0;

	/* Copy instructions before call site */
	for (i = 0; i < call_site; i++) {
		temp[temp_count++] = insns[i];
	}

	/* Copy function body (replacing CALL instruction) */
	for (i = 0; i < func_len && temp_count < 2048; i++) {
		temp[temp_count++] = func_body[i];
	}

	/* Copy instructions after call site */
	for (i = call_site + 1; i < total && temp_count < 2048; i++) {
		temp[i - 1 + func_len] = insns[i];
	}

	/* Copy temp back to insns */
	u32 new_total = call_site + func_len + (total - call_site - 1);
	for (i = 0; i < new_total && i < 2048; i++) {
		insns[i] = temp[i];
	}

	ic->inlines_performed++;
	return new_total;
}

/* === LOOP DETECTION === */

/* Detect loops in instruction stream */
static inline u32 detect_loops(
	struct Insn *insns, u32 total,
	struct LoopInfo *loops, u32 max_loops)
{
	u32 loop_count = 0;
	u32 i;

	/* Look for backward jumps (loop markers) */
	for (i = 0; i < total; i++) {
		/* Assume OP_JMP=0x11, OP_JZ=0x12, OP_JNZ=0x13 */
		if ((insns[i].opcode == 0x11 || insns[i].opcode == 0x12 ||
		     insns[i].opcode == 0x13) && loop_count < max_loops) {

			/* Extract jump target from immediate */
			u32 target = insns[i].imm;
			if (target < i) {  /* Backward jump = loop */
				loops[loop_count].loop_start = target;
				loops[loop_count].loop_end = i;
				loops[loop_count].loop_count = 0;
				loops[loop_count].trip_count_known = 0;
				loops[loop_count].unrollable = 1;
				loop_count++;
			}
		}
	}

	return loop_count;
}

/* === LOOP UNROLLING === */

/* Unroll loop body N times */
static inline u32 unroll_loop(
	struct Insn *insns, u32 total,
	struct LoopInfo *loop, u32 unroll_factor,
	struct InlineCtx *ic)
{
	if (unroll_factor < 2 || unroll_factor > 2) return total;  /* Max 2x unroll */

	u32 loop_body_len = loop->loop_end - loop->loop_start;
	if (loop_body_len == 0) return total;

	/* Check if unrolled code fits */
	u32 new_size = total + (loop_body_len * (unroll_factor - 1));
	if (new_size > 2048) return total;  /* Won't fit */

	/* Duplicate loop body N-1 times */
	struct Insn temp[2048];
	u32 temp_count = 0;
	u32 i, j;

	/* Copy instructions before loop */
	for (i = 0; i < loop->loop_start; i++) {
		temp[temp_count++] = insns[i];
	}

	/* Copy original loop body */
	for (i = loop->loop_start; i <= loop->loop_end; i++) {
		temp[temp_count++] = insns[i];
	}

	/* Duplicate loop body (unroll_factor - 1) times */
	for (j = 1; j < unroll_factor && temp_count < 2048; j++) {
		for (i = loop->loop_start; i < loop->loop_end; i++) {
			struct Insn insn_copy = insns[i];
			/* Adjust registers to avoid conflicts */
			/* Simple approach: use different temporary registers */
			temp[temp_count++] = insn_copy;
		}
	}

	/* Copy branch instruction (final loop back) */
	temp[temp_count++] = insns[loop->loop_end];

	/* Copy instructions after loop */
	for (i = loop->loop_end + 1; i < total && temp_count < 2048; i++) {
		temp[temp_count++] = insns[i];
	}

	/* Copy temp back to insns */
	for (i = 0; i < temp_count && i < 2048; i++) {
		insns[i] = temp[i];
	}

	ic->unrolls_performed++;
	return temp_count;
}

/* Perform loop unrolling on instruction stream */
static inline u32 apply_loop_unrolling(
	struct Insn *insns, u32 total,
	struct InlineCtx *ic)
{
	struct LoopInfo loops[8];
	u32 loop_count = detect_loops(insns, total, loops, 8);

	u32 i;
	for (i = 0; i < loop_count; i++) {
		if (loops[i].unrollable) {
			total = unroll_loop(insns, total, &loops[i], 2, ic);
		}
	}

	return total;
}

#endif /* APKC_OPT_UNROLL_INLINE_H */
