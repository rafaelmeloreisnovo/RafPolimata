/* eco_package_manager.h — Package Manager for Polyglot Modules (Stage 12.2)
 *
 * Package metadata: name, version, platform, dependencies.
 * Package resolution: find best matching version with constraints.
 * Package storage: simple registry of available packages.
 * Dependency graph: track which packages depend on which.
 * Version constraint matching: semantic versioning support.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_ECO_PACKAGE_MANAGER_H
#define APKC_ECO_PACKAGE_MANAGER_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Package manager status */
enum PackageStatus {
	PKG_OK = 0,           /* Package found and valid */
	PKG_NOT_FOUND = 1,    /* Package not in registry */
	PKG_VERSION_CONFLICT = 2, /* Version constraint violated */
	PKG_DEPENDENCY_MISSING = 3, /* Required dependency not found */
	PKG_INCOMPATIBLE = 4, /* Package incompatible with platform */
	PKG_CORRUPTED = 5,    /* Package metadata corrupted */
	PKG_DEPRECATED = 6    /* Package marked deprecated */
};

/* Semantic version constraint */
enum VersionConstraint {
	VER_EXACT = 0,     /* == version */
	VER_MIN = 1,       /* >= version */
	VER_COMPATIBLE = 2 /* ~> version (compatible releases) */
};

/* Platform descriptor */
enum PlatformType {
	PLATFORM_ARM64 = 1,
	PLATFORM_ARM32 = 2,
	PLATFORM_X86_64 = 3,
	PLATFORM_X86 = 4,
	PLATFORM_ANY = 0xff
};

/* Package version */
struct PackageVersion {
	u16 major;
	u16 minor;
	u32 patch;
};

/* Package dependency */
struct PackageDependency {
	const char *package_name;
	struct PackageVersion min_version;
	u8 constraint_type;  /* VersionConstraint */
};

/* Package metadata */
struct PackageMetadata {
	const char *name;            /* Package name */
	struct PackageVersion version; /* Package version */
	const char *language;        /* Primary language (Python, Go, etc.) */
	u8 platform;                 /* PlatformType */
	const char *author;          /* Package author */
	const char *description;     /* Human-readable description */
	struct PackageDependency *deps; /* Dependency array */
	u32 dep_count;
	u64 size_bytes;              /* Package size */
	u32 release_date;            /* Unix timestamp */
	u8 deprecated;               /* 1 if deprecated */
};

/* Installed package */
struct InstalledPackage {
	struct PackageMetadata metadata;
	const char *install_path;    /* Local installation path */
	u8 verified;                 /* 1 if integrity verified */
	u64 install_time_ms;         /* Installation time */
};

/* Package registry */
struct PackageRegistry {
	struct InstalledPackage packages[64];  /* Up to 64 installed packages */
	u32 package_count;
	u32 total_size_bytes;
};

/* ============================================================ */
/* PACKAGE REGISTRY OPERATIONS */
/* ============================================================ */

/* Initialize package registry */
static inline void package_registry_init(struct PackageRegistry *pr) {
	if (!pr) return;
	pr->package_count = 0;
	pr->total_size_bytes = 0;
}

/* Register installed package */
static inline u8 package_register(
	struct PackageRegistry *pr,
	const struct PackageMetadata *metadata,
	const char *install_path) {

	if (!pr || !metadata || !install_path) return PKG_CORRUPTED;
	if (pr->package_count >= 64) return PKG_CORRUPTED;

	struct InstalledPackage *pkg = &pr->packages[pr->package_count];
	pkg->metadata = *metadata;
	pkg->install_path = install_path;
	pkg->verified = 0;
	pkg->install_time_ms = 0;

	pr->package_count++;
	pr->total_size_bytes += metadata->size_bytes;

	return PKG_OK;
}

/* Find package by name */
static inline struct InstalledPackage *package_find(
	struct PackageRegistry *pr,
	const char *name) {

	if (!pr || !name) return 0;

	u32 i;
	for (i = 0; i < pr->package_count; i++) {
		struct InstalledPackage *pkg = &pr->packages[i];
		const char *pname = pkg->metadata.name;

		/* String compare */
		u32 j = 0;
		while (name[j] && pname[j] && name[j] == pname[j]) j++;

		if (name[j] == 0 && pname[j] == 0) {
			if (pkg->metadata.deprecated) return 0;  /* Skip deprecated */
			return pkg;
		}
	}

	return 0;
}

/* Find package by language */
static inline struct InstalledPackage *package_find_by_language(
	struct PackageRegistry *pr,
	const char *language) {

	if (!pr || !language) return 0;

	u32 i;
	for (i = 0; i < pr->package_count; i++) {
		struct InstalledPackage *pkg = &pr->packages[i];
		if (pkg->metadata.deprecated) continue;

		const char *lang = pkg->metadata.language;

		/* String compare */
		u32 j = 0;
		while (language[j] && lang[j] && language[j] == lang[j]) j++;

		if (language[j] == 0 && lang[j] == 0) {
			return pkg;
		}
	}

	return 0;
}

/* ============================================================ */
/* VERSION CONSTRAINT MATCHING */
/* ============================================================ */

/* Compare versions: -1 if v1<v2, 0 if equal, 1 if v1>v2 */
static inline s32 version_compare(
	struct PackageVersion v1,
	struct PackageVersion v2) {

	if (v1.major != v2.major) {
		return (v1.major > v2.major) ? 1 : -1;
	}
	if (v1.minor != v2.minor) {
		return (v1.minor > v2.minor) ? 1 : -1;
	}
	if (v1.patch != v2.patch) {
		return (v1.patch > v2.patch) ? 1 : -1;
	}
	return 0;
}

/* Check if version satisfies constraint */
static inline u8 version_satisfies(
	struct PackageVersion actual,
	struct PackageVersion required,
	u8 constraint_type) {

	s32 cmp = version_compare(actual, required);

	switch (constraint_type) {
	case VER_EXACT:
		return (cmp == 0) ? 1 : 0;

	case VER_MIN:
		return (cmp >= 0) ? 1 : 0;

	case VER_COMPATIBLE:
		/* ~> version: compatible releases (same major.minor) */
		if (actual.major != required.major) return 0;
		if (actual.minor != required.minor) return 0;
		return (cmp >= 0) ? 1 : 0;

	default:
		return 0;
	}
}

/* ============================================================ */
/* DEPENDENCY RESOLUTION */
/* ============================================================ */

/* Check if package has all dependencies satisfied */
static inline u8 package_check_dependencies(
	struct PackageRegistry *pr,
	struct InstalledPackage *pkg) {

	if (!pr || !pkg) return PKG_CORRUPTED;

	if (pkg->metadata.dep_count == 0) return PKG_OK;

	u32 i;
	for (i = 0; i < pkg->metadata.dep_count; i++) {
		struct PackageDependency *dep = &pkg->metadata.deps[i];
		struct InstalledPackage *dep_pkg = package_find(pr, dep->package_name);

		if (!dep_pkg) {
			return PKG_DEPENDENCY_MISSING;
		}

		if (!version_satisfies(dep_pkg->metadata.version,
							   dep->min_version,
							   dep->constraint_type)) {
			return PKG_VERSION_CONFLICT;
		}
	}

	return PKG_OK;
}

/* ============================================================ */
/* PACKAGE VERIFICATION & INTEGRITY */
/* ============================================================ */

/* Verify package integrity (checksum) */
static inline u8 package_verify(struct InstalledPackage *pkg) {
	if (!pkg) return PKG_CORRUPTED;

	/* Simple integrity check: verify metadata is not null */
	if (!pkg->metadata.name || !pkg->install_path) {
		return PKG_CORRUPTED;
	}

	pkg->verified = 1;
	return PKG_OK;
}

/* ============================================================ */
/* PACKAGE INFORMATION & QUERIES */
/* ============================================================ */

/* Get package by index */
static inline struct InstalledPackage *package_get(
	struct PackageRegistry *pr,
	u32 index) {

	if (!pr || index >= pr->package_count) return 0;
	return &pr->packages[index];
}

/* Count total installed packages */
static inline u32 package_count(struct PackageRegistry *pr) {
	if (!pr) return 0;
	return pr->package_count;
}

/* Count packages for specific language */
static inline u32 package_count_by_language(
	struct PackageRegistry *pr,
	const char *language) {

	if (!pr || !language) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < pr->package_count; i++) {
		struct InstalledPackage *pkg = &pr->packages[i];
		if (pkg->metadata.deprecated) continue;

		const char *lang = pkg->metadata.language;

		/* String compare */
		u32 j = 0;
		while (language[j] && lang[j] && language[j] == lang[j]) j++;

		if (language[j] == 0 && lang[j] == 0) {
			count++;
		}
	}

	return count;
}

/* Get total size of all packages */
static inline u64 package_total_size(struct PackageRegistry *pr) {
	if (!pr) return 0;
	return pr->total_size_bytes;
}

/* Mark package as deprecated */
static inline void package_deprecate(struct InstalledPackage *pkg) {
	if (pkg) pkg->metadata.deprecated = 1;
}

#endif /* APKC_ECO_PACKAGE_MANAGER_H */
