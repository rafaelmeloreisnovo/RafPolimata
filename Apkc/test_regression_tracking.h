/* test_regression_tracking.h — Regression Test Tracking (Stage 14.3)
 *
 * Regression test database: store and track known issues and fixes.
 * Issue linking: connect tests to bug reports and commits.
 * Regression detection: identify when fixed issues resurface.
 * Test metadata: timestamp, author, affected versions.
 * Bisect support: identify commit that introduced regression.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_TEST_REGRESSION_TRACKING_H
#define APKC_TEST_REGRESSION_TRACKING_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Regression test status */
enum RegressionStatus {
	REG_OK = 0,                 /* Test passing (issue fixed) */
	REG_RESURFACED = 1,         /* Issue has resurfaced */
	REG_NOT_REPRODUCIBLE = 2,   /* Issue cannot be reproduced */
	REG_UNKNOWN_STATUS = 3,     /* Status unknown */
	REG_BLOCKED = 4             /* Test blocked/skipped */
};

/* Test severity level */
enum RegressionSeverity {
	SEV_TRIVIAL = 0,            /* Cosmetic issue */
	SEV_MINOR = 1,              /* Minor functionality broken */
	SEV_MAJOR = 2,              /* Major functionality broken */
	SEV_BLOCKER = 3             /* Release blocker */
};

/* Regression test case */
struct RegressionTest {
	const char *test_name;      /* Test identifier */
	const char *issue_id;       /* Bug report ID (e.g., "BUG-123") */
	const char *description;    /* Issue description */
	u8 severity;                /* RegressionSeverity */
	const char *introduced_in;  /* First affected version */
	const char *fixed_in;       /* Version where fix landed */
	const char *fix_commit;     /* Git commit SHA that fixed it */
	u8 status;                  /* RegressionStatus */
	u32 test_count;             /* Times this test has run */
	u32 pass_count;             /* Times test passed */
	u32 fail_count;             /* Times test failed (resurfaced) */
	u64 last_run_time;          /* Unix timestamp of last run */
};

/* Test failure record */
struct FailureRecord {
	const char *test_name;      /* Which test failed */
	u64 timestamp;              /* When it failed */
	const char *git_sha;        /* Which commit was being tested */
	const char *error_message;  /* What went wrong */
	u32 fail_count;             /* Number of failures at this commit */
};

/* Regression test suite */
struct RegressionSuite {
	const char *suite_name;     /* Suite identifier */
	struct RegressionTest tests[64]; /* Up to 64 regression tests */
	u32 test_count;
	struct FailureRecord failures[32]; /* Up to 32 recent failures */
	u32 failure_count;
	u32 active_count;           /* Tests currently failing */
	u32 fixed_count;            /* Issues fixed in this suite */
	u64 last_scan_time;         /* When suite was last scanned */
};

/* ============================================================ */
/* REGRESSION SUITE MANAGEMENT */
/* ============================================================ */

/* Initialize regression suite */
static inline void reg_init_suite(
	struct RegressionSuite *suite,
	const char *suite_name) {

	if (!suite) return;
	suite->suite_name = suite_name;
	suite->test_count = 0;
	suite->failure_count = 0;
	suite->active_count = 0;
	suite->fixed_count = 0;
	suite->last_scan_time = 0;
}

/* ============================================================ */
/* REGRESSION TEST REGISTRATION */
/* ============================================================ */

/* Add regression test */
static inline u8 reg_add_test(
	struct RegressionSuite *suite,
	const char *test_name,
	const char *issue_id,
	const char *description,
	u8 severity,
	const char *introduced_in,
	const char *fixed_in,
	const char *fix_commit) {

	if (!suite || !test_name || !issue_id) return REG_UNKNOWN_STATUS;
	if (suite->test_count >= 64) return REG_UNKNOWN_STATUS;

	struct RegressionTest *test = &suite->tests[suite->test_count];
	test->test_name = test_name;
	test->issue_id = issue_id;
	test->description = description;
	test->severity = severity;
	test->introduced_in = introduced_in;
	test->fixed_in = fixed_in;
	test->fix_commit = fix_commit;
	test->status = REG_OK;
	test->test_count = 0;
	test->pass_count = 0;
	test->fail_count = 0;
	test->last_run_time = 0;

	suite->test_count++;
	return REG_OK;
}

/* ============================================================ */
/* TEST EXECUTION & TRACKING */
/* ============================================================ */

/* Record test run result */
static inline u8 reg_record_run(
	struct RegressionSuite *suite,
	const char *test_name,
	u8 passed) {

	if (!suite || !test_name) return REG_UNKNOWN_STATUS;

	/* Find test */
	u32 i;
	for (i = 0; i < suite->test_count; i++) {
		if (!suite->tests[i].test_name) continue;

		const char *tname = suite->tests[i].test_name;
		u32 j = 0;
		while (test_name[j] && tname[j] && test_name[j] == tname[j]) j++;

		if (test_name[j] == 0 && tname[j] == 0) {
			/* Found test */
			struct RegressionTest *test = &suite->tests[i];
			test->test_count++;
			test->last_run_time = 0;  /* Would be current timestamp */

			if (passed) {
				test->pass_count++;
				test->status = REG_OK;
			} else {
				test->fail_count++;
				test->status = REG_RESURFACED;
				suite->active_count++;
			}

			return test->status;
		}
	}

	return REG_UNKNOWN_STATUS;
}

/* Record failure with commit info */
static inline u8 reg_record_failure(
	struct RegressionSuite *suite,
	const char *test_name,
	u64 timestamp,
	const char *git_sha,
	const char *error_message) {

	if (!suite || !test_name || !git_sha) return REG_UNKNOWN_STATUS;
	if (suite->failure_count >= 32) return REG_UNKNOWN_STATUS;

	struct FailureRecord *record = &suite->failures[suite->failure_count];
	record->test_name = test_name;
	record->timestamp = timestamp;
	record->git_sha = git_sha;
	record->error_message = error_message;
	record->fail_count = 1;

	suite->failure_count++;
	return REG_OK;
}

/* ============================================================ */
/* REGRESSION ANALYSIS */
/* ============================================================ */

/* Check if issue has resurfaced */
static inline u8 reg_is_resurfaced(
	struct RegressionSuite *suite,
	const char *test_name) {

	if (!suite || !test_name) return 0;

	u32 i;
	for (i = 0; i < suite->test_count; i++) {
		if (!suite->tests[i].test_name) continue;

		const char *tname = suite->tests[i].test_name;
		u32 j = 0;
		while (test_name[j] && tname[j] && test_name[j] == tname[j]) j++;

		if (test_name[j] == 0 && tname[j] == 0) {
			return (suite->tests[i].status == REG_RESURFACED) ? 1 : 0;
		}
	}

	return 0;
}

/* Count failing tests by severity */
static inline u32 reg_count_by_severity(
	struct RegressionSuite *suite,
	u8 severity) {

	if (!suite) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < suite->test_count; i++) {
		if (suite->tests[i].severity == severity &&
			suite->tests[i].status == REG_RESURFACED) {
			count++;
		}
	}

	return count;
}

/* ============================================================ */
/* BISECT SUPPORT */
/* ============================================================ */

/* Find failure in commit history (would use binary search) */
static inline const char *reg_bisect_find_culprit(
	struct RegressionSuite *suite,
	const char *test_name) {

	if (!suite || !test_name) return 0;

	/* Find first failure in failure record */
	u32 i;
	for (i = 0; i < suite->failure_count; i++) {
		const char *fname = suite->failures[i].test_name;
		u32 j = 0;
		while (test_name[j] && fname[j] && test_name[j] == fname[j]) j++;

		if (test_name[j] == 0 && fname[j] == 0) {
			return suite->failures[i].git_sha;
		}
	}

	return 0;
}

/* ============================================================ */
/* STATISTICS & REPORTING */
/* ============================================================ */

/* Get total active (resurfaced) issues */
static inline u32 reg_get_active_count(struct RegressionSuite *suite) {
	if (!suite) return 0;
	return suite->active_count;
}

/* Get number of fixed issues */
static inline u32 reg_get_fixed_count(struct RegressionSuite *suite) {
	if (!suite) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < suite->test_count; i++) {
		if (suite->tests[i].status == REG_OK &&
			suite->tests[i].fixed_in != 0) {
			count++;
		}
	}

	return count;
}

/* Get total test count */
static inline u32 reg_get_test_count(struct RegressionSuite *suite) {
	if (!suite) return 0;
	return suite->test_count;
}

/* Check if suite is healthy (no active regressions) */
static inline u8 reg_is_healthy(struct RegressionSuite *suite) {
	if (!suite) return 1;

	u32 i;
	for (i = 0; i < suite->test_count; i++) {
		if (suite->tests[i].status == REG_RESURFACED) {
			return 0;
		}
	}

	return 1;
}

#endif /* APKC_TEST_REGRESSION_TRACKING_H */
