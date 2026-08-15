/* dist_artifact_management.h — Artifact Management (Stage 15.4)
 *
 * Artifact lifecycle: track creation, usage, storage, expiration.
 * Metadata indexing: rapid lookup by hash, version, or type.
 * Storage management: deduplication, cleanup, retention policies.
 * Verification: validate artifacts haven't been tampered with.
 * Distribution tracking: monitor artifact consumption across builds.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_DIST_ARTIFACT_MANAGEMENT_H
#define APKC_DIST_ARTIFACT_MANAGEMENT_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Artifact type classification */
enum ArtifactType {
	ARTIFACT_OBJECT_FILE = 0,   /* Compiled .o file */
	ARTIFACT_SHARED_LIBRARY = 1, /* Compiled .so file */
	ARTIFACT_STATIC_LIBRARY = 2, /* Static archive .a */
	ARTIFACT_EXECUTABLE = 3,     /* Compiled binary */
	ARTIFACT_INTERMEDIATE = 4,   /* Temporary compilation output */
	ARTIFACT_PACKAGE = 5,        /* APK or archive package */
	ARTIFACT_METADATA = 6        /* Build metadata/manifest */
};

/* Artifact storage location */
enum StorageLocation {
	STORAGE_LOCAL_CACHE = 0,    /* Local filesystem cache */
	STORAGE_REMOTE_CACHE = 1,   /* Remote network cache */
	STORAGE_BUILD_OUTPUT = 2,   /* Build directory output */
	STORAGE_ARCHIVED = 3        /* Long-term archive storage */
};

/* Artifact metadata */
struct ArtifactMetadata {
	const char *artifact_hash;  /* SHA256 content hash */
	const char *artifact_path;  /* File path or URL */
	u8 artifact_type;           /* ArtifactType */
	u8 storage_location;        /* StorageLocation */
	u64 size_bytes;             /* Artifact size in bytes */
	u64 created_time;           /* Creation timestamp */
	u64 accessed_time;          /* Last access time */
	u32 usage_count;            /* Times this artifact was used */
	u8 verified;                /* 1 if signature/hash verified */
	u8 is_expired;              /* 1 if past retention period */
};

/* Artifact distribution record */
struct DistributionRecord {
	const char *artifact_hash;  /* Which artifact */
	const char *consumer_name;  /* Who used it (build target, system) */
	u64 distribution_time;      /* When it was used */
	u32 copies_deployed;        /* Number of copies distributed */
};

/* Artifact manager */
struct ArtifactManager {
	struct ArtifactMetadata artifacts[128]; /* Up to 128 artifacts */
	u32 artifact_count;
	struct DistributionRecord dist_records[256]; /* Up to 256 distribution records */
	u32 dist_record_count;
	u64 total_storage_bytes;    /* Sum of all artifact sizes */
	u64 max_storage_bytes;      /* Storage limit (default 10GB) */
	u32 retention_days;         /* How long to keep artifacts (default 30) */
};

/* ============================================================ */
/* ARTIFACT MANAGER INITIALIZATION */
/* ============================================================ */

/* Initialize artifact manager */
static inline void artifact_mgr_init(
	struct ArtifactManager *mgr,
	u64 max_storage,
	u32 retention_days) {

	if (!mgr) return;
	mgr->artifact_count = 0;
	mgr->dist_record_count = 0;
	mgr->total_storage_bytes = 0;
	mgr->max_storage_bytes = max_storage;
	mgr->retention_days = retention_days;
}

/* ============================================================ */
/* ARTIFACT REGISTRATION & INDEXING */
/* ============================================================ */

/* Register artifact in manager */
static inline u8 artifact_register(
	struct ArtifactManager *mgr,
	const char *hash,
	const char *path,
	u8 artifact_type,
	u8 storage_location,
	u64 size) {

	if (!mgr || !hash || !path) return 0;
	if (mgr->artifact_count >= 128) return 0;

	struct ArtifactMetadata *art = &mgr->artifacts[mgr->artifact_count];
	art->artifact_hash = hash;
	art->artifact_path = path;
	art->artifact_type = artifact_type;
	art->storage_location = storage_location;
	art->size_bytes = size;
	art->created_time = 0;  /* Would be current time */
	art->accessed_time = 0;
	art->usage_count = 0;
	art->verified = 0;
	art->is_expired = 0;

	mgr->artifact_count++;
	mgr->total_storage_bytes += size;

	return 1;
}

/* Lookup artifact by hash */
static inline struct ArtifactMetadata *artifact_lookup(
	struct ArtifactManager *mgr,
	const char *hash) {

	if (!mgr || !hash) return 0;

	u32 i;
	for (i = 0; i < mgr->artifact_count; i++) {
		if (!mgr->artifacts[i].artifact_hash) continue;

		const char *art_hash = mgr->artifacts[i].artifact_hash;
		u32 j = 0;
		while (hash[j] && art_hash[j] && hash[j] == art_hash[j]) j++;

		if (hash[j] == 0 && art_hash[j] == 0) {
			mgr->artifacts[i].accessed_time = 0;  /* Would be current time */
			mgr->artifacts[i].usage_count++;
			return &mgr->artifacts[i];
		}
	}

	return 0;
}

/* ============================================================ */
/* ARTIFACT VERIFICATION & VALIDATION */
/* ============================================================ */

/* Verify artifact integrity (simplified) */
static inline u8 artifact_verify(
	struct ArtifactManager *mgr,
	const char *hash,
	const u8 *data,
	u32 data_len) {

	if (!mgr || !hash || !data) return 0;

	struct ArtifactMetadata *art = artifact_lookup(mgr, hash);
	if (!art) return 0;

	/* Simplified: verify size matches */
	if (art->size_bytes != data_len) {
		return 0;  /* Size mismatch */
	}

	/* In real implementation, would compute hash and compare */
	art->verified = 1;
	return 1;
}

/* Check if artifact is expired */
static inline u8 artifact_is_expired(
	struct ArtifactManager *mgr,
	const char *hash) {

	struct ArtifactMetadata *art = artifact_lookup(mgr, hash);
	if (!art) return 1;

	/* Simplified: no time tracking, assume not expired */
	return art->is_expired;
}

/* ============================================================ */
/* DISTRIBUTION TRACKING */
/* ============================================================ */

/* Record artifact distribution/usage */
static inline u8 artifact_record_distribution(
	struct ArtifactManager *mgr,
	const char *hash,
	const char *consumer_name,
	u32 copies) {

	if (!mgr || !hash || !consumer_name) return 0;
	if (mgr->dist_record_count >= 256) return 0;

	struct DistributionRecord *rec = &mgr->dist_records[mgr->dist_record_count];
	rec->artifact_hash = hash;
	rec->consumer_name = consumer_name;
	rec->distribution_time = 0;  /* Would be current time */
	rec->copies_deployed = copies;

	mgr->dist_record_count++;
	return 1;
}

/* Get usage count for artifact */
static inline u32 artifact_get_usage_count(
	struct ArtifactManager *mgr,
	const char *hash) {

	struct ArtifactMetadata *art = artifact_lookup(mgr, hash);
	if (!art) return 0;
	return art->usage_count;
}

/* ============================================================ */
/* STORAGE MANAGEMENT & CLEANUP */
/* ============================================================ */

/* Remove artifact from manager and update storage */
static inline u8 artifact_remove(
	struct ArtifactManager *mgr,
	const char *hash) {

	if (!mgr || !hash) return 0;

	u32 i;
	for (i = 0; i < mgr->artifact_count; i++) {
		if (!mgr->artifacts[i].artifact_hash) continue;

		const char *art_hash = mgr->artifacts[i].artifact_hash;
		u32 j = 0;
		while (hash[j] && art_hash[j] && hash[j] == art_hash[j]) j++;

		if (hash[j] == 0 && art_hash[j] == 0) {
			mgr->total_storage_bytes -= mgr->artifacts[i].size_bytes;
			mgr->artifacts[i].artifact_hash = 0;  /* Mark as empty */
			return 1;
		}
	}

	return 0;
}

/* Clean up expired artifacts to free storage */
static inline u32 artifact_cleanup_expired(struct ArtifactManager *mgr) {
	if (!mgr) return 0;

	u32 removed_count = 0;
	u32 i;
	for (i = 0; i < mgr->artifact_count; i++) {
		if (!mgr->artifacts[i].artifact_hash) continue;
		if (mgr->artifacts[i].is_expired) {
			mgr->total_storage_bytes -= mgr->artifacts[i].size_bytes;
			mgr->artifacts[i].artifact_hash = 0;
			removed_count++;
		}
	}

	return removed_count;
}

/* ============================================================ */
/* STATISTICS & REPORTING */
/* ============================================================ */

/* Get storage utilization percentage */
static inline u32 artifact_get_storage_usage_percent(struct ArtifactManager *mgr) {
	if (!mgr || mgr->max_storage_bytes == 0) return 0;
	return (u32)((mgr->total_storage_bytes * 100) / mgr->max_storage_bytes);
}

/* Get count of frequently-used artifacts */
static inline u32 artifact_count_hot(
	struct ArtifactManager *mgr,
	u32 usage_threshold) {

	if (!mgr) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < mgr->artifact_count; i++) {
		if (mgr->artifacts[i].artifact_hash &&
			mgr->artifacts[i].usage_count >= usage_threshold) {
			count++;
		}
	}
	return count;
}

/* Get count of cold artifacts (rarely used) */
static inline u32 artifact_count_cold(
	struct ArtifactManager *mgr,
	u32 usage_threshold) {

	if (!mgr) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < mgr->artifact_count; i++) {
		if (mgr->artifacts[i].artifact_hash &&
			mgr->artifacts[i].usage_count < usage_threshold) {
			count++;
		}
	}
	return count;
}

#endif /* APKC_DIST_ARTIFACT_MANAGEMENT_H */
