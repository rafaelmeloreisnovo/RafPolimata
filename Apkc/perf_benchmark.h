/* perf_benchmark.h — Benchmark Suite & Harness (Stage 20.2)
 *
 * Benchmark definition: standard test workloads.
 * Harness framework: run benchmarks consistently.
 * Result collection: gather and aggregate benchmark results.
 * Comparison: compare performance across versions.
 * Scaling analysis: measure performance with different input sizes.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_PERF_BENCHMARK_H
#define APKC_PERF_BENCHMARK_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Benchmark result */
struct BenchmarkResult {
	const char *benchmark_name;
	u64 elapsed_cycles;
	u64 elapsed_ns;
	u32 iterations;
	u32 throughput_ops_per_sec;
	u8 status;  /* 0=pass, 1=fail, 2=timeout */
};

/* Benchmark suite */
struct BenchmarkSuite {
	struct BenchmarkResult results[64];
	u32 result_count;
	u64 total_cycles;
	u32 passed_count;
	u32 failed_count;
};

/* Initialize benchmark suite */
static inline void bench_init(struct BenchmarkSuite *suite) {
	if (!suite) return;
	suite->result_count = 0;
	suite->total_cycles = 0;
	suite->passed_count = 0;
	suite->failed_count = 0;
}

/* Record benchmark result */
static inline u8 bench_record_result(
	struct BenchmarkSuite *suite,
	const char *name,
	u64 cycles,
	u32 iterations,
	u8 status) {

	if (!suite || !name || suite->result_count >= 64) return 0;

	struct BenchmarkResult *res = &suite->results[suite->result_count];
	res->benchmark_name = name;
	res->elapsed_cycles = cycles;
	res->iterations = iterations;
	res->throughput_ops_per_sec = iterations > 0 ? (iterations * 1000000000) / (cycles / 1000) : 0;
	res->status = status;

	suite->result_count++;
	suite->total_cycles += cycles;
	if (status == 0) suite->passed_count++;
	else suite->failed_count++;
	return 1;
}

/* Get average throughput */
static inline u32 bench_get_avg_throughput(struct BenchmarkSuite *suite) {
	if (!suite || suite->result_count == 0) return 0;

	u64 total_throughput = 0;
	u32 i;
	for (i = 0; i < suite->result_count; i++) {
		total_throughput += suite->results[i].throughput_ops_per_sec;
	}
	return (u32)(total_throughput / suite->result_count);
}

#endif /* APKC_PERF_BENCHMARK_H */
