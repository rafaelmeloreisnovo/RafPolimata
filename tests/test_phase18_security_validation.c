/* test_phase18_security_validation.c — Phase 18 Testing (Stages 18.1–18.4)
 *
 * Comprehensive tests for vulnerability scanning, binary validation,
 * dependency checking, and code attestation.
 *
 * Build: gcc -std=c99 -Wall -O2 -I. -I Apkc tests/test_phase18_security_validation.c -o test_phase18 && ./test_phase18
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Apkc/sec_vulnerability_scanner.h"
#include "Apkc/sec_binary_validation.h"
#include "Apkc/sec_dependency_checker.h"
#include "Apkc/sec_code_attestation.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

static u32 tests_passed = 0;
static u32 tests_failed = 0;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { \
		printf("FAIL: %s\n", msg); \
		tests_failed++; \
		return 0; \
	} \
} while (0)

#define PASS(msg) do { \
	printf("PASS: %s\n", msg); \
	tests_passed++; \
	return 1; \
} while (0)

/* ============================================================ */
/* STAGE 18.1: VULNERABILITY SCANNER TESTS */
/* ============================================================ */

static u8 test_vulnscan_init(void) {
	struct VulnScanner scanner = {0};
	vulnscan_init(&scanner);

	ASSERT(scanner.finding_count == 0, "finding_count initialized");
	ASSERT(scanner.critical_count == 0, "critical_count initialized");
	ASSERT(scanner.next_finding_id == 1, "next_finding_id initialized");
	PASS("vulnscan_init");
}

static u8 test_vulnscan_add_finding(void) {
	struct VulnScanner scanner = {0};
	vulnscan_init(&scanner);

	u8 result = vulnscan_add_finding(&scanner, VULN_BUFFER_OVERFLOW,
		SEVERITY_CRITICAL, "main.c:42", "Stack buffer overflow", "Use bounds checking", 95);

	ASSERT(result == 1, "add_finding returns 1");
	ASSERT(scanner.finding_count == 1, "finding_count incremented");
	ASSERT(scanner.critical_count == 1, "critical_count incremented");
	PASS("vulnscan_add_finding");
}

static u8 test_vulnscan_mark_false_positive(void) {
	struct VulnScanner scanner = {0};
	vulnscan_init(&scanner);

	vulnscan_add_finding(&scanner, VULN_BUFFER_OVERFLOW, SEVERITY_HIGH, "test.c:10", "desc", "fix", 80);
	u8 result = vulnscan_mark_false_positive(&scanner, 1);

	ASSERT(result == 1, "mark_false_positive returns 1");
	ASSERT(scanner.findings[0].is_false_positive == 1, "is_false_positive set");
	ASSERT(scanner.false_positives == 1, "false_positives incremented");
	PASS("vulnscan_mark_false_positive");
}

static u8 test_vulnscan_count_by_severity(void) {
	struct VulnScanner scanner = {0};
	vulnscan_init(&scanner);

	vulnscan_add_finding(&scanner, VULN_BUFFER_OVERFLOW, SEVERITY_CRITICAL, "a.c:1", "d", "f", 90);
	vulnscan_add_finding(&scanner, VULN_SQL_INJECTION, SEVERITY_CRITICAL, "b.c:2", "d", "f", 85);
	vulnscan_add_finding(&scanner, VULN_XSS, SEVERITY_HIGH, "c.c:3", "d", "f", 80);

	u32 critical_count = vulnscan_count_by_severity(&scanner, SEVERITY_CRITICAL);
	u32 high_count = vulnscan_count_by_severity(&scanner, SEVERITY_HIGH);

	ASSERT(critical_count == 2, "count_by_severity counts critical correctly");
	ASSERT(high_count == 1, "count_by_severity counts high correctly");
	PASS("vulnscan_count_by_severity");
}

static u8 test_vulnscan_has_critical(void) {
	struct VulnScanner scanner = {0};
	vulnscan_init(&scanner);

	vulnscan_add_finding(&scanner, VULN_BUFFER_OVERFLOW, SEVERITY_CRITICAL, "a.c:1", "d", "f", 90);

	u8 has_crit = vulnscan_has_critical(&scanner);

	ASSERT(has_crit == 1, "has_critical returns 1 when critical exists");
	PASS("vulnscan_has_critical");
}

/* ============================================================ */
/* STAGE 18.2: BINARY VALIDATION TESTS */
/* ============================================================ */

static u8 test_binval_init(void) {
	struct BinaryValidator validator = {0};
	binval_init(&validator, "test.so");

	ASSERT(validator.binary_path != 0, "binary_path set");
	ASSERT(validator.overall_status == BINARY_VALID, "overall_status initialized");
	ASSERT(validator.validated_sections == 0, "validated_sections initialized");
	PASS("binval_init");
}

static u8 test_binval_validate_section(void) {
	struct BinaryValidator validator = {0};
	binval_init(&validator, "test.elf");

	u8 result = binval_validate_section(&validator, ".text", 0x1000, 4096, 0xdeadbeef, 1, 1);

	ASSERT(result == 1, "validate_section returns 1");
	ASSERT(validator.validated_sections == 1, "validated_sections incremented");
	ASSERT(validator.sections[0].is_executable == 1, "is_executable set");
	PASS("binval_validate_section");
}

static u8 test_binval_validate_elf(void) {
	struct BinaryValidator validator = {0};
	binval_init(&validator, "test.elf");

	u8 result = binval_validate_elf(&validator);

	ASSERT(result == 1, "validate_elf returns 1");
	ASSERT(validator.binary_type == 1, "binary_type set to ELF");
	PASS("binval_validate_elf");
}

static u8 test_binval_verify_checksum(void) {
	struct BinaryValidator validator = {0};
	binval_init(&validator, "test.so");

	binval_compute_checksum(&validator, 0x12345678);
	u8 matches = binval_verify_checksum(&validator, 0x12345678);

	ASSERT(matches == 1, "verify_checksum returns 1 for matching checksums");
	PASS("binval_verify_checksum");
}

static u8 test_binval_is_valid(void) {
	struct BinaryValidator validator = {0};
	binval_init(&validator, "test.so");

	u8 valid = binval_is_valid(&validator);

	ASSERT(valid == 1, "is_valid returns 1 for valid binary");
	PASS("binval_is_valid");
}

/* ============================================================ */
/* STAGE 18.3: DEPENDENCY CHECKER TESTS */
/* ============================================================ */

static u8 test_depcheck_init(void) {
	struct DepChecker checker = {0};
	depcheck_init(&checker);

	ASSERT(checker.dependency_count == 0, "dependency_count initialized");
	ASSERT(checker.total_vulnerabilities == 0, "total_vulnerabilities initialized");
	PASS("depcheck_init");
}

static u8 test_depcheck_add_dependency(void) {
	struct DepChecker checker = {0};
	depcheck_init(&checker);

	u8 result = depcheck_add_dependency(&checker, "openssl", "1.0.2");

	ASSERT(result == 1, "add_dependency returns 1");
	ASSERT(checker.dependency_count == 1, "dependency_count incremented");
	PASS("depcheck_add_dependency");
}

static u8 test_depcheck_add_known_vuln(void) {
	struct DepChecker checker = {0};
	depcheck_init(&checker);

	u8 result = depcheck_add_known_vuln(&checker, "CVE-2021-1234", "openssl", "1.0.0", "1.0.2", SEVERITY_CRITICAL);

	ASSERT(result == 1, "add_known_vuln returns 1");
	ASSERT(checker.known_vuln_count == 1, "known_vuln_count incremented");
	PASS("depcheck_add_known_vuln");
}

static u8 test_depcheck_is_vulnerable(void) {
	struct DepChecker checker = {0};
	depcheck_init(&checker);

	depcheck_add_dependency(&checker, "openssl", "1.0.1");
	depcheck_add_known_vuln(&checker, "CVE-2021-1234", "openssl", "1.0.0", "1.0.2", SEVERITY_CRITICAL);

	u8 vuln = depcheck_is_vulnerable(&checker, 0);

	ASSERT(vuln == 1, "is_vulnerable detects vulnerable dependency");
	PASS("depcheck_is_vulnerable");
}

static u8 test_depcheck_report_issue(void) {
	struct DepChecker checker = {0};
	depcheck_init(&checker);
	depcheck_add_dependency(&checker, "openssl", "1.0.1");

	u8 result = depcheck_report_issue(&checker, 0, "outdated", "1.1.1", 1);

	ASSERT(result == 1, "report_issue returns 1");
	ASSERT(checker.issue_count == 1, "issue_count incremented");
	ASSERT(checker.critical_vulnerabilities == 1, "critical_vulnerabilities incremented");
	PASS("depcheck_report_issue");
}

static u8 test_depcheck_are_secure(void) {
	struct DepChecker checker = {0};
	depcheck_init(&checker);
	depcheck_add_dependency(&checker, "openssl", "1.1.1");

	u8 secure = depcheck_are_secure(&checker);

	ASSERT(secure == 1, "are_secure returns 1 with no critical issues");
	PASS("depcheck_are_secure");
}

/* ============================================================ */
/* STAGE 18.4: CODE ATTESTATION TESTS */
/* ============================================================ */

static u8 test_attest_init(void) {
	struct AttestationManager mgr = {0};
	attest_init(&mgr);

	ASSERT(mgr.certificate_count == 0, "certificate_count initialized");
	ASSERT(mgr.signature_count == 0, "signature_count initialized");
	ASSERT(mgr.verified_count == 0, "verified_count initialized");
	PASS("attest_init");
}

static u8 test_attest_add_certificate(void) {
	struct AttestationManager mgr = {0};
	attest_init(&mgr);

	u8 result = attest_add_certificate(&mgr, "CN=Test", "CN=Root", 100, 1000, 2048);

	ASSERT(result == 1, "add_certificate returns 1");
	ASSERT(mgr.certificate_count == 1, "certificate_count incremented");
	PASS("attest_add_certificate");
}

static u8 test_attest_trust_certificate(void) {
	struct AttestationManager mgr = {0};
	attest_init(&mgr);
	attest_add_certificate(&mgr, "CN=Test", "CN=Root", 100, 1000, 2048);

	u8 result = attest_trust_certificate(&mgr, 0);

	ASSERT(result == 1, "trust_certificate returns 1");
	ASSERT(mgr.certificates[0].is_trusted == 1, "is_trusted set");
	PASS("attest_trust_certificate");
}

static u8 test_attest_sign_code(void) {
	struct AttestationManager mgr = {0};
	attest_init(&mgr);
	attest_add_certificate(&mgr, "CN=Test", "CN=Root", 100, 1000, 2048);

	u8 result = attest_sign_code(&mgr, "abcd1234", "signature_data", 0);

	ASSERT(result == 1, "sign_code returns 1");
	ASSERT(mgr.signature_count == 1, "signature_count incremented");
	PASS("attest_sign_code");
}

static u8 test_attest_record_provenance(void) {
	struct AttestationManager mgr = {0};
	attest_init(&mgr);

	u8 result = attest_record_provenance(&mgr, "https://github.com/test", "abc123", "gcc", "9.3.0");

	ASSERT(result == 1, "record_provenance returns 1");
	ASSERT(mgr.provenance_count == 1, "provenance_count incremented");
	PASS("attest_record_provenance");
}

static u8 test_attest_mark_reproducible(void) {
	struct AttestationManager mgr = {0};
	attest_init(&mgr);
	attest_record_provenance(&mgr, "repo", "commit", "gcc", "9.3.0");

	u8 result = attest_mark_reproducible(&mgr, 0);

	ASSERT(result == 1, "mark_reproducible returns 1");
	ASSERT(mgr.provenances[0].is_reproducible == 1, "is_reproducible set");
	PASS("attest_mark_reproducible");
}

static u8 test_attest_create_attestation(void) {
	struct AttestationManager mgr = {0};
	attest_init(&mgr);
	attest_add_certificate(&mgr, "CN=Test", "CN=Root", 100, 1000, 2048);
	attest_sign_code(&mgr, "hash", "sig", 0);
	attest_record_provenance(&mgr, "repo", "commit", "gcc", "9.3.0");

	u8 result = attest_create_attestation(&mgr, 0, 0);

	ASSERT(result == 1, "create_attestation returns 1");
	ASSERT(mgr.attestation_count == 1, "attestation_count incremented");
	PASS("attest_create_attestation");
}

static u8 test_attest_all_valid(void) {
	struct AttestationManager mgr = {0};
	attest_init(&mgr);

	u8 all_valid = attest_all_valid(&mgr);

	ASSERT(all_valid == 1, "all_valid returns 1 with no failures");
	PASS("attest_all_valid");
}

int main(void) {
	printf("=== Phase 18: Security & Binary Validation Tests ===\n\n");

	printf("Stage 18.1: Vulnerability Scanner\n");
	test_vulnscan_init();
	test_vulnscan_add_finding();
	test_vulnscan_mark_false_positive();
	test_vulnscan_count_by_severity();
	test_vulnscan_has_critical();

	printf("\nStage 18.2: Binary Validation\n");
	test_binval_init();
	test_binval_validate_section();
	test_binval_validate_elf();
	test_binval_verify_checksum();
	test_binval_is_valid();

	printf("\nStage 18.3: Dependency Checker\n");
	test_depcheck_init();
	test_depcheck_add_dependency();
	test_depcheck_add_known_vuln();
	test_depcheck_is_vulnerable();
	test_depcheck_report_issue();
	test_depcheck_are_secure();

	printf("\nStage 18.4: Code Attestation\n");
	test_attest_init();
	test_attest_add_certificate();
	test_attest_trust_certificate();
	test_attest_sign_code();
	test_attest_record_provenance();
	test_attest_mark_reproducible();
	test_attest_create_attestation();
	test_attest_all_valid();

	printf("\n=== Test Results ===\n");
	printf("Passed: %u\n", tests_passed);
	printf("Failed: %u\n", tests_failed);

	if (tests_failed == 0) {
		printf("\nAll tests PASSED! ✓\n");
		return 0;
	} else {
		printf("\nSome tests FAILED! ✗\n");
		return 1;
	}
}
