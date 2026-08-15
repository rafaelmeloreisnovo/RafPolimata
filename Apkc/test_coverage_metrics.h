/* test_coverage_metrics.h — Test Coverage Metrics (Stage 14.4)
 *
 * Coverage measurement: instruction, branch, path coverage.
 * Coverage reporting: percentage covered, uncovered regions.
 * Coverage goals: set and track coverage targets by module.
 * Delta coverage: measure coverage changes across commits.
 * Coverage visualization: identify low-coverage areas.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_TEST_COVERAGE_METRICS_H
#define APKC_TEST_COVERAGE_METRICS_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Coverage type */
enum CoverageType {
	COVERAGE_STATEMENT = 1,     /* Statement/instruction coverage */
	COVERAGE_BRANCH = 2,        /* Branch/decision coverage */
	COVERAGE_PATH = 3,          /* Path coverage (all code paths) */
	COVERAGE_MC_DC = 4          /* Modified condition/decision coverage */
};

/* Coverage metric */
struct CoverageMetric {
	u8 coverage_type;           /* CoverageType */
	u32 total_items;            /* Total statements/branches */
	u32 covered_items;          /* Items executed in tests */
	u32 percentage;             /* Coverage percentage (0-100) */
};

/* Function coverage info */
struct FunctionCoverage {
	const char *function_name;  /* Function name */
	u32 total_lines;            /* Total lines in function */
	u32 covered_lines;          /* Lines executed */
	u32 coverage_percent;       /* Line coverage percentage */
	u8 is_fully_covered;        /* 1 if 100% covered */
	u8 is_uncovered;            /* 1 if 0% covered */
	u32 call_count;             /* Function invocation count */
};

/* Coverage report */
struct CoverageReport {
	const char *module_name;    /* Module being measured */
	const char *timestamp;      /* Report generation time */

	struct CoverageMetric metrics[4];  /* Statement, branch, path, MC/DC */
	u32 metric_count;

	struct FunctionCoverage functions[32]; /* Up to 32 functions */
	u32 function_count;

	u32 total_lines;            /* Total executable lines */
	u32 covered_lines;          /* Lines with at least one hit */
	u32 overall_percentage;     /* Overall coverage percentage */

	u32 target_percentage;      /* Coverage goal */
	u8 meets_target;            /* 1 if coverage >= target */

	u64 total_executions;       /* Total instruction executions */
	u32 unique_paths_covered;   /* Unique execution paths taken */
};

/* Coverage delta (comparison between builds) */
struct CoverageDelta {
	const char *baseline_commit;
	const char *current_commit;
	s32 percentage_change;      /* Change in coverage percentage */
	u32 new_covered_lines;      /* Lines newly covered */
	u32 newly_uncovered_lines;  /* Lines no longer covered */
	u8 is_improvement;          /* 1 if coverage improved */
	u8 is_regression;           /* 1 if coverage worsened */
};

/* ============================================================ */
/* COVERAGE REPORT INITIALIZATION */
/* ============================================================ */

/* Initialize coverage report */
static inline void cov_init_report(
	struct CoverageReport *report,
	const char *module_name,
	const char *timestamp) {

	if (!report) return;
	report->module_name = module_name;
	report->timestamp = timestamp;
	report->metric_count = 0;
	report->function_count = 0;
	report->total_lines = 0;
	report->covered_lines = 0;
	report->overall_percentage = 0;
	report->target_percentage = 80;  /* Default: 80% coverage target */
	report->meets_target = 0;
	report->total_executions = 0;
	report->unique_paths_covered = 0;
}

/* ============================================================ */
/* COVERAGE METRIC RECORDING */
/* ============================================================ */

/* Add coverage metric */
static inline void cov_add_metric(
	struct CoverageReport *report,
	u8 coverage_type,
	u32 total,
	u32 covered) {

	if (!report || report->metric_count >= 4) return;

	struct CoverageMetric *metric = &report->metrics[report->metric_count];
	metric->coverage_type = coverage_type;
	metric->total_items = total;
	metric->covered_items = covered;
	metric->percentage = (total > 0) ? (covered * 100) / total : 0;

	report->metric_count++;
}

/* Add function coverage */
static inline u8 cov_add_function(
	struct CoverageReport *report,
	const char *function_name,
	u32 total_lines,
	u32 covered_lines) {

	if (!report || !function_name) return 0;
	if (report->function_count >= 32) return 0;

	struct FunctionCoverage *func = &report->functions[report->function_count];
	func->function_name = function_name;
	func->total_lines = total_lines;
	func->covered_lines = covered_lines;
	func->coverage_percent = (total_lines > 0) ? (covered_lines * 100) / total_lines : 0;
	func->is_fully_covered = (covered_lines == total_lines && total_lines > 0) ? 1 : 0;
	func->is_uncovered = (covered_lines == 0) ? 1 : 0;
	func->call_count = 0;

	report->function_count++;
	report->total_lines += total_lines;
	report->covered_lines += covered_lines;

	return 1;
}

/* Record function call */
static inline void cov_record_call(
	struct CoverageReport *report,
	const char *function_name) {

	if (!report || !function_name) return;

	u32 i;
	for (i = 0; i < report->function_count; i++) {
		if (!report->functions[i].function_name) continue;

		const char *fname = report->functions[i].function_name;
		u32 j = 0;
		while (function_name[j] && fname[j] && function_name[j] == fname[j]) j++;

		if (function_name[j] == 0 && fname[j] == 0) {
			report->functions[i].call_count++;
			break;
		}
	}
}

/* ============================================================ */
/* COVERAGE ANALYSIS */
/* ============================================================ */

/* Compute overall coverage percentage */
static inline void cov_compute_overall(struct CoverageReport *report) {
	if (!report || report->total_lines == 0) {
		report->overall_percentage = 0;
		return;
	}

	report->overall_percentage = (report->covered_lines * 100) / report->total_lines;
	report->meets_target = (report->overall_percentage >= report->target_percentage) ? 1 : 0;
}

/* Set coverage target */
static inline void cov_set_target(
	struct CoverageReport *report,
	u32 target_percent) {

	if (!report) return;
	report->target_percentage = (target_percent > 100) ? 100 : target_percent;
}

/* ============================================================ */
/* UNCOVERED CODE IDENTIFICATION */
/* ============================================================ */

/* Count uncovered functions */
static inline u32 cov_count_uncovered_functions(struct CoverageReport *report) {
	if (!report) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < report->function_count; i++) {
		if (report->functions[i].is_uncovered) count++;
	}
	return count;
}

/* Count partially covered functions */
static inline u32 cov_count_partial_coverage(struct CoverageReport *report) {
	if (!report) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < report->function_count; i++) {
		if (!report->functions[i].is_fully_covered &&
			!report->functions[i].is_uncovered) {
			count++;
		}
	}
	return count;
}

/* Find lowest coverage function */
static inline struct FunctionCoverage *cov_find_lowest_coverage(
	struct CoverageReport *report) {

	if (!report || report->function_count == 0) return 0;

	struct FunctionCoverage *lowest = &report->functions[0];
	u32 i;
	for (i = 1; i < report->function_count; i++) {
		if (report->functions[i].coverage_percent < lowest->coverage_percent) {
			lowest = &report->functions[i];
		}
	}

	return lowest;
}

/* ============================================================ */
/* DELTA COVERAGE */
/* ============================================================ */

/* Compute coverage delta */
static inline struct CoverageDelta cov_compute_delta(
	struct CoverageReport *baseline,
	struct CoverageReport *current) {

	struct CoverageDelta delta = {0};

	if (!baseline || !current) {
		return delta;
	}

	delta.baseline_commit = baseline->timestamp;
	delta.current_commit = current->timestamp;

	s32 old_percent = (s32)baseline->overall_percentage;
	s32 new_percent = (s32)current->overall_percentage;
	delta.percentage_change = new_percent - old_percent;

	delta.new_covered_lines = current->covered_lines - baseline->covered_lines;
	delta.newly_uncovered_lines = baseline->covered_lines - current->covered_lines;

	delta.is_improvement = (delta.percentage_change > 0) ? 1 : 0;
	delta.is_regression = (delta.percentage_change < 0) ? 1 : 0;

	return delta;
}

/* ============================================================ */
/* STATISTICS & REPORTING */
/* ============================================================ */

/* Get average function coverage */
static inline u32 cov_get_average_function_coverage(struct CoverageReport *report) {
	if (!report || report->function_count == 0) return 0;

	u64 total = 0;
	u32 i;
	for (i = 0; i < report->function_count; i++) {
		total += report->functions[i].coverage_percent;
	}

	return (u32)(total / report->function_count);
}

/* Get number of fully covered functions */
static inline u32 cov_count_fully_covered(struct CoverageReport *report) {
	if (!report) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < report->function_count; i++) {
		if (report->functions[i].is_fully_covered) count++;
	}
	return count;
}

/* Check if coverage meets target */
static inline u8 cov_meets_target(struct CoverageReport *report) {
	if (!report) return 0;
	return report->meets_target;
}

/* Get coverage gap (target - current) */
static inline u32 cov_get_gap_to_target(struct CoverageReport *report) {
	if (!report) return 0;

	if (report->overall_percentage >= report->target_percentage) {
		return 0;
	}

	return report->target_percentage - report->overall_percentage;
}

#endif /* APKC_TEST_COVERAGE_METRICS_H */
