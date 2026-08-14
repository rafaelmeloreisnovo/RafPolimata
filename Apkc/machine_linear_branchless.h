/* machine_linear_branchless.h — Zero-abstraction linear machine
 *
 * No variables, no branches, no overhead, no functions.
 * Direct state machine: register[N] + program counter + memory linear.
 * Every language compiles to this ONE format.
 */

#ifndef MACHINE_LINEAR_BRANCHLESS_H
#define MACHINE_LINEAR_BRANCHLESS_H 1

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;
typedef signed long long i64;

/* === MACHINE STATE: 16 registers + linear memory + PC === */
struct Machine {
	u64 r[16];              /* r0..r15: no names, just indices */
	u8 m[0x1000000];        /* 16 MiB linear memory */
	u32 pc;                 /* program counter */
	u32 sp;                 /* stack pointer (r14) */
	u8 z, c, n, v;          /* flags: zero, carry, negative, overflow */
};

/* === INSTRUCTIONS: 32-bit fixed format === */
struct Insn {
	u8 op;                  /* opcode: 0..255 */
	u8 rd;                  /* dest register */
	u8 rs1;                 /* src1 register */
	u8 rs2;                 /* src2 register */
	u32 imm;                /* immediate (if needed) */
};

/* === OPCODES (no names, just numbers) === */
enum {
	/* 0x00-0x0F: ALU */
	OP_LOAD = 0x00,         /* rd = m[rs1 + imm] */
	OP_STORE = 0x01,        /* m[rs1 + imm] = rs2 */
	OP_ADD = 0x02,          /* rd = rs1 + rs2 */
	OP_SUB = 0x03,          /* rd = rs1 - rs2 */
	OP_AND = 0x04,          /* rd = rs1 & rs2 */
	OP_OR = 0x05,           /* rd = rs1 | rs2 */
	OP_XOR = 0x06,          /* rd = rs1 ^ rs2 */
	OP_SHL = 0x07,          /* rd = rs1 << rs2 */
	OP_SHR = 0x08,          /* rd = rs1 >> rs2 */
	OP_MUL = 0x09,          /* rd = rs1 * rs2 */
	OP_DIV = 0x0A,          /* rd = rs1 / rs2 */
	OP_MOD = 0x0B,          /* rd = rs1 % rs2 */
	OP_CMP = 0x0C,          /* cmp rs1, rs2 (sets flags) */
	OP_MOV = 0x0D,          /* rd = rs1 */
	OP_MOVI = 0x0E,         /* rd = imm */
	OP_NOOP = 0x0F,         /* no-op (for padding) */

	/* 0x10-0x1F: Predicated (branchless) */
	OP_CMOV_Z = 0x10,       /* if Z: rd = rs1 */
	OP_CMOV_NZ = 0x11,      /* if !Z: rd = rs1 */
	OP_CMOV_C = 0x12,       /* if C: rd = rs1 */
	OP_CMOV_NC = 0x13,      /* if !C: rd = rs1 */
	OP_CMOV_N = 0x14,       /* if N: rd = rs1 */
	OP_CMOV_NN = 0x15,      /* if !N: rd = rs1 */

	/* 0x20-0x2F: Jumps (for language support only) */
	OP_JMP = 0x20,          /* pc = imm */
	OP_JZ = 0x21,           /* if Z: pc = imm */
	OP_JNZ = 0x22,          /* if !Z: pc = imm */
	OP_CALL = 0x23,         /* r14 (sp) = pc; pc = imm */
	OP_RET = 0x24,          /* pc = r14 (sp) */

	/* 0x30-0x3F: I/O */
	OP_IN = 0x30,           /* rd = input */
	OP_OUT = 0x31,          /* output rs1 */

	/* 0xFF: HALT */
	OP_HALT = 0xFF,         /* stop execution */
};

/* === EXECUTION: no function calls, just loop === */
static inline u8 exec_one(struct Machine *m, struct Insn *code, u32 code_size) {
	if (m->pc >= code_size) return 1;  /* halt */

	struct Insn *i = &code[m->pc];
	u64 a, b, res;
	u8 cond;

	switch (i->op) {
	/* ALU: no branches, compute always */
	case OP_ADD:
		res = m->r[i->rs1] + m->r[i->rs2];
		m->r[i->rd] = res;
		m->z = (res == 0);
		m->c = (res < m->r[i->rs1]);  /* carry if overflow */
		m->n = (i64)res < 0;
		m->pc++;
		break;

	case OP_SUB:
		res = m->r[i->rs1] - m->r[i->rs2];
		m->r[i->rd] = res;
		m->z = (res == 0);
		m->c = (m->r[i->rs1] < m->r[i->rs2]);
		m->n = (i64)res < 0;
		m->pc++;
		break;

	case OP_AND:
		m->r[i->rd] = m->r[i->rs1] & m->r[i->rs2];
		m->z = (m->r[i->rd] == 0);
		m->pc++;
		break;

	case OP_OR:
		m->r[i->rd] = m->r[i->rs1] | m->r[i->rs2];
		m->z = (m->r[i->rd] == 0);
		m->pc++;
		break;

	case OP_XOR:
		m->r[i->rd] = m->r[i->rs1] ^ m->r[i->rs2];
		m->z = (m->r[i->rd] == 0);
		m->pc++;
		break;

	case OP_SHL:
		m->r[i->rd] = m->r[i->rs1] << (m->r[i->rs2] & 0x3F);
		m->z = (m->r[i->rd] == 0);
		m->pc++;
		break;

	case OP_SHR:
		m->r[i->rd] = m->r[i->rs1] >> (m->r[i->rs2] & 0x3F);
		m->z = (m->r[i->rd] == 0);
		m->pc++;
		break;

	case OP_MUL:
		res = m->r[i->rs1] * m->r[i->rs2];
		m->r[i->rd] = res;
		m->z = (res == 0);
		m->pc++;
		break;

	case OP_DIV:
		if (m->r[i->rs2] == 0) return 1;  /* division by zero */
		m->r[i->rd] = m->r[i->rs1] / m->r[i->rs2];
		m->z = (m->r[i->rd] == 0);
		m->pc++;
		break;

	case OP_MOD:
		if (m->r[i->rs2] == 0) return 1;
		m->r[i->rd] = m->r[i->rs1] % m->r[i->rs2];
		m->z = (m->r[i->rd] == 0);
		m->pc++;
		break;

	case OP_CMP:
		res = m->r[i->rs1] - m->r[i->rs2];
		m->z = (res == 0);
		m->c = (m->r[i->rs1] < m->r[i->rs2]);
		m->n = (i64)res < 0;
		m->pc++;
		break;

	case OP_MOV:
		m->r[i->rd] = m->r[i->rs1];
		m->pc++;
		break;

	case OP_MOVI:
		m->r[i->rd] = i->imm;
		m->z = (i->imm == 0);
		m->pc++;
		break;

	/* Memory */
	case OP_LOAD:
		if (m->r[i->rs1] + i->imm >= 0x1000000) return 1;
		m->r[i->rd] = *(u64*)&m->m[m->r[i->rs1] + i->imm];
		m->pc++;
		break;

	case OP_STORE:
		if (m->r[i->rs1] + i->imm >= 0x1000000) return 1;
		*(u64*)&m->m[m->r[i->rs1] + i->imm] = m->r[i->rs2];
		m->pc++;
		break;

	/* Conditional moves (branchless) */
	case OP_CMOV_Z:
		m->r[i->rd] = m->z ? m->r[i->rs1] : m->r[i->rd];
		m->pc++;
		break;

	case OP_CMOV_NZ:
		m->r[i->rd] = !m->z ? m->r[i->rs1] : m->r[i->rd];
		m->pc++;
		break;

	case OP_CMOV_C:
		m->r[i->rd] = m->c ? m->r[i->rs1] : m->r[i->rd];
		m->pc++;
		break;

	case OP_CMOV_NC:
		m->r[i->rd] = !m->c ? m->r[i->rs1] : m->r[i->rd];
		m->pc++;
		break;

	case OP_CMOV_N:
		m->r[i->rd] = m->n ? m->r[i->rs1] : m->r[i->rd];
		m->pc++;
		break;

	case OP_CMOV_NN:
		m->r[i->rd] = !m->n ? m->r[i->rs1] : m->r[i->rd];
		m->pc++;
		break;

	/* Jumps (for languages that need them) */
	case OP_JMP:
		m->pc = i->imm;
		break;

	case OP_JZ:
		m->pc = m->z ? i->imm : (m->pc + 1);
		break;

	case OP_JNZ:
		m->pc = !m->z ? i->imm : (m->pc + 1);
		break;

	case OP_CALL:
		m->r[14] = m->pc + 1;  /* save return address in r14 */
		m->pc = i->imm;
		break;

	case OP_RET:
		m->pc = m->r[14];
		break;

	case OP_NOOP:
		m->pc++;
		break;

	case OP_HALT:
		return 1;

	default:
		return 1;  /* unknown opcode */
	}

	return 0;  /* continue */
}

/* === RUN: no function overhead, direct loop === */
static inline u8 run(struct Machine *m, struct Insn *code, u32 code_size, u32 max_steps) {
	u32 steps = 0;
	while (steps < max_steps) {
		if (exec_one(m, code, code_size)) return 1;  /* halt or error */
		steps++;
	}
	return 0;  /* success */
}

#endif /* MACHINE_LINEAR_BRANCHLESS_H */
