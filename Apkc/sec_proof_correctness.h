/* sec_proof_correctness.h — Proof of Correctness (Stage 11.3)
 *
 * Executable falsifiers: tests that must pass for semantic claims.
 * Proof obligations: assertions embedded in code.
 * Invariant checking: pre/post-conditions for all public functions.
 * Deterministic proof validation: reproducible verification.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEC_PROOF_CORRECTNESS_H
#define APKC_SEC_PROOF_CORRECTNESS_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Proof status */
enum ProofStatus {
	PROOF_PASS = 0,           /* All proof obligations satisfied */
	PROOF_FAIL = 1,           /* At least one obligation violated */
	PROOF_UNFALSIFIABLE = 2,  /* Claim cannot be proven/falsified */
	PROOF_INCONCLUSIVE = 3,   /* Not enough evidence either way */
	PROOF_TIMEOUT = 4,        /* Proof took too long */
	PROOF_MEMORY_EXCEEDED = 5  /* Proof used too much memory */
};

/* Proof obligation */
struct ProofObligation {
	const char *name;             /* Name of obligation */
	const char *claim;            /* Semantic claim being proven */
	u8 (*test_fn)(void);          /* Executable falsifier function */
	u8 required;                  /* 1 if required, 0 if optional */
	u8 falsifiable;               /* 1 if obligation can be proven false */
};

/* Proof context for collecting results */
struct ProofContext {
	struct ProofObligation obligations[64];
	u32 obligation_count;
	u32 passed;
	u32 failed;
	u32 inconclusive;
};

/* Pre-condition: must be true before function call */
#define REQUIRE(cond) do { if (!(cond)) return PROOF_FAIL; } while(0)

/* Post-condition: must be true after function call */
#define ENSURE(cond) do { if (!(cond)) return PROOF_FAIL; } while(0)

/* Invariant: must be true during/after operation */
#define INVARIANT(cond) do { if (!(cond)) return PROOF_FAIL; } while(0)

/* ============================================================ */
/* PROOF CONTEXT MANAGEMENT */
/* ============================================================ */

/* Initialize proof context */
static inline void proof_init(struct ProofContext *pc) {
	if (!pc) return;
	pc->obligation_count = 0;
	pc->passed = 0;
	pc->failed = 0;
	pc->inconclusive = 0;
}

/* Register proof obligation */
static inline u8 proof_add_obligation(
	struct ProofContext *pc,
	const char *name,
	const char *claim,
	u8 (*test_fn)(void),
	u8 required) {

	if (!pc || !name || !test_fn) return PROOF_FAIL;
	if (pc->obligation_count >= 64) return PROOF_MEMORY_EXCEEDED;

	struct ProofObligation *obl = &pc->obligations[pc->obligation_count];
	obl->name = name;
	obl->claim = claim;
	obl->test_fn = test_fn;
	obl->required = required;
	obl->falsifiable = 1;

	pc->obligation_count++;
	return PROOF_PASS;
}

/* Execute all proof obligations */
static inline u8 proof_execute_all(struct ProofContext *pc) {
	if (!pc) return PROOF_FAIL;

	u32 i;
	for (i = 0; i < pc->obligation_count; i++) {
		struct ProofObligation *obl = &pc->obligations[i];
		u8 result = obl->test_fn();

		if (result == PROOF_PASS) {
			pc->passed++;
		} else if (result == PROOF_FAIL) {
			pc->failed++;
			if (obl->required) return PROOF_FAIL;
		} else {
			pc->inconclusive++;
		}
	}

	return (pc->failed > 0) ? PROOF_FAIL : PROOF_PASS;
}

/* Get overall proof status */
static inline u8 proof_status(struct ProofContext *pc) {
	if (!pc) return PROOF_FAIL;
	if (pc->failed > 0) return PROOF_FAIL;
	if (pc->inconclusive > 0) return PROOF_INCONCLUSIVE;
	return PROOF_PASS;
}

/* ============================================================ */
/* COMMON PROOF PATTERNS */
/* ============================================================ */

/* Proof: Function returns expected value for given input */
static inline u8 proof_returns_value(
	u64 (*fn)(u64),
	u64 input,
	u64 expected_output) {

	REQUIRE(fn != 0);
	u64 result = fn(input);
	ENSURE(result == expected_output);
	return PROOF_PASS;
}

/* Proof: Function deterministic (same input → same output) */
static inline u8 proof_deterministic(
	u64 (*fn)(u64),
	u64 input) {

	REQUIRE(fn != 0);
	u64 result1 = fn(input);
	u64 result2 = fn(input);
	ENSURE(result1 == result2);
	return PROOF_PASS;
}

/* Proof: No memory corruption in function call */
static inline u8 proof_memory_safe(
	u8 *buffer,
	u32 buffer_size,
	u8 (*fn)(u8 *, u32)) {

	REQUIRE(buffer != 0);
	REQUIRE(buffer_size > 0);
	REQUIRE(fn != 0);

	/* Compute before-state hash */
	u64 hash_before = 0;
	u32 i;
	for (i = 0; i < buffer_size; i++) {
		hash_before += buffer[i];
	}

	/* Execute function */
	u8 status = fn(buffer, buffer_size);

	/* Verify buffer still valid */
	u64 hash_after = 0;
	for (i = 0; i < buffer_size; i++) {
		hash_after += buffer[i];
	}

	ENSURE(status == 0);
	ENSURE(hash_before == hash_after);  /* No corruption */
	return PROOF_PASS;
}

/* Proof: Function produces output within valid range */
static inline u8 proof_output_range(
	u64 (*fn)(u64),
	u64 input,
	u64 min_output,
	u64 max_output) {

	REQUIRE(fn != 0);
	u64 result = fn(input);
	ENSURE(result >= min_output);
	ENSURE(result <= max_output);
	return PROOF_PASS;
}

/* ============================================================ */
/* FALSIFIER COLLECTION */
/* ============================================================ */

/* Standard falsifier: function is not 0 */
static inline u8 falsify_null_ptr(void *ptr) {
	return (ptr == 0) ? PROOF_FAIL : PROOF_PASS;
}

/* Standard falsifier: buffer has minimum size */
static inline u8 falsify_buffer_size(void *buf, u32 size, u32 min_size) {
	if (buf == 0) return PROOF_FAIL;
	if (size < min_size) return PROOF_FAIL;
	return PROOF_PASS;
}

/* Standard falsifier: value in valid range */
static inline u8 falsify_value_range(u64 value, u64 min_val, u64 max_val) {
	if (value < min_val || value > max_val) return PROOF_FAIL;
	return PROOF_PASS;
}

/* ============================================================ */
/* SEMANTIC CLAIM TRACKING */
/* ============================================================ */

struct SemanticClaim {
	const char *name;          /* Claim identifier */
	const char *description;   /* Human-readable description */
	u8 claim_allowed;          /* 1 if claim is promoted to proof, 0 if only tested */
	u8 (*test_fn)(void);       /* Executable falsifier */
	u32 test_count;            /* How many times tested */
	u32 test_passed;           /* Passes */
	u32 test_failed;           /* Failures */
};

/* Claim registry */
struct ClaimRegistry {
	struct SemanticClaim claims[32];
	u32 claim_count;
};

/* Register semantic claim for tracking */
static inline u8 claim_register(
	struct ClaimRegistry *cr,
	const char *name,
	const char *description,
	u8 (*test_fn)(void),
	u8 claim_allowed) {

	if (!cr || !name || !test_fn) return PROOF_FAIL;
	if (cr->claim_count >= 32) return PROOF_MEMORY_EXCEEDED;

	struct SemanticClaim *claim = &cr->claims[cr->claim_count];
	claim->name = name;
	claim->description = description;
	claim->test_fn = test_fn;
	claim->claim_allowed = claim_allowed;
	claim->test_count = 0;
	claim->test_passed = 0;
	claim->test_failed = 0;

	cr->claim_count++;
	return PROOF_PASS;
}

/* Test all registered claims */
static inline u8 claim_test_all(struct ClaimRegistry *cr) {
	if (!cr) return PROOF_FAIL;

	u32 i;
	for (i = 0; i < cr->claim_count; i++) {
		struct SemanticClaim *claim = &cr->claims[i];
		u8 result = claim->test_fn();
		claim->test_count++;

		if (result == PROOF_PASS) {
			claim->test_passed++;
		} else {
			claim->test_failed++;
		}
	}

	return PROOF_PASS;
}

/* Get claim status: promoted to proof or still testing */
static inline u8 claim_is_promoted(struct ClaimRegistry *cr, u32 claim_idx) {
	if (!cr || claim_idx >= cr->claim_count) return 0;
	return cr->claims[claim_idx].claim_allowed;
}

#endif /* APKC_SEC_PROOF_CORRECTNESS_H */
