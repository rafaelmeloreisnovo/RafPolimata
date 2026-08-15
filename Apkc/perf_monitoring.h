/* perf_monitoring.h — Performance Monitoring & Timing (Stage 20.1)
 *
 * Cycle counting: measure instruction execution counts.
 * Latency measurement: track time for individual operations.
 * Throughput tracking: measure operations per second.
 * Memory bandwidth: monitor memory access patterns.
 * Hotspot detection: identify performance bottlenecks.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_PERF_MONITORING_H
#define APKC_PERF_MONITORING_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Performance sample */
struct PerfSample {
	u64 timestamp;              /* When measurement taken */
	u64 cycles;                 /* CPU cycles elapsed */
	u64 latency_ns;             /* Operation latency in nanoseconds */
	u32 operations;             /* Operations completed */
	u32 bytes_read;             /* Memory read in bytes */
	u32 bytes_written;          /* Memory written in bytes */
	u32 cache_hits;             /* Cache hits */
	u32 cache_misses;           /* Cache misses */
};

/* Performance metric */
struct PerfMetric {
	const char *operation_name; /* Operation being measured */
	u64 total_cycles;           /* Total cycles spent */
	u64 total_ns;               /* Total time in nanoseconds */
	u32 total_operations;       /* Total operations */
	u32 min_cycles;             /* Minimum cycle count */
	u32 max_cycles;             /* Maximum cycle count */
	u32 avg_cycles;             /* Average cycle count */
	u32 sample_count;           /* Number of samples */
	u8 is_hotspot;              /* 1 if performance bottleneck */
};

/* Performance monitor */
struct PerfMonitor {
	struct PerfSample samples[256];   /* Up to 256 samples */
	u32 sample_count;
	struct PerfMetric metrics[32];    /* Up to 32 tracked metrics */
	u32 metric_count;
	u64 total_cycles_monitored;
	u32 total_operations_monitored;
	u32 hotspot_count;
};

/* Initialize performance monitor */
static inline void perfmon_init(struct PerfMonitor *mon) {
	if (!mon) return;
	mon->sample_count = 0;
	mon->metric_count = 0;
	mon->total_cycles_monitored = 0;
	mon->total_operations_monitored = 0;
	mon->hotspot_count = 0;
}

/* Record performance sample */
static inline u8 perfmon_record_sample(
	struct PerfMonitor *mon,
	u64 cycles,
	u64 latency,
	u32 operations) {

	if (!mon || mon->sample_count >= 256) return 0;

	struct PerfSample *sample = &mon->samples[mon->sample_count];
	sample->timestamp = 0;  /* Would be current time */
	sample->cycles = cycles;
	sample->latency_ns = latency;
	sample->operations = operations;
	sample->bytes_read = 0;
	sample->bytes_written = 0;

	mon->sample_count++;
	mon->total_cycles_monitored += cycles;
	mon->total_operations_monitored += operations;
	return 1;
}

/* Record metric */
static inline u8 perfmon_record_metric(
	struct PerfMonitor *mon,
	const char *op_name,
	u64 cycles,
	u32 operations) {

	if (!mon || !op_name || mon->metric_count >= 32) return 0;

	struct PerfMetric *metric = &mon->metrics[mon->metric_count];
	metric->operation_name = op_name;
	metric->total_cycles = cycles;
	metric->total_operations = operations;
	metric->avg_cycles = operations > 0 ? cycles / operations : 0;
	metric->sample_count = 1;

	mon->metric_count++;
	return 1;
}

/* Get average cycles per operation */
static inline u64 perfmon_get_avg_cycles(struct PerfMonitor *mon) {
	if (!mon || mon->total_operations_monitored == 0) return 0;
	return mon->total_cycles_monitored / mon->total_operations_monitored;
}

/* Find slowest operation */
static inline struct PerfMetric *perfmon_find_slowest(struct PerfMonitor *mon) {
	if (!mon || mon->metric_count == 0) return 0;

	struct PerfMetric *slowest = &mon->metrics[0];
	u32 i;
	for (i = 1; i < mon->metric_count; i++) {
		if (mon->metrics[i].avg_cycles > slowest->avg_cycles) {
			slowest = &mon->metrics[i];
		}
	}
	slowest->is_hotspot = 1;
	mon->hotspot_count++;
	return slowest;
}

#endif /* APKC_PERF_MONITORING_H */
