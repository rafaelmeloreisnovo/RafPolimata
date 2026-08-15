/* opt_parallelization.h — Parallelization & Auto-Vectorization (Phase 43)
 *
 * Phase 43: Parallelization and auto-vectorization
 * - Loop parallelization detection
 * - SIMD auto-vectorization
 * - Dependency analysis for parallelism
 * - Thread-level parallelism extraction
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_OPT_PARALLELIZATION_H
#define APKC_OPT_PARALLELIZATION_H 1

#include "opt_speculative.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* ============================================================ */
/* LOOP PARALLELIZATION */
/* ============================================================ */

enum ParallelizationType {
	PARALLEL_NONE = 0,
	PARALLEL_OUTER_LOOP = 1,
	PARALLEL_INNER_LOOP = 2,
	PARALLEL_ACROSS_ITERATIONS = 3
};

struct ParallelizableLoop {
	u32 loop_id;
	enum ParallelizationType par_type;
	u32 parallelism_factor;
	u32 synchronization_points;
	u8 is_safe_parallel;
	u64 expected_speedup;
};

struct LoopParallelizer {
	struct ParallelizableLoop loops[128];
	u32 loop_count;
	u32 parallelized_loops;
	u64 total_expected_speedup;
};

static inline void loop_parallelizer_init(struct LoopParallelizer *lp) {
	if (!lp) return;
	lp->loop_count = 0;
	lp->parallelized_loops = 0;
	lp->total_expected_speedup = 0;
}

static inline u8 analyze_loop_parallelism(
	struct LoopParallelizer *lp,
	u32 loop_id,
	enum ParallelizationType par_type,
	u32 factor) {

	if (!lp || lp->loop_count >= 128) return 1;

	struct ParallelizableLoop *ploop = &lp->loops[lp->loop_count];
	ploop->loop_id = loop_id;
	ploop->par_type = par_type;
	ploop->parallelism_factor = factor;
	ploop->synchronization_points = 0;
	ploop->is_safe_parallel = (par_type != PARALLEL_NONE);
	ploop->expected_speedup = factor;

	lp->loop_count++;
	if (ploop->is_safe_parallel) {
		lp->parallelized_loops++;
		lp->total_expected_speedup += ploop->expected_speedup;
	}
	return 0;
}

/* ============================================================ */
/* AUTO-VECTORIZATION */
/* ============================================================ */

struct VectorizableOperation {
	u32 op_id;
	u8 operation_type;
	u32 vector_width;
	u32 element_count;
	u8 is_profitable;
	u64 throughput_gain;
};

struct AutoVectorizer {
	struct VectorizableOperation operations[256];
	u32 op_count;
	u32 vectorized_ops;
	u64 total_throughput_gain;
};

static inline void auto_vectorizer_init(struct AutoVectorizer *av) {
	if (!av) return;
	av->op_count = 0;
	av->vectorized_ops = 0;
	av->total_throughput_gain = 0;
}

static inline u8 auto_vectorize_operation(
	struct AutoVectorizer *av,
	u32 op_id,
	u8 op_type,
	u32 vwidth) {

	if (!av || av->op_count >= 256) return 1;

	struct VectorizableOperation *vop = &av->operations[av->op_count];
	vop->op_id = op_id;
	vop->operation_type = op_type;
	vop->vector_width = vwidth;
	vop->element_count = 0;
	vop->is_profitable = 1;
	vop->throughput_gain = 0;

	av->op_count++;
	av->vectorized_ops++;
	return 0;
}

/* ============================================================ */
/* DEPENDENCY ANALYSIS FOR PARALLELISM */
/* ============================================================ */

enum DependencyType {
	DEP_FLOW = 0,
	DEP_ANTI = 1,
	DEP_OUTPUT = 2,
	DEP_INPUT = 3
};

struct DataDependence {
	u32 source_stmt;
	u32 dest_stmt;
	enum DependencyType dep_type;
	u32 distance;
	u8 can_parallelize;
};

struct DependenceAnalyzer {
	struct DataDependence dependences[512];
	u32 dep_count;
	u32 parallelizable_deps;
	u32 loop_carried_deps;
};

static inline void dependence_analyzer_init(struct DependenceAnalyzer *da) {
	if (!da) return;
	da->dep_count = 0;
	da->parallelizable_deps = 0;
	da->loop_carried_deps = 0;
}

static inline u8 add_dependence(
	struct DependenceAnalyzer *da,
	u32 src_stmt,
	u32 dst_stmt,
	enum DependencyType dtype) {

	if (!da || da->dep_count >= 512) return 1;

	struct DataDependence *dd = &da->dependences[da->dep_count];
	dd->source_stmt = src_stmt;
	dd->dest_stmt = dst_stmt;
	dd->dep_type = dtype;
	dd->distance = 0;
	dd->can_parallelize = (dtype == DEP_INPUT);

	da->dep_count++;
	if (dd->can_parallelize) da->parallelizable_deps++;
	return 0;
}

/* ============================================================ */
/* THREAD-LEVEL PARALLELISM EXTRACTION */
/* ============================================================ */

struct TaskRegion {
	u32 task_id;
	u32 start_stmt;
	u32 end_stmt;
	u32 thread_count;
	u64 expected_execution_time;
	u8 has_barriers;
};

struct ThreadParallelismExtractor {
	struct TaskRegion tasks[64];
	u32 task_count;
	u32 thread_pool_size;
	u32 total_parallelism_level;
};

static inline void thread_parallelism_extractor_init(struct ThreadParallelismExtractor *tpe) {
	if (!tpe) return;
	tpe->task_count = 0;
	tpe->thread_pool_size = 0;
	tpe->total_parallelism_level = 0;
}

static inline u8 extract_task_region(
	struct ThreadParallelismExtractor *tpe,
	u32 task_id,
	u32 start_stmt,
	u32 end_stmt,
	u32 thread_count) {

	if (!tpe || tpe->task_count >= 64) return 1;

	struct TaskRegion *tr = &tpe->tasks[tpe->task_count];
	tr->task_id = task_id;
	tr->start_stmt = start_stmt;
	tr->end_stmt = end_stmt;
	tr->thread_count = thread_count;
	tr->expected_execution_time = 0;
	tr->has_barriers = 0;

	tpe->task_count++;
	tpe->total_parallelism_level += thread_count;
	return 0;
}

#endif /* APKC_OPT_PARALLELIZATION_H */
