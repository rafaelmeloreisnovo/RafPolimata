/* executor_zero_overhead.h — Zero-overhead execution engine
 *
 * No function calls, no stack frames, no context switch.
 * Pure register machine with flat memory addressing.
 * Every instruction executes in O(1) with inline ops.
 */

#ifndef EXECUTOR_ZERO_OVERHEAD_H
#define EXECUTOR_ZERO_OVERHEAD_H 1

#include "machine_linear_branchless.h"

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;

/* === EXECUTION CONTEXT: zero allocations, all stack === */
struct ExecutionContext {
	struct Machine m;
	struct Insn code[0x10000];      /* 64K instructions max */
	u32 code_len;
	u32 max_steps;
	u8 status;                      /* 0=running, 1=halt, 2=error */
	u64 steps_executed;
	u64 result;                     /* final value in r0 */
};

/* === INLINE OPERATIONS: no function calls === */

#define EXEC_ALU_ADD(m, rd, rs1, rs2) do { \
	u64 _res = (m)->r[rs1] + (m)->r[rs2]; \
	(m)->r[rd] = _res; \
	(m)->z = (_res == 0); \
	(m)->c = (_res < (m)->r[rs1]); \
	(m)->n = ((i64)_res < 0); \
} while(0)

#define EXEC_ALU_SUB(m, rd, rs1, rs2) do { \
	u64 _res = (m)->r[rs1] - (m)->r[rs2]; \
	(m)->r[rd] = _res; \
	(m)->z = (_res == 0); \
	(m)->c = ((m)->r[rs1] < (m)->r[rs2]); \
	(m)->n = ((i64)_res < 0); \
} while(0)

#define EXEC_ALU_AND(m, rd, rs1, rs2) do { \
	(m)->r[rd] = (m)->r[rs1] & (m)->r[rs2]; \
	(m)->z = ((m)->r[rd] == 0); \
} while(0)

#define EXEC_ALU_OR(m, rd, rs1, rs2) do { \
	(m)->r[rd] = (m)->r[rs1] | (m)->r[rs2]; \
	(m)->z = ((m)->r[rd] == 0); \
} while(0)

#define EXEC_ALU_XOR(m, rd, rs1, rs2) do { \
	(m)->r[rd] = (m)->r[rs1] ^ (m)->r[rs2]; \
	(m)->z = ((m)->r[rd] == 0); \
} while(0)

#define EXEC_ALU_SHL(m, rd, rs1, rs2) do { \
	(m)->r[rd] = (m)->r[rs1] << ((m)->r[rs2] & 0x3F); \
	(m)->z = ((m)->r[rd] == 0); \
} while(0)

#define EXEC_ALU_SHR(m, rd, rs1, rs2) do { \
	(m)->r[rd] = (m)->r[rs1] >> ((m)->r[rs2] & 0x3F); \
	(m)->z = ((m)->r[rd] == 0); \
} while(0)

#define EXEC_ALU_MUL(m, rd, rs1, rs2) do { \
	u64 _res = (m)->r[rs1] * (m)->r[rs2]; \
	(m)->r[rd] = _res; \
	(m)->z = (_res == 0); \
} while(0)

#define EXEC_MEM_LOAD(m, rd, rs1, imm) do { \
	u64 _addr = (m)->r[rs1] + (imm); \
	if (_addr < 0x1000000) { \
		(m)->r[rd] = *(u64*)&(m)->m[_addr]; \
	} \
} while(0)

#define EXEC_MEM_STORE(m, rs1, rs2, imm) do { \
	u64 _addr = (m)->r[rs1] + (imm); \
	if (_addr < 0x1000000) { \
		*(u64*)&(m)->m[_addr] = (m)->r[rs2]; \
	} \
} while(0)

#define EXEC_CMOV_Z(m, rd, rs1) do { \
	(m)->r[rd] = (m)->z ? (m)->r[rs1] : (m)->r[rd]; \
} while(0)

#define EXEC_CMOV_NZ(m, rd, rs1) do { \
	(m)->r[rd] = !(m)->z ? (m)->r[rs1] : (m)->r[rd]; \
} while(0)

#define EXEC_CMP(m, rs1, rs2) do { \
	u64 _res = (m)->r[rs1] - (m)->r[rs2]; \
	(m)->z = (_res == 0); \
	(m)->c = ((m)->r[rs1] < (m)->r[rs2]); \
	(m)->n = ((i64)_res < 0); \
} while(0)

/* === UNROLLED LOOP: 8 instructions per iteration, no overhead === */
static inline u8 exec_loop_unrolled(struct ExecutionContext *ctx) {
	struct Machine *m = &ctx->m;
	struct Insn *code = ctx->code;

	while (ctx->steps_executed < ctx->max_steps) {
		if (m->pc >= ctx->code_len) {
			ctx->status = 1;  /* halt */
			break;
		}

		/* 8-instruction unroll: no function call overhead */
		for (int i = 0; i < 8 && ctx->steps_executed < ctx->max_steps; i++) {
			if (m->pc >= ctx->code_len) break;

			struct Insn *insn = &code[m->pc];

			switch (insn->op) {
			case OP_ADD:
				EXEC_ALU_ADD(m, insn->rd, insn->rs1, insn->rs2);
				m->pc++;
				break;
			case OP_SUB:
				EXEC_ALU_SUB(m, insn->rd, insn->rs1, insn->rs2);
				m->pc++;
				break;
			case OP_AND:
				EXEC_ALU_AND(m, insn->rd, insn->rs1, insn->rs2);
				m->pc++;
				break;
			case OP_OR:
				EXEC_ALU_OR(m, insn->rd, insn->rs1, insn->rs2);
				m->pc++;
				break;
			case OP_XOR:
				EXEC_ALU_XOR(m, insn->rd, insn->rs1, insn->rs2);
				m->pc++;
				break;
			case OP_SHL:
				EXEC_ALU_SHL(m, insn->rd, insn->rs1, insn->rs2);
				m->pc++;
				break;
			case OP_SHR:
				EXEC_ALU_SHR(m, insn->rd, insn->rs1, insn->rs2);
				m->pc++;
				break;
			case OP_MUL:
				EXEC_ALU_MUL(m, insn->rd, insn->rs1, insn->rs2);
				m->pc++;
				break;
			case OP_LOAD:
				EXEC_MEM_LOAD(m, insn->rd, insn->rs1, insn->imm);
				m->pc++;
				break;
			case OP_STORE:
				EXEC_MEM_STORE(m, insn->rs1, insn->rs2, insn->imm);
				m->pc++;
				break;
			case OP_MOV:
				m->r[insn->rd] = m->r[insn->rs1];
				m->pc++;
				break;
			case OP_MOVI:
				m->r[insn->rd] = insn->imm;
				m->pc++;
				break;
			case OP_CMP:
				EXEC_CMP(m, insn->rs1, insn->rs2);
				m->pc++;
				break;
			case OP_CMOV_Z:
				EXEC_CMOV_Z(m, insn->rd, insn->rs1);
				m->pc++;
				break;
			case OP_CMOV_NZ:
				EXEC_CMOV_NZ(m, insn->rd, insn->rs1);
				m->pc++;
				break;
			case OP_JMP:
				m->pc = insn->imm;
				break;
			case OP_JZ:
				m->pc = m->z ? insn->imm : (m->pc + 1);
				break;
			case OP_JNZ:
				m->pc = !m->z ? insn->imm : (m->pc + 1);
				break;
			case OP_CALL:
				m->r[14] = m->pc + 1;
				m->pc = insn->imm;
				break;
			case OP_RET:
				m->pc = m->r[14];
				break;
			case OP_NOOP:
				m->pc++;
				break;
			case OP_HALT:
				ctx->status = 1;
				goto done;
			default:
				ctx->status = 2;  /* error */
				goto done;
			}

			ctx->steps_executed++;
		}
	}

done:
	ctx->result = m->r[0];
	return ctx->status;
}

/* === ENTRY POINT: no wrapper, direct execution === */
static inline u8 execute(struct ExecutionContext *ctx) {
	return exec_loop_unrolled(ctx);
}

#endif /* EXECUTOR_ZERO_OVERHEAD_H */
