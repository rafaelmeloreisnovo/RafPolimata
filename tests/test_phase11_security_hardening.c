/* test_phase11_security_hardening.c — Phase 11 Security Hardening Tests
 *
 * Comprehensive test suite for:
 * - Stage 11.1: Input Validation & Fuzzing
 * - Stage 11.2: Memory Safety & Bounds Checking
 * - Stage 11.3: Proof of Correctness & Semantic Claims
 * - Stage 11.4: Hardening Gates & Audit Logging
 *
 * FREESTANDING: No malloc, no libc.
 */

#include <stdio.h>
#include <string.h>

/* Include Phase 11 headers */
#include "Apkc/sec_input_validation.h"
#include "Apkc/sec_memory_safety.h"
#include "Apkc/sec_proof_correctness.h"
#include "Apkc/sec_hardening_gates.h"

/* ============================================================ */
/* STAGE 11.1: INPUT VALIDATION TESTS */
/* ============================================================ */

static int test_val_ptr_nonnull(void) {
	void *valid_ptr = (void *)0x1000;
	void *null_ptr = NULL;

	if (val_ptr_nonnull(valid_ptr) != VAL_OK) {
		printf("❌ test_val_ptr_nonnull: valid pointer rejected\n");
		return 1;
	}

	if (val_ptr_nonnull(null_ptr) != VAL_NULL_PTR) {
		printf("❌ test_val_ptr_nonnull: null pointer not detected\n");
		return 1;
	}

	printf("✓ test_val_ptr_nonnull\n");
	return 0;
}

static int test_val_size_bounds(void) {
	if (val_size_bounds(100, 200) != VAL_OK) {
		printf("❌ test_val_size_bounds: valid size rejected\n");
		return 1;
	}

	if (val_size_bounds(300, 200) != VAL_SIZE_EXCEEDED) {
		printf("❌ test_val_size_bounds: overflow not detected\n");
		return 1;
	}

	printf("✓ test_val_size_bounds\n");
	return 0;
}

static int test_val_alignment(void) {
	if (val_alignment(0x1000, 8) != VAL_OK) {
		printf("❌ test_val_alignment: valid alignment rejected\n");
		return 1;
	}

	if (val_alignment(0x1001, 8) != VAL_ALIGNMENT) {
		printf("❌ test_val_alignment: misalignment not detected\n");
		return 1;
	}

	printf("✓ test_val_alignment\n");
	return 0;
}

static int test_val_range(void) {
	if (val_range(50, 0, 100) != VAL_OK) {
		printf("❌ test_val_range: valid value rejected\n");
		return 1;
	}

	if (val_range(150, 0, 100) != VAL_RANGE) {
		printf("❌ test_val_range: out-of-range not detected\n");
		return 1;
	}

	printf("✓ test_val_range\n");
	return 0;
}

static int test_val_add_overflow(void) {
	u64 result;

	if (val_add_overflow(100, 50, &result) != VAL_OK || result != 150) {
		printf("❌ test_val_add_overflow: valid addition failed\n");
		return 1;
	}

	if (val_add_overflow(0xffffffffffffffff, 1, &result) != VAL_OVERFLOW) {
		printf("❌ test_val_add_overflow: overflow not detected\n");
		return 1;
	}

	printf("✓ test_val_add_overflow\n");
	return 0;
}

static int test_val_utf8(void) {
	u8 valid_utf8[] = "Hello";
	if (val_utf8(valid_utf8, 5) != VAL_OK) {
		printf("❌ test_val_utf8: valid UTF-8 rejected\n");
		return 1;
	}

	u8 invalid_utf8[] = {0xff, 0xfe};
	if (val_utf8(invalid_utf8, 2) != VAL_ENCODING_BAD) {
		printf("❌ test_val_utf8: invalid encoding not detected\n");
		return 1;
	}

	printf("✓ test_val_utf8\n");
	return 0;
}

static int test_fuzz_generate(void) {
	struct FuzzContext fc = {0};
	u8 seed[] = {1, 2, 3, 4};
	fuzz_init(&fc, seed, 4);

	u32 size = fuzz_generate(&fc, 256);
	if (size > 256) {
		printf("❌ test_fuzz_generate: buffer size exceeded limit\n");
		return 1;
	}

	if (fuzz_done(&fc)) {
		printf("❌ test_fuzz_generate: should not be done after 1 iteration\n");
		return 1;
	}

	printf("✓ test_fuzz_generate\n");
	return 0;
}

/* ============================================================ */
/* STAGE 11.2: MEMORY SAFETY TESTS */
/* ============================================================ */

static int test_canary_operations(void) {
	u8 buffer[64] = {0};

	canary_write_before(&buffer[8]);
	canary_write_after(&buffer[8], 32);

	if (canary_check_before(&buffer[8]) != MEMSAFE_OK) {
		printf("❌ test_canary_operations: pre-canary check failed\n");
		return 1;
	}

	if (canary_check_after(&buffer[8], 32) != MEMSAFE_OK) {
		printf("❌ test_canary_operations: post-canary check failed\n");
		return 1;
	}

	printf("✓ test_canary_operations\n");
	return 0;
}

static int test_memsafety_alloc_free(void) {
	struct MemSafetyTracker mst = {0};
	memsafety_init(&mst);

	void *ptr = (void *)0x2000;
	if (memsafety_alloc(&mst, ptr, 256) != MEMSAFE_OK) {
		printf("❌ test_memsafety_alloc_free: allocation failed\n");
		return 1;
	}

	if (memsafety_free(&mst, ptr) != MEMSAFE_OK) {
		printf("❌ test_memsafety_alloc_free: free failed\n");
		return 1;
	}

	if (memsafety_free(&mst, ptr) != MEMSAFE_DOUBLE_FREE) {
		printf("❌ test_memsafety_alloc_free: double-free not detected\n");
		return 1;
	}

	printf("✓ test_memsafety_alloc_free\n");
	return 0;
}

static int test_bounds_check_range(void) {
	if (bounds_check_range(0, 10, 100) != MEMSAFE_OK) {
		printf("❌ test_bounds_check_range: valid range rejected\n");
		return 1;
	}

	if (bounds_check_range(50, 50, 100) != MEMSAFE_OK) {
		printf("❌ test_bounds_check_range: boundary case rejected\n");
		return 1;
	}

	if (bounds_check_range(50, 60, 100) != MEMSAFE_OVERFLOW) {
		printf("❌ test_bounds_check_range: overflow not detected\n");
		return 1;
	}

	printf("✓ test_bounds_check_range\n");
	return 0;
}

static int test_align_check_u64(void) {
	void *aligned_ptr = (void *)0x2000;  /* 8-byte aligned */
	void *unaligned_ptr = (void *)0x2001;

	if (align_check_u64(aligned_ptr) != MEMSAFE_OK) {
		printf("❌ test_align_check_u64: aligned pointer rejected\n");
		return 1;
	}

	if (align_check_u64(unaligned_ptr) != MEMSAFE_MISALIGNED) {
		printf("❌ test_align_check_u64: misalignment not detected\n");
		return 1;
	}

	printf("✓ test_align_check_u64\n");
	return 0;
}

/* ============================================================ */
/* STAGE 11.3: PROOF OF CORRECTNESS TESTS */
/* ============================================================ */

/* Test function: returns double the input */
static u64 test_fn_double(u64 x) {
	return x * 2;
}

/* Test falsifier: verify double function */
static u8 falsify_double_correct(void) {
	return proof_returns_value(test_fn_double, 5, 10);
}

static int test_proof_returns_value(void) {
	if (falsify_double_correct() != PROOF_PASS) {
		printf("❌ test_proof_returns_value: proof failed for correct function\n");
		return 1;
	}

	printf("✓ test_proof_returns_value\n");
	return 0;
}

/* Test falsifier: verify determinism */
static u8 falsify_deterministic(void) {
	return proof_deterministic(test_fn_double, 7);
}

static int test_proof_deterministic(void) {
	if (falsify_deterministic() != PROOF_PASS) {
		printf("❌ test_proof_deterministic: proof failed for deterministic function\n");
		return 1;
	}

	printf("✓ test_proof_deterministic\n");
	return 0;
}

/* Test falsifier: verify range */
static u8 falsify_output_range(void) {
	return proof_output_range(test_fn_double, 5, 5, 15);
}

static int test_proof_output_range(void) {
	if (falsify_output_range() != PROOF_PASS) {
		printf("❌ test_proof_output_range: proof failed for output range\n");
		return 1;
	}

	printf("✓ test_proof_output_range\n");
	return 0;
}

static int test_proof_context(void) {
	struct ProofContext pc = {0};
	proof_init(&pc);

	if (proof_add_obligation(&pc, "double_correct", "x*2 == 2x",
							 falsify_double_correct, 1) != PROOF_PASS) {
		printf("❌ test_proof_context: add_obligation failed\n");
		return 1;
	}

	if (proof_execute_all(&pc) != PROOF_PASS) {
		printf("❌ test_proof_context: proof execution failed\n");
		return 1;
	}

	if (pc.passed == 0) {
		printf("❌ test_proof_context: no proofs passed\n");
		return 1;
	}

	printf("✓ test_proof_context\n");
	return 0;
}

/* ============================================================ */
/* STAGE 11.4: HARDENING GATES TESTS */
/* ============================================================ */

static int test_harden_init(void) {
	struct HardeningContext hc = {0};
	harden_init(&hc);

	if (hc.fail_closed != 1) {
		printf("❌ test_harden_init: fail_closed not set\n");
		return 1;
	}

	if (hc.tamper_detected != 0) {
		printf("❌ test_harden_init: tamper flag not cleared\n");
		return 1;
	}

	printf("✓ test_harden_init\n");
	return 0;
}

static int test_harden_seal_verify(void) {
	struct HardeningContext hc = {0};
	harden_init(&hc);

	u8 state[] = {1, 2, 3, 4, 5};
	harden_seal_state(&hc, state, 5);

	if (harden_verify_seal(&hc, state, 5) != GATE_PASS) {
		printf("❌ test_harden_seal_verify: seal verification failed\n");
		return 1;
	}

	/* Tamper with state */
	state[0] = 0xff;

	if (harden_verify_seal(&hc, state, 5) != GATE_REJECT) {
		printf("❌ test_harden_seal_verify: tampering not detected\n");
		return 1;
	}

	printf("✓ test_harden_seal_verify\n");
	return 0;
}

static int test_harden_audit_log(void) {
	struct HardeningContext hc = {0};
	harden_init(&hc);

	u8 state[] = {1, 2, 3};
	harden_audit(&hc, "test_op", state, 3, 1);

	if (hc.audit_log.checkpoint_count != 1) {
		printf("❌ test_harden_audit_log: checkpoint not logged\n");
		return 1;
	}

	struct AuditCheckpoint *cp = harden_get_audit(&hc, 0);
	if (cp == NULL || cp->gate_passed != 1) {
		printf("❌ test_harden_audit_log: checkpoint data invalid\n");
		return 1;
	}

	printf("✓ test_harden_audit_log\n");
	return 0;
}

static int test_harden_gate_precondition(void) {
	struct HardeningContext hc = {0};
	harden_init(&hc);

	if (harden_gate_precondition(&hc, 1) != GATE_PASS) {
		printf("❌ test_harden_gate_precondition: valid condition rejected\n");
		return 1;
	}

	if (harden_gate_precondition(&hc, 0) != GATE_REJECT) {
		printf("❌ test_harden_gate_precondition: invalid condition not rejected\n");
		return 1;
	}

	printf("✓ test_harden_gate_precondition\n");
	return 0;
}

static int test_harden_detect_tampering(void) {
	struct HardeningContext hc = {0};
	harden_init(&hc);

	if (harden_detect_tampering(&hc) != 0) {
		printf("❌ test_harden_detect_tampering: false positive\n");
		return 1;
	}

	hc.tamper_detected = 1;

	if (harden_detect_tampering(&hc) == 0) {
		printf("❌ test_harden_detect_tampering: tampering flag not detected\n");
		return 1;
	}

	printf("✓ test_harden_detect_tampering\n");
	return 0;
}

static int test_harden_checkpoint(void) {
	struct HardeningContext hc = {0};
	harden_init(&hc);

	u8 state[] = {10, 20, 30};

	u8 result = harden_checkpoint(&hc, "test_op", state, 3, 1, 1, 1);

	if (result != GATE_PASS) {
		printf("❌ test_harden_checkpoint: checkpoint failed\n");
		return 1;
	}

	if (hc.operations_count != 1) {
		printf("❌ test_harden_checkpoint: operation count not incremented\n");
		return 1;
	}

	if (hc.audit_log.checkpoint_count != 1) {
		printf("❌ test_harden_checkpoint: audit log not updated\n");
		return 1;
	}

	printf("✓ test_harden_checkpoint\n");
	return 0;
}

/* ============================================================ */
/* MAIN TEST RUNNER */
/* ============================================================ */

int main(void) {
	printf("=== Phase 11: Security Hardening Tests ===\n\n");

	int failed = 0;

	printf("Stage 11.1: Input Validation & Fuzzing\n");
	failed += test_val_ptr_nonnull();
	failed += test_val_size_bounds();
	failed += test_val_alignment();
	failed += test_val_range();
	failed += test_val_add_overflow();
	failed += test_val_utf8();
	failed += test_fuzz_generate();

	printf("\nStage 11.2: Memory Safety\n");
	failed += test_canary_operations();
	failed += test_memsafety_alloc_free();
	failed += test_bounds_check_range();
	failed += test_align_check_u64();

	printf("\nStage 11.3: Proof of Correctness\n");
	failed += test_proof_returns_value();
	failed += test_proof_deterministic();
	failed += test_proof_output_range();
	failed += test_proof_context();

	printf("\nStage 11.4: Hardening Gates\n");
	failed += test_harden_init();
	failed += test_harden_seal_verify();
	failed += test_harden_audit_log();
	failed += test_harden_gate_precondition();
	failed += test_harden_detect_tampering();
	failed += test_harden_checkpoint();

	printf("\n=== All Phase 11 tests completed ===\n");
	return failed;
}
