/* test_coverage_metrics.h — Code Coverage Instrumentation (Stage 17.3)
 *
 * Line coverage: track which source lines are executed.
 * Branch coverage: count true/false taken for each condition.
 * Function coverage: track which functions are called.
 * Coverage reporting: generate human-readable coverage reports.
 * Coverage goals: set minimum coverage thresholds (80%, 90%, etc).
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_TEST_COVERAGE_METRICS_H
#define APKC_TEST_COVERAGE_METRICS_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Line coverage record */
struct LineCoverage {
	const char *file_path;      /* Source file */
	u32 line_number;            /* Line number (1-based) */
	u32 execution_count;        /* Times this line executed */
	u8 is_covered;              /* 1 if executed at least once */
};

/* Branch coverage record */
struct BranchCoverage {
	const char *file_path;      /* Source file */
	u32 line_number;            /* Line with branch */
	u32 true_count;             /* Times branch taken (true) */
	u32 false_count;            /* Times branch taken (false) */
	u8 both_branches_covered;   /* 1 if both paths executed */
};

/* Function coverage record */
struct FunctionCoverage {
	const char *function_name;  /* Function identifier */
	u32 call_count;             /* Number of times called */
	u8 is_covered;              /* 1 if called at least once */
	u32 total_lines;            /* Total lines in function */
	u32 covered_lines;          /* Lines executed in function */
};

/* Coverage metrics */
struct CoverageMetrics {
	struct LineCoverage lines[512];           /* Up to 512 lines */
	u32 line_count;
	struct BranchCoverage branches[256];      /* Up to 256 branches */
	u32 branch_count;
	struct FunctionCoverage functions[64];    /* Up to 64 functions */
	u32 function_count;
	u32 total_lines_covered;
	u32 total_lines_executable;
	u32 total_branches_covered;
	u32 total_branches;
	u32 total_functions_covered;
	u32 total_functions;
};

/* ============================================================ */
/* COVERAGE INITIALIZATION */
/* ============================================================ */

/* Initialize coverage metrics */
static inline void coverage_init(struct CoverageMetrics *cov) {
	if (!cov) return;
	cov->line_count = 0;
	cov->branch_count = 0;
	cov->function_count = 0;
	cov->total_lines_covered = 0;
	cov->total_lines_executable = 0;
	cov->total_branches_covered = 0;
	cov->total_branches = 0;
	cov->total_functions_covered = 0;
	cov->total_functions = 0;
}

/* ============================================================ */
/* COVERAGE RECORDING */
/* ============================================================ */

/* Record line execution */
static inline u8 coverage_record_line(
	struct CoverageMetrics *cov,
	const char *file,
	u32 line_num) {

	if (!cov || !file) return 0;
	if (cov->line_count >= 512) return 0;

	/* Find existing line entry */
	u32 i;
	for (i = 0; i < cov->line_count; i++) {
		if (cov->lines[i].line_number == line_num) {
			cov->lines[i].execution_count++;
			if (!cov->lines[i].is_covered) {
				cov->lines[i].is_covered = 1;
				cov->total_lines_covered++;
			}
			return 1;
		}
	}

	/* New line entry */
	struct LineCoverage *line = &cov->lines[cov->line_count];
	line->file_path = file;
	line->line_number = line_num;
	line->execution_count = 1;
	line->is_covered = 1;

	cov->line_count++;
	cov->total_lines_executable++;
	cov->total_lines_covered++;

	return 1;
}

/* Record branch execution */
static inline u8 coverage_record_branch(
	struct CoverageMetrics *cov,
	const char *file,
	u32 line_num,
	u8 taken) {

	if (!cov || !file) return 0;
	if (cov->branch_count >= 256) return 0;

	/* Find existing branch */
	u32 i;
	for (i = 0; i < cov->branch_count; i++) {
		if (cov->branches[i].line_number == line_num) {
			if (taken) {
				cov->branches[i].true_count++;
			} else {
				cov->branches[i].false_count++;
			}
			if (cov->branches[i].true_count > 0 && cov->branches[i].false_count > 0) {
				if (!cov->branches[i].both_branches_covered) {
					cov->branches[i].both_branches_covered = 1;
					cov->total_branches_covered++;
				}
			}
			return 1;
		}
	}

	/* New branch entry */
	struct BranchCoverage *branch = &cov->branches[cov->branch_count];
	branch->file_path = file;
	branch->line_number = line_num;
	branch->true_count = taken ? 1 : 0;
	branch->false_count = taken ? 0 : 1;
	branch->both_branches_covered = 0;

	cov->branch_count++;
	cov->total_branches++;

	return 1;
}

/* Record function call */
static inline u8 coverage_record_function_call(
	struct CoverageMetrics *cov,
	const char *function_name) {

	if (!cov || !function_name) return 0;
	if (cov->function_count >= 64) return 0;

	/* Find existing function */
	u32 i;
	for (i = 0; i < cov->function_count; i++) {
		if (cov->functions[i].function_name == function_name) {
			cov->functions[i].call_count++;
			if (!cov->functions[i].is_covered) {
				cov->functions[i].is_covered = 1;
				cov->total_functions_covered++;
			}
			return 1;
		}
	}

	/* New function entry */
	struct FunctionCoverage *func = &cov->functions[cov->function_count];
	func->function_name = function_name;
	func->call_count = 1;
	func->is_covered = 1;
	func->total_lines = 0;
	func->covered_lines = 0;

	cov->function_count++;
	cov->total_functions++;
	cov->total_functions_covered++;

	return 1;
}

/* ============================================================ */
/* COVERAGE ANALYSIS */
/* ============================================================ */

/* Get line coverage percentage */
static inline u32 coverage_get_line_percent(struct CoverageMetrics *cov) {
	if (!cov || cov->total_lines_executable == 0) return 0;
	return (cov->total_lines_covered * 100) / cov->total_lines_executable;
}

/* Get branch coverage percentage */
static inline u32 coverage_get_branch_percent(struct CoverageMetrics *cov) {
	if (!cov || cov->total_branches == 0) return 0;
	return (cov->total_branches_covered * 100) / cov->total_branches;
}

/* Get function coverage percentage */
static inline u32 coverage_get_function_percent(struct CoverageMetrics *cov) {
	if (!cov || cov->total_functions == 0) return 0;
	return (cov->total_functions_covered * 100) / cov->total_functions;
}

/* Find uncovered lines */
static inline u32 coverage_get_uncovered_lines(
	struct CoverageMetrics *cov,
	struct LineCoverage *out_lines,
	u32 max_count) {

	if (!cov || !out_lines) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < cov->line_count && count < max_count; i++) {
		if (!cov->lines[i].is_covered) {
			out_lines[count++] = cov->lines[i];
		}
	}

	return count;
}

/* Find partially covered branches */
static inline u32 coverage_get_uncovered_branches(
	struct CoverageMetrics *cov,
	struct BranchCoverage *out_branches,
	u32 max_count) {

	if (!cov || !out_branches) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < cov->branch_count && count < max_count; i++) {
		if (!cov->branches[i].both_branches_covered) {
			out_branches[count++] = cov->branches[i];
		}
	}

	return count;
}

/* Check if coverage meets goal */
static inline u8 coverage_meets_goal(
	struct CoverageMetrics *cov,
	u32 goal_percent) {

	if (!cov) return 0;

	u32 line_pct = coverage_get_line_percent(cov);
	u32 branch_pct = coverage_get_branch_percent(cov);
	u32 func_pct = coverage_get_function_percent(cov);

	/* All three metrics must meet goal */
	return (line_pct >= goal_percent) &&
	       (branch_pct >= goal_percent) &&
	       (func_pct >= goal_percent);
}

#endif /* APKC_TEST_COVERAGE_METRICS_H */
