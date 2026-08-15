/* test_property_based.h — Property-Based Testing Framework (Stage 17.2)
 *
 * Property definition: specify invariants that must hold for all inputs.
 * Random test generation: create diverse test cases from generator functions.
 * Shrinking: minimize failing test case to simplest reproduction.
 * Property validator: run property checks across input domain.
 * Statistics tracking: count successes/failures/edge cases.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_TEST_PROPERTY_BASED_H
#define APKC_TEST_PROPERTY_BASED_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Property validator function type */
typedef u8 (*PropertyValidator)(u64 input, u64 expected);

/* Property definition */
struct Property {
	const char *property_name;  /* Property identifier */
	PropertyValidator validator; /* Validation function */
	u32 property_id;            /* Unique property ID */
	u8 is_enabled;              /* 1 if property is active */
	u32 success_count;          /* Successful validations */
	u32 failure_count;          /* Failed validations */
	u64 failing_input;          /* Input that caused failure (if any) */
};

/* Test generator (produces random test inputs) */
typedef u64 (*TestGenerator)(u32 seed, u32 iteration);

/* Property-based test configuration */
struct PropertyTest {
	struct Property properties[32];     /* Up to 32 properties */
	u32 property_count;
	TestGenerator input_generator;     /* Input generation function */
	u32 iterations;                    /* Number of test iterations per property */
	u32 seed;                          /* Random seed for reproducibility */
	u32 shrink_count;                  /* Number of shrink iterations on failure */
	u32 total_tests_run;               /* Total tests executed */
	u32 total_properties_failed;       /* Properties that failed */
};

/* ============================================================ */
/* PROPERTY TEST INITIALIZATION */
/* ============================================================ */

/* Initialize property test */
static inline void proptest_init(
	struct PropertyTest *pt,
	TestGenerator gen,
	u32 iterations,
	u32 seed) {

	if (!pt) return;
	pt->property_count = 0;
	pt->input_generator = gen;
	pt->iterations = iterations;
	pt->seed = seed;
	pt->shrink_count = 0;
	pt->total_tests_run = 0;
	pt->total_properties_failed = 0;
}

/* ============================================================ */
/* PROPERTY REGISTRATION */
/* ============================================================ */

/* Register property */
static inline u8 proptest_register_property(
	struct PropertyTest *pt,
	const char *name,
	PropertyValidator validator) {

	if (!pt || !name || !validator) return 0;
	if (pt->property_count >= 32) return 0;

	struct Property *prop = &pt->properties[pt->property_count];
	prop->property_name = name;
	prop->validator = validator;
	prop->property_id = pt->property_count;
	prop->is_enabled = 1;
	prop->success_count = 0;
	prop->failure_count = 0;
	prop->failing_input = 0;

	pt->property_count++;
	return 1;
}

/* ============================================================ */
/* PROPERTY VALIDATION */
/* ============================================================ */

/* Run property test (single iteration) */
static inline u8 proptest_validate_property(
	struct PropertyTest *pt,
	u32 property_id,
	u64 input,
	u64 expected) {

	if (!pt || property_id >= pt->property_count) return 0;

	struct Property *prop = &pt->properties[property_id];
	if (!prop->is_enabled || !prop->validator) return 0;

	u8 result = prop->validator(input, expected);

	pt->total_tests_run++;

	if (result) {
		prop->success_count++;
	} else {
		prop->failure_count++;
		prop->failing_input = input;
		pt->total_properties_failed++;
	}

	return result;
}

/* Run all properties with generated inputs */
static inline u32 proptest_run_all(struct PropertyTest *pt) {
	if (!pt || !pt->input_generator) return 0;

	u32 failed_count = 0;
	u32 i;
	for (i = 0; i < pt->property_count; i++) {
		if (!pt->properties[i].is_enabled) continue;

		u32 j;
		for (j = 0; j < pt->iterations; j++) {
			u64 input = pt->input_generator(pt->seed + j, j);
			u8 result = proptest_validate_property(pt, i, input, 0);
			if (!result) {
				failed_count++;
			}
		}
	}

	return failed_count;
}

/* ============================================================ */
/* SHRINKING & MINIMIZATION */
/* ============================================================ */

/* Shrink failing input (find minimal reproduction) */
static inline u64 proptest_shrink_input(
	struct PropertyTest *pt,
	u32 property_id,
	u64 failing_input) {

	if (!pt || property_id >= pt->property_count) return failing_input;

	struct Property *prop = &pt->properties[property_id];
	u64 current = failing_input;
	u64 best_shrink = current;

	u32 i;
	for (i = 0; i < pt->shrink_count; i++) {
		/* Try halving the input */
		u64 candidate = current / 2;
		if (candidate == current) break;

		u8 still_fails = prop->validator(candidate, 0);
		if (still_fails) {
			best_shrink = candidate;
			current = candidate;
		}
	}

	return best_shrink;
}

/* ============================================================ */
/* STATISTICS & ANALYSIS */
/* ============================================================ */

/* Get property success rate */
static inline u32 proptest_get_success_rate(
	struct PropertyTest *pt,
	u32 property_id) {

	if (!pt || property_id >= pt->property_count) return 0;

	struct Property *prop = &pt->properties[property_id];
	u32 total = prop->success_count + prop->failure_count;
	if (total == 0) return 0;
	return (prop->success_count * 100) / total;
}

/* Count failing properties */
static inline u32 proptest_count_failures(struct PropertyTest *pt) {
	if (!pt) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < pt->property_count; i++) {
		if (pt->properties[i].failure_count > 0) {
			count++;
		}
	}

	return count;
}

/* Get property with most failures */
static inline struct Property *proptest_find_flakiest_property(struct PropertyTest *pt) {
	if (!pt || pt->property_count == 0) return 0;

	struct Property *flakiest = &pt->properties[0];
	u32 i;
	for (i = 1; i < pt->property_count; i++) {
		if (pt->properties[i].failure_count > flakiest->failure_count) {
			flakiest = &pt->properties[i];
		}
	}

	return flakiest;
}

#endif /* APKC_TEST_PROPERTY_BASED_H */
