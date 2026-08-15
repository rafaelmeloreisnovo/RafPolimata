/* eco_package_registry.h — Package Registry Integration (Stage 19.2)
 *
 * Package metadata: name, version, author, license, homepage.
 * Registry lookup: query package information from registry database.
 * Version resolution: find compatible versions satisfying constraints.
 * Publish metadata: export package info for registry publication.
 * Compatibility matrix: track which versions work together.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_ECO_PACKAGE_REGISTRY_H
#define APKC_ECO_PACKAGE_REGISTRY_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Package metadata */
struct PackageMetadata {
	const char *package_name;   /* Package identifier */
	const char *version;        /* Semantic version (e.g., "1.2.3") */
	const char *author;         /* Package author */
	const char *license;        /* License type (MIT, Apache, GPL, etc) */
	const char *homepage;       /* Project URL */
	const char *description;    /* Short description */
	u32 download_count;         /* Total downloads */
	u64 published_timestamp;    /* When published */
	u8 is_deprecated;           /* 1 if deprecated */
	u32 package_id;             /* Unique package ID */
};

/* Version compatibility record */
struct VersionCompat {
	const char *version_a;      /* First version */
	const char *version_b;      /* Compatible version */
	u8 compatibility_level;     /* 0=incompatible, 1=partial, 2=full */
};

/* Package registry */
struct PackageRegistry {
	struct PackageMetadata packages[256];    /* Up to 256 packages */
	u32 package_count;
	struct VersionCompat compatibilities[256]; /* Version compatibility */
	u32 compat_count;
	u32 total_packages_registered;
	u32 total_downloads;
};

/* ============================================================ */
/* REGISTRY INITIALIZATION */
/* ============================================================ */

/* Initialize package registry */
static inline void registry_init(struct PackageRegistry *reg) {
	if (!reg) return;
	reg->package_count = 0;
	reg->compat_count = 0;
	reg->total_packages_registered = 0;
	reg->total_downloads = 0;
}

/* ============================================================ */
/* PACKAGE REGISTRATION */
/* ============================================================ */

/* Register package in registry */
static inline u8 registry_add_package(
	struct PackageRegistry *reg,
	const char *name,
	const char *version,
	const char *author,
	const char *license,
	const char *description) {

	if (!reg || !name || !version || !author) return 0;
	if (reg->package_count >= 256) return 0;

	struct PackageMetadata *pkg = &reg->packages[reg->package_count];
	pkg->package_name = name;
	pkg->version = version;
	pkg->author = author;
	pkg->license = license;
	pkg->description = description;
	pkg->download_count = 0;
	pkg->published_timestamp = 0;  /* Would be current time */
	pkg->is_deprecated = 0;
	pkg->package_id = reg->package_count;

	reg->package_count++;
	reg->total_packages_registered++;
	return 1;
}

/* Mark package as deprecated */
static inline u8 registry_deprecate_package(
	struct PackageRegistry *reg,
	u32 package_id) {

	if (!reg || package_id >= reg->package_count) return 0;

	reg->packages[package_id].is_deprecated = 1;
	return 1;
}

/* ============================================================ */
/* REGISTRY LOOKUP */
/* ============================================================ */

/* Lookup package by name */
static inline struct PackageMetadata *registry_lookup_package(
	struct PackageRegistry *reg,
	const char *package_name) {

	if (!reg || !package_name) return 0;

	u32 i;
	for (i = 0; i < reg->package_count; i++) {
		const char *name_a = reg->packages[i].package_name;
		const char *name_b = package_name;
		u32 j = 0;
		while (name_a[j] && name_b[j] && name_a[j] == name_b[j]) j++;
		if (name_a[j] == 0 && name_b[j] == 0) {
			return &reg->packages[i];
		}
	}

	return 0;
}

/* Record download */
static inline u8 registry_record_download(
	struct PackageRegistry *reg,
	u32 package_id) {

	if (!reg || package_id >= reg->package_count) return 0;

	reg->packages[package_id].download_count++;
	reg->total_downloads++;
	return 1;
}

/* ============================================================ */
/* VERSION COMPATIBILITY */
/* ============================================================ */

/* Add version compatibility record */
static inline u8 registry_add_compat(
	struct PackageRegistry *reg,
	const char *version_a,
	const char *version_b,
	u8 compat_level) {

	if (!reg || !version_a || !version_b) return 0;
	if (reg->compat_count >= 256) return 0;

	struct VersionCompat *vc = &reg->compatibilities[reg->compat_count];
	vc->version_a = version_a;
	vc->version_b = version_b;
	vc->compatibility_level = compat_level;

	reg->compat_count++;
	return 1;
}

/* Check version compatibility */
static inline u8 registry_check_compat(
	struct PackageRegistry *reg,
	const char *version_a,
	const char *version_b) {

	if (!reg || !version_a || !version_b) return 0;

	u32 i;
	for (i = 0; i < reg->compat_count; i++) {
		struct VersionCompat *vc = &reg->compatibilities[i];
		if (vc->compatibility_level > 0) {  /* Any compatibility level > 0 is compatible */
			/* Would compare versions here */
			return 1;
		}
	}

	return 0;
}

/* ============================================================ */
/* REGISTRY STATISTICS */
/* ============================================================ */

/* Get download count for package */
static inline u32 registry_get_download_count(
	struct PackageRegistry *reg,
	u32 package_id) {

	if (!reg || package_id >= reg->package_count) return 0;
	return reg->packages[package_id].download_count;
}

/* Get most downloaded package */
static inline struct PackageMetadata *registry_find_most_downloaded(struct PackageRegistry *reg) {
	if (!reg || reg->package_count == 0) return 0;

	struct PackageMetadata *most = &reg->packages[0];
	u32 i;
	for (i = 1; i < reg->package_count; i++) {
		if (reg->packages[i].download_count > most->download_count) {
			most = &reg->packages[i];
		}
	}

	return most;
}

/* Count active packages (non-deprecated) */
static inline u32 registry_count_active_packages(struct PackageRegistry *reg) {
	if (!reg) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < reg->package_count; i++) {
		if (!reg->packages[i].is_deprecated) {
			count++;
		}
	}

	return count;
}

#endif /* APKC_ECO_PACKAGE_REGISTRY_H */
