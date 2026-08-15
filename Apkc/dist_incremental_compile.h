/* dist_incremental_compile.h — Incremental Compilation (Stage 15.2)
 *
 * Dependency tracking: maintain graph of source file dependencies.
 * Change detection: identify which files changed since last build.
 * Recompilation targeting: only recompile files with changed dependencies.
 * Parallel build scheduling: order recompilation to maximize parallelism.
 * Rebuild necessity: compute whether full rebuild vs incremental is faster.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_DIST_INCREMENTAL_COMPILE_H
#define APKC_DIST_INCREMENTAL_COMPILE_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* File change type */
enum FileChangeType {
	FILE_UNCHANGED = 0,         /* File and hash match */
	FILE_MODIFIED = 1,          /* File content changed */
	FILE_CREATED = 2,           /* New file since last build */
	FILE_DELETED = 3,           /* File removed */
	FILE_UNKNOWN = 4            /* Not seen before */
};

/* Dependency edge in source graph */
struct Dependency {
	u32 from_file_idx;          /* Source file index */
	u32 to_file_idx;            /* Target file index */
	u8 is_direct;               /* 1 if direct include, 0 if transitive */
};

/* Source file metadata */
struct SourceFile {
	const char *path;           /* File path */
	const char *content_hash;   /* SHA256(file content) */
	u64 modification_time;      /* File mtime */
	u32 size;                   /* File size in bytes */
	u32 dependency_count;       /* Number of dependencies */
	u32 dependencies[16];       /* Dependency indices (up to 16) */
	u8 change_type;             /* FileChangeType */
	u8 needs_recompile;         /* 1 if must be recompiled */
};

/* Incremental compilation state */
struct IncrementalState {
	struct SourceFile files[64];    /* Up to 64 source files */
	u32 file_count;
	struct Dependency deps[128];    /* Up to 128 dependencies */
	u32 dep_count;
	u32 files_changed;              /* Number of changed files */
	u32 files_need_recompile;       /* Number that need recompilation */
	u32 build_order[64];            /* Topological sort order */
};

/* ============================================================ */
/* INCREMENTAL STATE INITIALIZATION */
/* ============================================================ */

/* Initialize incremental compilation state */
static inline void incr_init(struct IncrementalState *state) {
	if (!state) return;
	state->file_count = 0;
	state->dep_count = 0;
	state->files_changed = 0;
	state->files_need_recompile = 0;
}

/* ============================================================ */
/* SOURCE FILE TRACKING */
/* ============================================================ */

/* Add source file to state */
static inline u8 incr_add_file(
	struct IncrementalState *state,
	const char *path,
	const char *content_hash,
	u64 mtime,
	u32 size) {

	if (!state || !path || !content_hash) return 0;
	if (state->file_count >= 64) return 0;

	struct SourceFile *f = &state->files[state->file_count];
	f->path = path;
	f->content_hash = content_hash;
	f->modification_time = mtime;
	f->size = size;
	f->dependency_count = 0;
	f->change_type = FILE_UNKNOWN;
	f->needs_recompile = 0;

	state->file_count++;
	return 1;
}

/* Add dependency edge between files */
static inline u8 incr_add_dependency(
	struct IncrementalState *state,
	u32 from_idx,
	u32 to_idx,
	u8 is_direct) {

	if (!state) return 0;
	if (from_idx >= state->file_count || to_idx >= state->file_count) return 0;
	if (state->dep_count >= 128) return 0;

	struct Dependency *dep = &state->deps[state->dep_count];
	dep->from_file_idx = from_idx;
	dep->to_file_idx = to_idx;
	dep->is_direct = is_direct;

	state->dep_count++;

	/* Track dependency on source file */
	if (state->files[from_idx].dependency_count < 16) {
		state->files[from_idx].dependencies[state->files[from_idx].dependency_count] = to_idx;
		state->files[from_idx].dependency_count++;
	}

	return 1;
}

/* ============================================================ */
/* CHANGE DETECTION */
/* ============================================================ */

/* Update file status based on current state (hash/mtime comparison) */
static inline void incr_detect_changes(
	struct IncrementalState *state,
	const char *previous_hash) {

	if (!state) return;

	u32 i;
	for (i = 0; i < state->file_count; i++) {
		struct SourceFile *f = &state->files[i];

		/* Simplified: if hash differs, mark as modified */
		/* In real implementation, would compare to persistent state */
		if (!previous_hash || !f->content_hash) {
			f->change_type = FILE_UNKNOWN;
		} else {
			/* Compare hashes (simplified string match) */
			u32 j = 0;
			while (previous_hash[j] && f->content_hash[j] &&
				   previous_hash[j] == f->content_hash[j]) j++;

			if (previous_hash[j] == 0 && f->content_hash[j] == 0) {
				f->change_type = FILE_UNCHANGED;
			} else {
				f->change_type = FILE_MODIFIED;
				state->files_changed++;
			}
		}
	}
}

/* ============================================================ */
/* RECOMPILATION SCHEDULING */
/* ============================================================ */

/* Mark file and all dependents for recompilation */
static inline void incr_mark_for_recompile(
	struct IncrementalState *state,
	u32 file_idx) {

	if (!state || file_idx >= state->file_count) return;

	struct SourceFile *f = &state->files[file_idx];
	if (f->needs_recompile) return;  /* Already marked */

	f->needs_recompile = 1;
	state->files_need_recompile++;

	/* Recursively mark files that depend on this one */
	u32 i;
	for (i = 0; i < state->dep_count; i++) {
		if (state->deps[i].to_file_idx == file_idx) {
			incr_mark_for_recompile(state, state->deps[i].from_file_idx);
		}
	}
}

/* Compute which files need recompilation based on changes */
static inline void incr_compute_rebuild_set(struct IncrementalState *state) {
	if (!state) return;

	state->files_need_recompile = 0;

	/* Mark all changed files for recompilation */
	u32 i;
	for (i = 0; i < state->file_count; i++) {
		if (state->files[i].change_type != FILE_UNCHANGED) {
			incr_mark_for_recompile(state, i);
		}
	}
}

/* Topological sort for build order (simplified BFS) */
static inline void incr_compute_build_order(struct IncrementalState *state) {
	if (!state) return;

	u32 order_idx = 0;

	/* Simple level-by-level BFS traversal */
	u32 i;
	for (i = 0; i < state->file_count; i++) {
		if (!state->files[i].path) continue;
		if (!state->files[i].needs_recompile) continue;

		/* Check if all dependencies are in order before this file */
		u8 deps_satisfied = 1;
		u32 j;
		for (j = 0; j < state->files[i].dependency_count; j++) {
			u32 dep_idx = state->files[i].dependencies[j];
			if (state->files[dep_idx].needs_recompile) {
				/* Check if dependency is already in build order */
				u8 found = 0;
				u32 k;
				for (k = 0; k < order_idx; k++) {
					if (state->build_order[k] == dep_idx) {
						found = 1;
						break;
					}
				}
				if (!found) {
					deps_satisfied = 0;
					break;
				}
			}
		}

		if (deps_satisfied && order_idx < 64) {
			state->build_order[order_idx++] = i;
		}
	}
}

/* ============================================================ */
/* REBUILD DECISION */
/* ============================================================ */

/* Estimate if incremental is faster than full rebuild */
static inline u8 incr_prefer_incremental(struct IncrementalState *state) {
	if (!state) return 0;

	/* Heuristic: if less than 30% of files changed, incremental is faster */
	if (state->file_count == 0) return 0;
	u32 pct_changed = (state->files_changed * 100) / state->file_count;

	return (pct_changed < 30) ? 1 : 0;
}

/* ============================================================ */
/* STATISTICS & QUERIES */
/* ============================================================ */

/* Get number of files that need recompilation */
static inline u32 incr_get_rebuild_count(struct IncrementalState *state) {
	if (!state) return 0;
	return state->files_need_recompile;
}

/* Get percentage of files that changed */
static inline u32 incr_get_change_percentage(struct IncrementalState *state) {
	if (!state || state->file_count == 0) return 0;
	return (state->files_changed * 100) / state->file_count;
}

#endif /* APKC_DIST_INCREMENTAL_COMPILE_H */
