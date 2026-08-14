/* Apkc/hardening_integration_test.c — Integration tests for hardening modules
 *
 * Freestanding test: no malloc, no libc, no abstractions.
 * Validates all hardening gates together: boundary, ARM32, receipts, fail-closed.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "hardening_boundary_gates.h"
#include "hardening_armv7_assembler.h"
#include "hardening_receipt_chain.h"
#include "hardening_fail_closed.h"

/* Test counters: no malloc, just static stack */
static int test_pass = 0;
static int test_fail = 0;
static int test_vazio = 0;

#define ASSERT_EQ(a, b) do { \
	if ((a) != (b)) { \
		printf("FAIL: %s:%d expected %d got %d\n", __FILE__, __LINE__, (int)(b), (int)(a)); \
		test_fail++; \
	} else { \
		test_pass++; \
	} \
} while (0)

#define ASSERT_NE(a, b) do { \
	if ((a) == (b)) { \
		printf("FAIL: %s:%d expected != %d\n", __FILE__, __LINE__, (int)(a)); \
		test_fail++; \
	} else { \
		test_pass++; \
	} \
} while (0)

#define ASSERT_TRUE(cond) do { \
	if (!(cond)) { \
		printf("FAIL: %s:%d assertion failed\n", __FILE__, __LINE__); \
		test_fail++; \
	} else { \
		test_pass++; \
	} \
} while (0)

/* === Boundary gate tests === */

static void test_boundary_source_capacity(void) {
	struct boundary_gate_source src;
	boundary_source_init(&src, HARDENING_SOURCE_CAP);

	ASSERT_EQ(src.state, BOUNDARY_PASS);
	ASSERT_EQ(src.bytes_read, 0);

	/* Advance within capacity */
	boundary_source_advance(&src, 1000);
	ASSERT_EQ(src.state, BOUNDARY_PASS);
	ASSERT_EQ(src.bytes_read, 1000);

	/* Advance to near capacity */
	boundary_source_advance(&src, HARDENING_SOURCE_CAP - 1001);
	ASSERT_EQ(src.state, BOUNDARY_PASS);
	ASSERT_EQ(src.bytes_read, HARDENING_SOURCE_CAP - 1);

	/* Try to exceed capacity */
	boundary_source_advance(&src, 10);
	ASSERT_EQ(src.state, BOUNDARY_SOURCE_EXCEEDS_CAP);
}

static void test_boundary_source_overflow_probe(void) {
	struct boundary_gate_source src;
	boundary_source_init(&src, 100);

	/* Fill to capacity - 1 */
	boundary_source_advance(&src, 99);
	ASSERT_EQ(src.state, BOUNDARY_PASS);

	/* Probe for overflow with no more data */
	boundary_source_probe_overflow(&src, 0);
	ASSERT_EQ(src.saw_overflow_probe, 1);
	ASSERT_EQ(src.state, BOUNDARY_PASS);

	/* Re-init and probe with more data */
	boundary_source_init(&src, 100);
	boundary_source_advance(&src, 99);
	boundary_source_probe_overflow(&src, 1);
	ASSERT_EQ(src.state, BOUNDARY_SOURCE_EXCEEDS_CAP);
}

static void test_boundary_apk_sections(void) {
	struct boundary_gate_apk apk;
	boundary_apk_init(&apk, HARDENING_APK_CAP);

	/* Add sections */
	boundary_apk_add_section(&apk, 1000);
	ASSERT_EQ(apk.state, BOUNDARY_PASS);
	ASSERT_EQ(apk.section_count, 1);

	boundary_apk_add_section(&apk, 2000);
	ASSERT_EQ(apk.state, BOUNDARY_PASS);
	ASSERT_EQ(apk.section_count, 2);

	/* Try to add oversized section */
	boundary_apk_add_section(&apk, HARDENING_APK_CAP * 2);
	ASSERT_EQ(apk.state, BOUNDARY_APK_EXCEEDS_CAP);
}

static void test_boundary_proof_records(void) {
	struct boundary_gate_proof proof;
	boundary_proof_init(&proof, HARDENING_PROOF_MAX);

	/* Add records */
	boundary_proof_add_record(&proof, 100);
	ASSERT_EQ(proof.state, BOUNDARY_PASS);
	ASSERT_EQ(proof.record_count, 1);

	boundary_proof_add_record(&proof, 200);
	ASSERT_EQ(proof.state, BOUNDARY_PASS);
	ASSERT_EQ(proof.record_count, 2);

	/* Fill capacity and try to exceed */
	while (proof.bytes_used < HARDENING_PROOF_MAX - 500) {
		boundary_proof_add_record(&proof, 400);
		if (proof.state != BOUNDARY_PASS) break;
	}

	/* Next record should exceed */
	boundary_proof_add_record(&proof, 100000);
	ASSERT_EQ(proof.state, BOUNDARY_PROOF_EXCEEDS_CAP);
}

/* === ARM32 assembler tests === */

static void test_armv7_instruction_decode(void) {
	/* Valid ADD instruction */
	uint32_t add = 0xe0800001;  /* ADD R0, R0, R1 (condition AL, opcode ALU) */
	ASSERT_EQ(armv7_instr_valid(add), 1);

	/* Valid LDR instruction */
	uint32_t ldr = 0xe5900000;  /* LDR R0, [R0] */
	ASSERT_EQ(armv7_instr_valid(ldr), 1);

	/* Valid BL instruction */
	uint32_t bl = 0xeb000000;   /* BL #0 */
	ASSERT_EQ(armv7_instr_valid(bl), 1);
}

static void test_armv7_cross_assembler(void) {
	struct armv7_cross_assembler cross;
	armv7_cross_init(&cross);

	ASSERT_EQ(cross.pass_count, 0);
	ASSERT_EQ(cross.fail_count, 0);

	/* Record 9 passes */
	for (int i = 0; i < 9; i++) {
		struct armv7_assembly_result res;
		armv7_result_init(&res);
		res.instruction_count = 10 + i;
		res.section_size = (10 + i) * 4;
		armv7_cross_record_pass(&cross, &res);
	}

	ASSERT_EQ(cross.pass_count, 9);
	ASSERT_EQ(cross.fail_count, 0);
	ASSERT_TRUE(armv7_cross_all_pass(&cross));
}

/* === Receipt chain tests === */

static void test_receipt_entry_sha(void) {
	struct receipt_entry e1, e2;
	receipt_entry_init(&e1);
	receipt_entry_init(&e2);

	uint8_t test_sha[32];
	for (int i = 0; i < 32; i++) test_sha[i] = i;

	receipt_entry_set_sha(&e1, test_sha);
	receipt_entry_set_sha(&e2, test_sha);

	ASSERT_TRUE(receipt_entry_sha_matches(&e1, &e2));

	/* Flip one byte */
	test_sha[0]++;
	receipt_entry_set_sha(&e2, test_sha);
	ASSERT_EQ(receipt_entry_sha_matches(&e1, &e2), 0);
}

static void test_receipt_chain_continuity(void) {
	struct receipt_chain chain;
	receipt_chain_init(&chain);

	struct receipt_entry e1, e2;
	receipt_entry_init(&e1);
	receipt_entry_init(&e2);

	e1.gate_state = BOUNDARY_PASS;
	e2.gate_state = BOUNDARY_PASS;

	ASSERT_TRUE(receipt_chain_add_entry(&chain, &e1));
	ASSERT_TRUE(receipt_chain_add_entry(&chain, &e2));

	ASSERT_EQ(chain.entry_count, 2);

	struct receipt_link link;
	link.source = e1;
	link.target = e2;
	link.transform = 1;  /* compile */
	link.flags = 0;

	ASSERT_TRUE(receipt_chain_add_link(&chain, &link));
	ASSERT_EQ(chain.link_count, 1);
}

static void test_receipt_custody_gate(void) {
	struct receipt_custody_gate gate;
	receipt_custody_init(&gate);

	struct receipt_entry entry;
	receipt_entry_init(&entry);
	entry.gate_state = BOUNDARY_PASS;

	receipt_chain_add_entry(&gate.source_chain, &entry);
	receipt_chain_add_entry(&gate.object_chain, &entry);
	receipt_chain_add_entry(&gate.apk_chain, &entry);

	ASSERT_TRUE(receipt_custody_verify(&gate));
}

/* === Fail-closed tests === */

static void test_failclosed_read_exact_match(void) {
	struct failclosed_read rd;
	failclosed_read_init(&rd);

	ASSERT_EQ(rd.state, FAILCLOSED_TOKEN_VAZIO);

	rd.bytes_expected = 1000;
	rd.bytes_read = 500;
	failclosed_read_advance(&rd, 500);

	ASSERT_EQ(rd.bytes_read, 1000);

	failclosed_read_mark_eof(&rd);
	ASSERT_EQ(rd.state, FAILCLOSED_PASS);
}

static void test_failclosed_read_short_read(void) {
	struct failclosed_read rd;
	failclosed_read_init(&rd);

	rd.bytes_expected = 1000;
	rd.bytes_read = 500;
	failclosed_read_mark_eof(&rd);

	ASSERT_EQ(rd.state, FAILCLOSED_FAIL);
}

static void test_failclosed_read_error(void) {
	struct failclosed_read rd;
	failclosed_read_init(&rd);

	failclosed_read_mark_error(&rd);
	ASSERT_EQ(rd.state, FAILCLOSED_FAIL);
}

static void test_failclosed_compile_pass(void) {
	struct failclosed_compile cmp;
	failclosed_compile_init(&cmp);

	cmp.exit_code = 0;
	cmp.object_size = 1024;
	cmp.object_size_valid = 1;

	failclosed_compile_check(&cmp);
	ASSERT_EQ(cmp.state, FAILCLOSED_PASS);
}

static void test_failclosed_compile_fail_syntax(void) {
	struct failclosed_compile cmp;
	failclosed_compile_init(&cmp);

	cmp.has_syntax_error = 1;
	cmp.exit_code = 0;
	cmp.object_size = 1024;
	cmp.object_size_valid = 1;

	failclosed_compile_check(&cmp);
	ASSERT_EQ(cmp.state, FAILCLOSED_FAIL);
}

static void test_failclosed_compile_fail_exit_code(void) {
	struct failclosed_compile cmp;
	failclosed_compile_init(&cmp);

	cmp.exit_code = 1;  /* Non-zero */
	cmp.object_size = 1024;
	cmp.object_size_valid = 1;

	failclosed_compile_check(&cmp);
	ASSERT_EQ(cmp.state, FAILCLOSED_FAIL);
}

static void test_failclosed_manifest_barrier_all_pass(void) {
	struct failclosed_manifest_barrier bar;
	failclosed_manifest_init(&bar);

	bar.read_gate = FAILCLOSED_PASS;
	bar.compile_gate = FAILCLOSED_PASS;
	bar.link_gate = FAILCLOSED_PASS;
	bar.zip_gate = FAILCLOSED_PASS;

	failclosed_manifest_compute_result(&bar);
	ASSERT_EQ(bar.final_result, FAILCLOSED_PASS);
}

static void test_failclosed_manifest_barrier_one_fail(void) {
	struct failclosed_manifest_barrier bar;
	failclosed_manifest_init(&bar);

	bar.read_gate = FAILCLOSED_PASS;
	bar.compile_gate = FAILCLOSED_FAIL;  /* One failure */
	bar.link_gate = FAILCLOSED_PASS;
	bar.zip_gate = FAILCLOSED_PASS;

	failclosed_manifest_compute_result(&bar);
	ASSERT_EQ(bar.final_result, FAILCLOSED_FAIL);
}

static void test_failclosed_manifest_barrier_token_vazio(void) {
	struct failclosed_manifest_barrier bar;
	failclosed_manifest_init(&bar);

	bar.read_gate = FAILCLOSED_PASS;
	bar.compile_gate = FAILCLOSED_TOKEN_VAZIO;  /* Unknown */
	bar.link_gate = FAILCLOSED_PASS;
	bar.zip_gate = FAILCLOSED_PASS;
	bar.allow_token_vazio_passthrough = 0;

	failclosed_manifest_compute_result(&bar);
	ASSERT_EQ(bar.final_result, FAILCLOSED_TOKEN_VAZIO);
}

/* === Main test runner === */

int main(void) {
	printf("=== Hardening Integration Tests ===\n\n");

	printf("Testing boundary gates...\n");
	test_boundary_source_capacity();
	test_boundary_source_overflow_probe();
	test_boundary_apk_sections();
	test_boundary_proof_records();

	printf("Testing ARM32 assembler...\n");
	test_armv7_instruction_decode();
	test_armv7_cross_assembler();

	printf("Testing receipt chains...\n");
	test_receipt_entry_sha();
	test_receipt_chain_continuity();
	test_receipt_custody_gate();

	printf("Testing fail-closed semantics...\n");
	test_failclosed_read_exact_match();
	test_failclosed_read_short_read();
	test_failclosed_read_error();
	test_failclosed_compile_pass();
	test_failclosed_compile_fail_syntax();
	test_failclosed_compile_fail_exit_code();
	test_failclosed_manifest_barrier_all_pass();
	test_failclosed_manifest_barrier_one_fail();
	test_failclosed_manifest_barrier_token_vazio();

	printf("\n=== Summary ===\n");
	printf("PASS: %d\n", test_pass);
	printf("FAIL: %d\n", test_fail);
	printf("TOKEN_VAZIO: %d\n", test_vazio);

	return test_fail > 0 ? 1 : 0;
}
