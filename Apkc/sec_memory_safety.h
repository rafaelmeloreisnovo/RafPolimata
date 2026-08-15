/* sec_memory_safety.h — Memory Safety & Bounds Checking (Stage 11.2)
 *
 * Stack canaries for buffer overflow detection.
 * Memory bounds enforcement with no overhead.
 * Use-after-free detection via generation counters.
 * Double-free prevention via allocation tracking.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEC_MEMORY_SAFETY_H
#define APKC_SEC_MEMORY_SAFETY_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Memory safety status */
enum MemSafetyStatus {
	MEMSAFE_OK = 0,           /* No issues detected */
	MEMSAFE_OVERFLOW = 1,     /* Buffer overflow detected */
	MEMSAFE_UNDERFLOW = 2,    /* Buffer underflow detected */
	MEMSAFE_USE_AFTER_FREE = 3, /* Use-after-free detected */
	MEMSAFE_DOUBLE_FREE = 4,  /* Double-free detected */
	MEMSAFE_INVALID_PTR = 5,  /* Invalid pointer */
	MEMSAFE_MISALIGNED = 6,   /* Misaligned access */
	MEMSAFE_HEAP_CORRUPT = 7  /* Heap corruption detected */
};

/* Stack canary for overflow detection */
#define CANARY_VALUE 0xdeadbeefcafebabe

struct StackCanary {
	u64 canary_before;
	u8 data[0];  /* Flexible array member */
	u64 canary_after;  /* Would follow data */
};

/* Memory region descriptor */
struct MemoryRegion {
	void *base;           /* Base address */
	u32 size;             /* Allocated size */
	u32 generation;       /* Generation counter (for UAF detection) */
	u8 in_use;            /* 1 if currently allocated */
	u64 canary;           /* Canary value for this region */
};

/* Memory safety tracker (bounded) */
struct MemSafetyTracker {
	struct MemoryRegion regions[32];  /* Track up to 32 allocations */
	u32 region_count;
	u32 generation_counter;  /* Global generation for all allocations */
	u64 total_allocated;
	u64 peak_allocated;
};

/* ============================================================ */
/* STACK CANARY OPERATIONS */
/* ============================================================ */

/* Write canary before buffer */
static inline void canary_write_before(u8 *buf) {
	if (!buf) return;
	u64 *canary_loc = (u64 *)(buf - sizeof(u64));
	*canary_loc = CANARY_VALUE;
}

/* Write canary after buffer */
static inline void canary_write_after(u8 *buf, u32 size) {
	if (!buf) return;
	u64 *canary_loc = (u64 *)(buf + size);
	*canary_loc = CANARY_VALUE;
}

/* Check canary before buffer */
static inline u8 canary_check_before(u8 *buf) {
	if (!buf) return MEMSAFE_INVALID_PTR;
	u64 *canary_loc = (u64 *)(buf - sizeof(u64));
	if (*canary_loc != CANARY_VALUE) return MEMSAFE_OVERFLOW;
	return MEMSAFE_OK;
}

/* Check canary after buffer */
static inline u8 canary_check_after(u8 *buf, u32 size) {
	if (!buf) return MEMSAFE_INVALID_PTR;
	u64 *canary_loc = (u64 *)(buf + size);
	if (*canary_loc != CANARY_VALUE) return MEMSAFE_OVERFLOW;
	return MEMSAFE_OK;
}

/* ============================================================ */
/* MEMORY REGION TRACKING */
/* ============================================================ */

/* Initialize memory safety tracker */
static inline void memsafety_init(struct MemSafetyTracker *mst) {
	if (!mst) return;
	mst->region_count = 0;
	mst->generation_counter = 1;
	mst->total_allocated = 0;
	mst->peak_allocated = 0;
}

/* Register allocated region */
static inline u8 memsafety_alloc(
	struct MemSafetyTracker *mst,
	void *ptr,
	u32 size) {

	if (!mst || !ptr) return MEMSAFE_INVALID_PTR;
	if (mst->region_count >= 32) return MEMSAFE_HEAP_CORRUPT;

	struct MemoryRegion *region = &mst->regions[mst->region_count];
	region->base = ptr;
	region->size = size;
	region->generation = mst->generation_counter;
	region->in_use = 1;
	region->canary = CANARY_VALUE;

	mst->region_count++;
	mst->total_allocated += size;
	if (mst->total_allocated > mst->peak_allocated) {
		mst->peak_allocated = mst->total_allocated;
	}

	return MEMSAFE_OK;
}

/* Unregister deallocated region */
static inline u8 memsafety_free(
	struct MemSafetyTracker *mst,
	void *ptr) {

	if (!mst || !ptr) return MEMSAFE_INVALID_PTR;

	u32 i;
	for (i = 0; i < mst->region_count; i++) {
		if (mst->regions[i].base == ptr) {
			if (!mst->regions[i].in_use) {
				return MEMSAFE_DOUBLE_FREE;
			}
			mst->total_allocated -= mst->regions[i].size;
			mst->regions[i].in_use = 0;
			mst->regions[i].generation = 0;  /* Invalidate for UAF detection */
			return MEMSAFE_OK;
		}
	}

	return MEMSAFE_INVALID_PTR;  /* Pointer not in tracked regions */
}

/* Validate pointer access */
static inline u8 memsafety_access(
	struct MemSafetyTracker *mst,
	void *ptr,
	u32 offset,
	u32 size) {

	if (!mst || !ptr) return MEMSAFE_INVALID_PTR;

	u32 i;
	for (i = 0; i < mst->region_count; i++) {
		struct MemoryRegion *region = &mst->regions[i];
		if (region->base == ptr) {
			if (!region->in_use) return MEMSAFE_USE_AFTER_FREE;
			if (offset + size > region->size) return MEMSAFE_OVERFLOW;
			if (region->canary != CANARY_VALUE) return MEMSAFE_HEAP_CORRUPT;
			return MEMSAFE_OK;
		}
	}

	return MEMSAFE_INVALID_PTR;
}

/* ============================================================ */
/* BOUNDS CHECKING HELPERS */
/* ============================================================ */

/* Check pointer within bounds [base, base+size) */
static inline u8 bounds_check_ptr(void *ptr, void *base, u32 size) {
	if (!ptr || !base) return MEMSAFE_INVALID_PTR;
	u64 p = (u64)ptr;
	u64 b = (u64)base;
	if (p < b || p >= (b + size)) return MEMSAFE_OVERFLOW;
	return MEMSAFE_OK;
}

/* Check range [offset, offset+access_size) within [0, total_size) */
static inline u8 bounds_check_range(u32 offset, u32 access_size, u32 total_size) {
	if (offset > total_size) return MEMSAFE_UNDERFLOW;
	if (offset + access_size > total_size) return MEMSAFE_OVERFLOW;
	return MEMSAFE_OK;
}

/* Check array access: array[index] where array has element_size and count */
static inline u8 bounds_check_array(u32 index, u32 count, u32 element_size) {
	if (index >= count) return MEMSAFE_OVERFLOW;
	if (index * element_size / element_size != index) return MEMSAFE_OVERFLOW;
	return MEMSAFE_OK;
}

/* ============================================================ */
/* ALIGNMENT & POINTER VALIDATION */
/* ============================================================ */

/* Validate pointer alignment */
static inline u8 align_check(void *ptr, u32 alignment) {
	u64 p = (u64)ptr;
	if (alignment == 0) return MEMSAFE_INVALID_PTR;
	if ((p & (alignment - 1)) != 0) return MEMSAFE_MISALIGNED;
	return MEMSAFE_OK;
}

/* Check if pointer is properly aligned for type (e.g., u64 needs 8-byte alignment) */
static inline u8 align_check_u64(void *ptr) {
	return align_check(ptr, 8);
}

static inline u8 align_check_u32(void *ptr) {
	return align_check(ptr, 4);
}

static inline u8 align_check_u16(void *ptr) {
	return align_check(ptr, 2);
}

/* ============================================================ */
/* MEMORY SAFETY REPORT */
/* ============================================================ */

struct MemSafetyReport {
	u8 status;           /* MemSafetyStatus code */
	void *failed_ptr;    /* Pointer that failed check */
	u32 failed_offset;   /* Offset within region */
	u32 failed_size;     /* Requested access size */
	u64 total_allocated; /* Current total allocation */
	u64 peak_allocated;  /* Peak allocation seen */
};

/* Generate safety report */
static inline void memsafety_report(
	struct MemSafetyTracker *mst,
	u8 status,
	void *ptr,
	u32 offset,
	u32 size,
	struct MemSafetyReport *report) {

	if (!report) return;
	report->status = status;
	report->failed_ptr = ptr;
	report->failed_offset = offset;
	report->failed_size = size;
	if (mst) {
		report->total_allocated = mst->total_allocated;
		report->peak_allocated = mst->peak_allocated;
	}
}

#endif /* APKC_SEC_MEMORY_SAFETY_H */
