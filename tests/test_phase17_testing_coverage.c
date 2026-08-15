/* test_phase17_testing_coverage.c — Phase 17 Testing (Stages 17.1–17.4)
 *
 * Comprehensive tests for unit generation, property-based testing, 
 * code coverage instrumentation, and regression tracking.
 *
 * Build: gcc -std=c99 -Wall -O2 -I. -I Apkc tests/test_phase17_testing_coverage.c -o test_phase17 && ./test_phase17
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Apkc/test_unit_generation.h"
#include "Apkc/test_property_based.h"
#include "Apkc/test_coverage_metrics.h"
#include "Apkc/test_regression_tracking.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

static u32 tests_passed = 0;
static u32 tests_failed = 0;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { \
		printf("FAIL: %s\n", msg); \
		tests_failed++; \
		return 0; \
	} \
} while (0)

#define PASS(msg) do { \
	printf("PASS: %s\n", msg); \
	tests_passed++; \
	return 1; \
} while (0)

/* ============================================================ */
/* STAGE 17.1: UNIT TEST GENERATION TESTS */
/* ============================================================ */

static u8 test_testsuite_init(void) {
	struct TestSuite suite = {0};
	testsuite_init(&suite);

	ASSERT(suite.test_count == 0, "test_count initialized");
	ASSERT(suite.function_count == 0, "function_count initialized");
	ASSERT(suite.next_test_id == 1, "next_test_id initialized");
	PASS("testsuite_init");
}

static u8 test_testsuite_register_function(void) {
	struct TestSuite suite = {0};
	testsuite_init(&suite);

	u8 result = testsuite_register_function(&suite, "add", 2, 0, 1);

	ASSERT(result == 1, "register_function returns 1");
	ASSERT(suite.function_count == 1, "function_count incremented");
	ASSERT(suite.functions[0].is_pure == 1, "is_pure set");
	PASS("testsuite_register_function");
}

static u8 test_testsuite_add_test_case(void) {
	struct TestSuite suite = {0};
	testsuite_init(&suite);
	testsuite_register_function(&suite, "add", 2, 0, 1);

	u64 inputs[] = {3, 5};
	u8 result = testsuite_add_test_case(&suite, "test_add_simple", 0, inputs, 2, 8, TEST_TYPE_UNIT);

	ASSERT(result == 1, "add_test_case returns 1");
	ASSERT(suite.test_count == 1, "test_count incremented");
	ASSERT(suite.test_cases[0].expected_output == 8, "expected_output set");
	PASS("testsuite_add_test_case");
}

static u8 test_testsuite_generate_edge_cases(void) {
	struct TestSuite suite = {0};
	testsuite_init(&suite);
	testsuite_register_function(&suite, "square", 1, 0, 1);

	u32 generated = testsuite_generate_edge_cases(&suite, 0, 10);

	ASSERT(generated == 4, "generate_edge_cases returns 4");
	ASSERT(suite.test_count == 4, "test_count incremented by 4");
	PASS("testsuite_generate_edge_cases");
}

static u8 test_testsuite_find_test(void) {
	struct TestSuite suite = {0};
	testsuite_init(&suite);

	u64 inputs[] = {5};
	testsuite_add_test_case(&suite, "test1", 0, inputs, 1, 10, TEST_TYPE_UNIT);

	struct TestCase *found = testsuite_find_test(&suite, 1);

	ASSERT(found != 0, "find_test returns non-null");
	ASSERT(found->expected_output == 10, "found correct test case");
	PASS("testsuite_find_test");
}

static u8 test_testsuite_record_result(void) {
	struct TestSuite suite = {0};
	testsuite_init(&suite);

	u8 result = testsuite_record_result(&suite, 1, 1, 42, 0);

	ASSERT(result == 1, "record_result returns 1");
	ASSERT(suite.total_tests_run == 1, "total_tests_run incremented");
	ASSERT(suite.total_tests_passed == 1, "total_tests_passed incremented");
	PASS("testsuite_record_result");
}

static u8 test_testsuite_get_pass_rate(void) {
	struct TestSuite suite = {0};
	testsuite_init(&suite);

	testsuite_record_result(&suite, 1, 1, 42, 0);
	testsuite_record_result(&suite, 2, 1, 42, 0);
	testsuite_record_result(&suite, 3, 0, 0, "failed");

	u32 rate = testsuite_get_pass_rate(&suite);

	ASSERT(rate == 66, "get_pass_rate returns ~66% for 2/3 passed");
	PASS("testsuite_get_pass_rate");
}

/* ============================================================ */
/* STAGE 17.2: PROPERTY-BASED TESTING TESTS */
/* ============================================================ */

/* Simple property validator */
static u8 prop_always_positive(u64 input, u64 expected) {
	return (input > 0) ? 1 : 0;
}

static u64 simple_generator(u32 seed, u32 iteration) {
	return (seed + iteration) % 100 + 1;
}

static u8 test_proptest_init(void) {
	struct PropertyTest pt = {0};
	proptest_init(&pt, simple_generator, 10, 42);

	ASSERT(pt.property_count == 0, "property_count initialized");
	ASSERT(pt.iterations == 10, "iterations set");
	ASSERT(pt.seed == 42, "seed set");
	PASS("proptest_init");
}

static u8 test_proptest_register_property(void) {
	struct PropertyTest pt = {0};
	proptest_init(&pt, simple_generator, 10, 42);

	u8 result = proptest_register_property(&pt, "positive", prop_always_positive);

	ASSERT(result == 1, "register_property returns 1");
	ASSERT(pt.property_count == 1, "property_count incremented");
	ASSERT(pt.properties[0].is_enabled == 1, "is_enabled set");
	PASS("proptest_register_property");
}

static u8 test_proptest_validate_property(void) {
	struct PropertyTest pt = {0};
	proptest_init(&pt, simple_generator, 10, 42);
	proptest_register_property(&pt, "positive", prop_always_positive);

	u8 result = proptest_validate_property(&pt, 0, 5, 0);

	ASSERT(result == 1, "validate_property returns 1 for valid input");
	ASSERT(pt.total_tests_run == 1, "total_tests_run incremented");
	ASSERT(pt.properties[0].success_count == 1, "success_count incremented");
	PASS("proptest_validate_property");
}

static u8 test_proptest_get_success_rate(void) {
	struct PropertyTest pt = {0};
	proptest_init(&pt, simple_generator, 10, 42);
	proptest_register_property(&pt, "positive", prop_always_positive);

	proptest_validate_property(&pt, 0, 5, 0);
	proptest_validate_property(&pt, 0, 10, 0);
	proptest_validate_property(&pt, 0, 0, 0);  /* Fails for 0 */

	u32 rate = proptest_get_success_rate(&pt, 0);

	ASSERT(rate == 66, "get_success_rate returns ~66% for 2/3");
	PASS("proptest_get_success_rate");
}

static u8 test_proptest_count_failures(void) {
	struct PropertyTest pt = {0};
	proptest_init(&pt, simple_generator, 10, 42);
	proptest_register_property(&pt, "prop1", prop_always_positive);
	proptest_register_property(&pt, "prop2", prop_always_positive);

	proptest_validate_property(&pt, 0, 0, 0);
	proptest_validate_property(&pt, 1, 1, 0);

	u32 failures = proptest_count_failures(&pt);

	ASSERT(failures == 1, "count_failures returns 1");
	PASS("proptest_count_failures");
}

/* ============================================================ */
/* STAGE 17.3: CODE COVERAGE INSTRUMENTATION TESTS */
/* ============================================================ */

static u8 test_coverage_init(void) {
	struct CoverageMetrics cov = {0};
	coverage_init(&cov);

	ASSERT(cov.line_count == 0, "line_count initialized");
	ASSERT(cov.branch_count == 0, "branch_count initialized");
	ASSERT(cov.function_count == 0, "function_count initialized");
	PASS("coverage_init");
}

static u8 test_coverage_record_line(void) {
	struct CoverageMetrics cov = {0};
	coverage_init(&cov);

	u8 result = coverage_record_line(&cov, "main.c", 10);

	ASSERT(result == 1, "record_line returns 1");
	ASSERT(cov.line_count == 1, "line_count incremented");
	ASSERT(cov.total_lines_covered == 1, "total_lines_covered incremented");
	PASS("coverage_record_line");
}

static u8 test_coverage_record_branch(void) {
	struct CoverageMetrics cov = {0};
	coverage_init(&cov);

	u8 result = coverage_record_branch(&cov, "main.c", 15, 1);

	ASSERT(result == 1, "record_branch returns 1");
	ASSERT(cov.branch_count == 1, "branch_count incremented");
	ASSERT(cov.total_branches == 1, "total_branches incremented");
	PASS("coverage_record_branch");
}

static u8 test_coverage_record_function_call(void) {
	struct CoverageMetrics cov = {0};
	coverage_init(&cov);

	u8 result = coverage_record_function_call(&cov, "printf");

	ASSERT(result == 1, "record_function_call returns 1");
	ASSERT(cov.function_count == 1, "function_count incremented");
	ASSERT(cov.total_functions_covered == 1, "total_functions_covered incremented");
	PASS("coverage_record_function_call");
}

static u8 test_coverage_get_line_percent(void) {
	struct CoverageMetrics cov = {0};
	coverage_init(&cov);

	coverage_record_line(&cov, "main.c", 1);
	coverage_record_line(&cov, "main.c", 2);
	coverage_record_line(&cov, "main.c", 3);
	cov.total_lines_executable = 3;

	u32 pct = coverage_get_line_percent(&cov);

	ASSERT(pct == 100, "get_line_percent returns 100 for 3/3 covered");
	PASS("coverage_get_line_percent");
}

static u8 test_coverage_meets_goal(void) {
	struct CoverageMetrics cov = {0};
	coverage_init(&cov);

	coverage_record_line(&cov, "main.c", 1);
	cov.total_lines_executable = 1;
	cov.total_branches = 1;
	cov.total_branches_covered = 1;
	cov.total_functions = 1;
	cov.total_functions_covered = 1;

	u8 meets = coverage_meets_goal(&cov, 80);

	ASSERT(meets == 1, "meets_goal returns 1 for 100% coverage vs 80% goal");
	PASS("coverage_meets_goal");
}

/* ============================================================ */
/* STAGE 17.4: REGRESSION TRACKING TESTS */
/* ============================================================ */

static u8 test_regdb_init(void) {
	struct RegressionDb db = {0};
	regdb_init(&db);

	ASSERT(db.test_count == 0, "test_count initialized");
	ASSERT(db.history_count == 0, "history_count initialized");
	ASSERT(db.next_build_number == 1, "next_build_number initialized");
	PASS("regdb_init");
}

static u8 test_regdb_register_test(void) {
	struct RegressionDb db = {0};
	regdb_init(&db);

	u8 result = regdb_register_test(&db, 1, "test_add", "abc123");

	ASSERT(result == 1, "register_test returns 1");
	ASSERT(db.test_count == 1, "test_count incremented");
	ASSERT(db.latest_report.total_tests == 1, "total_tests incremented");
	PASS("regdb_register_test");
}

static u8 test_regdb_mark_known_failure(void) {
	struct RegressionDb db = {0};
	regdb_init(&db);
	regdb_register_test(&db, 1, "test_bug", "abc123");

	u8 result = regdb_mark_known_failure(&db, 1);

	ASSERT(result == 1, "mark_known_failure returns 1");
	ASSERT(db.tests[0].was_failing == 1, "was_failing set");
	PASS("regdb_mark_known_failure");
}

static u8 test_regdb_record_result(void) {
	struct RegressionDb db = {0};
	regdb_init(&db);
	regdb_register_test(&db, 1, "test1", "abc123");

	u8 result = regdb_record_result(&db, 1, 1, "abc123", 0);

	ASSERT(result == 1, "record_result returns 1");
	ASSERT(db.history_count == 1, "history_count incremented");
	ASSERT(db.tests[0].consecutive_passes == 1, "consecutive_passes incremented");
	PASS("regdb_record_result");
}

static u8 test_regdb_is_newly_failing(void) {
	struct RegressionDb db = {0};
	regdb_init(&db);
	regdb_register_test(&db, 1, "test1", "abc123");
	regdb_record_result(&db, 1, 0, "abc123", "error");

	u8 is_new = regdb_is_newly_failing(&db, 1);

	ASSERT(is_new == 1, "is_newly_failing detects new failure");
	PASS("regdb_is_newly_failing");
}

static u8 test_regdb_get_consecutive_failures(void) {
	struct RegressionDb db = {0};
	regdb_init(&db);
	regdb_register_test(&db, 1, "flaky", "abc123");

	regdb_record_result(&db, 1, 0, "abc123", "failed");
	regdb_record_result(&db, 1, 0, "abc123", "failed");
	regdb_record_result(&db, 1, 0, "abc123", "failed");

	u32 count = regdb_get_consecutive_failures(&db, 1);

	ASSERT(count == 3, "get_consecutive_failures returns 3");
	PASS("regdb_get_consecutive_failures");
}

int main(void) {
	printf("=== Phase 17: Testing & Coverage Infrastructure Tests ===\n\n");

	printf("Stage 17.1: Unit Test Generation\n");
	test_testsuite_init();
	test_testsuite_register_function();
	test_testsuite_add_test_case();
	test_testsuite_generate_edge_cases();
	test_testsuite_find_test();
	test_testsuite_record_result();
	test_testsuite_get_pass_rate();

	printf("\nStage 17.2: Property-Based Testing\n");
	test_proptest_init();
	test_proptest_register_property();
	test_proptest_validate_property();
	test_proptest_get_success_rate();
	test_proptest_count_failures();

	printf("\nStage 17.3: Code Coverage Instrumentation\n");
	test_coverage_init();
	test_coverage_record_line();
	test_coverage_record_branch();
	test_coverage_record_function_call();
	test_coverage_get_line_percent();
	test_coverage_meets_goal();

	printf("\nStage 17.4: Regression Test Tracking\n");
	test_regdb_init();
	test_regdb_register_test();
	test_regdb_mark_known_failure();
	test_regdb_record_result();
	test_regdb_is_newly_failing();
	test_regdb_get_consecutive_failures();

	printf("\n=== Test Results ===\n");
	printf("Passed: %u\n", tests_passed);
	printf("Failed: %u\n", tests_failed);

	if (tests_failed == 0) {
		printf("\nAll tests PASSED! ✓\n");
		return 0;
	} else {
		printf("\nSome tests FAILED! ✗\n");
		return 1;
	}
}
