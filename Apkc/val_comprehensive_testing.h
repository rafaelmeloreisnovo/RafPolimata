/* Apkc/val_comprehensive_testing.h — Phase 49: Comprehensive Testing Framework
 *
 * Freestanding validation suite for all 48 compiler phases.
 * - Unit test registry for phases 1-48
 * - Integration test harness for end-to-end compilation
 * - Regression tests for all 12 language targets
 * - Performance benchmarking (compilation time, binary size, execution speed)
 * - Memory profiling (peak RSS, allocation counts)
 * - Determinism verification (bitwise identical builds)
 *
 * NO malloc/libc — all test data stack-allocated. */

#ifndef APKC_VAL_COMPREHENSIVE_TESTING_H_
#define APKC_VAL_COMPREHENSIVE_TESTING_H_

/* Freestanding: no stdint.h */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef int s32;
typedef long long i64;
typedef __SIZE_TYPE__ sz;

/* ─────────────────────────────────────────────────────────────────────── */
/* Test Result Tracking */
/* ─────────────────────────────────────────────────────────────────────── */

typedef enum {
    TEST_PASS=0,
    TEST_FAIL=1,
    TEST_SKIP=2,
    TEST_TIMEOUT=3,
    TEST_CRASH=4,
} TestStatus;

typedef struct {
    u32 phase_id;           /* 1-48: which phase being tested */
    const char *name;       /* test name */
    TestStatus status;      /* result */
    u64 duration_us;        /* execution time */
    u32 assertion_count;    /* number of assertions */
    u32 assertion_pass;     /* passed assertions */
    const char *error_msg;  /* on failure, error description */
    u32 line_no;            /* source line where test failed */
} TestResult;

typedef struct {
    TestResult results[256];
    u32 result_count;
    u32 pass_count;
    u32 fail_count;
    u32 skip_count;
    u64 total_time_us;
} TestSuite;

/* ─────────────────────────────────────────────────────────────────────── */
/* Phase Coverage Matrix */
/* ─────────────────────────────────────────────────────────────────────── */

typedef struct {
    u32 phase_id;                    /* 1-48 */
    const char *phase_name;
    u32 unit_test_count;
    u32 integration_test_count;
    u32 regression_test_count;
    u8 tested[256];                  /* bitmask of tested scenarios */
    u32 tested_count;
    u32 coverage_percent;            /* (tested_count / 256) * 100 */
} PhaseCoverage;

typedef struct {
    PhaseCoverage phases[48];
    u32 total_phases;
    u32 phases_with_100_coverage;
    u32 average_coverage_percent;
} CoverageReport;

/* ─────────────────────────────────────────────────────────────────────── */
/* Language Regression Tests */
/* ─────────────────────────────────────────────────────────────────────── */

typedef enum {
    LANG_TEST_C=0, LANG_TEST_CPP=1, LANG_TEST_RS=2, LANG_TEST_GO=3,
    LANG_TEST_PY=4, LANG_TEST_JS=5, LANG_TEST_KT=6, LANG_TEST_JV=7,
    LANG_TEST_CS=8, LANG_TEST_SH=9, LANG_TEST_RB=10, LANG_TEST_PH=11,
} LanguageTestKind;

typedef struct {
    LanguageTestKind lang;
    const char *lang_name;
    const char *source_code;        /* minimal valid program */
    sz source_len;
    const char *expected_binary_sig; /* expected binary signature/hash */
    u32 expected_binary_size_min;
    u32 expected_binary_size_max;
    u8 should_execute;              /* can we run the APK to verify output? */
    u64 max_compile_time_us;        /* max acceptable compilation time */
} LanguageRegressionTest;

typedef struct {
    LanguageRegressionTest tests[12];
    u32 test_count;
    u32 pass_count;
    u32 fail_count;
} LanguageRegressionSuite;

/* ─────────────────────────────────────────────────────────────────────── */
/* Performance Benchmarking */
/* ─────────────────────────────────────────────────────────────────────── */

typedef struct {
    u32 phase_id;
    const char *phase_name;
    u64 time_us;
    u64 memory_peak_bytes;
    u64 allocation_count;
} PhaseBenchmark;

typedef struct {
    PhaseBenchmark phases[48];
    u32 phase_count;
    u64 total_compile_time_us;
    u64 total_memory_peak_bytes;
    u32 phases_over_budget;
} CompilationBenchmark;

typedef struct {
    u32 input_size_bytes;
    u64 compile_time_us;
    u32 binary_size_bytes;
    u32 compression_ratio_percent;
    u64 throughput_bytes_per_sec;
} EndToEndBenchmark;

/* ─────────────────────────────────────────────────────────────────────── */
/* Determinism Verification */
/* ─────────────────────────────────────────────────────────────────────── */

typedef struct {
    u64 hash_build_1;      /* FNV-64 hash of binary 1 */
    u64 hash_build_2;      /* FNV-64 hash of binary 2 */
    u64 hash_build_3;      /* FNV-64 hash of binary 3 */
    u8 all_match;          /* 1 if all hashes identical */
    u32 byte_diff_count;   /* number of differing bytes (should be 0) */
} DeterminismTest;

typedef struct {
    DeterminismTest tests[16];      /* 16 different source files */
    u32 test_count;
    u32 pass_count;
    const char *error_source;       /* if failed, which source file */
    u64 first_diff_offset;          /* byte offset of first difference */
} DeterminismVerification;

/* ─────────────────────────────────────────────────────────────────────── */
/* Freestanding Compliance */
/* ─────────────────────────────────────────────────────────────────────── */

typedef struct {
    u32 malloc_calls;       /* should be 0 */
    u32 calloc_calls;       /* should be 0 */
    u32 free_calls;         /* should be 0 */
    u32 libc_includes;      /* should be 0 */
    u32 heap_allocations;   /* should be 0 */
    u8 is_compliant;        /* 1 if all zeros */
} FreestandingAudit;

/* ─────────────────────────────────────────────────────────────────────── */
/* Compiler Validation Report */
/* ─────────────────────────────────────────────────────────────────────── */

typedef struct {
    const char *compiler_version;
    const char *build_date;
    const char *build_platform;

    /* results */
    TestSuite unit_tests;
    LanguageRegressionSuite lang_tests;
    CoverageReport coverage;
    CompilationBenchmark benchmarks;
    DeterminismVerification determinism;
    FreestandingAudit freestanding;

    /* summary */
    u32 total_tests;
    u32 total_passed;
    u32 total_failed;
    float pass_rate_percent;

    u8 production_ready;    /* 1 if all critical gates pass */
    const char *status_msg;
} ValidationReport;

/* ─────────────────────────────────────────────────────────────────────── */
/* Test Registry & Execution */
/* ─────────────────────────────────────────────────────────────────────── */

typedef int (*test_fn_t)(void);  /* test function returning 0=pass, 1=fail */

typedef struct {
    u32 phase_id;
    const char *test_name;
    test_fn_t fn;
} RegisteredTest;

typedef struct {
    RegisteredTest tests[512];  /* max 512 tests */
    u32 test_count;
} TestRegistry;

/* Singleton registry */
extern TestRegistry g_test_registry;

/* Register a test for a phase */
static inline void val_register_test(u32 phase_id, const char *name, test_fn_t fn) {
    if (g_test_registry.test_count >= 512) return;
    g_test_registry.tests[g_test_registry.test_count].phase_id = phase_id;
    g_test_registry.tests[g_test_registry.test_count].test_name = name;
    g_test_registry.tests[g_test_registry.test_count].fn = fn;
    g_test_registry.test_count++;
}

/* Execute all registered tests */
int val_run_all_tests(TestSuite *suite_out);

/* Execute tests for a specific phase */
int val_run_phase_tests(u32 phase_id, TestSuite *suite_out);

/* ─────────────────────────────────────────────────────────────────────── */
/* Assertion Helpers */
/* ─────────────────────────────────────────────────────────────────────── */

#define VAL_ASSERT_EQ(a, b, msg) \
    do { if ((a) != (b)) { \
        return 1;  /* fail */ \
    } } while(0)

#define VAL_ASSERT_NE(a, b, msg) \
    do { if ((a) == (b)) { \
        return 1;  /* fail */ \
    } } while(0)

#define VAL_ASSERT(cond, msg) \
    do { if (!(cond)) { \
        return 1;  /* fail */ \
    } } while(0)

#define VAL_ASSERT_NULL(ptr, msg) \
    do { if ((ptr) != 0) { \
        return 1;  /* fail */ \
    } } while(0)

#define VAL_ASSERT_NOT_NULL(ptr, msg) \
    do { if ((ptr) == 0) { \
        return 1;  /* fail */ \
    } } while(0)

/* ─────────────────────────────────────────────────────────────────────── */
/* Integration Test Helpers */
/* ─────────────────────────────────────────────────────────────────────── */

typedef struct {
    const char *source;     /* source code to compile */
    sz source_len;
    const char *lang;       /* file extension: .c, .py, .rs, etc */
    const char *expected_binary_contains;  /* check if output contains string */
    u32 expected_exit_code;
    u64 max_time_us;
} IntegrationTestCase;

int val_run_integration_test(const IntegrationTestCase *test);
int val_verify_end_to_end_pipeline(void);

/* ─────────────────────────────────────────────────────────────────────── */
/* Performance Baseline Targets */
/* ─────────────────────────────────────────────────────────────────────── */

typedef struct {
    const char *phase_name;
    u64 max_time_us;        /* must complete within this time */
    u64 max_memory_bytes;   /* must not exceed this memory */
    float max_regression_percent;  /* vs baseline, allow this % slowdown */
} PerformanceTarget;

/* Baseline targets for each phase */
extern PerformanceTarget g_performance_targets[48];

int val_check_performance_compliance(const CompilationBenchmark *bench);

/* ─────────────────────────────────────────────────────────────────────── */
/* Reporting */
/* ─────────────────────────────────────────────────────────────────────── */

void val_print_test_report(const TestSuite *suite);
void val_print_coverage_report(const CoverageReport *cov);
void val_print_benchmark_report(const CompilationBenchmark *bench);
void val_print_determinism_report(const DeterminismVerification *det);
void val_print_validation_report(const ValidationReport *report);

/* ─────────────────────────────────────────────────────────────────────── */
/* Phase 49 Initialization */
/* ─────────────────────────────────────────────────────────────────────── */

int val_init_comprehensive_testing(void);
int val_run_full_validation(ValidationReport *report_out);

#endif /* APKC_VAL_COMPREHENSIVE_TESTING_H_ */
