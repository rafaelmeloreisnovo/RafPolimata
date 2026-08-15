/* eco_ecosystem_health.h — Ecosystem Health Metrics (Stage 19.4)
 *
 * Health score: calculate ecosystem health based on metrics.
 * Dependency freshness: track how often packages are updated.
 * Security posture: measure vulnerability prevalence.
 * Community metrics: engagement, contributions, maintainer activity.
 * Trend analysis: detect ecosystem improvement or decline.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_ECO_ECOSYSTEM_HEALTH_H
#define APKC_ECO_ECOSYSTEM_HEALTH_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Ecosystem health metrics */
struct HealthMetrics {
	u32 total_packages;         /* Total packages in ecosystem */
	u32 packages_updated_recently; /* Updated in last 30 days */
	u32 packages_with_vulnerabilities; /* Packages with known CVEs */
	u32 critical_vulns;         /* Number of critical vulnerabilities */
	u32 security_patches_available; /* Updates fixing vulnerabilities */
	u32 average_update_frequency; /* Days between updates */
	u32 maintainer_count;       /* Active maintainers */
	u32 contribution_rate;      /* Commits per month (aggregate) */
};

/* Ecosystem health report */
struct EcosystemHealth {
	struct HealthMetrics metrics;
	u32 overall_score;          /* 0-100 health score */
	u8 score_trend;             /* 0=declining, 1=stable, 2=improving */
	u64 last_assessment_time;   /* When metrics last updated */
	u32 vulnerable_packages_critical; /* Count of critical CVEs */
	u32 vulnerable_packages_high; /* Count of high CVEs */
	u8 is_healthy;              /* 1 if score >= 75 */
};

/* Initialize health report */
static inline void ecohealth_init(struct EcosystemHealth *health) {
	if (!health) return;
	health->metrics.total_packages = 0;
	health->metrics.packages_updated_recently = 0;
	health->metrics.packages_with_vulnerabilities = 0;
	health->metrics.critical_vulns = 0;
	health->metrics.security_patches_available = 0;
	health->overall_score = 0;
	health->score_trend = 1;  /* Start as stable */
	health->vulnerable_packages_critical = 0;
	health->vulnerable_packages_high = 0;
	health->is_healthy = 0;
}

/* Update health metrics */
static inline void ecohealth_update_metrics(
	struct EcosystemHealth *health,
	u32 total_pkgs,
	u32 recently_updated,
	u32 with_vulns,
	u32 critical_count) {

	if (!health) return;
	health->metrics.total_packages = total_pkgs;
	health->metrics.packages_updated_recently = recently_updated;
	health->metrics.packages_with_vulnerabilities = with_vulns;
	health->metrics.critical_vulns = critical_count;
	health->last_assessment_time = 0;  /* Would be current time */
}

/* Calculate health score */
static inline u32 ecohealth_calculate_score(struct EcosystemHealth *health) {
	if (!health || health->metrics.total_packages == 0) return 0;

	/* Score based on: update frequency, vulnerability ratio, critical count */
	u32 update_score = health->metrics.packages_updated_recently * 100 /
		(health->metrics.total_packages > 0 ? health->metrics.total_packages : 1);
	u32 vuln_score = 100 - (health->metrics.packages_with_vulnerabilities * 50 /
		(health->metrics.total_packages > 0 ? health->metrics.total_packages : 1));
	u32 critical_score = 100 - (health->metrics.critical_vulns * 10);

	u32 overall = (update_score + vuln_score + critical_score) / 3;
	health->overall_score = overall > 100 ? 100 : overall;
	health->is_healthy = overall >= 75 ? 1 : 0;

	return health->overall_score;
}

/* Assess trend */
static inline void ecohealth_assess_trend(
	struct EcosystemHealth *prev,
	struct EcosystemHealth *current) {

	if (!prev || !current) return;

	if (current->overall_score > prev->overall_score) {
		current->score_trend = 2;  /* Improving */
	} else if (current->overall_score < prev->overall_score) {
		current->score_trend = 0;  /* Declining */
	} else {
		current->score_trend = 1;  /* Stable */
	}
}

/* Check if ecosystem is healthy */
static inline u8 ecohealth_is_healthy(struct EcosystemHealth *health) {
	if (!health) return 0;
	return health->is_healthy;
}

#endif /* APKC_ECO_ECOSYSTEM_HEALTH_H */
