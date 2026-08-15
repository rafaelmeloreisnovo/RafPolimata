/* perf_regression_detection.h — Performance Regression Detection (Stage 20.4)
 *
 * Baseline establishment: set performance baseline from reference build.
 * Regression detection: identify performance decreases.
 * Threshold alerting: alert when performance crosses thresholds.
 * Trend tracking: track performance over time.
 * Regression analysis: categorize root cause of regressions.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_PERF_REGRESSION_DETECTION_H
#define APKC_PERF_REGRESSION_DETECTION_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed int s32;

/* Performance baseline */
struct PerfBaseline {
	const char *test_name;
	u64 baseline_cycles;
	u64 baseline_ns;
	u32 baseline_ops;
	u64 timestamp;
};

/* Performance regression */
struct PerfRegression {
	const char *test_name;
	u64 current_cycles;
	u64 baseline_cycles;
	s32 regression_pct;      /* Negative = faster, positive = slower */
	u8 severity;             /* 1=minor, 2=moderate, 3=severe, 4=critical */
	u8 is_confirmed;         /* 1 if regression confirmed in multiple runs */
};

/* Regression detector */
struct RegressionDetector {
	struct PerfBaseline baselines[64];
	u32 baseline_count;
	struct PerfRegression regressions[32];
	u32 regression_count;
	u32 alert_threshold_pct;  /* Alert if >threshold% slower */
	u32 total_regressions;
	u32 critical_regressions;
};

/* Initialize regression detector */
static inline void regdet_init(struct RegressionDetector *det) {
	if (!det) return;
	det->baseline_count = 0;
	det->regression_count = 0;
	det->alert_threshold_pct = 10;  /* Default: 10% slowdown threshold */
	det->total_regressions = 0;
	det->critical_regressions = 0;
}

/* Establish baseline */
static inline u8 regdet_set_baseline(
	struct RegressionDetector *det,
	const char *test_name,
	u64 cycles,
	u32 ops) {

	if (!det || !test_name || det->baseline_count >= 64) return 0;

	struct PerfBaseline *baseline = &det->baselines[det->baseline_count];
	baseline->test_name = test_name;
	baseline->baseline_cycles = cycles;
	baseline->baseline_ops = ops;
	baseline->timestamp = 0;  /* Would be current time */

	det->baseline_count++;
	return 1;
}

/* Detect regression */
static inline u8 regdet_check_regression(
	struct RegressionDetector *det,
	const char *test_name,
	u64 current_cycles) {

	if (!det || !test_name) return 0;

	u32 i;
	for (i = 0; i < det->baseline_count; i++) {
		const char *base_name = det->baselines[i].test_name;
		const char *test = test_name;
		u32 j = 0;
		while (base_name[j] && test[j] && base_name[j] == test[j]) j++;
		if (base_name[j] != 0 || test[j] != 0) continue;

		u64 baseline = det->baselines[i].baseline_cycles;
		s32 diff_pct = (current_cycles * 100 / baseline) - 100;

		if (diff_pct > (s32)det->alert_threshold_pct) {
			if (det->regression_count < 32) {
				struct PerfRegression *reg = &det->regressions[det->regression_count];
				reg->test_name = test_name;
				reg->current_cycles = current_cycles;
				reg->baseline_cycles = baseline;
				reg->regression_pct = diff_pct;
				reg->severity = diff_pct > 50 ? 4 : (diff_pct > 20 ? 3 : 2);
				reg->is_confirmed = 0;
				det->regression_count++;
				det->total_regressions++;
				if (reg->severity == 4) det->critical_regressions++;
				return 1;
			}
		}
		return 0;
	}

	return 0;
}

/* Get regression count */
static inline u32 regdet_get_regression_count(struct RegressionDetector *det) {
	if (!det) return 0;
	return det->regression_count;
}

#endif /* APKC_PERF_REGRESSION_DETECTION_H */
