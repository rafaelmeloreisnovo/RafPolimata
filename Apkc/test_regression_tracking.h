/* test_regression_tracking.h — Regression Test Tracking (Stage 17.4)
 *
 * Regression database: store known failing test cases and expected results.
 * Test history: track test results across builds for trend analysis.
 * Failure analysis: categorize regressions (new failure vs reappearance).
 * Quarantine system: mark flaky tests for investigation.
 * Trend reporting: generate historical analysis of test health.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_TEST_REGRESSION_TRACKING_H
#define APKC_TEST_REGRESSION_TRACKING_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Regression test entry */
struct RegressionTest {
	u32 test_id;                /* Test identifier */
	const char *test_name;      /* Test name */
	const char *commit_hash;    /* Git commit where test was added */
	u8 was_failing;             /* 1 if test was originally failing */
	u8 is_quarantined;          /* 1 if test is flaky/under investigation */
	u32 consecutive_failures;   /* Times failed in a row */
	u32 consecutive_passes;     /* Times passed in a row */
};

/* Test result history entry */
struct TestHistory {
	u32 test_id;                /* Test identifier */
	u64 timestamp;              /* When test was run (unix seconds) */
	u8 passed;                  /* 1 if test passed */
	const char *commit_hash;    /* Commit being tested */
	u32 build_number;           /* CI build number */
	const char *failure_message;/* Error message if failed */
};

/* Regression analysis report */
struct RegressionReport {
	u32 total_tests;            /* Total tracked tests */
	u32 currently_failing;      /* Tests failing in current build */
	u32 newly_failing;          /* Tests that just started failing */
	u32 reappeared_failures;    /* Tests that fail again after passing */
	u32 quarantined_count;      /* Flaky tests under investigation */
	u32 trend_improving;        /* Tests improving (more passes lately) */
	u32 trend_worsening;        /* Tests worsening (more failures lately) */
	u32 build_number;           /* Current build number */
};

/* Regression tracking database */
struct RegressionDb {
	struct RegressionTest tests[256];       /* Up to 256 tracked tests */
	u32 test_count;
	struct TestHistory history[1024];       /* Up to 1024 history entries */
	u32 history_count;
	struct RegressionReport latest_report;
	u32 next_build_number;
};

/* ============================================================ */
/* REGRESSION DATABASE INITIALIZATION */
/* ============================================================ */

/* Initialize regression database */
static inline void regdb_init(struct RegressionDb *db) {
	if (!db) return;
	db->test_count = 0;
	db->history_count = 0;
	db->next_build_number = 1;
	db->latest_report.total_tests = 0;
	db->latest_report.currently_failing = 0;
	db->latest_report.newly_failing = 0;
	db->latest_report.reappeared_failures = 0;
	db->latest_report.quarantined_count = 0;
	db->latest_report.trend_improving = 0;
	db->latest_report.trend_worsening = 0;
	db->latest_report.build_number = 0;
}

/* ============================================================ */
/* REGRESSION TEST REGISTRATION */
/* ============================================================ */

/* Register regression test */
static inline u8 regdb_register_test(
	struct RegressionDb *db,
	u32 test_id,
	const char *test_name,
	const char *commit_hash) {

	if (!db || !test_name || !commit_hash) return 0;
	if (db->test_count >= 256) return 0;

	struct RegressionTest *test = &db->tests[db->test_count];
	test->test_id = test_id;
	test->test_name = test_name;
	test->commit_hash = commit_hash;
	test->was_failing = 0;
	test->is_quarantined = 0;
	test->consecutive_failures = 0;
	test->consecutive_passes = 0;

	db->test_count++;
	db->latest_report.total_tests++;

	return 1;
}

/* Mark test as originally failing (known bug) */
static inline u8 regdb_mark_known_failure(
	struct RegressionDb *db,
	u32 test_id) {

	if (!db) return 0;

	u32 i;
	for (i = 0; i < db->test_count; i++) {
		if (db->tests[i].test_id == test_id) {
			db->tests[i].was_failing = 1;
			return 1;
		}
	}

	return 0;
}

/* Quarantine flaky test */
static inline u8 regdb_quarantine_test(
	struct RegressionDb *db,
	u32 test_id) {

	if (!db) return 0;

	u32 i;
	for (i = 0; i < db->test_count; i++) {
		if (db->tests[i].test_id == test_id) {
			db->tests[i].is_quarantined = 1;
			db->latest_report.quarantined_count++;
			return 1;
		}
	}

	return 0;
}

/* ============================================================ */
/* TEST RESULT RECORDING */
/* ============================================================ */

/* Record test result in history */
static inline u8 regdb_record_result(
	struct RegressionDb *db,
	u32 test_id,
	u8 passed,
	const char *commit_hash,
	const char *failure_msg) {

	if (!db || !commit_hash) return 0;
	if (db->history_count >= 1024) return 0;

	struct TestHistory *entry = &db->history[db->history_count];
	entry->test_id = test_id;
	entry->timestamp = 0;  /* Would be current time */
	entry->passed = passed;
	entry->commit_hash = commit_hash;
	entry->build_number = db->next_build_number;
	entry->failure_message = failure_msg;

	db->history_count++;

	/* Update consecutive counters */
	u32 i;
	for (i = 0; i < db->test_count; i++) {
		if (db->tests[i].test_id == test_id) {
			if (passed) {
				db->tests[i].consecutive_passes++;
				db->tests[i].consecutive_failures = 0;
			} else {
				db->tests[i].consecutive_failures++;
				db->tests[i].consecutive_passes = 0;
				db->latest_report.currently_failing++;
			}
			break;
		}
	}

	return 1;
}

/* ============================================================ */
/* REGRESSION ANALYSIS */
/* ============================================================ */

/* Detect newly failing test */
static inline u8 regdb_is_newly_failing(
	struct RegressionDb *db,
	u32 test_id) {

	if (!db) return 0;

	u32 i;
	for (i = 0; i < db->test_count; i++) {
		if (db->tests[i].test_id == test_id) {
			/* New failure if: not originally failing, and just started failing */
			return !db->tests[i].was_failing && db->tests[i].consecutive_failures == 1;
		}
	}

	return 0;
}

/* Count consecutive failures for test */
static inline u32 regdb_get_consecutive_failures(
	struct RegressionDb *db,
	u32 test_id) {

	if (!db) return 0;

	u32 i;
	for (i = 0; i < db->test_count; i++) {
		if (db->tests[i].test_id == test_id) {
			return db->tests[i].consecutive_failures;
		}
	}

	return 0;
}

/* Get test history for trend analysis */
static inline u32 regdb_get_test_history(
	struct RegressionDb *db,
	u32 test_id,
	struct TestHistory *out_history,
	u32 max_entries) {

	if (!db || !out_history) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < db->history_count && count < max_entries; i++) {
		if (db->history[i].test_id == test_id) {
			out_history[count++] = db->history[i];
		}
	}

	return count;
}

/* Identify improving tests (more recent passes) */
static inline u32 regdb_find_improving_tests(
	struct RegressionDb *db,
	u32 *test_ids,
	u32 max_count) {

	if (!db || !test_ids) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < db->test_count && count < max_count; i++) {
		/* Improving if: was failing, consecutive_passes > 0 */
		if (db->tests[i].was_failing && db->tests[i].consecutive_passes > 0) {
			test_ids[count++] = db->tests[i].test_id;
			db->latest_report.trend_improving++;
		}
	}

	return count;
}

/* Identify worsening tests (recently started failing) */
static inline u32 regdb_find_worsening_tests(
	struct RegressionDb *db,
	u32 *test_ids,
	u32 max_count) {

	if (!db || !test_ids) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < db->test_count && count < max_count; i++) {
		/* Worsening if: was passing, recent failures > threshold */
		if (!db->tests[i].was_failing && db->tests[i].consecutive_failures >= 2) {
			test_ids[count++] = db->tests[i].test_id;
			db->latest_report.trend_worsening++;
		}
	}

	return count;
}

/* Get regression report */
static inline struct RegressionReport *regdb_get_report(struct RegressionDb *db) {
	if (!db) return 0;
	db->latest_report.build_number = db->next_build_number;
	return &db->latest_report;
}

#endif /* APKC_TEST_REGRESSION_TRACKING_H */
