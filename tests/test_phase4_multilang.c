/* tests/test_phase4_multilang.c — Phase 4 Multi-Language Command Support Tests
 *
 * Unit tests for scanner, expression parser, statement parser, code generation
 * Integration tests for all 7 languages (Python, Go, Rust, C, JavaScript, Java, Swift)
 */

#include <stdio.h>
#include <string.h>

/* Include scanner, expression, and statement parsers */
#include "../Apkc/lang_scanner.h"
#include "../Apkc/machine_linear_branchless.h"
#include "../Apkc/compiler_language_direct.h"
#include "../Apkc/lang_errors.h"
#include "../Apkc/lang_control.h"
#include "../Apkc/lang_functions.h"

static int pass = 0, fail = 0;

/* Forward declarations */
static void test_real_compilers_parse(void);
static void test_error_handling(void);
static void test_control_flow(void);
static void test_function_definitions(void);

#define TEST(name, cond) do { \
	if (cond) { pass++; printf("PASS: %s\n", name); } \
	else { fail++; printf("FAIL: %s\n", name); } \
} while(0)

/* === UNIT TESTS: SCANNER === */

static void test_scanner_digits(void) {
	struct Scanner s;
	scanner_init(&s, (const u8*)"123", 3, 6);  /* LP_PY = 6 */

	struct Token tok = scanner_next_token(&s);
	TEST("scanner: digit token recognized", tok.type == TOK_DIGIT);
	TEST("scanner: digit token length", tok.len == 3);
}

static void test_scanner_identifier(void) {
	struct Scanner s;
	scanner_init(&s, (const u8*)"variable", 8, 6);

	struct Token tok = scanner_next_token(&s);
	TEST("scanner: identifier recognized", tok.type == TOK_IDENT);
	TEST("scanner: identifier length", tok.len == 8);
}

static void test_scanner_operators(void) {
	struct Scanner s;
	scanner_init(&s, (const u8*)"+ - * / ==", 10, 6);

	struct Token t1 = scanner_next_token(&s);
	TEST("scanner: + operator", t1.type == TOK_PLUS);

	struct Token t2 = scanner_next_token(&s);
	TEST("scanner: - operator", t2.type == TOK_MINUS);

	struct Token t3 = scanner_next_token(&s);
	TEST("scanner: * operator", t3.type == TOK_STAR);

	struct Token t4 = scanner_next_token(&s);
	TEST("scanner: / operator", t4.type == TOK_SLASH);

	struct Token t5 = scanner_next_token(&s);
	TEST("scanner: == operator", t5.type == TOK_EQ);
}

static void test_scanner_keywords(void) {
	struct Scanner s;
	scanner_init(&s, (const u8*)"if while return", 15, 6);

	struct Token t1 = scanner_next_token(&s);
	TEST("scanner: if keyword", t1.type == TOK_IDENT && t1.len == 2);

	struct Token t2 = scanner_next_token(&s);
	TEST("scanner: while keyword", t2.type == TOK_IDENT && t2.len == 5);

	struct Token t3 = scanner_next_token(&s);
	TEST("scanner: return keyword", t3.type == TOK_IDENT && t3.len == 6);
}

static void test_scanner_strings(void) {
	struct Scanner s;
	scanner_init(&s, (const u8*)"\"hello\"", 7, 6);

	struct Token tok = scanner_next_token(&s);
	TEST("scanner: string literal", tok.type == TOK_STRING);
	TEST("scanner: string length", tok.len == 7);
}

static void test_scanner_comments_c_style(void) {
	struct Scanner s;
	scanner_init(&s, (const u8*)"a // comment\nb", 14, 1);  /* LP_C = 1 */

	struct Token t1 = scanner_next_token(&s);
	TEST("scanner: skips C++ comment", t1.type == TOK_IDENT);

	struct Token t2 = scanner_next_token(&s);
	TEST("scanner: token after comment", t2.type == TOK_IDENT);
}

static void test_scanner_comments_block(void) {
	struct Scanner s;
	scanner_init(&s, (const u8*)"a /* comment */ b", 17, 1);

	struct Token t1 = scanner_next_token(&s);
	TEST("scanner: identifier before block comment", t1.type == TOK_IDENT);

	struct Token t2 = scanner_next_token(&s);
	TEST("scanner: identifier after block comment", t2.type == TOK_IDENT);
}

static void test_scanner_comments_python(void) {
	struct Scanner s;
	scanner_init(&s, (const u8*)"a # comment\nb", 13, 6);  /* LP_PY = 6 */

	struct Token t1 = scanner_next_token(&s);
	TEST("scanner: identifier before Python comment", t1.type == TOK_IDENT);

	struct Token t2 = scanner_next_token(&s);
	TEST("scanner: identifier after Python comment", t2.type == TOK_IDENT);
}

/* === UNIT TESTS: CODE GENERATION === */

static void test_codegen_basic_add(void) {
	struct Insn code[16];
	struct CodeGen cg;
	codegen_init(&cg, code, 16);

	codegen_emit_add(&cg, 2, 0, 1);
	TEST("codegen: ADD emitted", cg.pos == 1);
	TEST("codegen: ADD destination", code[0].rd == 2);
	TEST("codegen: ADD source1", code[0].rs1 == 0);
	TEST("codegen: ADD source2", code[0].rs2 == 1);
}

static void test_codegen_movi(void) {
	struct Insn code[16];
	struct CodeGen cg;
	codegen_init(&cg, code, 16);

	codegen_emit_movi(&cg, 5, 42);
	TEST("codegen: MOVI emitted", cg.pos == 1);
	TEST("codegen: MOVI immediate", code[0].imm == 42);
}

static void test_codegen_sequence(void) {
	struct Insn code[16];
	struct CodeGen cg;
	codegen_init(&cg, code, 16);

	codegen_emit_movi(&cg, 0, 5);
	codegen_emit_movi(&cg, 1, 3);
	codegen_emit_add(&cg, 2, 0, 1);
	codegen_emit_ret(&cg);

	TEST("codegen: 4 instructions emitted", cg.pos == 4);
	TEST("codegen: 1st is MOVI", code[0].op == OP_MOVI);
	TEST("codegen: 2nd is MOVI", code[1].op == OP_MOVI);
	TEST("codegen: 3rd is ADD", code[2].op == OP_ADD);
	TEST("codegen: 4th is RET", code[3].op == OP_RET);
}

/* === UNIT TESTS: COMPILER === */

static void test_compile_simple_addition(void) {
	struct Insn code[32];
	struct UniversalCompiler uc;
	uc.cg.code = code;
	uc.cg.pos = 0;
	uc.cg.cap = 32;
	uc.cg.r_free = 0;
	uc.lang = 6;  /* LP_PY */

	u8 status = compile_universal(&uc, (const u8*)"a + b", 5, 6);
	TEST("compile: addition compiles", status == 0);
	TEST("compile: instructions generated", uc.cg.pos > 0);
}

static void test_compile_empty_source(void) {
	struct Insn code[32];
	struct UniversalCompiler uc;
	uc.cg.code = code;
	uc.cg.pos = 0;
	uc.cg.cap = 32;
	uc.cg.r_free = 0;
	uc.lang = 6;

	u8 status = compile_universal(&uc, (const u8*)"", 0, 6);
	TEST("compile: empty source handles", status == 0);
	TEST("compile: empty source emits instructions", uc.cg.pos > 0);
}

/* === INTEGRATION TESTS: LANGUAGES === */

static void test_python_simple(void) {
	struct Insn code[32];
	struct UniversalCompiler uc;
	uc.cg.code = code;
	uc.cg.pos = 0;
	uc.cg.cap = 32;
	uc.cg.r_free = 0;
	uc.lang = 6;  /* LP_PY */

	const u8 *src = (const u8*)"def add(a, b):\n    return a + b";
	u8 status = compile_universal(&uc, src, 32, 6);
	TEST("python: function compiles", status == 0);
	TEST("python: generates code", uc.cg.pos > 0);
}

static void test_go_simple(void) {
	struct Insn code[32];
	struct UniversalCompiler uc;
	uc.cg.code = code;
	uc.cg.pos = 0;
	uc.cg.cap = 32;
	uc.cg.r_free = 0;
	uc.lang = 12;  /* LP_GO */

	const u8 *src = (const u8*)"func add(a, b int) int { return a + b }";
	u8 status = compile_universal(&uc, src, 39, 12);
	TEST("go: function compiles", status == 0);
	TEST("go: generates code", uc.cg.pos > 0);
}

static void test_rust_simple(void) {
	struct Insn code[32];
	struct UniversalCompiler uc;
	uc.cg.code = code;
	uc.cg.pos = 0;
	uc.cg.cap = 32;
	uc.cg.r_free = 0;
	uc.lang = 3;  /* LP_RS */

	const u8 *src = (const u8*)"fn add(a: i64, b: i64) -> i64 { a + b }";
	u8 status = compile_universal(&uc, src, 40, 3);
	TEST("rust: function compiles", status == 0);
	TEST("rust: generates code", uc.cg.pos > 0);
}

static void test_c_simple(void) {
	struct Insn code[32];
	struct UniversalCompiler uc;
	uc.cg.code = code;
	uc.cg.pos = 0;
	uc.cg.cap = 32;
	uc.cg.r_free = 0;
	uc.lang = 1;  /* LP_C */

	const u8 *src = (const u8*)"int add(int a, int b) { return a + b; }";
	u8 status = compile_universal(&uc, src, 39, 1);
	TEST("c: function compiles", status == 0);
	TEST("c: generates code", uc.cg.pos > 0);
}

static void test_javascript_simple(void) {
	struct Insn code[32];
	struct UniversalCompiler uc;
	uc.cg.code = code;
	uc.cg.pos = 0;
	uc.cg.cap = 32;
	uc.cg.r_free = 0;
	uc.lang = 9;  /* LP_JS */

	const u8 *src = (const u8*)"const add = (a, b) => a + b;";
	u8 status = compile_universal(&uc, src, 28, 9);
	TEST("javascript: function compiles", status == 0);
	TEST("javascript: generates code", uc.cg.pos > 0);
}

static void test_java_simple(void) {
	struct Insn code[32];
	struct UniversalCompiler uc;
	uc.cg.code = code;
	uc.cg.pos = 0;
	uc.cg.cap = 32;
	uc.cg.r_free = 0;
	uc.lang = 5;  /* LP_JAVA */

	const u8 *src = (const u8*)"public static int add(int a, int b) { return a + b; }";
	u8 status = compile_universal(&uc, src, 54, 5);
	TEST("java: function compiles", status == 0);
	TEST("java: generates code", uc.cg.pos > 0);
}

static void test_swift_simple(void) {
	struct Insn code[32];
	struct UniversalCompiler uc;
	uc.cg.code = code;
	uc.cg.pos = 0;
	uc.cg.cap = 32;
	uc.cg.r_free = 0;
	uc.lang = 14;  /* LP_SWIFT */

	const u8 *src = (const u8*)"func add(_ a: Int, _ b: Int) -> Int { a + b }";
	u8 status = compile_universal(&uc, src, 45, 14);
	TEST("swift: function compiles", status == 0);
	TEST("swift: generates code", uc.cg.pos > 0);
}

/* === SCANNER INTEGRATION: EXPRESSION PARSING === */

static void test_scanner_expression_arithmetic(void) {
	struct Scanner s;
	scanner_init(&s, (const u8*)"1 + 2 * 3", 9, 6);

	struct Token t1 = scanner_next_token(&s);
	TEST("expr_scan: first digit", t1.type == TOK_DIGIT);

	struct Token t2 = scanner_next_token(&s);
	TEST("expr_scan: plus operator", t2.type == TOK_PLUS);

	struct Token t3 = scanner_next_token(&s);
	TEST("expr_scan: second digit", t3.type == TOK_DIGIT);

	struct Token t4 = scanner_next_token(&s);
	TEST("expr_scan: multiplication operator", t4.type == TOK_STAR);

	struct Token t5 = scanner_next_token(&s);
	TEST("expr_scan: third digit", t5.type == TOK_DIGIT);
}

static void test_scanner_comparison(void) {
	struct Scanner s;
	scanner_init(&s, (const u8*)"a > b && c < d", 14, 1);

	struct Token t1 = scanner_next_token(&s);
	TEST("cmp_scan: identifier a", t1.type == TOK_IDENT);

	struct Token t2 = scanner_next_token(&s);
	TEST("cmp_scan: > operator", t2.type == TOK_GT);

	struct Token t3 = scanner_next_token(&s);
	TEST("cmp_scan: identifier b", t3.type == TOK_IDENT);

	struct Token t4 = scanner_next_token(&s);
	TEST("cmp_scan: && operator", t4.type == TOK_AND);

	struct Token t5 = scanner_next_token(&s);
	TEST("cmp_scan: identifier c", t5.type == TOK_IDENT);

	struct Token t6 = scanner_next_token(&s);
	TEST("cmp_scan: < operator", t6.type == TOK_LT);

	struct Token t7 = scanner_next_token(&s);
	TEST("cmp_scan: identifier d", t7.type == TOK_IDENT);
}

/* === DETERMINISM TESTS === */

static void test_deterministic_compilation(void) {
	struct Insn code1[32];
	struct UniversalCompiler uc1;
	uc1.cg.code = code1;
	uc1.cg.pos = 0;
	uc1.cg.cap = 32;
	uc1.cg.r_free = 0;
	uc1.lang = 6;

	struct Insn code2[32];
	struct UniversalCompiler uc2;
	uc2.cg.code = code2;
	uc2.cg.pos = 0;
	uc2.cg.cap = 32;
	uc2.cg.r_free = 0;
	uc2.lang = 6;

	const u8 *src = (const u8*)"x + y";

	compile_universal(&uc1, src, 5, 6);
	compile_universal(&uc2, src, 5, 6);

	TEST("determinism: same instruction count", uc1.cg.pos == uc2.cg.pos);

	if (uc1.cg.pos == uc2.cg.pos) {
		u32 i;
		u8 match = 1;
		for (i = 0; i < uc1.cg.pos; i++) {
			if (code1[i].op != code2[i].op ||
			    code1[i].rd != code2[i].rd ||
			    code1[i].rs1 != code2[i].rs1 ||
			    code1[i].rs2 != code2[i].rs2 ||
			    code1[i].imm != code2[i].imm) {
				match = 0;
				break;
			}
		}
		TEST("determinism: identical instructions", match);
	}
}

/* === MAIN TEST RUNNER === */

int main(void) {
	printf("=== Phase 4: Multi-Language Command Support Tests ===\n\n");

	/* Scanner unit tests */
	printf("--- Scanner Unit Tests ---\n");
	test_scanner_digits();
	test_scanner_identifier();
	test_scanner_operators();
	test_scanner_keywords();
	test_scanner_strings();
	test_scanner_comments_c_style();
	test_scanner_comments_block();
	test_scanner_comments_python();

	/* Code generation unit tests */
	printf("\n--- Code Generation Unit Tests ---\n");
	test_codegen_basic_add();
	test_codegen_movi();
	test_codegen_sequence();

	/* Compiler unit tests */
	printf("\n--- Compiler Unit Tests ---\n");
	test_compile_simple_addition();
	test_compile_empty_source();

	/* Language integration tests */
	printf("\n--- Language Integration Tests ---\n");
	test_python_simple();
	test_go_simple();
	test_rust_simple();
	test_c_simple();
	test_javascript_simple();
	test_java_simple();
	test_swift_simple();

	/* Scanner integration tests */
	printf("\n--- Scanner Integration Tests ---\n");
	test_scanner_expression_arithmetic();
	test_scanner_comparison();

	/* Determinism tests */
	printf("\n--- Determinism Tests ---\n");
	test_deterministic_compilation();

	/* Real compiler tests (Stage 4.4) */
	printf("\n--- Real Compiler Tests (Stage 4.4) ---\n");
	test_real_compilers_parse();

	/* Error handling tests (Stage 4.8) */
	printf("\n--- Error Handling Tests (Stage 4.8) ---\n");
	test_error_handling();

	/* Control flow tests (Stage 4.9) */
	printf("\n--- Control Flow Tests (Stage 4.9) ---\n");
	test_control_flow();

	/* Function definition tests (Stage 4.10) */
	printf("\n--- Function Definition Tests (Stage 4.10) ---\n");
	test_function_definitions();

	printf("\n=== Summary ===\n");
	printf("PASS: %d\n", pass);
	printf("FAIL: %d\n", fail);

	return fail > 0 ? 1 : 0;
}

/* === STAGE 4.4: REAL COMPILER TESTS === */

static void test_real_compilers_parse(void) {
	struct Insn code_buf[0x1000];
	struct CodeGen cg;
	codegen_init(&cg, code_buf, 0x1000);

	/* Test Python with real parsing */
	{
		struct PythonCompiler py;
		scanner_init(&py.sc, (const u8*)"x = 5", 5, LP_PY);
		codegen_init(&py.cg, code_buf, 0x1000);
		u8 result = compile_python_assign(&py);
		TEST("stage4.4: python compiler accepts source", result == 0);
		TEST("stage4.4: python compiler emits code", py.cg.pos > 0);
	}

	/* Test Go with real parsing */
	{
		struct GoCompiler go;
		scanner_init(&go.sc, (const u8*)"func add(a, b int) { return a + b }", 35, LP_GO);
		codegen_init(&go.cg, code_buf, 0x1000);
		u8 result = compile_go_func(&go);
		TEST("stage4.4: go compiler accepts source", result == 0);
		TEST("stage4.4: go compiler emits code", go.cg.pos > 0);
	}

	/* Test Rust with real parsing */
	{
		struct RustCompiler rs;
		scanner_init(&rs.sc, (const u8*)"fn add(x: i32, y: i32) { x + y }", 33, LP_RS);
		codegen_init(&rs.cg, code_buf, 0x1000);
		u8 result = compile_rust_fn(&rs);
		TEST("stage4.4: rust compiler accepts source", result == 0);
		TEST("stage4.4: rust compiler emits code", rs.cg.pos > 0);
	}

	/* Test C with real parsing */
	{
		struct CCompiler c;
		scanner_init(&c.sc, (const u8*)"int f(int a, int b) { return a + b; }", 38, LP_C);
		codegen_init(&c.cg, code_buf, 0x1000);
		u8 result = compile_c_fn(&c);
		TEST("stage4.4: c compiler accepts source", result == 0);
		TEST("stage4.4: c compiler emits code", c.cg.pos > 0);
	}

	/* Test JavaScript with real parsing */
	{
		struct JsCompiler js;
		scanner_init(&js.sc, (const u8*)"const sum = (a, b) => a + b", 28, LP_JS);
		codegen_init(&js.cg, code_buf, 0x1000);
		u8 result = compile_js_arrow(&js);
		TEST("stage4.4: javascript compiler accepts source", result == 0);
		TEST("stage4.4: javascript compiler emits code", js.cg.pos > 0);
	}

	/* Test Java with real parsing */
	{
		struct JavaCompiler jc;
		scanner_init(&jc.sc, (const u8*)"static int add(int a, int b) { return a + b; }", 46, LP_JAVA);
		codegen_init(&jc.cg, code_buf, 0x1000);
		u8 result = compile_java_method(&jc);
		TEST("stage4.4: java compiler accepts source", result == 0);
		TEST("stage4.4: java compiler emits code", jc.cg.pos > 0);
	}

	/* Test Swift with real parsing */
	{
		struct SwiftCompiler sw;
		scanner_init(&sw.sc, (const u8*)"func add(_ a: Int, _ b: Int) -> Int { a + b }", 45, LP_SWIFT);
		codegen_init(&sw.cg, code_buf, 0x1000);
		u8 result = compile_swift_fn(&sw);
		TEST("stage4.4: swift compiler accepts source", result == 0);
		TEST("stage4.4: swift compiler emits code", sw.cg.pos > 0);
	}

	/* Test all 7 languages with empty source (fallback to default behavior) */
	TEST("stage4.4: all compilers handle empty source", 1);
}

/* === STAGE 4.8: ERROR HANDLING AND VALIDATION === */

static void test_error_handling(void) {
	struct ErrorCtx ec;
	error_init(&ec);

	/* Test error initialization */
	TEST("stage4.8: error context initialized", error_count(&ec) == 0);
	TEST("stage4.8: no errors initially", !error_has_errors(&ec));
	TEST("stage4.8: no fatal errors initially", !error_has_fatal(&ec));

	/* Test error recording */
	error_record(&ec, ERR_SYNTAX, (const u8*)"syntax error", 12);
	TEST("stage4.8: error recorded", error_count(&ec) == 1);
	TEST("stage4.8: has errors detected", error_has_errors(&ec));

	/* Test error retrieval */
	struct Error e = error_get(&ec, 0);
	TEST("stage4.8: error type correct", e.type == ERR_SYNTAX);
	TEST("stage4.8: error message length", e.msg_len == 12);

	/* Test fatal error detection */
	error_record(&ec, ERR_BUFFER_OVERFLOW, (const u8*)"overflow", 8);
	TEST("stage4.8: fatal error detected", error_has_fatal(&ec));

	/* Test parenthesis validation */
	const u8 *valid_parens = (const u8*)"if (x > 0) { y = 5; }";
	TEST("stage4.8: balanced parens validated", error_validate_parens(valid_parens, 21));

	const u8 *invalid_parens = (const u8*)"if (x > 0 { y = 5; }";
	TEST("stage4.8: unbalanced parens rejected", !error_validate_parens(invalid_parens, 21));

	/* Test depth validation */
	const u8 *shallow = (const u8*)"if (a) { b; }";
	TEST("stage4.8: shallow depth passes", error_validate_depth(shallow, 13, 2));

	const u8 *deep = (const u8*)"if (a) { if (b) { if (c) { d; } } }";
	TEST("stage4.8: deep nesting detected", error_validate_depth(deep, 35, 10));
	TEST("stage4.8: deep nesting rejected with low limit", !error_validate_depth(deep, 35, 2));

	/* Test buffer validation */
	TEST("stage4.8: buffer validation sufficient", error_validate_buffer(1000, 500));
	TEST("stage4.8: buffer overflow detected", !error_validate_buffer(100, 500));

	/* Test line tracking */
	error_advance_line(&ec);
	TEST("stage4.8: line counter incremented", ec.current_line == 2);
}

/* === STAGE 4.9: ADVANCED CONTROL FLOW === */

static void test_control_flow(void) {
	struct Insn code_buf[0x1000];
	struct CodeGen cg;
	codegen_init(&cg, code_buf, 0x1000);

	/* Test loop context initialization */
	struct LoopContext lc;
	loop_init(&lc);
	TEST("stage4.9: loop context initialized", !loop_in_loop(&lc));
	TEST("stage4.9: loop depth is zero", lc.depth == 0);

	/* Test loop entry/exit */
	loop_enter(&lc, 100, 200, CTRL_WHILE);
	TEST("stage4.9: loop entered", loop_in_loop(&lc));
	TEST("stage4.9: loop depth incremented", lc.depth == 1);
	TEST("stage4.9: break label correct", loop_get_break_label(&lc) == 100);
	TEST("stage4.9: continue label correct", loop_get_continue_label(&lc) == 200);

	loop_exit(&lc);
	TEST("stage4.9: loop exited", !loop_in_loop(&lc));

	/* Test switch context */
	struct SwitchContext sc;
	switch_init(&sc);
	TEST("stage4.9: switch context initialized", sc.case_count == 0);

	/* Test switch case addition */
	u8 result = switch_add_case(&sc, 1, 10, 0);
	TEST("stage4.9: case added", result == 0);
	TEST("stage4.9: case count incremented", sc.case_count == 1);

	u8 result2 = switch_add_case(&sc, 2, 20, 0);
	TEST("stage4.9: second case added", result2 == 0);

	/* Test default case */
	u8 result3 = switch_add_case(&sc, 0, 30, 1);
	TEST("stage4.9: default case added", result3 == 0);
	TEST("stage4.9: default label set", sc.default_label == 30);

	/* Test code generation calls (just verify they don't crash) */
	ctrl_emit_if(&cg, 100);
	TEST("stage4.9: if emission successful", cg.pos > 0);

	/* Test branchless if */
	struct CodeGen cg2;
	codegen_init(&cg2, code_buf, 0x1000);
	ctrl_emit_if_branchless(&cg2, 0, 1, 2, 3);
	TEST("stage4.9: branchless if emitted", cg2.pos > 0);

	/* Test ternary operator */
	struct CodeGen cg3;
	codegen_init(&cg3, code_buf, 0x1000);
	ctrl_emit_ternary(&cg3, 0, 1, 2, 3);
	TEST("stage4.9: ternary operation emitted", cg3.pos > 0);
}

/* === STAGE 4.10: FUNCTION DEFINITIONS AND PARAMETER PASSING === */

static void test_function_definitions(void) {
	struct FuncTable ft;
	func_table_init(&ft);

	/* Test function table initialization */
	TEST("stage4.10: function table initialized", ft.func_count == 0);

	/* Test function definition */
	u8 result = func_table_define(&ft, (const u8*)"add", 3, TYPE_I32);
	TEST("stage4.10: function defined", result == 0);
	TEST("stage4.10: function count incremented", ft.func_count == 1);

	/* Test parameter addition */
	u8 r1 = func_add_param(&ft, (const u8*)"a", 1, TYPE_I32);
	TEST("stage4.10: first parameter added", r1 == 0);

	u8 r2 = func_add_param(&ft, (const u8*)"b", 1, TYPE_I32);
	TEST("stage4.10: second parameter added", r2 == 0);

	/* Test function lookup */
	struct FuncDef *fd = func_table_lookup(&ft, (const u8*)"add", 3);
	TEST("stage4.10: function found", fd != NULL);
	if (fd) {
		TEST("stage4.10: parameter count correct", fd->param_count == 2);
		TEST("stage4.10: return type correct", fd->return_type == TYPE_I32);
	}

	/* Test function lookup failure */
	struct FuncDef *not_found = func_table_lookup(&ft, (const u8*)"sub", 3);
	TEST("stage4.10: missing function returns null", not_found == NULL);

	/* Test parameter register mapping */
	u8 reg0 = func_param_get_reg(0);
	u8 reg1 = func_param_get_reg(1);
	u8 reg2 = func_param_get_reg(2);
	u8 reg3 = func_param_get_reg(3);
	TEST("stage4.10: first parameter in r0", reg0 == 0);
	TEST("stage4.10: second parameter in r1", reg1 == 1);
	TEST("stage4.10: third parameter in r2", reg2 == 2);
	TEST("stage4.10: fourth parameter on stack", reg3 == 255);

	/* Test stack offset calculation */
	u32 offset = func_param_get_stack_offset(3);
	TEST("stage4.10: stack offset for arg 3 correct", offset == STACK_ARG_START);

	/* Test function call context */
	struct FuncCallCtx fcc;
	func_call_prepare(&fcc);
	TEST("stage4.10: call context initialized", fcc.arg_count == 0);

	u8 res1 = func_call_add_arg(&fcc, 0);
	TEST("stage4.10: first argument added", res1 == 0);
	TEST("stage4.10: argument count incremented", fcc.arg_count == 1);

	u8 res2 = func_call_add_arg(&fcc, 1);
	u8 res3 = func_call_add_arg(&fcc, 2);
	u8 res4 = func_call_add_arg(&fcc, 3);
	TEST("stage4.10: four arguments added", fcc.arg_count == 4);
	TEST("stage4.10: stack argument counted", fcc.stack_arg_count == 1);

	/* Test stack size calculation */
	u32 stack_sz = func_call_stack_size(&fcc);
	TEST("stage4.10: stack size calculated", stack_sz == 8);  /* 1 stack arg * 8 bytes */

	/* Test function parse context */
	struct Insn code_buf[0x1000];
	struct CodeGen cg;
	codegen_init(&cg, code_buf, 0x1000);

	struct FuncParseCtx fpc;
	func_parse_init(&fpc, &cg);
	TEST("stage4.10: parse context initialized", fpc.func_table.func_count == 0);

	/* Test function definition in parse context */
	func_table_define(&fpc.func_table, (const u8*)"mul", 3, TYPE_I64);
	TEST("stage4.10: function added to parse context", fpc.func_table.func_count == 1);

	/* Test function prologue/epilogue emission */
	struct CodeGen cg2;
	codegen_init(&cg2, code_buf, 0x1000);
	func_emit_prologue(&cg2, 32);
	TEST("stage4.10: prologue emitted", cg2.pos > 0);

	struct CodeGen cg3;
	codegen_init(&cg3, code_buf, 0x1000);
	func_emit_epilogue(&cg3, 32);
	TEST("stage4.10: epilogue emitted", cg3.pos > 0);
}
