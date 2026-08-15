/* test_property_based.h — Property-Based Testing Framework (Stage 14.2)
 *
 * Property-based testing: generate random inputs and check properties.
 * Shrinking: reduce failing inputs to minimal counterexamples.
 * Property specification: express expected behavior as predicates.
 * Test generation strategies: custom input generation per type.
 * Deterministic replay: reproduce failures from seed values.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_TEST_PROPERTY_BASED_H
#define APKC_TEST_PROPERTY_BASED_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Property test status */
enum PropertyStatus {
	PROP_OK = 0,                /* Property holds for all tests */
	PROP_FALSIFIED = 1,         /* Counterexample found */
	PROP_INCONCLUSIVE = 2,      /* Cannot determine (insufficient data) */
	PROP_ERROR = 3,             /* Error during test execution */
	PROP_EXHAUSTED = 4          /* Ran out of test cases */
};

/* Test generation strategy */
enum GenerationStrategy {
	STRAT_UNIFORM = 1,          /* Uniform random over range */
	STRAT_SHRINK = 2,           /* Binary search for minimal counterexample */
	STRAT_BIASED = 3,           /* Biased toward edge cases (0, max, -1, etc.) */
	STRAT_RECURSIVE = 4         /* Recursive structure generation */
};

/* Property specification */
struct Property {
	const char *property_name;  /* Property identifier */
	u32 min_tests;              /* Minimum tests before passing */
	u32 max_tests;              /* Maximum tests to run */
	u8 generation_strategy;     /* GenerationStrategy */
	u64 random_seed;            /* PRNG seed for reproducibility */
	u32 shrink_depth;           /* Max shrinking attempts */
};

/* Test counterexample */
struct Counterexample {
	u64 inputs[8];              /* Input values */
	u32 input_count;
	u64 output;                 /* Actual output */
	u64 expected;               /* Expected output */
	const char *failure_reason; /* Why property failed */
	u64 seed;                   /* Random seed that produced counterexample */
};

/* Property test result */
struct PropertyResult {
	const char *property_name;
	u8 status;                  /* PropertyStatus */
	u32 tests_run;
	u32 tests_passed;
	struct Counterexample counterexample;
	u64 execution_time_ms;
};

/* PRNG state for deterministic testing */
struct TestRandom {
	u64 state;                  /* PRNG state */
	u64 seed;                   /* Original seed */
};

/* ============================================================ */
/* PRNG IMPLEMENTATION (Simple Linear Congruential) */
/* ============================================================ */

/* Initialize PRNG */
static inline void test_random_init(struct TestRandom *rng, u64 seed) {
	if (!rng) return;
	rng->seed = seed;
	rng->state = seed;
}

/* Get next random value */
static inline u64 test_random_next(struct TestRandom *rng) {
	if (!rng) return 0;

	/* Linear congruential generator: x = (a*x + c) mod m */
	/* Using parameters: a=6364136223846793005, c=1442695040888963407 */
	rng->state = rng->state * 6364136223846793005ULL + 1442695040888963407ULL;
	return rng->state;
}

/* Get random value in range [min, max) */
static inline u64 test_random_range(
	struct TestRandom *rng,
	u64 min,
	u64 max) {

	if (!rng || min >= max) return min;

	u64 range = max - min;
	u64 rand = test_random_next(rng);
	return min + (rand % range);
}

/* ============================================================ */
/* PROPERTY SPECIFICATION */
/* ============================================================ */

/* Initialize property */
static inline void prop_init(
	struct Property *prop,
	const char *name,
	u64 seed) {

	if (!prop) return;
	prop->property_name = name;
	prop->min_tests = 100;
	prop->max_tests = 1000;
	prop->generation_strategy = STRAT_UNIFORM;
	prop->random_seed = seed;
	prop->shrink_depth = 5;
}

/* Set generation strategy */
static inline void prop_set_strategy(
	struct Property *prop,
	u8 strategy) {

	if (!prop) return;
	prop->generation_strategy = strategy;
}

/* ============================================================ */
/* BIAS TOWARD EDGE CASES */
/* ============================================================ */

/* Generate biased value (prefer edge cases) */
static inline u64 prop_generate_biased(
	struct TestRandom *rng,
	u64 min,
	u64 max) {

	if (!rng || min >= max) return min;

	u64 rand = test_random_next(rng) % 100;

	/* 20% chance: return min */
	if (rand < 20) return min;

	/* 20% chance: return max-1 */
	if (rand < 40) return max - 1;

	/* 20% chance: return 0 */
	if (rand < 60) return 0;

	/* 20% chance: return -1 (as u64) */
	if (rand < 80) return (u64)(-1);

	/* 20% chance: uniform random */
	return test_random_range(rng, min, max);
}

/* ============================================================ */
/* SHRINKING (REDUCE COUNTEREXAMPLE) */
/* ============================================================ */

/* Attempt to shrink counterexample by modifying one input */
static inline u8 prop_shrink_step(
	struct Counterexample *ceex,
	u32 depth) {

	if (!ceex || depth == 0 || ceex->input_count == 0) return 0;

	/* Try reducing first input by half */
	u64 original = ceex->inputs[0];
	u64 shrunk = original / 2;

	if (shrunk != original) {
		ceex->inputs[0] = shrunk;
		return 1;  /* Shrinking modified the counterexample */
	}

	return 0;  /* Cannot shrink further */
}

/* ============================================================ */
/* PROPERTY TEST EXECUTION */
/* ============================================================ */

/* Run property test with generated inputs */
static inline struct PropertyResult prop_run_test(
	struct Property *prop,
	u8 (*property_check)(const u64 *inputs, u32 input_count, u64 *output)) {

	struct PropertyResult result = {0};
	if (!prop || !property_check) {
		result.status = PROP_ERROR;
		return result;
	}

	result.property_name = prop->property_name;
	result.tests_run = 0;
	result.tests_passed = 0;

	struct TestRandom rng = {0};
	test_random_init(&rng, prop->random_seed);

	u32 test_count;
	for (test_count = 0; test_count < prop->max_tests; test_count++) {
		u64 inputs[8] = {0};
		u32 input_count = 3;  /* Default: test with 3 inputs */

		/* Generate inputs based on strategy */
		u32 i;
		for (i = 0; i < input_count; i++) {
			if (prop->generation_strategy == STRAT_BIASED) {
				inputs[i] = prop_generate_biased(&rng, 0, 1000);
			} else {
				inputs[i] = test_random_next(&rng) % 1000;
			}
		}

		u64 output = 0;
		u8 check_result = property_check(inputs, input_count, &output);

		result.tests_run++;

		if (check_result == 0) {
			/* Property failed */
			result.status = PROP_FALSIFIED;
			result.counterexample.output = output;
			result.counterexample.seed = prop->random_seed;
			result.counterexample.input_count = input_count;

			u32 j;
			for (j = 0; j < input_count; j++) {
				result.counterexample.inputs[j] = inputs[j];
			}

			/* Try shrinking */
			u32 shrink_attempt;
			for (shrink_attempt = 0; shrink_attempt < prop->shrink_depth; shrink_attempt++) {
				if (!prop_shrink_step(&result.counterexample, shrink_attempt)) {
					break;
				}

				u64 shrunk_output = 0;
				u8 shrunk_check = property_check(
					result.counterexample.inputs,
					result.counterexample.input_count,
					&shrunk_output);

				if (shrunk_check != 0) {
					/* Shrinking broke the counterexample, revert */
					u32 k;
					for (k = 0; k < input_count; k++) {
						result.counterexample.inputs[k] = inputs[k];
					}
					break;
				}
			}

			return result;
		}

		result.tests_passed++;

		if (test_count >= prop->min_tests) {
			/* Enough tests passed, property holds */
			result.status = PROP_OK;
		}
	}

	if (result.status != PROP_OK && result.tests_run >= prop->min_tests) {
		result.status = PROP_OK;
	}

	return result;
}

/* ============================================================ */
/* RESULT REPORTING */
/* ============================================================ */

/* Check if property passed */
static inline u8 prop_passed(struct PropertyResult *result) {
	if (!result) return 0;
	return (result->status == PROP_OK) ? 1 : 0;
}

/* Get test count */
static inline u32 prop_get_test_count(struct PropertyResult *result) {
	if (!result) return 0;
	return result->tests_run;
}

/* Get pass count */
static inline u32 prop_get_pass_count(struct PropertyResult *result) {
	if (!result) return 0;
	return result->tests_passed;
}

#endif /* APKC_TEST_PROPERTY_BASED_H */
