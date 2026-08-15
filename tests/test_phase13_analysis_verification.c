/* test_phase13_analysis_verification.c — Phase 13 Analysis & Verification Tests
 *
 * Comprehensive test suite for:
 * - Stage 13.1: Module Analysis
 * - Stage 13.2: Binary Compatibility Checking
 * - Stage 13.3: Performance Profiling Metadata
 * - Stage 13.4: Security Audit Reports
 *
 * FREESTANDING: No malloc, no libc.
 */

#include <stdio.h>
#include <string.h>

/* Include Phase 13 headers */
#include "Apkc/ana_module_analysis.h"
#include "Apkc/compat_binary_check.h"
#include "Apkc/prof_metrics_metadata.h"
#include "Apkc/audit_security_report.h"

/* ============================================================ */
/* STAGE 13.1: MODULE ANALYSIS TESTS */
/* ============================================================ */

static int test_analysis_init_graph(void) {
	struct ModuleGraph graph = {0};
	analysis_init_graph(&graph);

	if (graph.module_count != 0) {
		printf("❌ test_analysis_init_graph: module count not zeroed\n");
		return 1;
	}
	if (graph.dependency_count != 0) {
		printf("❌ test_analysis_init_graph: dependency count not zeroed\n");
		return 1;
	}

	printf("✓ test_analysis_init_graph\n");
	return 0;
}

static int test_analysis_add_module(void) {
	struct ModuleGraph graph = {0};
	analysis_init_graph(&graph);

	if (analysis_add_module(&graph, "module_a", 1024, 500) != ANALYSIS_OK) {
		printf("❌ test_analysis_add_module: failed to add module\n");
		return 1;
	}

	if (graph.module_count != 1) {
		printf("❌ test_analysis_add_module: count not incremented\n");
		return 1;
	}

	if (graph.modules[0].is_leaf != 1) {
		printf("❌ test_analysis_add_module: new module should be leaf\n");
		return 1;
	}

	printf("✓ test_analysis_add_module\n");
	return 0;
}

static int test_analysis_add_dependency(void) {
	struct ModuleGraph graph = {0};
	analysis_init_graph(&graph);

	analysis_add_module(&graph, "module_a", 1024, 500);
	analysis_add_module(&graph, "module_b", 2048, 700);

	if (analysis_add_dependency(&graph, "module_a", "module_b", 75) != ANALYSIS_OK) {
		printf("❌ test_analysis_add_dependency: failed to add dependency\n");
		return 1;
	}

	if (graph.dependency_count != 1) {
		printf("❌ test_analysis_add_dependency: count not incremented\n");
		return 1;
	}

	if (graph.modules[0].is_leaf != 0) {
		printf("❌ test_analysis_add_dependency: module_a should no longer be leaf\n");
		return 1;
	}

	printf("✓ test_analysis_add_dependency\n");
	return 0;
}

static int test_analysis_detect_cycles(void) {
	struct ModuleGraph graph = {0};
	analysis_init_graph(&graph);

	analysis_add_module(&graph, "pkg1", 1024, 500);
	analysis_add_module(&graph, "pkg2", 2048, 700);
	analysis_add_module(&graph, "pkg3", 512, 300);

	analysis_add_dependency(&graph, "pkg1", "pkg2", 50);
	analysis_add_dependency(&graph, "pkg2", "pkg3", 50);
	analysis_add_dependency(&graph, "pkg3", "pkg1", 50);  /* Creates cycle */

	if (analysis_detect_cycles(&graph, "pkg1", 0) != ANALYSIS_CYCLE_DETECTED) {
		printf("❌ test_analysis_detect_cycles: cycle not detected\n");
		return 1;
	}

	printf("✓ test_analysis_detect_cycles\n");
	return 0;
}

static int test_analysis_compute_depths(void) {
	struct ModuleGraph graph = {0};
	analysis_init_graph(&graph);

	analysis_add_module(&graph, "root", 1024, 500);
	analysis_add_module(&graph, "child1", 2048, 700);
	analysis_add_module(&graph, "child2", 512, 300);

	analysis_add_dependency(&graph, "root", "child1", 50);
	analysis_add_dependency(&graph, "root", "child2", 50);

	analysis_compute_depths(&graph);

	if (graph.max_depth != 1) {
		printf("❌ test_analysis_compute_depths: max depth incorrect\n");
		return 1;
	}

	printf("✓ test_analysis_compute_depths\n");
	return 0;
}

static int test_analysis_total_size(void) {
	struct ModuleGraph graph = {0};
	analysis_init_graph(&graph);

	analysis_add_module(&graph, "mod1", 1024, 500);
	analysis_add_module(&graph, "mod2", 2048, 700);

	u64 total = analysis_total_size(&graph);
	if (total != 3072) {
		printf("❌ test_analysis_total_size: total size incorrect\n");
		return 1;
	}

	printf("✓ test_analysis_total_size\n");
	return 0;
}

/* ============================================================ */
/* STAGE 13.2: BINARY COMPATIBILITY TESTS */
/* ============================================================ */

static int test_compat_init_checker(void) {
	struct CompatibilityChecker checker = {0};
	compat_init_checker(&checker, ARCH_ARM64);

	if (checker.check_count != 0) {
		printf("❌ test_compat_init_checker: check count not zeroed\n");
		return 1;
	}
	if (checker.current_target.target_arch != ARCH_ARM64) {
		printf("❌ test_compat_init_checker: arch not set\n");
		return 1;
	}

	printf("✓ test_compat_init_checker\n");
	return 0;
}

static int test_compat_check_arch(void) {
	if (compat_check_arch(ARCH_ARM64, ARCH_ARM64) != COMPAT_OK) {
		printf("❌ test_compat_check_arch: same arch should be compatible\n");
		return 1;
	}

	if (compat_check_arch(ARCH_ARM64, ARCH_X86_64) != COMPAT_ARCH_MISMATCH) {
		printf("❌ test_compat_check_arch: different arch should be incompatible\n");
		return 1;
	}

	printf("✓ test_compat_check_arch\n");
	return 0;
}

static int test_compat_check_features(void) {
	u8 required = COMPAT_FEATURE_BASE | COMPAT_FEATURE_NEON;
	u8 provided = COMPAT_FEATURE_BASE | COMPAT_FEATURE_NEON | COMPAT_FEATURE_FMA;

	if (compat_check_features(required, provided) != COMPAT_OK) {
		printf("❌ test_compat_check_features: features should be compatible\n");
		return 1;
	}

	u8 missing = COMPAT_FEATURE_BASE | COMPAT_FEATURE_SVE;
	if (compat_check_features(missing, provided) != COMPAT_FEATURE_MISSING) {
		printf("❌ test_compat_check_features: missing feature should be detected\n");
		return 1;
	}

	printf("✓ test_compat_check_features\n");
	return 0;
}

static int test_compat_version_compare(void) {
	struct SymbolVersion v1 = {1, 0, 0};
	struct SymbolVersion v2 = {2, 0, 0};

	s32 cmp = compat_version_compare(v1, v2);
	if (cmp >= 0) {
		printf("❌ test_compat_version_compare: v1 should be less than v2\n");
		return 1;
	}

	cmp = compat_version_compare(v2, v1);
	if (cmp <= 0) {
		printf("❌ test_compat_version_compare: v2 should be greater than v1\n");
		return 1;
	}

	struct SymbolVersion v3 = {1, 0, 0};
	cmp = compat_version_compare(v1, v3);
	if (cmp != 0) {
		printf("❌ test_compat_version_compare: equal versions should return 0\n");
		return 1;
	}

	printf("✓ test_compat_version_compare\n");
	return 0;
}

static int test_compat_check_version(void) {
	struct SymbolVersion required = {1, 0, 0};
	struct SymbolVersion provided = {1, 5, 0};

	if (compat_check_version(required, provided) != COMPAT_OK) {
		printf("❌ test_compat_check_version: provided should satisfy required\n");
		return 1;
	}

	if (compat_check_version(provided, required) != COMPAT_VERSION_CONFLICT) {
		printf("❌ test_compat_check_version: lower version should conflict\n");
		return 1;
	}

	printf("✓ test_compat_check_version\n");
	return 0;
}

/* ============================================================ */
/* STAGE 13.3: PROFILING METADATA TESTS */
/* ============================================================ */

static int test_prof_init_metadata(void) {
	struct ProfilingMetadata prof = {0};
	prof_init_metadata(&prof, "test_module");

	if (prof.function_count != 0) {
		printf("❌ test_prof_init_metadata: function count not zeroed\n");
		return 1;
	}
	if (prof.module_name == 0) {
		printf("❌ test_prof_init_metadata: module name not set\n");
		return 1;
	}

	printf("✓ test_prof_init_metadata\n");
	return 0;
}

static int test_prof_record_function_call(void) {
	struct ProfilingMetadata prof = {0};
	prof_init_metadata(&prof, "test");

	if (prof_record_function_call(&prof, "func_a", 1000, 50) != PROF_OK) {
		printf("❌ test_prof_record_function_call: failed to record\n");
		return 1;
	}

	if (prof.function_count != 1) {
		printf("❌ test_prof_record_function_call: function count not incremented\n");
		return 1;
	}

	if (prof.functions[0].call_count != 1) {
		printf("❌ test_prof_record_function_call: call count not set\n");
		return 1;
	}

	printf("✓ test_prof_record_function_call\n");
	return 0;
}

static int test_prof_record_function_duplicate(void) {
	struct ProfilingMetadata prof = {0};
	prof_init_metadata(&prof, "test");

	prof_record_function_call(&prof, "func_a", 1000, 50);
	prof_record_function_call(&prof, "func_a", 2000, 75);  /* Same function */

	if (prof.function_count != 1) {
		printf("❌ test_prof_record_function_duplicate: should not create new entry\n");
		return 1;
	}

	if (prof.functions[0].call_count != 2) {
		printf("❌ test_prof_record_function_duplicate: call count not updated\n");
		return 1;
	}

	if (prof.functions[0].total_cycles != 3000) {
		printf("❌ test_prof_record_function_duplicate: total cycles not summed\n");
		return 1;
	}

	printf("✓ test_prof_record_function_duplicate\n");
	return 0;
}

static int test_prof_average_function_duration(void) {
	struct ProfilingMetadata prof = {0};
	prof_init_metadata(&prof, "test");

	prof_record_function_call(&prof, "func_a", 1000, 50);
	prof_record_function_call(&prof, "func_b", 2000, 75);

	u32 avg = prof_average_function_duration(&prof);
	if (avg != 1500) {
		printf("❌ test_prof_average_function_duration: average incorrect\n");
		return 1;
	}

	printf("✓ test_prof_average_function_duration\n");
	return 0;
}

/* ============================================================ */
/* STAGE 13.4: SECURITY AUDIT TESTS */
/* ============================================================ */

static int test_audit_init_report(void) {
	struct SecurityAuditReport report = {0};
	audit_init_report(&report, "test_module", "2026-08-15");

	if (report.finding_count != 0) {
		printf("❌ test_audit_init_report: finding count not zeroed\n");
		return 1;
	}
	if (report.overall_status != AUDIT_OK) {
		printf("❌ test_audit_init_report: status not OK\n");
		return 1;
	}

	printf("✓ test_audit_init_report\n");
	return 0;
}

static int test_audit_add_finding(void) {
	struct SecurityAuditReport report = {0};
	audit_init_report(&report, "test", "2026-08-15");

	if (audit_add_finding(&report, VULN_BUFFER_OVERFLOW, SEV_HIGH,
			"Potential buffer overflow", "main.c:42", "Add bounds checking") != AUDIT_OK) {
		printf("❌ test_audit_add_finding: failed to add finding\n");
		return 1;
	}

	if (report.finding_count != 1) {
		printf("❌ test_audit_add_finding: count not incremented\n");
		return 1;
	}

	if (report.high_count != 1) {
		printf("❌ test_audit_add_finding: high count not incremented\n");
		return 1;
	}

	printf("✓ test_audit_add_finding\n");
	return 0;
}

static int test_audit_add_metric(void) {
	struct SecurityAuditReport report = {0};
	audit_init_report(&report, "test", "2026-08-15");

	if (audit_add_metric(&report, "cyclomatic_complexity", 25, 20) != AUDIT_OK) {
		printf("❌ test_audit_add_metric: failed to add metric\n");
		return 1;
	}

	if (report.metric_count != 1) {
		printf("❌ test_audit_add_metric: count not incremented\n");
		return 1;
	}

	if (report.metrics[0].passes != 0) {
		printf("❌ test_audit_add_metric: metric should not pass (25 > 20)\n");
		return 1;
	}

	printf("✓ test_audit_add_metric\n");
	return 0;
}

static int test_audit_verify_access(void) {
	struct SecurityAuditReport report = {0};
	audit_init_report(&report, "test", "2026-08-15");

	if (audit_verify_access(&report, "file_read", "READ_PERMISSION", 1) != AUDIT_OK) {
		printf("❌ test_audit_verify_access: failed to verify access\n");
		return 1;
	}

	if (report.permission_count != 1) {
		printf("❌ test_audit_verify_access: count not incremented\n");
		return 1;
	}

	printf("✓ test_audit_verify_access\n");
	return 0;
}

static int test_audit_risk_score(void) {
	struct SecurityAuditReport report = {0};
	audit_init_report(&report, "test", "2026-08-15");

	audit_add_finding(&report, VULN_BUFFER_OVERFLOW, SEV_CRITICAL, "desc", "loc", "fix");
	audit_add_finding(&report, VULN_USE_AFTER_FREE, SEV_HIGH, "desc", "loc", "fix");

	u32 score = audit_risk_score(&report);
	/* Critical = 40, High = 20, so total = 60 */
	if (score != 60) {
		printf("❌ test_audit_risk_score: risk score incorrect (expected 60, got %u)\n", score);
		return 1;
	}

	printf("✓ test_audit_risk_score\n");
	return 0;
}

static int test_audit_passed(void) {
	struct SecurityAuditReport report = {0};
	audit_init_report(&report, "test", "2026-08-15");

	if (!audit_passed(&report)) {
		printf("❌ test_audit_passed: empty report should pass\n");
		return 1;
	}

	audit_add_finding(&report, VULN_BUFFER_OVERFLOW, SEV_HIGH, "desc", "loc", "fix");

	if (audit_passed(&report)) {
		printf("❌ test_audit_passed: report with HIGH should not pass\n");
		return 1;
	}

	printf("✓ test_audit_passed\n");
	return 0;
}

/* ============================================================ */
/* MAIN TEST RUNNER */
/* ============================================================ */

int main(void) {
	printf("=== Phase 13: Module Analysis & Verification Tests ===\n\n");

	int failed = 0;

	printf("Stage 13.1: Module Analysis\n");
	failed += test_analysis_init_graph();
	failed += test_analysis_add_module();
	failed += test_analysis_add_dependency();
	failed += test_analysis_detect_cycles();
	failed += test_analysis_compute_depths();
	failed += test_analysis_total_size();

	printf("\nStage 13.2: Binary Compatibility Checking\n");
	failed += test_compat_init_checker();
	failed += test_compat_check_arch();
	failed += test_compat_check_features();
	failed += test_compat_version_compare();
	failed += test_compat_check_version();

	printf("\nStage 13.3: Performance Profiling Metadata\n");
	failed += test_prof_init_metadata();
	failed += test_prof_record_function_call();
	failed += test_prof_record_function_duplicate();
	failed += test_prof_average_function_duration();

	printf("\nStage 13.4: Security Audit Reports\n");
	failed += test_audit_init_report();
	failed += test_audit_add_finding();
	failed += test_audit_add_metric();
	failed += test_audit_verify_access();
	failed += test_audit_risk_score();
	failed += test_audit_passed();

	printf("\n=== All Phase 13 tests completed ===\n");
	return failed;
}
