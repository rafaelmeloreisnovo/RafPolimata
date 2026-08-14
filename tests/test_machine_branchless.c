/* tests/test_machine_branchless.c — Zero-overhead machine tests
 *
 * No abstractions, no function calls except main.
 * Freestanding: no stdio, no stdlib.
 */

#include <stdio.h>

#include "../Apkc/machine_linear_branchless.h"
#include "../Apkc/compiler_language_direct.h"
#include "../Apkc/executor_zero_overhead.h"

static int pass = 0, fail = 0;

#define TEST(name, cond) do { \
	if (cond) { pass++; } else { fail++; printf("FAIL: %s\n", name); } \
} while(0)

/* Test 1: Machine init and basic register write */
static void test_machine_init(void) {
	static struct Machine m;
	for (int i = 0; i < 16; i++) m.r[i] = 0;
	for (int i = 0; i < 0x1000000; i += 0x100000) m.m[i] = 0;
	m.pc = 0;
	m.sp = 0x1000000 - 1;
	m.z = m.c = m.n = m.v = 0;

	TEST("machine_init: r0 zero", m.r[0] == 0);
	TEST("machine_init: pc zero", m.pc == 0);
}

/* Test 2: ADD instruction */
static void test_add_instruction(void) {
	static struct Machine m;
	for (int i = 0; i < 16; i++) m.r[i] = 0;
	m.pc = 0;
	m.z = m.c = m.n = m.v = 0;

	m.r[0] = 5;
	m.r[1] = 3;

	struct Insn code[10];
	code[0].op = OP_ADD;
	code[0].rd = 2;
	code[0].rs1 = 0;
	code[0].rs2 = 1;
	code[0].imm = 0;

	exec_one(&m, code, 10);

	TEST("ADD: r2 = r0 + r1", m.r[2] == 8);
	TEST("ADD: z flag clear", !m.z);
}

/* Test 3: Subtract with flags */
static void test_sub_instruction(void) {
	static struct Machine m;
	for (int i = 0; i < 16; i++) m.r[i] = 0;
	m.pc = 0;
	m.z = m.c = m.n = m.v = 0;

	m.r[0] = 5;
	m.r[1] = 5;

	struct Insn code[10];
	code[0].op = OP_SUB;
	code[0].rd = 2;
	code[0].rs1 = 0;
	code[0].rs2 = 1;
	code[0].imm = 0;

	exec_one(&m, code, 10);

	TEST("SUB: r2 = r0 - r1", m.r[2] == 0);
	TEST("SUB: z flag set on zero", m.z);
}

/* Test 4: Immediate move */
static void test_movi_instruction(void) {
	static struct Machine m;
	for (int i = 0; i < 16; i++) m.r[i] = 0;
	m.pc = 0;
	m.z = m.c = m.n = m.v = 0;

	struct Insn code[10];
	code[0].op = OP_MOVI;
	code[0].rd = 3;
	code[0].rs1 = 0;
	code[0].rs2 = 0;
	code[0].imm = 42;

	exec_one(&m, code, 10);

	TEST("MOVI: r3 = 42", m.r[3] == 42);
}

/* Test 5: Branchless conditional move */
static void test_cmov_instruction(void) {
	static struct Machine m;
	for (int i = 0; i < 16; i++) m.r[i] = 0;
	m.pc = 0;
	m.z = 1;  /* set zero flag */
	m.c = m.n = m.v = 0;

	m.r[0] = 100;
	m.r[1] = 7;

	struct Insn code[10];
	code[0].op = OP_CMOV_Z;
	code[0].rd = 1;
	code[0].rs1 = 0;
	code[0].rs2 = 0;
	code[0].imm = 0;

	exec_one(&m, code, 10);

	TEST("CMOV_Z: when Z=1, r1 = r0", m.r[1] == 100);
}

/* Test 6: Bitwise AND */
static void test_and_instruction(void) {
	static struct Machine m;
	for (int i = 0; i < 16; i++) m.r[i] = 0;
	m.pc = 0;
	m.z = m.c = m.n = m.v = 0;

	m.r[0] = 0xFF;
	m.r[1] = 0x0F;

	struct Insn code[10];
	code[0].op = OP_AND;
	code[0].rd = 2;
	code[0].rs1 = 0;
	code[0].rs2 = 1;
	code[0].imm = 0;

	exec_one(&m, code, 10);

	TEST("AND: r2 = r0 & r1", m.r[2] == 0x0F);
}

/* Test 7: Shift left */
static void test_shl_instruction(void) {
	static struct Machine m;
	for (int i = 0; i < 16; i++) m.r[i] = 0;
	m.pc = 0;
	m.z = m.c = m.n = m.v = 0;

	m.r[0] = 1;
	m.r[1] = 3;  /* shift amount */

	struct Insn code[10];
	code[0].op = OP_SHL;
	code[0].rd = 2;
	code[0].rs1 = 0;
	code[0].rs2 = 1;
	code[0].imm = 0;

	exec_one(&m, code, 10);

	TEST("SHL: r2 = r0 << 3", m.r[2] == 8);
}

/* Test 8: Execution context run */
static void test_execution_context(void) {
	static struct ExecutionContext ctx;
	for (int i = 0; i < 16; i++) ctx.m.r[i] = 0;
	ctx.m.pc = 0;
	ctx.m.z = ctx.m.c = ctx.m.n = ctx.m.v = 0;
	ctx.code_len = 3;
	ctx.max_steps = 100;
	ctx.status = 0;
	ctx.steps_executed = 0;
	ctx.result = 0;

	/* Program: r0=5, r1=3, r2=r0+r1 */
	ctx.code[0].op = OP_MOVI;
	ctx.code[0].rd = 0;
	ctx.code[0].imm = 5;

	ctx.code[1].op = OP_MOVI;
	ctx.code[1].rd = 1;
	ctx.code[1].imm = 3;

	ctx.code[2].op = OP_ADD;
	ctx.code[2].rd = 2;
	ctx.code[2].rs1 = 0;
	ctx.code[2].rs2 = 1;

	execute(&ctx);

	TEST("execution: r2 = 8", ctx.m.r[2] == 8);
	TEST("execution: status halt", ctx.status == 1);
	TEST("execution: steps > 0", ctx.steps_executed > 0);
}

/* Test 9: Compare instruction */
static void test_cmp_instruction(void) {
	static struct Machine m;
	for (int i = 0; i < 16; i++) m.r[i] = 0;
	m.pc = 0;
	m.z = m.c = m.n = m.v = 0;

	m.r[0] = 10;
	m.r[1] = 10;

	struct Insn code[10];
	code[0].op = OP_CMP;
	code[0].rd = 0;
	code[0].rs1 = 0;
	code[0].rs2 = 1;
	code[0].imm = 0;

	exec_one(&m, code, 10);

	TEST("CMP: z flag set when equal", m.z);
	TEST("CMP: c flag clear when not less", !m.c);
}

/* Test 10: Multiply instruction */
static void test_mul_instruction(void) {
	static struct Machine m;
	for (int i = 0; i < 16; i++) m.r[i] = 0;
	m.pc = 0;
	m.z = m.c = m.n = m.v = 0;

	m.r[0] = 6;
	m.r[1] = 7;

	struct Insn code[10];
	code[0].op = OP_MUL;
	code[0].rd = 2;
	code[0].rs1 = 0;
	code[0].rs2 = 1;
	code[0].imm = 0;

	exec_one(&m, code, 10);

	TEST("MUL: r2 = r0 * r1", m.r[2] == 42);
}

/* Test 11: Compiler routing */
static void test_compiler_routing(void) {
	static struct UniversalCompiler uc;
	static struct Insn buf[100];

	codegen_init(&uc.cg, buf, 100);

	compile_universal(&uc, (const u8*)"test", 4, 0);

	TEST("compiler: code generated", uc.cg.pos > 0);
	TEST("compiler: opcode ADD", buf[0].op == OP_ADD);
	TEST("compiler: return instruction", buf[1].op == OP_RET);
}

int main(void) {
	printf("=== Branchless Machine Tests ===\n\n");

	test_machine_init();
	test_add_instruction();
	test_sub_instruction();
	test_movi_instruction();
	test_cmov_instruction();
	test_and_instruction();
	test_shl_instruction();
	test_execution_context();
	test_cmp_instruction();
	test_mul_instruction();
	test_compiler_routing();

	printf("\n=== Summary ===\n");
	printf("PASS: %d\n", pass);
	printf("FAIL: %d\n", fail);

	return fail > 0 ? 1 : 0;
}
