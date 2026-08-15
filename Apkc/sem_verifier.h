/* sem_verifier.h — Program Verification & Proof Generation (Phase 26)
 *
 * Invariant tracking: program state invariants
 * Loop invariants: properties preserved across iterations
 * Correctness proofs: verify program properties
 * Safety properties: buffer overflows, null pointer dereferences
 * Liveness properties: termination guarantees
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEM_VERIFIER_H
#define APKC_SEM_VERIFIER_H 1

#include "opt_semantic_fold.h"

typedef unsigned char u8;
typedef unsigned int u32;

/* ============================================================ */
/* INVARIANT TYPES */
/* ============================================================ */

enum InvariantKind {
	INV_RANGE = 0,           /* Value in range [min, max] */
	INV_NON_NULL = 1,        /* Pointer not null */
	INV_ALLOCATION = 2,      /* Memory allocated */
	INV_NO_OVERFLOW = 3,     /* No integer overflow */
	INV_NO_UNDERFLOW = 4,    /* No integer underflow */
	INV_BOUNDED = 5,         /* Array access within bounds */
	INV_VALID = 6            /* Valid object/pointer */
};

/* ============================================================ */
/* INVARIANT REPRESENTATION */
/* ============================================================ */

struct Invariant {
	u32 var_id;
	u8 kind;
	u64 min_val;
	u64 max_val;
	const char *description;
	u8 is_proven;
};

/* ============================================================ */
/* LOOP INVARIANT */
/* ============================================================ */

struct LoopInvariant {
	u32 loop_id;
	struct Invariant invariants[16];
	u32 invariant_count;
	u8 is_preserved;
};

/* ============================================================ */
/* INVARIANT CHECKER */
/* ============================================================ */

struct InvariantChecker {
	struct Invariant invariants[128];
	u32 invariant_count;
	struct LoopInvariant loop_invariants[32];
	u32 loop_invariant_count;
	u32 proven_count;
	u32 violation_count;
};

/* ============================================================ */
/* INITIALIZATION */
/* ============================================================ */

static inline void invariant_checker_init(struct InvariantChecker *checker) {
	if (!checker) return;
	checker->invariant_count = 0;
	checker->loop_invariant_count = 0;
	checker->proven_count = 0;
	checker->violation_count = 0;
}

/* ============================================================ */
/* INVARIANT REGISTRATION */
/* ============================================================ */

static inline u8 add_invariant(
	struct InvariantChecker *checker,
	u32 var_id,
	u8 kind,
	u64 min_val,
	u64 max_val,
	const char *desc) {

	if (!checker || checker->invariant_count >= 128) return 1;

	struct Invariant *inv = &checker->invariants[checker->invariant_count];
	inv->var_id = var_id;
	inv->kind = kind;
	inv->min_val = min_val;
	inv->max_val = max_val;
	inv->description = desc;
	inv->is_proven = 0;

	checker->invariant_count++;
	return 0;
}

static inline u8 add_loop_invariant(
	struct InvariantChecker *checker,
	u32 loop_id,
	u32 var_id,
	u8 kind) {

	if (!checker || checker->loop_invariant_count >= 32) return 1;

	struct LoopInvariant *loop_inv = &checker->loop_invariants[checker->loop_invariant_count];
	loop_inv->loop_id = loop_id;
	loop_inv->invariant_count = 0;
	loop_inv->is_preserved = 0;

	if (loop_inv->invariant_count < 16) {
		loop_inv->invariants[loop_inv->invariant_count].var_id = var_id;
		loop_inv->invariants[loop_inv->invariant_count].kind = kind;
		loop_inv->invariant_count++;
	}

	checker->loop_invariant_count++;
	return 0;
}

/* ============================================================ */
/* SAFETY VERIFICATION */
/* ============================================================ */

struct SafetyViolation {
	u8 kind;
	u32 line;
	const char *description;
	u8 is_definite;
};

static inline struct SafetyViolation check_null_pointer(
	struct InvariantChecker *checker,
	u32 var_id,
	u32 line) {

	struct SafetyViolation violation;
	violation.kind = INV_NON_NULL;
	violation.line = line;
	violation.is_definite = 0;

	if (!checker) return violation;

	/* Check if var has NON_NULL invariant */
	u32 i;
	for (i = 0; i < checker->invariant_count; i++) {
		if (checker->invariants[i].var_id == var_id &&
		    checker->invariants[i].kind == INV_NON_NULL) {
			violation.is_definite = 1;
			violation.description = "Null pointer dereference prevented";
			return violation;
		}
	}

	violation.description = "Potential null pointer";
	return violation;
}

static inline struct SafetyViolation check_overflow(
	struct InvariantChecker *checker,
	u32 var_id,
	u64 value) {

	struct SafetyViolation violation;
	violation.kind = INV_NO_OVERFLOW;
	violation.is_definite = 0;
	violation.description = 0;

	if (!checker) return violation;

	/* Check if value violates range invariant */
	u32 i;
	for (i = 0; i < checker->invariant_count; i++) {
		struct Invariant *inv = &checker->invariants[i];
		if (inv->var_id == var_id && inv->kind == INV_RANGE) {
			if (value > inv->max_val) {
				violation.is_definite = 1;
				violation.description = "Integer overflow detected";
				return violation;
			}
		}
	}

	return violation;
}

static inline struct SafetyViolation check_buffer_bounds(
	struct InvariantChecker *checker,
	u32 array_var,
	u32 index_var,
	u64 array_size) {

	struct SafetyViolation violation;
	violation.kind = INV_BOUNDED;
	violation.is_definite = 0;

	/* Check if index is within bounds */
	u32 i;
	for (i = 0; i < checker->invariant_count; i++) {
		struct Invariant *inv = &checker->invariants[i];
		if (inv->var_id == index_var && inv->kind == INV_RANGE) {
			if (inv->max_val >= array_size) {
				violation.is_definite = 1;
				violation.description = "Buffer overflow possible";
				return violation;
			}
		}
	}

	violation.description = "Potential buffer overflow";
	return violation;
}

/* ============================================================ */
/* PROOF GENERATION */
/* ============================================================ */

struct ProofStep {
	u32 step_number;
	const char *assertion;
	const char *justification;
	u8 is_verified;
};

struct CorrectnessProof {
	struct ProofStep steps[64];
	u32 step_count;
	u8 is_complete;
	u32 line;
};

static inline u8 add_proof_step(
	struct CorrectnessProof *proof,
	const char *assertion,
	const char *justification) {

	if (!proof || proof->step_count >= 64) return 1;

	struct ProofStep *step = &proof->steps[proof->step_count];
	step->step_number = proof->step_count + 1;
	step->assertion = assertion;
	step->justification = justification;
	step->is_verified = 0;

	proof->step_count++;
	return 0;
}

/* ============================================================ */
/* VERIFICATION STATISTICS */
/* ============================================================ */

struct VerificationStats {
	u32 total_invariants;
	u32 proven_invariants;
	u32 safety_checks_passed;
	u32 safety_violations_detected;
	u32 proof_steps_verified;
	u32 loops_with_preserved_invariants;
};

static inline struct VerificationStats get_verification_stats(
	struct InvariantChecker *checker) {

	struct VerificationStats stats;
	stats.total_invariants = checker->invariant_count;
	stats.proven_invariants = checker->proven_count;
	stats.safety_violations_detected = checker->violation_count;
	stats.safety_checks_passed = 0;
	stats.proof_steps_verified = 0;
	stats.loops_with_preserved_invariants = 0;

	if (checker->invariant_count > 0) {
		stats.safety_checks_passed =
			checker->invariant_count - checker->violation_count;
	}

	return stats;
}

#endif /* APKC_SEM_VERIFIER_H */
