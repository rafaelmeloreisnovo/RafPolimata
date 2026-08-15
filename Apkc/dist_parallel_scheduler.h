/* dist_parallel_scheduler.h — Parallel Build Scheduler (Stage 15.3)
 *
 * Task parallelization: identify independent compilation tasks.
 * Critical path analysis: compute minimum build time with optimal parallelism.
 * Load balancing: distribute tasks across available CPU cores.
 * Job queue management: schedule tasks respecting dependency order.
 * Build throughput optimization: maximize CPU utilization.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_DIST_PARALLEL_SCHEDULER_H
#define APKC_DIST_PARALLEL_SCHEDULER_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Task status in build queue */
enum TaskStatus {
	TASK_PENDING = 0,           /* Waiting for dependencies */
	TASK_READY = 1,             /* Dependencies satisfied, ready to run */
	TASK_RUNNING = 2,           /* Currently executing */
	TASK_COMPLETED = 3,         /* Finished successfully */
	TASK_FAILED = 4             /* Execution failed */
};

/* Compilation task */
struct CompileTask {
	u32 task_id;                /* Unique task identifier */
	u32 source_file_idx;        /* Source file to compile */
	const char *source_path;    /* Source file path */
	const char *output_path;    /* Output .so path */
	u32 dependency_count;       /* Number of task dependencies */
	u32 dependencies[8];        /* Task IDs this depends on (max 8) */
	u64 estimated_time_ms;      /* Estimated compilation time */
	u64 actual_time_ms;         /* Actual execution time */
	u8 status;                  /* TaskStatus */
	u8 assigned_cpu;            /* CPU core assignment (0-15) */
};

/* CPU core load tracker */
struct CoreLoad {
	u8 core_id;                 /* CPU core index */
	u32 active_tasks;           /* Current running tasks */
	u64 total_work_ms;          /* Cumulative work assigned */
	u64 completion_time_ms;     /* Estimated completion time */
};

/* Parallel build scheduler */
struct ParallelScheduler {
	struct CompileTask tasks[64];    /* Up to 64 tasks */
	u32 task_count;
	struct CoreLoad cores[16];       /* Up to 16 CPU cores */
	u32 core_count;
	u64 critical_path_ms;           /* Minimum possible build time */
	u64 estimated_total_ms;         /* Estimated total with scheduling */
	u32 parallelism_factor;         /* (estimated_total / critical_path) */
	u8 all_tasks_ready;             /* 1 if no dependencies remain */
};

/* ============================================================ */
/* SCHEDULER INITIALIZATION */
/* ============================================================ */

/* Initialize parallel scheduler */
static inline void sched_init(
	struct ParallelScheduler *sched,
	u32 core_count) {

	if (!sched) return;
	sched->task_count = 0;
	sched->core_count = (core_count > 16) ? 16 : core_count;
	sched->critical_path_ms = 0;
	sched->estimated_total_ms = 0;
	sched->parallelism_factor = 0;
	sched->all_tasks_ready = 0;

	/* Initialize core load trackers */
	u32 i;
	for (i = 0; i < sched->core_count; i++) {
		sched->cores[i].core_id = i;
		sched->cores[i].active_tasks = 0;
		sched->cores[i].total_work_ms = 0;
		sched->cores[i].completion_time_ms = 0;
	}
}

/* ============================================================ */
/* TASK REGISTRATION */
/* ============================================================ */

/* Add compilation task to scheduler */
static inline u8 sched_add_task(
	struct ParallelScheduler *sched,
	u32 source_file_idx,
	const char *source_path,
	const char *output_path,
	u64 estimated_time_ms) {

	if (!sched || !source_path || !output_path) return 0;
	if (sched->task_count >= 64) return 0;

	struct CompileTask *task = &sched->tasks[sched->task_count];
	task->task_id = sched->task_count;
	task->source_file_idx = source_file_idx;
	task->source_path = source_path;
	task->output_path = output_path;
	task->dependency_count = 0;
	task->estimated_time_ms = estimated_time_ms;
	task->actual_time_ms = 0;
	task->status = TASK_PENDING;
	task->assigned_cpu = 0;

	sched->task_count++;
	return 1;
}

/* Add dependency between tasks */
static inline u8 sched_add_task_dependency(
	struct ParallelScheduler *sched,
	u32 task_id,
	u32 depends_on_id) {

	if (!sched || task_id >= sched->task_count || depends_on_id >= sched->task_count) return 0;

	struct CompileTask *task = &sched->tasks[task_id];
	if (task->dependency_count >= 8) return 0;

	task->dependencies[task->dependency_count] = depends_on_id;
	task->dependency_count++;

	return 1;
}

/* ============================================================ */
/* SCHEDULING & LOAD BALANCING */
/* ============================================================ */

/* Assign task to least-loaded CPU core */
static inline void sched_assign_task(
	struct ParallelScheduler *sched,
	u32 task_id,
	u8 core_id) {

	if (!sched || task_id >= sched->task_count || core_id >= sched->core_count) return;

	struct CompileTask *task = &sched->tasks[task_id];
	task->assigned_cpu = core_id;
	task->status = TASK_READY;

	/* Update core load */
	sched->cores[core_id].active_tasks++;
	sched->cores[core_id].total_work_ms += task->estimated_time_ms;
	sched->cores[core_id].completion_time_ms += task->estimated_time_ms;
}

/* Find least-loaded core for next task */
static inline u8 sched_find_least_loaded_core(struct ParallelScheduler *sched) {
	if (!sched || sched->core_count == 0) return 0;

	u8 least_loaded = 0;
	u64 min_work = sched->cores[0].total_work_ms;

	u32 i;
	for (i = 1; i < sched->core_count; i++) {
		if (sched->cores[i].total_work_ms < min_work) {
			min_work = sched->cores[i].total_work_ms;
			least_loaded = i;
		}
	}

	return least_loaded;
}

/* Compute schedule (assign all ready tasks to cores) */
static inline void sched_compute_schedule(struct ParallelScheduler *sched) {
	if (!sched) return;

	u32 i;
	for (i = 0; i < sched->task_count; i++) {
		struct CompileTask *task = &sched->tasks[i];

		/* Check if all dependencies are completed */
		u8 deps_satisfied = 1;
		u32 j;
		for (j = 0; j < task->dependency_count; j++) {
			if (sched->tasks[task->dependencies[j]].status != TASK_COMPLETED) {
				deps_satisfied = 0;
				break;
			}
		}

		/* If dependencies satisfied and not yet assigned, assign to least-loaded core */
		if (deps_satisfied && task->status == TASK_PENDING) {
			u8 core = sched_find_least_loaded_core(sched);
			sched_assign_task(sched, i, core);
		}
	}

	/* Compute total schedule time (max completion time across cores) */
	u64 max_time = 0;
	for (i = 0; i < sched->core_count; i++) {
		if (sched->cores[i].completion_time_ms > max_time) {
			max_time = sched->cores[i].completion_time_ms;
		}
	}
	sched->estimated_total_ms = max_time;

	/* Compute critical path (sum of longest dependency chain) */
	/* Simplified: assume tasks with longest total_work form critical path */
	u64 total_work = 0;
	for (i = 0; i < sched->task_count; i++) {
		total_work += sched->tasks[i].estimated_time_ms;
	}
	sched->critical_path_ms = (total_work / sched->core_count) + total_work % sched->core_count;

	/* Compute parallelism factor */
	if (sched->estimated_total_ms > 0) {
		sched->parallelism_factor = (total_work + sched->estimated_total_ms - 1) /
									  sched->estimated_total_ms;
	}
}

/* ============================================================ */
/* TASK COMPLETION & PROGRESS */
/* ============================================================ */

/* Mark task as completed with actual execution time */
static inline void sched_complete_task(
	struct ParallelScheduler *sched,
	u32 task_id,
	u64 actual_time_ms) {

	if (!sched || task_id >= sched->task_count) return;

	struct CompileTask *task = &sched->tasks[task_id];
	task->status = TASK_COMPLETED;
	task->actual_time_ms = actual_time_ms;
}

/* Get total completed tasks */
static inline u32 sched_get_completed_count(struct ParallelScheduler *sched) {
	if (!sched) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < sched->task_count; i++) {
		if (sched->tasks[i].status == TASK_COMPLETED) {
			count++;
		}
	}
	return count;
}

/* ============================================================ */
/* STATISTICS & ANALYSIS */
/* ============================================================ */

/* Compute actual speedup from parallel execution */
static inline u32 sched_compute_actual_speedup(struct ParallelScheduler *sched) {
	if (!sched) return 1;

	u64 total_actual_time = 0;
	u32 i;
	for (i = 0; i < sched->task_count; i++) {
		total_actual_time += sched->tasks[i].actual_time_ms;
	}

	if (total_actual_time == 0) return 1;

	/* Find max completion time across cores */
	u64 max_core_time = 0;
	for (i = 0; i < sched->core_count; i++) {
		u64 core_time = 0;
		u32 j;
		for (j = 0; j < sched->task_count; j++) {
			if (sched->tasks[j].assigned_cpu == i) {
				core_time += sched->tasks[j].actual_time_ms;
			}
		}
		if (core_time > max_core_time) {
			max_core_time = core_time;
		}
	}

	if (max_core_time == 0) return 1;
	return (u32)(total_actual_time / max_core_time);
}

/* Get estimated build time */
static inline u64 sched_get_estimated_time_ms(struct ParallelScheduler *sched) {
	if (!sched) return 0;
	return sched->estimated_total_ms;
}

/* Get CPU core utilization percentage */
static inline u32 sched_get_utilization_percent(struct ParallelScheduler *sched) {
	if (!sched || sched->core_count == 0 || sched->estimated_total_ms == 0) return 0;

	u64 total_work = 0;
	u32 i;
	for (i = 0; i < sched->task_count; i++) {
		total_work += sched->tasks[i].estimated_time_ms;
	}

	return (u32)((total_work * 100) / (sched->core_count * sched->estimated_total_ms));
}

#endif /* APKC_DIST_PARALLEL_SCHEDULER_H */
