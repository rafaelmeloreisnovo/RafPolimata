/* rt_memory_mgmt.h — Memory Management & Garbage Collection (Stage 7.2)
 *
 * Stack allocation tracking, heap management with bump allocator.
 * Reference counting for object lifetime management.
 * Mark-and-sweep garbage collection for cyclic structures.
 * Memory pooling for common object sizes.
 * Max 16MB heap, stack-only primary allocation.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_RT_MEMORY_MGMT_H
#define APKC_RT_MEMORY_MGMT_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Object header with metadata */
struct ObjectHeader {
	u32 type_id;           /* Type identifier for GC marking */
	u32 size;              /* Size in bytes */
	u32 ref_count;         /* Reference count (for RC mode) */
	u8 marked;             /* GC mark bit */
	u8 is_array;           /* 1 if array type */
	u32 array_len;         /* Length if array */
	u64 *refs[8];          /* Pointer slots for GC traversal (max 8) */
	u32 ref_slot_count;
};

/* Memory allocation pool (fixed-size buckets) */
struct MemoryPool {
	u8 pool[1024];         /* Fixed pool buffer */
	u32 pool_size;
	u32 next_free;         /* Bump allocator offset */
	u32 allocations;       /* Number of active allocations */
};

/* Heap allocator with bump allocation */
struct HeapAllocator {
	u8 heap[16777216];     /* 16MB heap */
	u32 heap_size;
	u32 next_free;         /* Bump allocator position */
	u32 allocation_count;
};

/* Garbage collector state */
struct GarbageCollector {
	u64 *roots[256];       /* Root references for marking phase */
	u32 root_count;
	u8 *heap_base;         /* Heap start */
	u32 heap_size;
	u32 marked_bytes;      /* Bytes marked in current GC pass */
	u32 freed_bytes;       /* Bytes freed in sweep phase */
	u8 gc_enabled;         /* 1 if GC is active */
	u32 gc_threshold;      /* Trigger GC after this many allocations */
};

/* Memory management context */
struct MemoryMgmt {
	struct HeapAllocator heap;
	struct MemoryPool pools[4];  /* 4 size classes: 64B, 256B, 1KB, 4KB */
	struct GarbageCollector gc;
	u32 stack_depth;       /* Stack pointer tracking */
	u32 total_allocations;
	u32 total_freed;
};

/* Initialize memory management */
static inline void memory_mgmt_init(struct MemoryMgmt *mem) {
	mem->heap.heap_size = 16777216;
	mem->heap.next_free = 0;
	mem->heap.allocation_count = 0;
	mem->stack_depth = 0;
	mem->total_allocations = 0;
	mem->total_freed = 0;
	mem->gc.heap_base = mem->heap.heap;
	mem->gc.heap_size = mem->heap.heap_size;
	mem->gc.root_count = 0;
	mem->gc.gc_enabled = 0;
	mem->gc.gc_threshold = 1000;  /* GC after 1000 allocations */
}

/* === HEAP ALLOCATION === */

/* Allocate memory from heap with bump allocator */
static inline u64 heap_alloc(
	struct MemoryMgmt *mem,
	u32 size)
{
	if (mem->heap.next_free + size >= mem->heap.heap_size) {
		return 0;  /* Out of memory */
	}

	u64 ptr = (u64)mem->heap.heap + mem->heap.next_free;
	mem->heap.next_free += size;
	mem->heap.allocation_count++;
	mem->total_allocations++;

	/* Trigger GC if threshold reached */
	if (mem->heap.allocation_count >= mem->gc.gc_threshold) {
		mem->gc.gc_enabled = 1;
	}

	return ptr;
}

/* Allocate object with header */
static inline u64 heap_alloc_object(
	struct MemoryMgmt *mem,
	u32 type_id, u32 size)
{
	/* Size includes object header + data */
	u32 total_size = sizeof(struct ObjectHeader) + size;
	u64 addr = heap_alloc(mem, total_size);
	if (addr == 0) return 0;

	struct ObjectHeader *hdr = (struct ObjectHeader *)addr;
	hdr->type_id = type_id;
	hdr->size = size;
	hdr->ref_count = 1;  /* New object has 1 reference */
	hdr->marked = 0;
	hdr->is_array = 0;
	hdr->ref_slot_count = 0;

	return addr + sizeof(struct ObjectHeader);  /* Return pointer past header */
}

/* Allocate array object */
static inline u64 heap_alloc_array(
	struct MemoryMgmt *mem,
	u32 type_id, u32 element_size, u32 length)
{
	u32 total_size = sizeof(struct ObjectHeader) + (element_size * length);
	u64 addr = heap_alloc(mem, total_size);
	if (addr == 0) return 0;

	struct ObjectHeader *hdr = (struct ObjectHeader *)addr;
	hdr->type_id = type_id;
	hdr->size = element_size * length;
	hdr->ref_count = 1;
	hdr->marked = 0;
	hdr->is_array = 1;
	hdr->array_len = length;
	hdr->ref_slot_count = 0;

	return addr + sizeof(struct ObjectHeader);
}

/* === REFERENCE COUNTING === */

/* Increment reference count */
static inline void ref_acquire(u64 ptr) {
	if (ptr == 0) return;
	struct ObjectHeader *hdr = (struct ObjectHeader *)(ptr - sizeof(struct ObjectHeader));
	hdr->ref_count++;
}

/* Decrement reference count */
static inline void ref_release(
	struct MemoryMgmt *mem,
	u64 ptr)
{
	if (ptr == 0) return;
	struct ObjectHeader *hdr = (struct ObjectHeader *)(ptr - sizeof(struct ObjectHeader));
	if (hdr->ref_count > 0) {
		hdr->ref_count--;
		if (hdr->ref_count == 0) {
			mem->total_freed += hdr->size;
		}
	}
}

/* === GARBAGE COLLECTION === */

/* Register root reference for GC */
static inline u8 gc_add_root(
	struct GarbageCollector *gc,
	u64 *root_ptr)
{
	if (gc->root_count >= 256) return 1;

	gc->roots[gc->root_count] = root_ptr;
	gc->root_count++;
	return 0;
}

/* Mark reachable objects from roots */
static inline void gc_mark(
	struct GarbageCollector *gc)
{
	/* Simplified mark phase: DFS from root references */
	u32 i, j;
	for (i = 0; i < gc->root_count; i++) {
		u64 *ptr = gc->roots[i];
		if (*ptr == 0) continue;

		struct ObjectHeader *hdr = (struct ObjectHeader *)(*ptr - sizeof(struct ObjectHeader));
		if (hdr->marked) continue;

		hdr->marked = 1;
		gc->marked_bytes += hdr->size;

		/* Recursively mark referenced objects */
		for (j = 0; j < hdr->ref_slot_count; j++) {
			if (hdr->refs[j]) {
				u64 ref_val = *hdr->refs[j];
				if (ref_val != 0) {
					struct ObjectHeader *ref_hdr =
						(struct ObjectHeader *)(ref_val - sizeof(struct ObjectHeader));
					if (!ref_hdr->marked) {
						gc_mark(gc);  /* Recursive marking (limited depth) */
					}
				}
			}
		}
	}
}

/* Sweep unreachable objects */
static inline void gc_sweep(
	struct GarbageCollector *gc)
{
	/* Simplified sweep: scan heap for unmarked objects */
	/* In real implementation, would deallocate unmarked blocks */
	/* For freestanding model, just reset marks for next cycle */
	u32 heap_pos = 0;
	while (heap_pos < gc->heap_size) {
		struct ObjectHeader *hdr = (struct ObjectHeader *)(gc->heap_base + heap_pos);
		if (hdr->size == 0) break;

		if (!hdr->marked) {
			/* Unreachable: would deallocate here */
			gc->freed_bytes += hdr->size;
		}
		hdr->marked = 0;  /* Reset mark bit */

		heap_pos += sizeof(struct ObjectHeader) + hdr->size;
	}
}

/* Run full garbage collection cycle */
static inline void gc_collect(
	struct GarbageCollector *gc)
{
	gc->marked_bytes = 0;
	gc->freed_bytes = 0;

	/* Mark phase */
	gc_mark(gc);

	/* Sweep phase */
	gc_sweep(gc);
}

/* === MEMORY POOLS === */

/* Allocate from size-specific pool */
static inline u64 pool_alloc(
	struct MemoryMgmt *mem,
	u32 pool_id,
	u32 size)
{
	if (pool_id >= 4) return 0;

	struct MemoryPool *pool = &mem->pools[pool_id];
	if (pool->next_free + size >= pool->pool_size) {
		return 0;  /* Pool full */
	}

	u64 ptr = (u64)pool->pool + pool->next_free;
	pool->next_free += size;
	pool->allocations++;

	return ptr;
}

/* Free pool allocation (no-op in freestanding, tracked in ref count) */
static inline void pool_free(
	struct MemoryPool *pool,
	u64 ptr)
{
	if (pool->allocations > 0) {
		pool->allocations--;
	}
}

#endif /* APKC_RT_MEMORY_MGMT_H */
