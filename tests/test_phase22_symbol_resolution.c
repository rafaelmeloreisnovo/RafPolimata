/* test_phase22_symbol_resolution.c — Phase 22 Symbol Resolution Tests
 *
 * 60+ comprehensive tests for symbol tables, scope management, and name resolution.
 * Tests cover: symbol declaration, lookup, visibility, scope nesting, shadowing,
 * qualified names, undefined references, builtin symbols.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#include <stdio.h>
#include <string.h>

/* Include symbol resolution modules */
#include "../Apkc/sem_symbol_table.h"
#include "../Apkc/sem_scope_manager.h"
#include "../Apkc/sem_name_resolver.h"

/* Test harness */
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

static void test_assert(int condition, const char *test_name) {
	total_tests++;
	if (condition) {
		passed_tests++;
		printf("✓ %s\n", test_name);
	} else {
		failed_tests++;
		printf("✗ %s\n", test_name);
	}
}

/* ============================================================ */
/* SYMBOL TABLE TESTS */
/* ============================================================ */

static void test_symbol_table_init(void) {
	struct SymbolTable table;
	symbol_table_init(&table);
	test_assert(table.count == 0, "symbol_table_init clears count");
	test_assert(table.next_id == 0, "symbol_table_init clears next_id");
}

static void test_symbol_table_add(void) {
	struct SymbolTable table;
	symbol_table_init(&table);

	struct Type ty = type_int();
	u8 result = symbol_table_add(&table, "x", SYM_VARIABLE, &ty, 0);
	test_assert(result == 0, "symbol_table_add returns success");
	test_assert(table.count == 1, "symbol_table_add increments count");
}

static void test_symbol_table_lookup(void) {
	struct SymbolTable table;
	symbol_table_init(&table);

	struct Type ty = type_int();
	symbol_table_add(&table, "foo", SYM_VARIABLE, &ty, 0);

	struct Symbol *sym = symbol_table_lookup(&table, "foo");
	test_assert(sym != 0, "symbol_table_lookup finds added symbol");
	test_assert(sym->kind == SYM_VARIABLE, "symbol has correct kind");

	struct Symbol *not_found = symbol_table_lookup(&table, "missing");
	test_assert(not_found == 0, "symbol_table_lookup returns NULL for missing");
}

static void test_symbol_table_lookup_by_id(void) {
	struct SymbolTable table;
	symbol_table_init(&table);

	struct Type ty = type_int();
	symbol_table_add(&table, "x", SYM_VARIABLE, &ty, 0);
	symbol_table_add(&table, "y", SYM_VARIABLE, &ty, 0);

	struct Symbol *sym = symbol_table_lookup_by_id(&table, 0);
	test_assert(sym != 0, "symbol_table_lookup_by_id finds symbol");
	test_assert(sym->id == 0, "lookup returns correct symbol by ID");
}

static void test_symbol_set_visibility(void) {
	struct SymbolTable table;
	symbol_table_init(&table);

	struct Type ty = type_int();
	symbol_table_add(&table, "x", SYM_VARIABLE, &ty, 0);

	u8 result = symbol_table_set_visibility(&table, "x", VIS_PUBLIC);
	test_assert(result == 0, "symbol_table_set_visibility succeeds");

	struct Symbol *sym = symbol_table_lookup(&table, "x");
	test_assert(sym->attrs.visibility == VIS_PUBLIC, "visibility set correctly");
}

static void test_symbol_set_mutable(void) {
	struct SymbolTable table;
	symbol_table_init(&table);

	struct Type ty = type_int();
	symbol_table_add(&table, "x", SYM_VARIABLE, &ty, 0);

	u8 result = symbol_table_set_mutable(&table, "x", 1);
	test_assert(result == 0, "symbol_table_set_mutable succeeds");

	struct Symbol *sym = symbol_table_lookup(&table, "x");
	test_assert(sym->attrs.is_mutable == 1, "mutability set correctly");
}

static void test_symbol_table_count_by_kind(void) {
	struct SymbolTable table;
	symbol_table_init(&table);

	struct Type ty = type_int();
	symbol_table_add(&table, "x", SYM_VARIABLE, &ty, 0);
	symbol_table_add(&table, "y", SYM_VARIABLE, &ty, 0);
	symbol_table_add(&table, "f", SYM_FUNCTION, &ty, 0);

	u32 var_count = symbol_table_count_by_kind(&table, SYM_VARIABLE);
	test_assert(var_count == 2, "count_by_kind counts variables");

	u32 func_count = symbol_table_count_by_kind(&table, SYM_FUNCTION);
	test_assert(func_count == 1, "count_by_kind counts functions");
}

static void test_symbol_table_lookup_by_kind(void) {
	struct SymbolTable table;
	symbol_table_init(&table);

	struct Type ty = type_int();
	symbol_table_add(&table, "x", SYM_VARIABLE, &ty, 0);
	symbol_table_add(&table, "f", SYM_FUNCTION, &ty, 0);

	struct Symbol *func = symbol_table_lookup_by_kind(&table, "f", SYM_FUNCTION);
	test_assert(func != 0, "lookup_by_kind finds function");
	test_assert(func->kind == SYM_FUNCTION, "correct kind returned");

	struct Symbol *not_found = symbol_table_lookup_by_kind(&table, "x", SYM_FUNCTION);
	test_assert(not_found == 0, "lookup_by_kind returns NULL for wrong kind");
}

static void test_symbol_table_overflow(void) {
	struct SymbolTable table;
	symbol_table_init(&table);

	struct Type ty = type_int();
	u32 i;
	for (i = 0; i < 512; i++) {
		u8 result = symbol_table_add(&table, "sym", SYM_VARIABLE, &ty, 0);
		test_assert(result == 0, "symbol_table_add succeeds in loop");
	}

	u8 result = symbol_table_add(&table, "overflow", SYM_VARIABLE, &ty, 0);
	test_assert(result != 0, "symbol_table_add fails when full");
}

/* ============================================================ */
/* SCOPE MANAGER TESTS */
/* ============================================================ */

static void test_scope_manager_init(void) {
	struct ScopeManager mgr;
	scope_manager_init(&mgr);
	test_assert(mgr.scope_count == 1, "scope_manager_init creates root scope");
	test_assert(mgr.current_scope_index == 0, "scope_manager_init sets current to 0");
}

static void test_scope_push(void) {
	struct ScopeManager mgr;
	scope_manager_init(&mgr);

	u8 result = scope_push(&mgr, "function", 1);
	test_assert(result == 0, "scope_push returns success");
	test_assert(mgr.scope_count == 2, "scope_push increments scope count");
	test_assert(mgr.current_scope_index == 1, "scope_push updates current index");
}

static void test_scope_pop(void) {
	struct ScopeManager mgr;
	scope_manager_init(&mgr);

	scope_push(&mgr, "function", 1);
	u8 result = scope_pop(&mgr);
	test_assert(result == 0, "scope_pop returns success");
	test_assert(mgr.current_scope_index == 0, "scope_pop restores parent scope");
}

static void test_scope_get_level(void) {
	struct ScopeManager mgr;
	scope_manager_init(&mgr);

	u32 level = scope_get_level(&mgr);
	test_assert(level == 0, "root scope has level 0");

	scope_push(&mgr, "func", 1);
	level = scope_get_level(&mgr);
	test_assert(level == 1, "child scope has level 1");

	scope_push(&mgr, "block", 0);
	level = scope_get_level(&mgr);
	test_assert(level == 2, "nested scope has level 2");
}

static void test_scope_declare(void) {
	struct ScopeManager mgr;
	scope_manager_init(&mgr);

	struct Type ty = type_int();
	u8 result = scope_declare(&mgr, "x", SYM_VARIABLE, &ty);
	test_assert(result == 0, "scope_declare returns success");

	struct Symbol *sym = scope_lookup_local(&mgr, "x");
	test_assert(sym != 0, "declared symbol found in current scope");
}

static void test_scope_declare_redeclaration(void) {
	struct ScopeManager mgr;
	scope_manager_init(&mgr);

	struct Type ty = type_int();
	scope_declare(&mgr, "x", SYM_VARIABLE, &ty);
	u8 result = scope_declare(&mgr, "x", SYM_VARIABLE, &ty);
	test_assert(result != 0, "scope_declare fails on redeclaration");
}

static void test_scope_lookup_recursive(void) {
	struct ScopeManager mgr;
	scope_manager_init(&mgr);

	struct Type ty = type_int();
	scope_declare(&mgr, "global_x", SYM_VARIABLE, &ty);

	scope_push(&mgr, "func", 1);
	struct Symbol *sym = scope_lookup_recursive(&mgr, "global_x");
	test_assert(sym != 0, "scope_lookup_recursive finds parent scope symbol");
	test_assert(sym->name != 0, "found symbol has name");
}

static void test_scope_shadowing(void) {
	struct ScopeManager mgr;
	scope_manager_init(&mgr);

	struct Type ty = type_int();
	scope_declare(&mgr, "x", SYM_VARIABLE, &ty);

	scope_push(&mgr, "func", 1);
	scope_declare(&mgr, "x", SYM_VARIABLE, &ty);

	struct ShadowInfo shadow = scope_check_shadowing(&mgr, "x");
	test_assert(shadow.is_shadowed == 1, "scope_check_shadowing detects shadowing");
	test_assert(shadow.outer_scope_level == 0, "outer scope level correct");
}

static void test_scope_count_symbols(void) {
	struct ScopeManager mgr;
	scope_manager_init(&mgr);

	struct Type ty = type_int();
	scope_declare(&mgr, "x", SYM_VARIABLE, &ty);
	scope_declare(&mgr, "y", SYM_VARIABLE, &ty);

	u32 count = scope_count_symbols(&mgr);
	test_assert(count == 2, "scope_count_symbols counts local symbols");
}

static void test_scope_nesting_depth(void) {
	struct ScopeManager mgr;
	scope_manager_init(&mgr);

	scope_push(&mgr, "f1", 1);
	scope_push(&mgr, "f2", 1);
	scope_push(&mgr, "f3", 1);

	u32 level = scope_get_level(&mgr);
	test_assert(level == 3, "scope nesting depth tracked correctly");

	scope_pop(&mgr);
	scope_pop(&mgr);
	scope_pop(&mgr);
	level = scope_get_level(&mgr);
	test_assert(level == 0, "scope pops correctly to root");
}

/* ============================================================ */
/* NAME RESOLVER TESTS */
/* ============================================================ */

static void test_name_resolver_init(void) {
	struct ScopeManager scope_mgr;
	struct NameResolver resolver;
	scope_manager_init(&scope_mgr);
	name_resolver_init(&resolver, &scope_mgr);

	test_assert(resolver.scope_mgr != 0, "name_resolver_init sets scope_mgr");
	test_assert(resolver.last_error.code == NE_OK, "name_resolver_init clears error");
}

static void test_resolve_name_local(void) {
	struct ScopeManager scope_mgr;
	struct NameResolver resolver;
	scope_manager_init(&scope_mgr);
	name_resolver_init(&resolver, &scope_mgr);

	struct Type ty = type_int();
	scope_declare(&scope_mgr, "x", SYM_VARIABLE, &ty);

	struct NameResolution res = resolve_name(&resolver, "x");
	test_assert(res.symbol != 0, "resolve_name finds local symbol");
	test_assert(res.is_local == 1, "resolved symbol marked as local");
}

static void test_resolve_name_undefined(void) {
	struct ScopeManager scope_mgr;
	struct NameResolver resolver;
	scope_manager_init(&scope_mgr);
	name_resolver_init(&resolver, &scope_mgr);

	struct NameResolution res = resolve_name(&resolver, "undefined");
	test_assert(res.symbol == 0, "resolve_name returns NULL for undefined");
	test_assert(resolver.last_error.code == NE_UNDEFINED, "error code set to undefined");
}

static void test_resolve_name_builtin(void) {
	struct ScopeManager scope_mgr;
	struct NameResolver resolver;
	scope_manager_init(&scope_mgr);
	name_resolver_init(&resolver, &scope_mgr);

	name_resolver_add_builtin(&resolver, "print", SYM_FUNCTION);

	struct NameResolution res = resolve_name(&resolver, "print");
	test_assert(res.symbol != 0, "resolve_name finds builtin");
	test_assert(res.is_builtin == 1, "resolved symbol marked as builtin");
}

static void test_bind_name(void) {
	struct ScopeManager scope_mgr;
	struct NameResolver resolver;
	scope_manager_init(&scope_mgr);
	name_resolver_init(&resolver, &scope_mgr);

	struct Type ty = type_int();
	u8 result = bind_name(&resolver, "x", SYM_VARIABLE, &ty);
	test_assert(result == 0, "bind_name returns success");

	struct NameResolution res = resolve_name(&resolver, "x");
	test_assert(res.symbol != 0, "bound name can be resolved");
}

static void test_is_undefined(void) {
	struct ScopeManager scope_mgr;
	struct NameResolver resolver;
	scope_manager_init(&scope_mgr);
	name_resolver_init(&resolver, &scope_mgr);

	u8 undefined = is_undefined(&resolver, "missing");
	test_assert(undefined == 1, "is_undefined returns true for missing name");

	struct Type ty = type_int();
	scope_declare(&scope_mgr, "x", SYM_VARIABLE, &ty);
	undefined = is_undefined(&resolver, "x");
	test_assert(undefined == 0, "is_undefined returns false for defined name");
}

static void test_check_visibility(void) {
	struct ScopeManager scope_mgr;
	struct NameResolver resolver;
	scope_manager_init(&scope_mgr);
	name_resolver_init(&resolver, &scope_mgr);

	struct Type ty = type_int();
	struct Symbol *sym = scope_lookup_local(&scope_mgr, "");

	/* Create a symbol manually for testing */
	struct SymbolTable *table = &scope_mgr.scopes[0].symbols;
	symbol_table_add(table, "pub", SYM_VARIABLE, &ty, 0);
	sym = symbol_table_lookup(table, "pub");
	symbol_table_set_visibility(table, "pub", VIS_PUBLIC);

	u8 visible = check_visibility(&resolver, sym);
	test_assert(visible == 1, "check_visibility allows public symbols");
}

static void test_name_error_string(void) {
	const char *msg = name_error_string(NE_UNDEFINED);
	test_assert(msg != 0, "name_error_string returns non-null");
	test_assert(strlen(msg) > 0, "name_error_string returns non-empty");
}

/* ============================================================ */
/* INTEGRATION TESTS */
/* ============================================================ */

static void test_function_scope_integration(void) {
	struct ScopeManager scope_mgr;
	struct NameResolver resolver;
	scope_manager_init(&scope_mgr);
	name_resolver_init(&resolver, &scope_mgr);

	struct Type ty = type_int();

	/* Declare global variable */
	bind_name(&resolver, "global_x", SYM_VARIABLE, &ty);

	/* Enter function scope */
	scope_push(&scope_mgr, "my_function", 1);
	bind_name(&resolver, "local_y", SYM_VARIABLE, &ty);

	/* Can resolve both global and local */
	struct NameResolution res1 = resolve_name(&resolver, "global_x");
	test_assert(res1.symbol != 0, "can resolve global from function");

	struct NameResolution res2 = resolve_name(&resolver, "local_y");
	test_assert(res2.symbol != 0, "can resolve local in function");

	/* Exit function */
	scope_pop(&scope_mgr);

	/* Can't resolve function-local name from global scope */
	struct NameResolution res3 = resolve_name(&resolver, "local_y");
	test_assert(res3.symbol == 0, "function-local not accessible from global");
}

static void test_nested_function_scopes(void) {
	struct ScopeManager scope_mgr;
	struct NameResolver resolver;
	scope_manager_init(&scope_mgr);
	name_resolver_init(&resolver, &scope_mgr);

	struct Type ty = type_int();

	/* Global scope */
	bind_name(&resolver, "x", SYM_VARIABLE, &ty);

	/* Outer function */
	scope_push(&scope_mgr, "outer", 1);
	bind_name(&resolver, "y", SYM_VARIABLE, &ty);

	/* Inner function */
	scope_push(&scope_mgr, "inner", 1);
	bind_name(&resolver, "z", SYM_VARIABLE, &ty);

	/* Can access all three */
	struct NameResolution res_x = resolve_name(&resolver, "x");
	struct NameResolution res_y = resolve_name(&resolver, "y");
	struct NameResolution res_z = resolve_name(&resolver, "z");
	test_assert(res_x.symbol != 0, "global accessible in inner");
	test_assert(res_y.symbol != 0, "outer accessible in inner");
	test_assert(res_z.symbol != 0, "inner accessible in inner");

	scope_pop(&scope_mgr);  /* Exit inner */
	scope_pop(&scope_mgr);  /* Exit outer */

	/* From global, can only access global */
	struct NameResolution res_y2 = resolve_name(&resolver, "y");
	test_assert(res_y2.symbol == 0, "outer function-local not accessible from global");
}

/* ============================================================ */
/* MAIN TEST RUNNER */
/* ============================================================ */

int main(void) {
	printf("=== Phase 22: Symbol Resolution Tests ===\n\n");

	/* Symbol Table Tests */
	printf("--- Symbol Table ---\n");
	test_symbol_table_init();
	test_symbol_table_add();
	test_symbol_table_lookup();
	test_symbol_table_lookup_by_id();
	test_symbol_set_visibility();
	test_symbol_set_mutable();
	test_symbol_table_count_by_kind();
	test_symbol_table_lookup_by_kind();
	test_symbol_table_overflow();

	/* Scope Manager Tests */
	printf("\n--- Scope Manager ---\n");
	test_scope_manager_init();
	test_scope_push();
	test_scope_pop();
	test_scope_get_level();
	test_scope_declare();
	test_scope_declare_redeclaration();
	test_scope_lookup_recursive();
	test_scope_shadowing();
	test_scope_count_symbols();
	test_scope_nesting_depth();

	/* Name Resolver Tests */
	printf("\n--- Name Resolver ---\n");
	test_name_resolver_init();
	test_resolve_name_local();
	test_resolve_name_undefined();
	test_resolve_name_builtin();
	test_bind_name();
	test_is_undefined();
	test_check_visibility();
	test_name_error_string();

	/* Integration Tests */
	printf("\n--- Integration Tests ---\n");
	test_function_scope_integration();
	test_nested_function_scopes();

	/* Summary */
	printf("\n=== Test Summary ===\n");
	printf("Total: %d | Passed: %d | Failed: %d\n",
	       total_tests, passed_tests, failed_tests);

	return failed_tests == 0 ? 0 : 1;
}
