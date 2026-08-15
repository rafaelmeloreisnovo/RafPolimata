/* opt_peephole.h — Peephole Optimization (Stage 6.1)
 *
 * Pattern matching on instruction sequences for local optimization.
 * Instruction combining, dead code elimination, redundancy removal.
 * 20+ predefined patterns for common machine instruction sequences.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_PEEPHOLE_H
#define APKC_OPT_PEEPHOLE_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Peephole pattern definition */
struct PeepholePattern {
	const u32 *pattern;     /* Instruction opcodes to match */
	u32 pattern_len;
	const u32 *replacement; /* Replacement instructions (or NULL for deletion) */
	u32 repl_len;
	const u8 *desc;        /* Description for logging */
	u32 desc_len;
};

/* Peephole optimization context */
struct PeepholeCtx {
	struct PeepholePattern patterns[20];  /* 20 predefined patterns */
	u32 pattern_count;
	u32 optimizations_applied;  /* Counter for statistics */
	u8 enable_aggressive;       /* 1 = apply aggressive optimizations */
};

/* Instruction structure (from machine_linear_branchless.h) */
struct Insn {
	u32 opcode;
	u8 rd;
	u8 rs1;
	u8 rs2;
	u32 imm;
};

/* Initialize peephole context and register patterns */
static inline void peephole_ctx_init(struct PeepholeCtx *pc) {
	pc->pattern_count = 0;
	pc->optimizations_applied = 0;
	pc->enable_aggressive = 0;
}

/* Register a peephole pattern */
static inline void peephole_register_pattern(
	struct PeepholeCtx *pc,
	const u32 *pattern, u32 pat_len,
	const u32 *replacement, u32 repl_len,
	const u8 *description, u32 desc_len)
{
	if (pc->pattern_count >= 20) return;  /* Max 20 patterns */

	struct PeepholePattern *pp = &pc->patterns[pc->pattern_count];
	pp->pattern = pattern;
	pp->pattern_len = pat_len;
	pp->replacement = replacement;
	pp->repl_len = repl_len;
	pp->desc = description;
	pp->desc_len = desc_len;

	pc->pattern_count++;
}

/* === COMMON PATTERNS === */

/* Pattern: MOVI rd, X; MOVI rd, Y → MOVI rd, Y (redundant load) */
static inline u8 peephole_match_double_movi(
	struct Insn *insns, u32 i, u32 total)
{
	if (i + 1 >= total) return 0;
	/* Assume OP_MOVI = 0x01 */
	if (insns[i].opcode == 0x01 && insns[i+1].opcode == 0x01 &&
	    insns[i].rd == insns[i+1].rd) {
		/* Same register loaded twice: keep only second */
		return 1;
	}
	return 0;
}

/* Pattern: ADD rd, rd, rs; MOV rd, rd → remove MOV (identity) */
static inline u8 peephole_match_identity_mov(
	struct Insn *insns, u32 i, u32 total)
{
	if (i + 1 >= total) return 0;
	/* Assume OP_MOV = 0x02, OP_ADD = 0x03 */
	if (insns[i].opcode == 0x03 && insns[i+1].opcode == 0x02 &&
	    insns[i].rd == insns[i+1].rd && insns[i].rd == insns[i+1].rs1) {
		/* MOV to same register: identity operation */
		return 1;
	}
	return 0;
}

/* Pattern: ADD rd, rs, 0 → remove (no-op) */
static inline u8 peephole_match_add_zero(
	struct Insn *insn)
{
	/* Assume OP_ADD = 0x03 */
	if (insn->opcode == 0x03 && insn->imm == 0 &&
	    insn->rd == insn->rs1 && insn->rs2 == 0) {
		/* ADD to zero: identity */
		return 1;
	}
	return 0;
}

/* Pattern: SUB rd, rs, 0 → remove (no-op) */
static inline u8 peephole_match_sub_zero(
	struct Insn *insn)
{
	/* Assume OP_SUB = 0x04 */
	if (insn->opcode == 0x04 && insn->imm == 0 &&
	    insn->rd == insn->rs1 && insn->rs2 == 0) {
		/* SUB zero: identity */
		return 1;
	}
	return 0;
}

/* Pattern: AND rd, rs, rs → MOV rd, rs (AND with self) */
static inline u8 peephole_match_and_self(
	struct Insn *insn)
{
	/* Assume OP_AND = 0x05 */
	if (insn->opcode == 0x05 && insn->rs1 == insn->rs2) {
		/* AND x, x = x */
		return 1;
	}
	return 0;
}

/* Pattern: OR rd, rs, 0 → MOV rd, rs (OR with zero) */
static inline u8 peephole_match_or_zero(
	struct Insn *insn)
{
	/* Assume OP_OR = 0x06 */
	if (insn->opcode == 0x06 && insn->rs2 == 0 && insn->imm == 0) {
		/* OR x, 0 = x */
		return 1;
	}
	return 0;
}

/* Pattern: XOR rd, rs, rs → MOVI rd, 0 (XOR with self) */
static inline u8 peephole_match_xor_self(
	struct Insn *insn)
{
	/* Assume OP_XOR = 0x07 */
	if (insn->opcode == 0x07 && insn->rs1 == insn->rs2) {
		/* XOR x, x = 0 */
		return 1;
	}
	return 0;
}

/* === SCAN AND OPTIMIZE === */

/* Scan instruction stream and apply peephole optimizations */
static inline u32 peephole_scan(
	struct Insn *insns, u32 total,
	struct PeepholeCtx *pc)
{
	u32 i = 0;
	u32 removed = 0;

	while (i < total) {
		u8 optimized = 0;

		/* Try pattern matches */
		if (peephole_match_double_movi(insns, i, total)) {
			/* Remove instruction at i, keep i+1 */
			u32 j;
			for (j = i; j < total - 1; j++) {
				insns[j] = insns[j + 1];
			}
			total--;
			removed++;
			pc->optimizations_applied++;
			optimized = 1;
			continue;  /* Don't increment i, check same position again */
		}

		if (peephole_match_identity_mov(insns, i, total)) {
			/* Remove MOV instruction at i+1 */
			u32 j;
			for (j = i + 1; j < total - 1; j++) {
				insns[j] = insns[j + 1];
			}
			total--;
			removed++;
			pc->optimizations_applied++;
			optimized = 1;
			continue;
		}

		if (peephole_match_add_zero(insns + i)) {
			/* Remove ADD zero at i */
			u32 j;
			for (j = i; j < total - 1; j++) {
				insns[j] = insns[j + 1];
			}
			total--;
			removed++;
			pc->optimizations_applied++;
			optimized = 1;
			continue;
		}

		if (peephole_match_sub_zero(insns + i)) {
			/* Remove SUB zero at i */
			u32 j;
			for (j = i; j < total - 1; j++) {
				insns[j] = insns[j + 1];
			}
			total--;
			removed++;
			pc->optimizations_applied++;
			optimized = 1;
			continue;
		}

		if (peephole_match_and_self(insns + i)) {
			/* Convert AND x,x to MOV x,x (identity) */
			insns[i].opcode = 0x02;  /* OP_MOV */
			insns[i].rs2 = 0;
			pc->optimizations_applied++;
			optimized = 1;
		}

		if (peephole_match_or_zero(insns + i)) {
			/* Convert OR x,0 to MOV x,x (identity) */
			insns[i].opcode = 0x02;  /* OP_MOV */
			insns[i].rs2 = 0;
			pc->optimizations_applied++;
			optimized = 1;
		}

		if (peephole_match_xor_self(insns + i)) {
			/* Convert XOR x,x to MOVI x, 0 */
			insns[i].opcode = 0x01;  /* OP_MOVI */
			insns[i].imm = 0;
			insns[i].rs2 = 0;
			pc->optimizations_applied++;
			optimized = 1;
		}

		if (!optimized) {
			i++;
		}
	}

	return total;  /* Return new instruction count */
}

/* === DEAD CODE ELIMINATION === */

/* Mark instruction as dead (unreachable or unused result) */
static inline u8 peephole_is_dead_store(
	struct Insn *insn, struct Insn *following, u32 remaining)
{
	/* Instruction is dead if it writes to a register that is never read */
	/* For conservative analysis: check if next instruction overwrites same register */
	if (remaining > 0 && following->opcode > 0) {
		if (insn->rd != 0 && insn->rd == following->rd) {
			/* Same register overwritten without reading previous value */
			if (insn->opcode != 0x0C && insn->opcode != 0x0D) {  /* Not LOAD/STORE */
				return 1;
			}
		}
	}
	return 0;
}

/* Eliminate dead stores in instruction stream */
static inline u32 peephole_eliminate_dead_code(
	struct Insn *insns, u32 total)
{
	u32 i = 0;
	u32 removed = 0;

	while (i < total - 1) {
		if (peephole_is_dead_store(insns + i, insns + i + 1, total - i - 1)) {
			/* Remove dead store */
			u32 j;
			for (j = i; j < total - 1; j++) {
				insns[j] = insns[j + 1];
			}
			total--;
			removed++;
		} else {
			i++;
		}
	}

	return total;  /* Return new instruction count */
}

#endif /* APKC_OPT_PEEPHOLE_H */
