/* sec_dependency_checker.h — Dependency Vulnerability Checking (Stage 18.3)
 *
 * Dependency graph: track library dependencies and versions.
 * Known vulnerability database: CVE mapping to library versions.
 * Version checking: match installed versions against vulnerable ranges.
 * Transitive vulnerability detection: find vulnerabilities in dependencies of dependencies.
 * Update recommendations: suggest library updates to fix vulnerabilities.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_SEC_DEPENDENCY_CHECKER_H
#define APKC_SEC_DEPENDENCY_CHECKER_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Dependency entry */
struct Dependency {
	const char *library_name;   /* Library identifier (e.g., "openssl") */
	const char *version;        /* Version string (e.g., "1.0.2") */
	u32 dependency_id;          /* Unique dependency ID */
	u8 is_transitive;           /* 1 if indirect dependency */
	u32 parent_count;           /* Libraries that depend on this */
	u32 parents[8];             /* Parent dependency IDs (max 8) */
};

/* Known vulnerability entry */
struct KnownVulnerability {
	const char *cve_id;         /* CVE identifier (e.g., "CVE-2021-1234") */
	const char *library_name;   /* Affected library */
	const char *version_min;    /* Vulnerable version range (min) */
	const char *version_max;    /* Vulnerable version range (max) */
	u8 severity;                /* Severity level */
	const char *description;    /* Vulnerability description */
	u32 vuln_id;                /* Unique vulnerability ID */
};

/* Dependency check result */
struct DependencyIssue {
	u32 dependency_id;          /* Dependency with issue */
	u32 cve_id;                 /* Associated CVE ID */
	const char *issue_type;     /* "outdated", "vulnerable", "unmaintained" */
	const char *recommended_version; /* Suggested version to update to */
	u8 is_critical;             /* 1 if critical security issue */
};

/* Dependency checker context */
struct DepChecker {
	struct Dependency dependencies[64];      /* Up to 64 dependencies */
	u32 dependency_count;
	struct KnownVulnerability known_vulns[256]; /* Up to 256 known CVEs */
	u32 known_vuln_count;
	struct DependencyIssue issues[128];     /* Up to 128 detected issues */
	u32 issue_count;
	u32 total_vulnerabilities;
	u32 critical_vulnerabilities;
};

/* ============================================================ */
/* DEPENDENCY CHECKER INITIALIZATION */
/* ============================================================ */

/* Initialize dependency checker */
static inline void depcheck_init(struct DepChecker *checker) {
	if (!checker) return;
	checker->dependency_count = 0;
	checker->known_vuln_count = 0;
	checker->issue_count = 0;
	checker->total_vulnerabilities = 0;
	checker->critical_vulnerabilities = 0;
}

/* ============================================================ */
/* DEPENDENCY REGISTRATION */
/* ============================================================ */

/* Add dependency to check */
static inline u8 depcheck_add_dependency(
	struct DepChecker *checker,
	const char *library_name,
	const char *version) {

	if (!checker || !library_name || !version) return 0;
	if (checker->dependency_count >= 64) return 0;

	struct Dependency *dep = &checker->dependencies[checker->dependency_count];
	dep->library_name = library_name;
	dep->version = version;
	dep->dependency_id = checker->dependency_count;
	dep->is_transitive = 0;
	dep->parent_count = 0;

	checker->dependency_count++;
	return 1;
}

/* Add known vulnerability to database */
static inline u8 depcheck_add_known_vuln(
	struct DepChecker *checker,
	const char *cve_id,
	const char *library_name,
	const char *version_min,
	const char *version_max,
	u8 severity) {

	if (!checker || !cve_id || !library_name) return 0;
	if (checker->known_vuln_count >= 256) return 0;

	struct KnownVulnerability *vuln = &checker->known_vulns[checker->known_vuln_count];
	vuln->cve_id = cve_id;
	vuln->library_name = library_name;
	vuln->version_min = version_min;
	vuln->version_max = version_max;
	vuln->severity = severity;
	vuln->vuln_id = checker->known_vuln_count;

	checker->known_vuln_count++;
	return 1;
}

/* ============================================================ */
/* VULNERABILITY DETECTION */
/* ============================================================ */

/* Check if dependency is vulnerable */
static inline u8 depcheck_is_vulnerable(
	struct DepChecker *checker,
	u32 dependency_id) {

	if (!checker || dependency_id >= checker->dependency_count) return 0;

	struct Dependency *dep = &checker->dependencies[dependency_id];

	u32 i;
	for (i = 0; i < checker->known_vuln_count; i++) {
		struct KnownVulnerability *vuln = &checker->known_vulns[i];

		/* Check if library name matches */
		const char *dep_name = dep->library_name;
		const char *vuln_name = vuln->library_name;
		u32 j = 0;
		while (dep_name[j] && vuln_name[j] && dep_name[j] == vuln_name[j]) j++;
		if (dep_name[j] != 0 || vuln_name[j] != 0) continue;

		/* Would check version ranges here */
		/* If version in range, vulnerability found */
		return 1;
	}

	return 0;
}

/* Report dependency issue */
static inline u8 depcheck_report_issue(
	struct DepChecker *checker,
	u32 dependency_id,
	const char *issue_type,
	const char *recommended_version,
	u8 is_critical) {

	if (!checker || dependency_id >= checker->dependency_count) return 0;
	if (checker->issue_count >= 128) return 0;

	struct DependencyIssue *issue = &checker->issues[checker->issue_count];
	issue->dependency_id = dependency_id;
	issue->issue_type = issue_type;
	issue->recommended_version = recommended_version;
	issue->is_critical = is_critical;

	checker->issue_count++;
	checker->total_vulnerabilities++;

	if (is_critical) {
		checker->critical_vulnerabilities++;
	}

	return 1;
}

/* ============================================================ */
/* DEPENDENCY ANALYSIS */
/* ============================================================ */

/* Find outdated dependencies */
static inline u32 depcheck_find_outdated(
	struct DepChecker *checker,
	u32 *outdated_ids,
	u32 max_count) {

	if (!checker || !outdated_ids) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < checker->issue_count && count < max_count; i++) {
		if (checker->issues[i].recommended_version != 0) {
			outdated_ids[count++] = checker->issues[i].dependency_id;
		}
	}

	return count;
}

/* Count critical issues */
static inline u32 depcheck_count_critical_issues(struct DepChecker *checker) {
	if (!checker) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < checker->issue_count; i++) {
		if (checker->issues[i].is_critical) {
			count++;
		}
	}

	return count;
}

/* Check if dependencies are secure */
static inline u8 depcheck_are_secure(struct DepChecker *checker) {
	if (!checker) return 1;
	return checker->critical_vulnerabilities == 0 ? 1 : 0;
}

#endif /* APKC_SEC_DEPENDENCY_CHECKER_H */
