/* prof_metrics_metadata.h — Performance Profiling Metadata (Stage 13.3)
 *
 * Performance metrics collection: instruction counts, timing data.
 * Function hotspot identification: find performance-critical functions.
 * Resource usage tracking: memory, I/O, CPU cycles.
 * Optimization opportunity detection: profile-guided optimization hints.
 * Metrics serialization: embed performance data in module metadata.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_PROF_METRICS_METADATA_H
#define APKC_PROF_METRICS_METADATA_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Profiling status */
enum ProfilingStatus {
	PROF_OK = 0,                /* Metrics collected successfully */
	PROF_NO_DATA = 1,           /* No profiling data available */
	PROF_INVALID_FORMAT = 2,    /* Malformed metrics data */
	PROF_BUFFER_OVERFLOW = 3,   /* Metrics buffer exceeded */
	PROF_INCOMPLETE = 4,        /* Some metrics missing */
	PROF_SAMPLING_ERROR = 5     /* Sampling encountered error */
};

/* Function call metric */
struct FunctionMetrics {
	const char *function_name;  /* Function name */
	u32 call_count;             /* Number of calls */
	u64 total_cycles;           /* Total CPU cycles */
	u64 self_cycles;            /* Self time (excluding callees) */
	u32 min_cycles;             /* Minimum call duration */
	u32 max_cycles;             /* Maximum call duration */
	u32 instruction_count;      /* Number of instructions executed */
	u8 is_hotspot;              /* 1 if on critical path */
};

/* Basic block metric */
struct BlockMetrics {
	u32 block_id;               /* Basic block identifier */
	u32 execution_count;        /* Times executed */
	u64 total_cycles;           /* Total cycles in block */
	u32 instruction_count;      /* Instructions in block */
	u8 is_loop_header;          /* 1 if block is loop header */
};

/* Memory operation metric */
struct MemoryMetric {
	u32 load_count;             /* Number of load operations */
	u32 store_count;            /* Number of store operations */
	u32 cache_hits;             /* L1/L2 cache hits (estimate) */
	u32 cache_misses;           /* L1/L2 cache misses (estimate) */
	u64 memory_bytes_accessed;  /* Total bytes read/written */
	u64 peak_memory_usage;      /* Peak memory allocation */
};

/* Profiling session metadata */
struct ProfilingMetadata {
	const char *module_name;    /* Module being profiled */
	struct FunctionMetrics functions[32]; /* Up to 32 functions */
	u32 function_count;
	struct BlockMetrics blocks[64]; /* Up to 64 basic blocks */
	u32 block_count;
	struct MemoryMetric memory_ops;
	u64 total_cycles_observed;  /* Total cycles for entire module */
	u64 sampling_period;        /* Cycles between samples */
	u32 sample_count;           /* Number of samples collected */
};

/* Optimization hint */
struct OptimizationHint {
	const char *function_name;  /* Function to optimize */
	u8 hint_type;               /* Type of optimization hint */
	u32 potential_speedup;      /* Estimated speedup percentage */
	const char *suggestion;     /* Text description of hint */
};

#define HINT_INLINE 1           /* Function should be inlined */
#define HINT_UNROLL 2           /* Loop should be unrolled */
#define HINT_VECTORIZE 3        /* Operation should be vectorized */
#define HINT_PARALLELIZE 4      /* Operation should be parallelized */
#define HINT_REDUCE_MEMORY 5    /* Reduce memory usage */
#define HINT_CACHE_FRIENDLY 6   /* Improve cache locality */

/* ============================================================ */
/* PROFILING METADATA INITIALIZATION */
/* ============================================================ */

/* Initialize profiling metadata */
static inline void prof_init_metadata(
	struct ProfilingMetadata *prof,
	const char *module_name) {

	if (!prof) return;
	prof->module_name = module_name;
	prof->function_count = 0;
	prof->block_count = 0;
	prof->total_cycles_observed = 0;
	prof->sample_count = 0;
	prof->sampling_period = 1000;  /* Default: sample every 1000 cycles */
}

/* ============================================================ */
/* FUNCTION METRICS TRACKING */
/* ============================================================ */

/* Record function call */
static inline u8 prof_record_function_call(
	struct ProfilingMetadata *prof,
	const char *function_name,
	u64 cycles,
	u32 instruction_count) {

	if (!prof || !function_name) return PROF_INVALID_FORMAT;

	/* Find existing function entry */
	u32 i;
	for (i = 0; i < prof->function_count; i++) {
		if (!prof->functions[i].function_name) continue;

		const char *fname = prof->functions[i].function_name;
		u32 j = 0;
		while (function_name[j] && fname[j] && function_name[j] == fname[j]) j++;

		if (function_name[j] == 0 && fname[j] == 0) {
			/* Found existing entry, update metrics */
			prof->functions[i].call_count++;
			prof->functions[i].total_cycles += cycles;
			if (cycles < prof->functions[i].min_cycles) {
				prof->functions[i].min_cycles = (u32)cycles;
			}
			if (cycles > prof->functions[i].max_cycles) {
				prof->functions[i].max_cycles = (u32)cycles;
			}
			prof->functions[i].instruction_count += instruction_count;
			return PROF_OK;
		}
	}

	/* Create new entry if not found */
	if (prof->function_count >= 32) return PROF_BUFFER_OVERFLOW;

	struct FunctionMetrics *func = &prof->functions[prof->function_count];
	func->function_name = function_name;
	func->call_count = 1;
	func->total_cycles = cycles;
	func->self_cycles = cycles;
	func->min_cycles = (u32)cycles;
	func->max_cycles = (u32)cycles;
	func->instruction_count = instruction_count;
	func->is_hotspot = 0;

	prof->function_count++;
	prof->total_cycles_observed += cycles;
	return PROF_OK;
}

/* ============================================================ */
/* BASIC BLOCK METRICS */
/* ============================================================ */

/* Record basic block execution */
static inline u8 prof_record_block_execution(
	struct ProfilingMetadata *prof,
	u32 block_id,
	u64 cycles,
	u32 instruction_count) {

	if (!prof) return PROF_INVALID_FORMAT;

	/* Find existing block entry */
	u32 i;
	for (i = 0; i < prof->block_count; i++) {
		if (prof->blocks[i].block_id == block_id) {
			/* Found existing entry, update metrics */
			prof->blocks[i].execution_count++;
			prof->blocks[i].total_cycles += cycles;
			return PROF_OK;
		}
	}

	/* Create new entry if not found */
	if (prof->block_count >= 64) return PROF_BUFFER_OVERFLOW;

	struct BlockMetrics *block = &prof->blocks[prof->block_count];
	block->block_id = block_id;
	block->execution_count = 1;
	block->total_cycles = cycles;
	block->instruction_count = instruction_count;
	block->is_loop_header = 0;

	prof->block_count++;
	return PROF_OK;
}

/* ============================================================ */
/* MEMORY OPERATION METRICS */
/* ============================================================ */

/* Record memory access */
static inline void prof_record_memory_access(
	struct ProfilingMetadata *prof,
	u8 is_load,
	u32 bytes,
	u8 is_cache_hit) {

	if (!prof) return;

	if (is_load) {
		prof->memory_ops.load_count++;
	} else {
		prof->memory_ops.store_count++;
	}

	prof->memory_ops.memory_bytes_accessed += bytes;

	if (is_cache_hit) {
		prof->memory_ops.cache_hits++;
	} else {
		prof->memory_ops.cache_misses++;
	}
}

/* ============================================================ */
/* HOTSPOT IDENTIFICATION */
/* ============================================================ */

/* Identify hotspot functions (top N by cycle count) */
static inline void prof_identify_hotspots(
	struct ProfilingMetadata *prof,
	u32 threshold_percent) {

	if (!prof) return;

	/* Clear previous hotspot marks */
	u32 i;
	for (i = 0; i < prof->function_count; i++) {
		prof->functions[i].is_hotspot = 0;
	}

	/* If no cycles observed, nothing is a hotspot */
	if (prof->total_cycles_observed == 0) return;

	/* Mark functions consuming > threshold% of total cycles */
	for (i = 0; i < prof->function_count; i++) {
		u32 percentage = (prof->functions[i].total_cycles * 100) / prof->total_cycles_observed;
		if (percentage >= threshold_percent) {
			prof->functions[i].is_hotspot = 1;
		}
	}
}

/* ============================================================ */
/* STATISTICS & REPORTING */
/* ============================================================ */

/* Get average function call duration */
static inline u32 prof_average_function_duration(struct ProfilingMetadata *prof) {
	if (!prof || prof->function_count == 0) return 0;

	u64 total = 0;
	u32 total_calls = 0;
	u32 i;
	for (i = 0; i < prof->function_count; i++) {
		total += prof->functions[i].total_cycles;
		total_calls += prof->functions[i].call_count;
	}

	if (total_calls == 0) return 0;
	return (u32)(total / total_calls);
}

/* Get average block execution */
static inline u32 prof_average_block_duration(struct ProfilingMetadata *prof) {
	if (!prof || prof->block_count == 0) return 0;

	u64 total = 0;
	u32 total_execs = 0;
	u32 i;
	for (i = 0; i < prof->block_count; i++) {
		total += prof->blocks[i].total_cycles;
		total_execs += prof->blocks[i].execution_count;
	}

	if (total_execs == 0) return 0;
	return (u32)(total / total_execs);
}

/* Get cache hit ratio */
static inline u32 prof_cache_hit_ratio(struct ProfilingMetadata *prof) {
	if (!prof) return 0;

	u32 total_accesses = prof->memory_ops.load_count + prof->memory_ops.store_count;
	if (total_accesses == 0) return 0;

	return (prof->memory_ops.cache_hits * 100) / total_accesses;
}

/* Count hotspot functions */
static inline u32 prof_count_hotspots(struct ProfilingMetadata *prof) {
	if (!prof) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < prof->function_count; i++) {
		if (prof->functions[i].is_hotspot) count++;
	}
	return count;
}

/* ============================================================ */
/* OPTIMIZATION HINTS */
/* ============================================================ */

/* Generate optimization hints based on profile */
static inline void prof_generate_hints(
	struct ProfilingMetadata *prof,
	struct OptimizationHint *hints,
	u32 *hint_count) {

	if (!prof || !hints || !hint_count) return;

	u32 count = 0;

	/* Hint 1: Inline frequently-called small functions */
	u32 i;
	for (i = 0; i < prof->function_count && count < 16; i++) {
		if (prof->functions[i].call_count > 100 &&
			prof->functions[i].instruction_count < 50) {

			struct OptimizationHint *hint = &hints[count];
			hint->function_name = prof->functions[i].function_name;
			hint->hint_type = HINT_INLINE;
			hint->potential_speedup = 5 + (prof->functions[i].call_count / 50);
			hint->suggestion = "Frequently called; small body suitable for inlining";
			count++;
		}
	}

	/* Hint 2: Unroll hot loops */
	for (i = 0; i < prof->block_count && count < 16; i++) {
		if (prof->blocks[i].is_loop_header &&
			prof->blocks[i].execution_count > 1000) {

			struct OptimizationHint *hint = &hints[count];
			hint->hint_type = HINT_UNROLL;
			hint->potential_speedup = 10;
			hint->suggestion = "High-iteration loop; unrolling could improve performance";
			count++;
		}
	}

	/* Hint 3: Improve cache locality for high memory usage */
	if (prof->memory_ops.cache_hits + prof->memory_ops.cache_misses > 0) {
		u32 hit_ratio = prof_cache_hit_ratio(prof);
		if (hit_ratio < 80 && count < 16) {
			struct OptimizationHint *hint = &hints[count];
			hint->hint_type = HINT_CACHE_FRIENDLY;
			hint->potential_speedup = 15;
			hint->suggestion = "Low cache hit ratio; reorder memory access patterns";
			count++;
		}
	}

	*hint_count = count;
}

#endif /* APKC_PROF_METRICS_METADATA_H */
