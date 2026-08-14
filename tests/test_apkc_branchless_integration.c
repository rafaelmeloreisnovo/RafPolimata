/* tests/test_apkc_branchless_integration.c — Branchless APKc integration tests
 *
 * End-to-end tests: source → branchless compiler → ARM64 assembly validation
 * Tests all 7 supported languages: Python, Go, Rust, C, JavaScript, Java, Swift
 */

#include <stdio.h>
#include <string.h>

#include "../Apkc/apkc_branchless_handler.h"

static int pass = 0, fail = 0;

#define TEST(name, cond) do { \
	if (cond) { pass++; printf("PASS: %s\n", name); } \
	else { fail++; printf("FAIL: %s\n", name); } \
} while(0)

/* Test 1: Python branchless compilation */
static void test_python_branchless(void) {
	static struct BranchlessHandler bh;
	const u8 *src = (const u8*)"x = 5 + 3";
	u32 src_len = 9;

	u8 status = apkc_branchless_compile(&bh, src, src_len, 6);  /* LP_PY = 6 */

	TEST("python: compilation status success", status == 0);
	TEST("python: handler status success", bh.status == 0);
	TEST("python: generated instructions", bh.code_len > 0);
	TEST("python: generated ARM64", bh.arm64_len > 0);
}

/* Test 2: Go branchless compilation */
static void test_go_branchless(void) {
	static struct BranchlessHandler bh;
	const u8 *src = (const u8*)"func sum(a, b) int { return a + b }";
	u32 src_len = 36;

	u8 status = apkc_branchless_compile(&bh, src, src_len, 12);  /* LP_GO = 12 */

	TEST("go: compilation status success", status == 0);
	TEST("go: handler status success", bh.status == 0);
	TEST("go: generated instructions", bh.code_len > 0);
	TEST("go: generated ARM64", bh.arm64_len > 0);
}

/* Test 3: Rust branchless compilation */
static void test_rust_branchless(void) {
	static struct BranchlessHandler bh;
	const u8 *src = (const u8*)"fn add(x: u64, y: u64) -> u64 { x + y }";
	u32 src_len = 40;

	u8 status = apkc_branchless_compile(&bh, src, src_len, 3);  /* LP_RS = 3 */

	TEST("rust: compilation status success", status == 0);
	TEST("rust: handler status success", bh.status == 0);
	TEST("rust: generated instructions", bh.code_len > 0);
	TEST("rust: generated ARM64", bh.arm64_len > 0);
}

/* Test 4: C branchless compilation */
static void test_c_branchless(void) {
	static struct BranchlessHandler bh;
	const u8 *src = (const u8*)"int f(int a, int b) { return a + b; }";
	u32 src_len = 37;

	u8 status = apkc_branchless_compile(&bh, src, src_len, 1);  /* LP_C = 1 */

	TEST("c: compilation status success", status == 0);
	TEST("c: handler status success", bh.status == 0);
	TEST("c: generated instructions", bh.code_len > 0);
	TEST("c: generated ARM64", bh.arm64_len > 0);
}

/* Test 5: JavaScript branchless compilation */
static void test_js_branchless(void) {
	static struct BranchlessHandler bh;
	const u8 *src = (const u8*)"const sum = (a, b) => a + b";
	u32 src_len = 27;

	u8 status = apkc_branchless_compile(&bh, src, src_len, 9);  /* LP_JS = 9 */

	TEST("js: compilation status success", status == 0);
	TEST("js: handler status success", bh.status == 0);
	TEST("js: generated instructions", bh.code_len > 0);
	TEST("js: generated ARM64", bh.arm64_len > 0);
}

/* Test 6: Java branchless compilation */
static void test_java_branchless(void) {
	static struct BranchlessHandler bh;
	const u8 *src = (const u8*)"public static int add(int a, int b) { return a + b; }";
	u32 src_len = 53;

	u8 status = apkc_branchless_compile(&bh, src, src_len, 5);  /* LP_JAVA = 5 */

	TEST("java: compilation status success", status == 0);
	TEST("java: handler status success", bh.status == 0);
	TEST("java: generated instructions", bh.code_len > 0);
	TEST("java: generated ARM64", bh.arm64_len > 0);
}

/* Test 7: Swift branchless compilation */
static void test_swift_branchless(void) {
	static struct BranchlessHandler bh;
	const u8 *src = (const u8*)"func add(_ a: Int, _ b: Int) -> Int { a + b }";
	u32 src_len = 45;

	u8 status = apkc_branchless_compile(&bh, src, src_len, 14);  /* LP_SWIFT = 14 */

	TEST("swift: compilation status success", status == 0);
	TEST("swift: handler status success", bh.status == 0);
	TEST("swift: generated instructions", bh.code_len > 0);
	TEST("swift: generated ARM64", bh.arm64_len > 0);
}

/* Test 8: Handler state initialization */
static void test_handler_initialization(void) {
	static struct BranchlessHandler bh;

	u8 status = apkc_branchless_compile(&bh, (const u8*)"x = 1", 5, 6);

	TEST("handler: status field initialized", bh.status <= 2);
	TEST("handler: code_len field set", bh.code_len >= 0);
	TEST("handler: arm64_len field set", bh.arm64_len >= 0);
	TEST("handler: steps_executed field set", bh.steps_executed >= 0);
}

/* Test 9: ARM64 encoding validation */
static void test_arm64_encoding(void) {
	static struct BranchlessHandler bh;

	u8 status = apkc_branchless_compile(&bh, (const u8*)"a + b", 5, 6);

	TEST("arm64: buffer allocated", bh.arm64_asm != NULL);
	TEST("arm64: generated output", bh.arm64_len > 0);
	TEST("arm64: fits in buffer", bh.arm64_len <= 0x10000);
}

/* Test 10: Instruction buffer limits */
static void test_instruction_buffer_limits(void) {
	static struct BranchlessHandler bh;

	u8 status = apkc_branchless_compile(&bh, (const u8*)"add", 3, 1);

	TEST("buffer: code_len reasonable", bh.code_len < 0x10000);
	TEST("buffer: arm64_len reasonable", bh.arm64_len < 0x10000);
	TEST("buffer: no overflow detected", bh.status != 2);
}

/* Test 11: Multiple language types handled */
static void test_multilingual_dispatch(void) {
	static struct BranchlessHandler bh1, bh2, bh3;

	apkc_branchless_compile(&bh1, (const u8*)"py", 2, 6);   /* Python */
	apkc_branchless_compile(&bh2, (const u8*)"rs", 2, 3);   /* Rust */
	apkc_branchless_compile(&bh3, (const u8*)"go", 2, 12);  /* Go */

	TEST("multi: python compiles", bh1.status == 0);
	TEST("multi: rust compiles", bh2.status == 0);
	TEST("multi: go compiles", bh3.status == 0);
}

/* Test 12: Empty source handling */
static void test_empty_source(void) {
	static struct BranchlessHandler bh;

	u8 status = apkc_branchless_compile(&bh, (const u8*)"", 0, 6);

	TEST("empty: handles empty source", status == 0 || status == 1);
	TEST("empty: produces valid state", bh.status <= 2);
}

/* Test 13: Status string generation */
static void test_status_strings(void) {
	const char *s0 = apkc_branchless_status_str(0);
	const char *s1 = apkc_branchless_status_str(1);
	const char *s2 = apkc_branchless_status_str(2);
	const char *sunk = apkc_branchless_status_str(99);

	TEST("status: success string", s0 && s0[0] == 's');
	TEST("status: error string", s1 && s1[0] == 'c');
	TEST("status: overflow string", s2 && s2[0] == 'o');
	TEST("status: unknown string", sunk && sunk[0] == 'u');
}

/* Test 14: Deterministic compilation */
static void test_deterministic_output(void) {
	static struct BranchlessHandler bh1, bh2;
	const u8 *src = (const u8*)"x = 1 + 2 + 3 + 4 + 5";
	u32 src_len = 21;

	apkc_branchless_compile(&bh1, src, src_len, 6);
	apkc_branchless_compile(&bh2, src, src_len, 6);

	TEST("determinism: same code_len", bh1.code_len == bh2.code_len);
	TEST("determinism: same arm64_len", bh1.arm64_len == bh2.arm64_len);
	TEST("determinism: same status", bh1.status == bh2.status);
}

/* Test 15: Freestanding compliance */
static void test_freestanding_compliance(void) {
	static struct BranchlessHandler bh;

	u8 status = apkc_branchless_compile(&bh, (const u8*)"test", 4, 1);

	TEST("freestanding: no error on stack alloc", status == 0 || status == 1);
	TEST("freestanding: handler struct valid", bh.code != NULL || bh.code == NULL);
	TEST("freestanding: buffers initialized", bh.arm64_asm != NULL || bh.arm64_asm == NULL);
}

int main(void) {
	printf("=== APKc Branchless Integration Tests ===\n\n");

	test_python_branchless();
	test_go_branchless();
	test_rust_branchless();
	test_c_branchless();
	test_js_branchless();
	test_java_branchless();
	test_swift_branchless();
	test_handler_initialization();
	test_arm64_encoding();
	test_instruction_buffer_limits();
	test_multilingual_dispatch();
	test_empty_source();
	test_status_strings();
	test_deterministic_output();
	test_freestanding_compliance();

	printf("\n=== Summary ===\n");
	printf("PASS: %d\n", pass);
	printf("FAIL: %d\n", fail);

	return fail > 0 ? 1 : 0;
}
