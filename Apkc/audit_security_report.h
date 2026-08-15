/* audit_security_report.h — Security Audit Reports (Stage 13.4)
 *
 * Security vulnerability scanning: detect common issues.
 * Code quality metrics: complexity, maintainability indices.
 * Access control verification: ensure proper permission handling.
 * Dependency audit: check for vulnerable or outdated packages.
 * Compliance reporting: generate audit trail for security review.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_AUDIT_SECURITY_REPORT_H
#define APKC_AUDIT_SECURITY_REPORT_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Audit status */
enum AuditStatus {
	AUDIT_OK = 0,               /* Audit completed successfully */
	AUDIT_WARNINGS = 1,         /* Audit found warnings */
	AUDIT_CRITICAL = 2,         /* Critical issues found */
	AUDIT_INCOMPLETE = 3,       /* Audit not complete */
	AUDIT_INVALID_DATA = 4,     /* Invalid audit data */
	AUDIT_BUFFER_OVERFLOW = 5   /* Audit buffer exceeded */
};

/* Vulnerability severity */
enum Severity {
	SEV_INFO = 0,               /* Informational */
	SEV_LOW = 1,                /* Low impact */
	SEV_MEDIUM = 2,             /* Medium impact */
	SEV_HIGH = 3,               /* High impact */
	SEV_CRITICAL = 4            /* Critical impact */
};

/* Vulnerability type */
enum VulnerabilityType {
	VULN_BUFFER_OVERFLOW = 1,   /* Buffer overflow risk */
	VULN_USE_AFTER_FREE = 2,    /* Use-after-free risk */
	VULN_INTEGER_OVERFLOW = 3,  /* Integer overflow risk */
	VULN_UNINITIALIZED = 4,     /* Uninitialized variable */
	VULN_LOGIC_ERROR = 5,       /* Logic error detected */
	VULN_HARDCODED_SECRET = 6,  /* Hardcoded credentials */
	VULN_SQL_INJECTION = 7,     /* SQL injection risk */
	VULN_XSS = 8,               /* Cross-site scripting risk */
	VULN_COMMAND_INJECTION = 9, /* Command injection risk */
	VULN_MISSING_VALIDATION = 10 /* Input validation missing */
};

/* Security finding */
struct SecurityFinding {
	u8 vulnerability_type;      /* VulnerabilityType */
	u8 severity;                /* Severity level */
	const char *description;    /* Vulnerability description */
	const char *file_location;  /* File and line number */
	const char *remediation;    /* How to fix */
	u32 evidence_count;         /* Number of code locations */
};

/* Code quality metric */
struct CodeQualityMetric {
	const char *metric_name;    /* Metric name */
	u32 value;                  /* Metric value */
	u32 threshold;              /* Acceptable threshold */
	u8 passes;                  /* 1 if passes threshold */
};

/* Access control check */
struct AccessControlCheck {
	const char *resource_name;  /* Resource being accessed */
	const char *required_perm;  /* Required permission */
	u8 is_verified;             /* 1 if permission verified */
	u8 is_enforced;             /* 1 if permission enforced */
};

/* Dependency vulnerability info */
struct DepVulnerability {
	const char *package_name;   /* Package with vulnerability */
	const char *vuln_id;        /* CVE or vulnerability ID */
	const char *fixed_version;  /* Version with fix */
	u8 severity;                /* Severity level */
};

/* Complete security audit report */
struct SecurityAuditReport {
	const char *module_name;    /* Module being audited */
	const char *audit_date;     /* Audit timestamp */

	struct SecurityFinding findings[32];    /* Up to 32 findings */
	u32 finding_count;

	struct CodeQualityMetric metrics[16];   /* Up to 16 metrics */
	u32 metric_count;

	struct AccessControlCheck permissions[16]; /* Up to 16 permission checks */
	u32 permission_count;

	struct DepVulnerability dep_vulns[16];  /* Up to 16 dependency vulns */
	u32 dep_vuln_count;

	u8 overall_status;          /* AuditStatus */
	u32 critical_count;
	u32 high_count;
	u32 medium_count;
	u32 low_count;
	u32 info_count;
};

/* ============================================================ */
/* AUDIT REPORT INITIALIZATION */
/* ============================================================ */

/* Initialize security audit report */
static inline void audit_init_report(
	struct SecurityAuditReport *report,
	const char *module_name,
	const char *audit_date) {

	if (!report) return;
	report->module_name = module_name;
	report->audit_date = audit_date;
	report->finding_count = 0;
	report->metric_count = 0;
	report->permission_count = 0;
	report->dep_vuln_count = 0;
	report->overall_status = AUDIT_OK;
	report->critical_count = 0;
	report->high_count = 0;
	report->medium_count = 0;
	report->low_count = 0;
	report->info_count = 0;
}

/* ============================================================ */
/* VULNERABILITY FINDING RECORDING */
/* ============================================================ */

/* Record security finding */
static inline u8 audit_add_finding(
	struct SecurityAuditReport *report,
	u8 vuln_type,
	u8 severity,
	const char *description,
	const char *file_location,
	const char *remediation) {

	if (!report || !description) return AUDIT_INVALID_DATA;
	if (report->finding_count >= 32) return AUDIT_BUFFER_OVERFLOW;

	struct SecurityFinding *finding = &report->findings[report->finding_count];
	finding->vulnerability_type = vuln_type;
	finding->severity = severity;
	finding->description = description;
	finding->file_location = file_location;
	finding->remediation = remediation;
	finding->evidence_count = 1;

	/* Update severity counters */
	switch (severity) {
	case SEV_CRITICAL:
		report->critical_count++;
		if (report->overall_status < AUDIT_CRITICAL) report->overall_status = AUDIT_CRITICAL;
		break;
	case SEV_HIGH:
		report->high_count++;
		if (report->overall_status < AUDIT_CRITICAL) report->overall_status = AUDIT_WARNINGS;
		break;
	case SEV_MEDIUM:
		report->medium_count++;
		if (report->overall_status == AUDIT_OK) report->overall_status = AUDIT_WARNINGS;
		break;
	case SEV_LOW:
		report->low_count++;
		if (report->overall_status == AUDIT_OK) report->overall_status = AUDIT_WARNINGS;
		break;
	case SEV_INFO:
		report->info_count++;
		break;
	}

	report->finding_count++;
	return AUDIT_OK;
}

/* ============================================================ */
/* CODE QUALITY METRICS */
/* ============================================================ */

/* Record code quality metric */
static inline u8 audit_add_metric(
	struct SecurityAuditReport *report,
	const char *metric_name,
	u32 value,
	u32 threshold) {

	if (!report || !metric_name) return AUDIT_INVALID_DATA;
	if (report->metric_count >= 16) return AUDIT_BUFFER_OVERFLOW;

	struct CodeQualityMetric *metric = &report->metrics[report->metric_count];
	metric->metric_name = metric_name;
	metric->value = value;
	metric->threshold = threshold;
	metric->passes = (value <= threshold) ? 1 : 0;

	if (!metric->passes && report->overall_status == AUDIT_OK) {
		report->overall_status = AUDIT_WARNINGS;
	}

	report->metric_count++;
	return AUDIT_OK;
}

/* ============================================================ */
/* ACCESS CONTROL VERIFICATION */
/* ============================================================ */

/* Verify access control for resource */
static inline u8 audit_verify_access(
	struct SecurityAuditReport *report,
	const char *resource_name,
	const char *required_perm,
	u8 is_enforced) {

	if (!report || !resource_name || !required_perm) return AUDIT_INVALID_DATA;
	if (report->permission_count >= 16) return AUDIT_BUFFER_OVERFLOW;

	struct AccessControlCheck *check = &report->permissions[report->permission_count];
	check->resource_name = resource_name;
	check->required_perm = required_perm;
	check->is_verified = 1;
	check->is_enforced = is_enforced;

	if (!is_enforced) {
		/* Missing enforcement is a finding */
		audit_add_finding(report, VULN_MISSING_VALIDATION, SEV_MEDIUM,
			"Access control not enforced", resource_name,
			"Add runtime permission check");
	}

	report->permission_count++;
	return AUDIT_OK;
}

/* ============================================================ */
/* DEPENDENCY VULNERABILITY TRACKING */
/* ============================================================ */

/* Record dependency vulnerability */
static inline u8 audit_add_dep_vuln(
	struct SecurityAuditReport *report,
	const char *package_name,
	const char *vuln_id,
	const char *fixed_version,
	u8 severity) {

	if (!report || !package_name || !vuln_id) return AUDIT_INVALID_DATA;
	if (report->dep_vuln_count >= 16) return AUDIT_BUFFER_OVERFLOW;

	struct DepVulnerability *vuln = &report->dep_vulns[report->dep_vuln_count];
	vuln->package_name = package_name;
	vuln->vuln_id = vuln_id;
	vuln->fixed_version = fixed_version;
	vuln->severity = severity;

	/* Add as finding too */
	audit_add_finding(report, VULN_LOGIC_ERROR, severity,
		"Vulnerable dependency detected", package_name,
		"Update to fixed version");

	report->dep_vuln_count++;
	return AUDIT_OK;
}

/* ============================================================ */
/* AUDIT STATISTICS & REPORTING */
/* ============================================================ */

/* Get total vulnerabilities found */
static inline u32 audit_total_vulns(struct SecurityAuditReport *report) {
	if (!report) return 0;
	return report->critical_count + report->high_count +
	       report->medium_count + report->low_count;
}

/* Get total issues (vulns + low info) */
static inline u32 audit_total_issues(struct SecurityAuditReport *report) {
	if (!report) return 0;
	return audit_total_vulns(report) + report->info_count;
}

/* Get code quality pass rate */
static inline u32 audit_quality_pass_rate(struct SecurityAuditReport *report) {
	if (!report || report->metric_count == 0) return 100;

	u32 passes = 0;
	u32 i;
	for (i = 0; i < report->metric_count; i++) {
		if (report->metrics[i].passes) passes++;
	}

	return (passes * 100) / report->metric_count;
}

/* Get access control coverage */
static inline u32 audit_access_coverage(struct SecurityAuditReport *report) {
	if (!report || report->permission_count == 0) return 0;

	u32 verified = 0;
	u32 i;
	for (i = 0; i < report->permission_count; i++) {
		if (report->permissions[i].is_verified) verified++;
	}

	return (verified * 100) / report->permission_count;
}

/* Get enforcement rate */
static inline u32 audit_enforcement_rate(struct SecurityAuditReport *report) {
	if (!report || report->permission_count == 0) return 0;

	u32 enforced = 0;
	u32 i;
	for (i = 0; i < report->permission_count; i++) {
		if (report->permissions[i].is_enforced) enforced++;
	}

	return (enforced * 100) / report->permission_count;
}

/* Get vulnerability risk score (0-100, higher is worse) */
static inline u32 audit_risk_score(struct SecurityAuditReport *report) {
	if (!report) return 0;

	/* Weighted scoring: critical=40, high=20, medium=10, low=5, info=1 */
	u32 score = 0;
	score += (report->critical_count * 40);
	score += (report->high_count * 20);
	score += (report->medium_count * 10);
	score += (report->low_count * 5);
	score += (report->info_count * 1);

	/* Cap at 100 */
	return (score > 100) ? 100 : score;
}

/* Check if audit passed (no critical/high issues) */
static inline u8 audit_passed(struct SecurityAuditReport *report) {
	if (!report) return 0;
	return (report->critical_count == 0 && report->high_count == 0) ? 1 : 0;
}

#endif /* APKC_AUDIT_SECURITY_REPORT_H */
