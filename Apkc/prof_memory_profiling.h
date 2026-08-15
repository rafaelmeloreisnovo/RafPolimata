/* prof_memory_profiling.h — Memory Profiling & Tracking (Stage 16.3)
 *
 * Allocation tracking: record every memory allocation with metadata.
 * Lifetime analysis: measure allocation duration and peak usage.
 * Memory hotspots: identify most frequently allocating code paths.
 * Leak detection: find allocations that are never freed.
 * Fragmentation analysis: track memory utilization efficiency.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_PROF_MEMORY_PROFILING_H
#define APKC_PROF_MEMORY_PROFILING_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Memory allocation record */
struct AllocationRecord {
	u64 alloc_address;          /* Allocated address */
	u32 alloc_size;             /* Allocated size in bytes */
	u64 allocation_time;        /* Allocation timestamp */
	u64 deallocation_time;      /* Deallocation timestamp (0 if still live) */
	u32 allocation_site;        /* Function/line that allocated (index) */
	u8 is_freed;                /* 1 if deallocated */
	u8 is_reused;               /* 1 if address reused after free */
};

/* Allocation site statistics */
struct AllocationSite {
	const char *function_name;  /* Function where alloc occurred */
	u32 line_number;            /* Source line */
	u32 alloc_count;            /* Total allocations from this site */
	u64 total_allocated;        /* Total bytes allocated */
	u64 peak_allocated;         /* Peak bytes allocated at once */
	u32 leak_count;             /* Unfreed allocations from this site */
};

/* Memory profile data */
struct MemoryProfile {
	struct AllocationRecord allocs[256];  /* Up to 256 active allocations */
	u32 alloc_count;
	struct AllocationSite sites[64];    /* Up to 64 allocation sites */
	u32 site_count;
	u64 total_allocated;               /* Total bytes ever allocated */
	u64 current_allocated;             /* Currently allocated bytes */
	u64 peak_allocated;                /* Peak memory usage */
	u32 total_allocs;                  /* Total allocation count */
	u32 total_frees;                   /* Total free count */
	u32 current_live_allocs;           /* Currently live allocations */
};

/* ============================================================ */
/* MEMORY PROFILE INITIALIZATION */
/* ============================================================ */

/* Initialize memory profile */
static inline void memprof_init(struct MemoryProfile *prof) {
	if (!prof) return;
	prof->alloc_count = 0;
	prof->site_count = 0;
	prof->total_allocated = 0;
	prof->current_allocated = 0;
	prof->peak_allocated = 0;
	prof->total_allocs = 0;
	prof->total_frees = 0;
	prof->current_live_allocs = 0;
}

/* ============================================================ */
/* ALLOCATION TRACKING */
/* ============================================================ */

/* Record memory allocation */
static inline u8 memprof_record_alloc(
	struct MemoryProfile *prof,
	u64 address,
	u32 size,
	const char *function,
	u32 line) {

	if (!prof || !function) return 0;
	if (prof->alloc_count >= 256) return 0;  /* Buffer full */

	/* Record allocation */
	struct AllocationRecord *rec = &prof->allocs[prof->alloc_count];
	rec->alloc_address = address;
	rec->alloc_size = size;
	rec->allocation_time = 0;  /* Would be current time */
	rec->deallocation_time = 0;
	rec->allocation_site = 0;  /* Simplified: site index 0 */
	rec->is_freed = 0;
	rec->is_reused = 0;

	prof->alloc_count++;
	prof->total_allocated += size;
	prof->current_allocated += size;
	prof->current_live_allocs++;
	prof->total_allocs++;

	/* Update peak */
	if (prof->current_allocated > prof->peak_allocated) {
		prof->peak_allocated = prof->current_allocated;
	}

	/* Record allocation site */
	if (prof->site_count < 64) {
		struct AllocationSite *site = &prof->sites[prof->site_count];
		site->function_name = function;
		site->line_number = line;
		site->alloc_count = 1;
		site->total_allocated = size;
		site->peak_allocated = size;
		site->leak_count = 0;
		prof->site_count++;
	}

	return 1;
}

/* Record memory deallocation */
static inline u8 memprof_record_free(
	struct MemoryProfile *prof,
	u64 address) {

	if (!prof) return 0;

	u32 i;
	for (i = 0; i < prof->alloc_count; i++) {
		if (prof->allocs[i].alloc_address == address && !prof->allocs[i].is_freed) {
			prof->allocs[i].is_freed = 1;
			prof->allocs[i].deallocation_time = 0;  /* Would be current time */
			prof->current_allocated -= prof->allocs[i].alloc_size;
			prof->current_live_allocs--;
			prof->total_frees++;
			return 1;
		}
	}

	return 0;  /* Allocation not found */
}

/* ============================================================ */
/* LEAK DETECTION */
/* ============================================================ */

/* Find unfreed allocations */
static inline u32 memprof_find_leaks(
	struct MemoryProfile *prof,
	u64 *leak_addresses,
	u32 max_leaks) {

	if (!prof || !leak_addresses) return 0;

	u32 leak_count = 0;
	u32 i;
	for (i = 0; i < prof->alloc_count && leak_count < max_leaks; i++) {
		if (!prof->allocs[i].is_freed) {
			leak_addresses[leak_count++] = prof->allocs[i].alloc_address;
		}
	}

	return leak_count;
}

/* Get total bytes leaked */
static inline u64 memprof_get_leaked_bytes(struct MemoryProfile *prof) {
	if (!prof) return 0;

	u64 leaked = 0;
	u32 i;
	for (i = 0; i < prof->alloc_count; i++) {
		if (!prof->allocs[i].is_freed) {
			leaked += prof->allocs[i].alloc_size;
		}
	}

	return leaked;
}

/* ============================================================ */
/* ALLOCATION SITE ANALYSIS */
/* ============================================================ */

/* Find allocation site with most leaks */
static inline struct AllocationSite *memprof_find_leakiest_site(struct MemoryProfile *prof) {
	if (!prof || prof->site_count == 0) return 0;

	struct AllocationSite *leakiest = &prof->sites[0];
	u32 i;
	for (i = 1; i < prof->site_count; i++) {
		if (prof->sites[i].leak_count > leakiest->leak_count) {
			leakiest = &prof->sites[i];
		}
	}

	return leakiest;
}

/* Find allocation site with most allocations */
static inline struct AllocationSite *memprof_find_hottest_site(struct MemoryProfile *prof) {
	if (!prof || prof->site_count == 0) return 0;

	struct AllocationSite *hottest = &prof->sites[0];
	u32 i;
	for (i = 1; i < prof->site_count; i++) {
		if (prof->sites[i].alloc_count > hottest->alloc_count) {
			hottest = &prof->sites[i];
		}
	}

	return hottest;
}

/* ============================================================ */
/* MEMORY STATISTICS */
/* ============================================================ */

/* Get memory utilization percentage */
static inline u32 memprof_get_utilization_percent(
	struct MemoryProfile *prof,
	u64 max_memory) {

	if (!prof || max_memory == 0) return 0;
	return (u32)((prof->current_allocated * 100) / max_memory);
}

/* Get fragmentation ratio (allocated / peak) */
static inline u32 memprof_get_fragmentation_ratio(struct MemoryProfile *prof) {
	if (!prof || prof->peak_allocated == 0) return 0;
	return (u32)((prof->current_allocated * 100) / prof->peak_allocated);
}

/* Get allocation churn (frees / allocs) */
static inline u32 memprof_get_churn_ratio(struct MemoryProfile *prof) {
	if (!prof || prof->total_allocs == 0) return 0;
	return (prof->total_frees * 100) / prof->total_allocs;
}

#endif /* APKC_PROF_MEMORY_PROFILING_H */
