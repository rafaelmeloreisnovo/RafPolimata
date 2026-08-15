/* test_phase20_performance.c — Performance Optimization & Benchmarking Tests
 *
 * Tests for Phase 20 modules:
 * - perf_monitoring.h (cycle counting, latency measurement, hotspot detection)
 * - perf_benchmark.h (benchmark suite, result collection, scaling analysis)
 * - perf_optimization_hints.h (optimization recommendations, impact prediction)
 * - perf_regression_detection.h (baseline establishment, regression detection)
 */

#include <stdio.h>
#include <string.h>

#define ASSERT(cond, msg) do { \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s\n", msg); \
		return 0; \
	} \
} while (0)

#define PASS() fprintf(stderr, "PASS\n")

/* Forward declare structures */
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed int s32;

/* ============================================================ */
/* Stage 20.1: Performance Monitoring Tests */
/* ============================================================ */

#include "../Apkc/perf_monitoring.h"

static int test_perfmon_init(void) {
	fprintf(stderr, "test_perfmon_init... ");
	struct PerfMonitor mon = {0};
	perfmon_init(&mon);
	ASSERT(mon.sample_count == 0, "sample_count should be 0");
	ASSERT(mon.metric_count == 0, "metric_count should be 0");
	PASS();
	return 1;
}

static int test_perfmon_record_sample(void) {
	fprintf(stderr, "test_perfmon_record_sample... ");
	struct PerfMonitor mon = {0};
	perfmon_init(&mon);

	u8 result = perfmon_record_sample(&mon, 0, 1000, 500000);
	ASSERT(result == 1, "record_sample should succeed");
	ASSERT(mon.sample_count == 1, "sample_count should be 1");
	PASS();
	return 1;
}

static int test_perfmon_record_metric(void) {
	fprintf(stderr, "test_perfmon_record_metric... ");
	struct PerfMonitor mon = {0};
	perfmon_init(&mon);

	u8 result = perfmon_record_metric(&mon, "test_op", 5000, 100);
	ASSERT(result == 1, "record_metric should succeed");
	ASSERT(mon.metric_count == 1, "metric_count should be 1");
	PASS();
	return 1;
}

static int test_perfmon_buffer_limit(void) {
	fprintf(stderr, "test_perfmon_buffer_limit... ");
	struct PerfMonitor mon = {0};
	perfmon_init(&mon);

	/* Fill to capacity */
	for (u32 i = 0; i < 256; i++) {
		perfmon_record_sample(&mon, 0, 1000, 500000);
	}
	ASSERT(mon.sample_count == 256, "sample_count should be 256");

	/* Try to exceed capacity */
	u8 result = perfmon_record_sample(&mon, 0, 1000, 500000);
	ASSERT(result == 0, "record_sample should fail when full");
	PASS();
	return 1;
}

static int test_perfmon_find_slowest(void) {
	fprintf(stderr, "test_perfmon_find_slowest... ");
	struct PerfMonitor mon = {0};
	perfmon_init(&mon);

	perfmon_record_metric(&mon, "fast", 1000, 100);
	perfmon_record_metric(&mon, "slow", 5000, 100);

	struct PerfMetric *slowest = perfmon_find_slowest(&mon);
	ASSERT(slowest != NULL, "slowest should not be NULL");
	ASSERT(slowest->total_cycles == 5000, "slowest should have 5000 cycles");
	PASS();
	return 1;
}

static int test_perfmon_get_avg_cycles(void) {
	fprintf(stderr, "test_perfmon_get_avg_cycles... ");
	struct PerfMonitor mon = {0};
	perfmon_init(&mon);

	perfmon_record_sample(&mon, 1000, 500000, 10);
	u64 avg = perfmon_get_avg_cycles(&mon);
	ASSERT(avg == 100, "average cycles should be 100");
	PASS();
	return 1;
}

/* ============================================================ */
/* Stage 20.2: Benchmark Suite Tests */
/* ============================================================ */

#include "../Apkc/perf_benchmark.h"

static int test_bench_init(void) {
	fprintf(stderr, "test_bench_init... ");
	struct BenchmarkSuite suite = {0};
	bench_init(&suite);
	ASSERT(suite.result_count == 0, "result_count should be 0");
	ASSERT(suite.passed_count == 0, "passed_count should be 0");
	ASSERT(suite.failed_count == 0, "failed_count should be 0");
	PASS();
	return 1;
}

static int test_bench_record_result(void) {
	fprintf(stderr, "test_bench_record_result... ");
	struct BenchmarkSuite suite = {0};
	bench_init(&suite);

	u8 result = bench_record_result(&suite, "benchmark1", 1000, 100, 0);
	ASSERT(result == 1, "record_result should succeed");
	ASSERT(suite.result_count == 1, "result_count should be 1");
	PASS();
	return 1;
}

static int test_bench_result_status_tracking(void) {
	fprintf(stderr, "test_bench_result_status_tracking... ");
	struct BenchmarkSuite suite = {0};
	bench_init(&suite);

	bench_record_result(&suite, "pass_bench", 1000, 100, 0);  /* status=0 pass */
	bench_record_result(&suite, "fail_bench", 1000, 100, 1);  /* status=1 fail */

	ASSERT(suite.result_count == 2, "result_count should be 2");
	ASSERT(suite.passed_count == 1, "passed_count should be 1");
	ASSERT(suite.failed_count == 1, "failed_count should be 1");
	PASS();
	return 1;
}

static int test_bench_buffer_limit(void) {
	fprintf(stderr, "test_bench_buffer_limit... ");
	struct BenchmarkSuite suite = {0};
	bench_init(&suite);

	/* Fill to capacity */
	for (u32 i = 0; i < 64; i++) {
		bench_record_result(&suite, "bench", 1000, 100, 0);
	}
	ASSERT(suite.result_count == 64, "result_count should be 64");

	/* Try to exceed capacity */
	u8 result = bench_record_result(&suite, "bench", 1000, 100, 0);
	ASSERT(result == 0, "record_result should fail when full");
	PASS();
	return 1;
}

static int test_bench_throughput_calculation(void) {
	fprintf(stderr, "test_bench_throughput_calculation... ");
	struct BenchmarkSuite suite = {0};
	bench_init(&suite);

	bench_record_result(&suite, "throughput_bench", 10000, 100, 0);
	ASSERT(suite.result_count == 1, "result_count should be 1");

	struct BenchmarkResult *res = &suite.results[0];
	u32 throughput = res->throughput_ops_per_sec;
	ASSERT(throughput > 0, "throughput should be positive");
	PASS();
	return 1;
}

/* ============================================================ */
/* Stage 20.3: Optimization Engine Tests */
/* ============================================================ */

#include "../Apkc/perf_optimization_hints.h"

static int test_optengine_init(void) {
	fprintf(stderr, "test_optengine_init... ");
	struct OptimizationEngine engine = {0};
	optengine_init(&engine);
	ASSERT(engine.hint_count == 0, "hint_count should be 0");
	ASSERT(engine.total_speedup_potential == 0, "total_speedup_potential should be 0");
	ASSERT(engine.critical_hints == 0, "critical_hints should be 0");
	PASS();
	return 1;
}

static int test_optengine_add_hint(void) {
	fprintf(stderr, "test_optengine_add_hint... ");
	struct OptimizationEngine engine = {0};
	optengine_init(&engine);

	u8 result = optengine_add_hint(&engine, "test_func", "inline", "Consider inlining", 25, 2);
	ASSERT(result == 1, "add_hint should succeed");
	ASSERT(engine.hint_count == 1, "hint_count should be 1");
	PASS();
	return 1;
}

static int test_optengine_hint_priority(void) {
	fprintf(stderr, "test_optengine_hint_priority... ");
	struct OptimizationEngine engine = {0};
	optengine_init(&engine);

	optengine_add_hint(&engine, "func1", "inline", "desc1", 10, 1);  /* low priority */
	optengine_add_hint(&engine, "func2", "loop-unroll", "desc2", 50, 4);  /* critical */

	ASSERT(engine.hint_count == 2, "hint_count should be 2");
	ASSERT(engine.critical_hints == 1, "critical_hints should be 1");
	PASS();
	return 1;
}

static int test_optengine_speedup_accumulation(void) {
	fprintf(stderr, "test_optengine_speedup_accumulation... ");
	struct OptimizationEngine engine = {0};
	optengine_init(&engine);

	optengine_add_hint(&engine, "func1", "inline", "desc1", 20, 2);
	optengine_add_hint(&engine, "func2", "vectorize", "desc2", 30, 3);

	u32 total_speedup = optengine_get_total_speedup(&engine);
	ASSERT(total_speedup == 50, "total_speedup should be 50");
	PASS();
	return 1;
}

static int test_optengine_buffer_limit(void) {
	fprintf(stderr, "test_optengine_buffer_limit... ");
	struct OptimizationEngine engine = {0};
	optengine_init(&engine);

	/* Fill to capacity */
	for (u32 i = 0; i < 64; i++) {
		optengine_add_hint(&engine, "func", "inline", "desc", 10, 2);
	}
	ASSERT(engine.hint_count == 64, "hint_count should be 64");

	/* Try to exceed capacity */
	u8 result = optengine_add_hint(&engine, "func", "inline", "desc", 10, 2);
	ASSERT(result == 0, "add_hint should fail when full");
	PASS();
	return 1;
}

/* ============================================================ */
/* Stage 20.4: Regression Detection Tests */
/* ============================================================ */

#include "../Apkc/perf_regression_detection.h"

static int test_regdet_init(void) {
	fprintf(stderr, "test_regdet_init... ");
	struct RegressionDetector det = {0};
	regdet_init(&det);
	ASSERT(det.baseline_count == 0, "baseline_count should be 0");
	ASSERT(det.regression_count == 0, "regression_count should be 0");
	ASSERT(det.alert_threshold_pct == 10, "alert_threshold_pct should be 10");
	PASS();
	return 1;
}

static int test_regdet_set_baseline(void) {
	fprintf(stderr, "test_regdet_set_baseline... ");
	struct RegressionDetector det = {0};
	regdet_init(&det);

	u8 result = regdet_set_baseline(&det, "test1", 1000, 100);
	ASSERT(result == 1, "set_baseline should succeed");
	ASSERT(det.baseline_count == 1, "baseline_count should be 1");
	PASS();
	return 1;
}

static int test_regdet_check_regression_no_regression(void) {
	fprintf(stderr, "test_regdet_check_regression_no_regression... ");
	struct RegressionDetector det = {0};
	regdet_init(&det);

	regdet_set_baseline(&det, "test1", 1000, 100);

	/* Current performance within threshold */
	u8 is_regressed = regdet_check_regression(&det, "test1", 1050);
	ASSERT(is_regressed == 0, "should not detect regression for 5% slower");
	PASS();
	return 1;
}

static int test_regdet_check_regression_detected(void) {
	fprintf(stderr, "test_regdet_check_regression_detected... ");
	struct RegressionDetector det = {0};
	regdet_init(&det);

	regdet_set_baseline(&det, "test1", 1000, 100);

	/* Current performance exceeds threshold */
	u8 is_regressed = regdet_check_regression(&det, "test1", 1200);
	ASSERT(is_regressed == 1, "should detect regression for 20% slower");
	ASSERT(det.regression_count == 1, "regression_count should be 1");
	PASS();
	return 1;
}

static int test_regdet_regression_severity(void) {
	fprintf(stderr, "test_regdet_regression_severity... ");
	struct RegressionDetector det = {0};
	regdet_init(&det);

	regdet_set_baseline(&det, "test1", 1000, 100);
	regdet_check_regression(&det, "test1", 1150);  /* 15% slower */

	struct PerfRegression *reg = &det.regressions[0];
	ASSERT(reg->severity > 0, "regression should have severity > 0");
	PASS();
	return 1;
}

static int test_regdet_multiple_baselines(void) {
	fprintf(stderr, "test_regdet_multiple_baselines... ");
	struct RegressionDetector det = {0};
	regdet_init(&det);

	regdet_set_baseline(&det, "test1", 1000, 100);
	regdet_set_baseline(&det, "test2", 2000, 200);
	regdet_set_baseline(&det, "test3", 500, 50);

	ASSERT(det.baseline_count == 3, "baseline_count should be 3");
	PASS();
	return 1;
}

static int test_regdet_buffer_limit(void) {
	fprintf(stderr, "test_regdet_buffer_limit... ");
	struct RegressionDetector det = {0};
	regdet_init(&det);

	/* Fill baselines to capacity */
	for (u32 i = 0; i < 64; i++) {
		char name[32];
		snprintf(name, sizeof(name), "test_%u", i);
		regdet_set_baseline(&det, name, 1000, 100);
	}
	ASSERT(det.baseline_count == 64, "baseline_count should be 64");

	/* Try to exceed capacity */
	u8 result = regdet_set_baseline(&det, "extra", 1000, 100);
	ASSERT(result == 0, "set_baseline should fail when full");
	PASS();
	return 1;
}

/* ============================================================ */
/* Integration Tests */
/* ============================================================ */

static int test_performance_monitoring_workflow(void) {
	fprintf(stderr, "test_performance_monitoring_workflow... ");
	struct PerfMonitor mon = {0};
	perfmon_init(&mon);

	/* Record multiple samples */
	perfmon_record_sample(&mon, 1000, 500000, 100);
	perfmon_record_sample(&mon, 2000, 1000000, 200);
	perfmon_record_sample(&mon, 1500, 750000, 150);

	ASSERT(mon.sample_count == 3, "should have 3 samples");
	ASSERT(mon.total_cycles_monitored == 4500, "total_cycles_monitored should be 4500");
	PASS();
	return 1;
}

static int test_optimization_and_regression_integration(void) {
	fprintf(stderr, "test_optimization_and_regression_integration... ");
	struct OptimizationEngine engine = {0};
	struct RegressionDetector det = {0};

	optengine_init(&engine);
	regdet_init(&det);

	/* Add optimization hints */
	optengine_add_hint(&engine, "hot_func", "inline", "inline candidate", 40, 3);
	u32 potential_speedup = optengine_get_total_speedup(&engine);

	/* Establish baseline before optimization */
	regdet_set_baseline(&det, "hot_func", 1000, 100);

	/* After "optimization" (simulated), check if regression detected */
	u8 regressed = regdet_check_regression(&det, "hot_func", 900);  /* 10% faster */
	ASSERT(regressed == 0, "improvement should not be flagged as regression");
	ASSERT(potential_speedup == 40, "speedup potential should be 40");
	PASS();
	return 1;
}

static int test_full_benchmark_and_regression_cycle(void) {
	fprintf(stderr, "test_full_benchmark_and_regression_cycle... ");
	struct BenchmarkSuite suite = {0};
	struct RegressionDetector det = {0};

	bench_init(&suite);
	regdet_init(&det);

	/* Run benchmarks */
	bench_record_result(&suite, "algo1", 5000, 100, 0);
	bench_record_result(&suite, "algo2", 10000, 200, 0);

	/* Establish baselines from benchmark results */
	regdet_set_baseline(&det, "algo1", 5000, 100);
	regdet_set_baseline(&det, "algo2", 10000, 200);

	/* Check for regressions */
	u8 reg1 = regdet_check_regression(&det, "algo1", 5600);   /* 12% slower (exceeds 10% threshold) */
	u8 reg2 = regdet_check_regression(&det, "algo2", 10000);  /* no change */

	ASSERT(reg1 == 1, "algo1 should be flagged as regressed");
	ASSERT(reg2 == 0, "algo2 should not be flagged");
	ASSERT(suite.passed_count == 2, "both benchmarks should pass");
	PASS();
	return 1;
}

/* ============================================================ */
/* Main Test Driver */
/* ============================================================ */

int main(void) {
	fprintf(stderr, "\n=== Phase 20: Performance Optimization & Benchmarking Tests ===\n\n");

	int pass_count = 0;
	int test_count = 0;

	/* Stage 20.1: Performance Monitoring */
	fprintf(stderr, "--- Stage 20.1: Performance Monitoring ---\n");
	test_count++; pass_count += test_perfmon_init();
	test_count++; pass_count += test_perfmon_record_sample();
	test_count++; pass_count += test_perfmon_record_metric();
	test_count++; pass_count += test_perfmon_buffer_limit();
	test_count++; pass_count += test_perfmon_find_slowest();
	test_count++; pass_count += test_perfmon_get_avg_cycles();

	/* Stage 20.2: Benchmark Suite */
	fprintf(stderr, "\n--- Stage 20.2: Benchmark Suite ---\n");
	test_count++; pass_count += test_bench_init();
	test_count++; pass_count += test_bench_record_result();
	test_count++; pass_count += test_bench_result_status_tracking();
	test_count++; pass_count += test_bench_buffer_limit();
	test_count++; pass_count += test_bench_throughput_calculation();

	/* Stage 20.3: Optimization Engine */
	fprintf(stderr, "\n--- Stage 20.3: Optimization Engine ---\n");
	test_count++; pass_count += test_optengine_init();
	test_count++; pass_count += test_optengine_add_hint();
	test_count++; pass_count += test_optengine_hint_priority();
	test_count++; pass_count += test_optengine_speedup_accumulation();
	test_count++; pass_count += test_optengine_buffer_limit();

	/* Stage 20.4: Regression Detection */
	fprintf(stderr, "\n--- Stage 20.4: Regression Detection ---\n");
	test_count++; pass_count += test_regdet_init();
	test_count++; pass_count += test_regdet_set_baseline();
	test_count++; pass_count += test_regdet_check_regression_no_regression();
	test_count++; pass_count += test_regdet_check_regression_detected();
	test_count++; pass_count += test_regdet_regression_severity();
	test_count++; pass_count += test_regdet_multiple_baselines();
	test_count++; pass_count += test_regdet_buffer_limit();

	/* Integration Tests */
	fprintf(stderr, "\n--- Integration Tests ---\n");
	test_count++; pass_count += test_performance_monitoring_workflow();
	test_count++; pass_count += test_optimization_and_regression_integration();
	test_count++; pass_count += test_full_benchmark_and_regression_cycle();

	fprintf(stderr, "\n=== Summary ===\n");
	fprintf(stderr, "Passed: %d/%d tests\n", pass_count, test_count);

	return (pass_count == test_count) ? 0 : 1;
}
