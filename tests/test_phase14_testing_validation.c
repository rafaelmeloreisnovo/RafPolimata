/* test_phase14_testing_validation.c — Phase 14 Testing & Validation Tests
 *
 * Comprehensive test suite for:
 * - Stage 14.1: Unit Test Generation
 * - Stage 14.2: Property-Based Testing
 * - Stage 14.3: Regression Test Tracking
 * - Stage 14.4: Test Coverage Metrics
 *
 * FREESTANDING: No malloc, no libc.
 */

#include <stdio.h>
#include <string.h>

/* Include Phase 14 headers */
#include "Apkc/test_unit_generation.h"
#include "Apkc/test_property_based.h"
#include "Apkc/test_regression_tracking.h"
#include "Apkc/test_coverage_metrics.h"

/* ============================================================ */
/* STAGE 14.1: UNIT TEST GENERATION TESTS */
/* ============================================================ */

static int test_unit_init_suite(void) {
	struct TestSuite suite = {0};
	test_init_suite(&suite, "example_module");

	if (suite.test_count != 0) {
		printf("❌ test_unit_init_suite: test count not zeroed\n");
		return 1;
	}
	if (suite.module_name == 0) {
		printf("❌ test_unit_init_suite: module name not set\n");
		return 1;
	}

	printf("✓ test_unit_init_suite\n");
	return 0;
}

static int test_unit_add_case(void) {
	struct TestSuite suite = {0};
	test_init_suite(&suite, "test");

	if (test_add_case(&suite, "test_add", "add", TEST_UNIT, 50) != TEST_OK) {
		printf("❌ test_unit_add_case: failed to add test case\n");
		return 1;
	}

	if (suite.test_count != 1) {
		printf("❌ test_unit_add_case: test count not incremented\n");
		return 1;
	}

	printf("✓ test_unit_add_case\n");
	return 0;
}

static int test_unit_add_input(void) {
	struct TestSuite suite = {0};
	test_init_suite(&suite, "test");
	test_add_case(&suite, "test_1", "func", TEST_UNIT, 50);

	if (test_add_input(&suite, 0, 42, 4) != TEST_OK) {
		printf("❌ test_unit_add_input: failed to add input\n");
		return 1;
	}

	if (suite.tests[0].input_count != 1) {
		printf("❌ test_unit_add_input: input count not incremented\n");
		return 1;
	}

	printf("✓ test_unit_add_input\n");
	return 0;
}

static int test_unit_coverage_tracking(void) {
	struct TestSuite suite = {0};
	test_init_suite(&suite, "test");

	test_add_coverage_point(&suite, "main.c:10");
	test_add_coverage_point(&suite, "main.c:20");

	if (suite.coverage_count != 2) {
		printf("❌ test_unit_coverage_tracking: coverage count not incremented\n");
		return 1;
	}

	test_record_coverage_hit(&suite, "main.c:10");
	if (suite.covered_points != 1) {
		printf("❌ test_unit_coverage_tracking: covered points not updated\n");
		return 1;
	}

	printf("✓ test_unit_coverage_tracking\n");
	return 0;
}

static int test_unit_coverage_percent(void) {
	struct TestSuite suite = {0};
	test_init_suite(&suite, "test");

	test_add_coverage_point(&suite, "file.c:1");
	test_add_coverage_point(&suite, "file.c:2");
	test_add_coverage_point(&suite, "file.c:3");
	test_add_coverage_point(&suite, "file.c:4");

	test_record_coverage_hit(&suite, "file.c:1");
	test_record_coverage_hit(&suite, "file.c:2");

	u32 percent = test_get_coverage_percent(&suite);
	if (percent != 50) {
		printf("❌ test_unit_coverage_percent: coverage percentage incorrect\n");
		return 1;
	}

	printf("✓ test_unit_coverage_percent\n");
	return 0;
}

/* ============================================================ */
/* STAGE 14.2: PROPERTY-BASED TESTING TESTS */
/* ============================================================ */

static int test_property_random_init(void) {
	struct TestRandom rng = {0};
	test_random_init(&rng, 12345);

	if (rng.seed != 12345) {
		printf("❌ test_property_random_init: seed not set\n");
		return 1;
	}

	printf("✓ test_property_random_init\n");
	return 0;
}

static int test_property_random_sequence(void) {
	struct TestRandom rng1 = {0}, rng2 = {0};
	test_random_init(&rng1, 42);
	test_random_init(&rng2, 42);

	u64 v1 = test_random_next(&rng1);
	u64 v2 = test_random_next(&rng2);

	if (v1 != v2) {
		printf("❌ test_property_random_sequence: determinism broken\n");
		return 1;
	}

	printf("✓ test_property_random_sequence\n");
	return 0;
}

static int test_property_random_range(void) {
	struct TestRandom rng = {0};
	test_random_init(&rng, 999);

	u32 i;
	for (i = 0; i < 100; i++) {
		u64 val = test_random_range(&rng, 10, 20);
		if (val < 10 || val >= 20) {
			printf("❌ test_property_random_range: value out of range\n");
			return 1;
		}
	}

	printf("✓ test_property_random_range\n");
	return 0;
}

static int test_property_biased_generation(void) {
	struct TestRandom rng = {0};
	test_random_init(&rng, 777);

	u32 i;
	u32 saw_zero = 0, saw_edge = 0;
	for (i = 0; i < 1000; i++) {
		u64 val = prop_generate_biased(&rng, 0, 1000);
		if (val == 0) saw_zero = 1;
		if (val == 999) saw_edge = 1;
	}

	if (!saw_zero || !saw_edge) {
		printf("❌ test_property_biased_generation: edge cases not generated\n");
		return 1;
	}

	printf("✓ test_property_biased_generation\n");
	return 0;
}

/* ============================================================ */
/* STAGE 14.3: REGRESSION TEST TRACKING TESTS */
/* ============================================================ */

static int test_regression_init_suite(void) {
	struct RegressionSuite suite = {0};
	reg_init_suite(&suite, "my_suite");

	if (suite.test_count != 0) {
		printf("❌ test_regression_init_suite: test count not zeroed\n");
		return 1;
	}

	printf("✓ test_regression_init_suite\n");
	return 0;
}

static int test_regression_add_test(void) {
	struct RegressionSuite suite = {0};
	reg_init_suite(&suite, "suite");

	if (reg_add_test(&suite, "test_crash", "BUG-42", "App crashes on startup",
			SEV_BLOCKER, "v1.0", "v1.1", "abc123") != REG_OK) {
		printf("❌ test_regression_add_test: failed to add test\n");
		return 1;
	}

	if (suite.test_count != 1) {
		printf("❌ test_regression_add_test: test count not incremented\n");
		return 1;
	}

	printf("✓ test_regression_add_test\n");
	return 0;
}

static int test_regression_record_pass(void) {
	struct RegressionSuite suite = {0};
	reg_init_suite(&suite, "suite");
	reg_add_test(&suite, "test_1", "BUG-1", "desc", SEV_MAJOR, "v1", "v2", "sha1");

	u8 status = reg_record_run(&suite, "test_1", 1);  /* passed=1 */
	if (status != REG_OK) {
		printf("❌ test_regression_record_pass: status not OK\n");
		return 1;
	}

	if (suite.tests[0].pass_count != 1) {
		printf("❌ test_regression_record_pass: pass count not incremented\n");
		return 1;
	}

	printf("✓ test_regression_record_pass\n");
	return 0;
}

static int test_regression_is_resurfaced(void) {
	struct RegressionSuite suite = {0};
	reg_init_suite(&suite, "suite");
	reg_add_test(&suite, "test_1", "BUG-1", "desc", SEV_MAJOR, "v1", "v2", "sha1");

	reg_record_run(&suite, "test_1", 0);  /* failed=0 */

	if (!reg_is_resurfaced(&suite, "test_1")) {
		printf("❌ test_regression_is_resurfaced: issue should be resurfaced\n");
		return 1;
	}

	printf("✓ test_regression_is_resurfaced\n");
	return 0;
}

static int test_regression_is_healthy(void) {
	struct RegressionSuite suite = {0};
	reg_init_suite(&suite, "suite");

	if (!reg_is_healthy(&suite)) {
		printf("❌ test_regression_is_healthy: empty suite should be healthy\n");
		return 1;
	}

	reg_add_test(&suite, "test_1", "BUG-1", "desc", SEV_MAJOR, "v1", "v2", "sha1");
	reg_record_run(&suite, "test_1", 0);  /* Fail */

	if (reg_is_healthy(&suite)) {
		printf("❌ test_regression_is_healthy: suite with failures should not be healthy\n");
		return 1;
	}

	printf("✓ test_regression_is_healthy\n");
	return 0;
}

/* ============================================================ */
/* STAGE 14.4: COVERAGE METRICS TESTS */
/* ============================================================ */

static int test_coverage_init_report(void) {
	struct CoverageReport report = {0};
	cov_init_report(&report, "my_module", "2026-08-15");

	if (report.function_count != 0) {
		printf("❌ test_coverage_init_report: function count not zeroed\n");
		return 1;
	}
	if (report.target_percentage != 80) {
		printf("❌ test_coverage_init_report: default target not 80\n");
		return 1;
	}

	printf("✓ test_coverage_init_report\n");
	return 0;
}

static int test_coverage_add_function(void) {
	struct CoverageReport report = {0};
	cov_init_report(&report, "test", "now");

	if (!cov_add_function(&report, "add", 10, 10)) {
		printf("❌ test_coverage_add_function: failed to add function\n");
		return 1;
	}

	if (report.function_count != 1) {
		printf("❌ test_coverage_add_function: function count not incremented\n");
		return 1;
	}

	if (!report.functions[0].is_fully_covered) {
		printf("❌ test_coverage_add_function: fully covered flag not set\n");
		return 1;
	}

	printf("✓ test_coverage_add_function\n");
	return 0;
}

static int test_coverage_partial_coverage(void) {
	struct CoverageReport report = {0};
	cov_init_report(&report, "test", "now");

	cov_add_function(&report, "func1", 100, 50);  /* 50% covered */
	cov_add_function(&report, "func2", 20, 20);   /* 100% covered */
	cov_add_function(&report, "func3", 30, 0);    /* 0% covered */

	if (cov_count_partial_coverage(&report) != 1) {
		printf("❌ test_coverage_partial_coverage: partial count incorrect\n");
		return 1;
	}

	if (cov_count_uncovered_functions(&report) != 1) {
		printf("❌ test_coverage_partial_coverage: uncovered count incorrect\n");
		return 1;
	}

	printf("✓ test_coverage_partial_coverage\n");
	return 0;
}

static int test_coverage_overall_computation(void) {
	struct CoverageReport report = {0};
	cov_init_report(&report, "test", "now");

	cov_add_function(&report, "a", 100, 50);
	cov_add_function(&report, "b", 100, 50);

	cov_compute_overall(&report);

	if (report.overall_percentage != 50) {
		printf("❌ test_coverage_overall_computation: overall percentage incorrect\n");
		return 1;
	}

	printf("✓ test_coverage_overall_computation\n");
	return 0;
}

static int test_coverage_target_check(void) {
	struct CoverageReport report = {0};
	cov_init_report(&report, "test", "now");

	cov_set_target(&report, 80);
	cov_add_function(&report, "func", 100, 90);  /* 90% covered */
	cov_compute_overall(&report);

	if (!cov_meets_target(&report)) {
		printf("❌ test_coverage_target_check: should meet 80%% target at 90%%\n");
		return 1;
	}

	printf("✓ test_coverage_target_check\n");
	return 0;
}

static int test_coverage_delta(void) {
	struct CoverageReport baseline = {0}, current = {0};
	cov_init_report(&baseline, "test", "baseline");
	cov_init_report(&current, "test", "current");

	cov_add_function(&baseline, "func", 100, 50);
	cov_compute_overall(&baseline);

	cov_add_function(&current, "func", 100, 70);
	cov_compute_overall(&current);

	struct CoverageDelta delta = cov_compute_delta(&baseline, &current);
	if (delta.percentage_change != 20) {
		printf("❌ test_coverage_delta: delta change incorrect (expected 20, got %d)\n", delta.percentage_change);
		return 1;
	}

	if (!delta.is_improvement) {
		printf("❌ test_coverage_delta: should be marked as improvement\n");
		return 1;
	}

	printf("✓ test_coverage_delta\n");
	return 0;
}

/* ============================================================ */
/* MAIN TEST RUNNER */
/* ============================================================ */

int main(void) {
	printf("=== Phase 14: Testing & Validation Infrastructure Tests ===\n\n");

	int failed = 0;

	printf("Stage 14.1: Unit Test Generation\n");
	failed += test_unit_init_suite();
	failed += test_unit_add_case();
	failed += test_unit_add_input();
	failed += test_unit_coverage_tracking();
	failed += test_unit_coverage_percent();

	printf("\nStage 14.2: Property-Based Testing\n");
	failed += test_property_random_init();
	failed += test_property_random_sequence();
	failed += test_property_random_range();
	failed += test_property_biased_generation();

	printf("\nStage 14.3: Regression Test Tracking\n");
	failed += test_regression_init_suite();
	failed += test_regression_add_test();
	failed += test_regression_record_pass();
	failed += test_regression_is_resurfaced();
	failed += test_regression_is_healthy();

	printf("\nStage 14.4: Test Coverage Metrics\n");
	failed += test_coverage_init_report();
	failed += test_coverage_add_function();
	failed += test_coverage_partial_coverage();
	failed += test_coverage_overall_computation();
	failed += test_coverage_target_check();
	failed += test_coverage_delta();

	printf("\n=== All Phase 14 tests completed ===\n");
	return failed;
}
