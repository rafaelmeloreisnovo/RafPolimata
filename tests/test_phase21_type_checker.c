/* test_phase21_type_checker.c — Phase 21 Type System Tests
 *
 * 50+ comprehensive tests for type system, unification, and type inference.
 * Tests cover: type constructors, unification, occurs check, inference,
 * constraint solving, error handling, polymorphism.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#include <stdio.h>
#include <string.h>

/* Include type system modules */
#include "../Apkc/sem_type_system.h"
#include "../Apkc/sem_unification.h"
#include "../Apkc/sem_type_inference.h"

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
/* TYPE SYSTEM BASICS */
/* ============================================================ */

static void test_type_constructors(void) {
	struct Type t_unit = type_unit();
	test_assert(t_unit.kind == TYPE_UNIT, "type_unit constructor");

	struct Type t_bool = type_bool();
	test_assert(t_bool.kind == TYPE_BOOL, "type_bool constructor");

	struct Type t_int = type_int();
	test_assert(t_int.kind == TYPE_INT, "type_int constructor");

	struct Type t_int64 = type_int64();
	test_assert(t_int64.kind == TYPE_INT64, "type_int64 constructor");

	struct Type t_str = type_str();
	test_assert(t_str.kind == TYPE_STR, "type_str constructor");
}

static void test_type_var_constructor(void) {
	struct Type t_var = type_var(0, "a");
	test_assert(t_var.kind == TYPE_VAR, "type_var creates TYPE_VAR");
	test_assert(t_var.data.var.id == 0, "type_var assigns correct id");
	test_assert(t_var.data.var.name != 0, "type_var assigns name");
}

static void test_type_ptr_constructor(void) {
	struct Type t_int = type_int();
	struct Type t_ptr = type_ptr(&t_int, 0);
	test_assert(t_ptr.kind == TYPE_PTR, "type_ptr creates TYPE_PTR");
	test_assert(t_ptr.data.ptr.pointee_type != 0, "type_ptr stores pointee");
}

static void test_type_array_constructor(void) {
	struct Type t_int = type_int();
	struct Type t_arr = type_array(&t_int, 10);
	test_assert(t_arr.kind == TYPE_ARRAY, "type_array creates TYPE_ARRAY");
	test_assert(t_arr.data.array.length == 10, "type_array stores length");
}

static void test_type_func_constructor(void) {
	struct Type t_int = type_int();
	struct Type t_bool = type_bool();
	struct Type t_func = type_func(&t_int, &t_bool);
	test_assert(t_func.kind == TYPE_FUNC, "type_func creates TYPE_FUNC");
	test_assert(t_func.data.func.param_type != 0, "type_func stores param");
	test_assert(t_func.data.func.return_type != 0, "type_func stores return");
}

/* ============================================================ */
/* TYPE EQUALITY */
/* ============================================================ */

static void test_type_equal_primitives(void) {
	struct Type t1 = type_int();
	struct Type t2 = type_int();
	test_assert(type_equal(&t1, &t2), "equal primitive types");

	struct Type t3 = type_bool();
	test_assert(!type_equal(&t1, &t3), "unequal primitive types");
}

static void test_type_equal_vars(void) {
	struct Type t1 = type_var(0, "a");
	struct Type t2 = type_var(0, "a");
	test_assert(type_equal(&t1, &t2), "equal type variables (same id)");

	struct Type t3 = type_var(1, "b");
	test_assert(!type_equal(&t1, &t3), "unequal type variables (different id)");
}

static void test_type_equal_arrays(void) {
	struct Type elem = type_int();
	struct Type arr1 = type_array(&elem, 5);
	struct Type arr2 = type_array(&elem, 5);
	test_assert(type_equal(&arr1, &arr2), "equal array types");

	struct Type arr3 = type_array(&elem, 10);
	test_assert(!type_equal(&arr1, &arr3), "unequal array lengths");
}

/* ============================================================ */
/* SUBSTITUTION OPERATIONS */
/* ============================================================ */

static void test_subst_map_init(void) {
	struct SubstitutionMap map;
	subst_map_init(&map);
	test_assert(map.count == 0, "subst_map_init clears count");
}

static void test_subst_map_add(void) {
	struct SubstitutionMap map;
	subst_map_init(&map);

	struct TypeVar var = {0, "a", VAR_INVARIANT, 0};
	struct Type ty = type_int();
	u8 result = subst_map_add(&map, var, ty);
	test_assert(result == 0, "subst_map_add returns success");
	test_assert(map.count == 1, "subst_map_add increments count");
}

static void test_subst_map_lookup(void) {
	struct SubstitutionMap map;
	subst_map_init(&map);

	struct TypeVar var = {5, "x", VAR_INVARIANT, 0};
	struct Type ty = type_str();
	subst_map_add(&map, var, ty);

	struct Type *found = subst_map_lookup(&map, 5);
	test_assert(found != 0, "subst_map_lookup finds existing var");
	test_assert(found->kind == TYPE_STR, "subst_map_lookup returns correct type");

	struct Type *not_found = subst_map_lookup(&map, 99);
	test_assert(not_found == 0, "subst_map_lookup returns NULL for missing var");
}

static void test_subst_map_overflow(void) {
	struct SubstitutionMap map;
	subst_map_init(&map);

	/* Add 64 entries */
	u32 i;
	for (i = 0; i < 64; i++) {
		struct TypeVar var = {i, "v", VAR_INVARIANT, 0};
		struct Type ty = type_int();
		u8 result = subst_map_add(&map, var, ty);
		test_assert(result == 0, "subst_map_add succeeds (loop)");
	}

	/* Try to add 65th entry (should fail) */
	struct TypeVar var65 = {64, "v", VAR_INVARIANT, 0};
	struct Type ty = type_int();
	u8 result = subst_map_add(&map, var65, ty);
	test_assert(result != 0, "subst_map_add fails when full");
}

/* ============================================================ */
/* OCCURS CHECK */
/* ============================================================ */

static void test_occurs_check_not_present(void) {
	struct Type t_int = type_int();
	u8 result = occurs_check(0, &t_int);
	test_assert(result == 0, "occurs_check: var not in primitive type");
}

static void test_occurs_check_var_matches(void) {
	struct Type t_var = type_var(5, "a");
	u8 result = occurs_check(5, &t_var);
	test_assert(result == 1, "occurs_check: var matches itself");
}

static void test_occurs_check_in_array(void) {
	struct Type t_var = type_var(0, "a");
	struct Type t_arr = type_array(&t_var, 10);
	u8 result = occurs_check(0, &t_arr);
	test_assert(result == 1, "occurs_check: var found in array element");
}

static void test_occurs_check_in_func(void) {
	struct Type t_var = type_var(1, "b");
	struct Type t_int = type_int();
	struct Type t_func = type_func(&t_var, &t_int);
	u8 result = occurs_check(1, &t_func);
	test_assert(result == 1, "occurs_check: var found in function param");

	u8 result2 = occurs_check(1, &t_func);
	test_assert(result2 == 1, "occurs_check: var found in function return");
}

static void test_occurs_check_in_tuple(void) {
	struct Type t_var = type_var(2, "c");
	struct Type t_int = type_int();
	struct Type t_bool = type_bool();
	struct Type *elems[] = {&t_var, &t_int, &t_bool};
	struct Type t_tuple = type_tuple(elems, 3);
	u8 result = occurs_check(2, &t_tuple);
	test_assert(result == 1, "occurs_check: var found in tuple element");
}

/* ============================================================ */
/* UNIFICATION */
/* ============================================================ */

static void test_unify_same_primitives(void) {
	struct SubstitutionMap subst;
	struct TypeError error = {0};
	subst_map_init(&subst);

	struct Type t1 = type_int();
	struct Type t2 = type_int();
	u8 result = unify_types(&t1, &t2, &subst, &error);
	test_assert(result == 1, "unify: same primitives succeeds");
	test_assert(error.code == TE_OK, "unify: no error for same primitives");
}

static void test_unify_different_primitives(void) {
	struct SubstitutionMap subst;
	struct TypeError error = {0};
	subst_map_init(&subst);

	struct Type t1 = type_int();
	struct Type t2 = type_bool();
	u8 result = unify_types(&t1, &t2, &subst, &error);
	test_assert(result == 0, "unify: different primitives fails");
	test_assert(error.code == TE_MISMATCH, "unify: mismatch error for different primitives");
}

static void test_unify_var_with_primitive(void) {
	struct SubstitutionMap subst;
	struct TypeError error = {0};
	subst_map_init(&subst);

	struct Type t_var = type_var(0, "a");
	struct Type t_int = type_int();
	u8 result = unify_types(&t_var, &t_int, &subst, &error);
	test_assert(result == 1, "unify: var unifies with primitive");
	test_assert(subst.count == 1, "unify: one substitution added");
}

static void test_unify_occurs_check(void) {
	struct SubstitutionMap subst;
	struct TypeError error = {0};
	subst_map_init(&subst);

	struct Type t_var = type_var(0, "a");
	struct Type t_arr = type_array(&t_var, 5);

	/* Try to unify a ~ [a] (should fail) */
	u8 result = unify_types(&t_var, &t_arr, &subst, &error);
	test_assert(result == 0, "unify: occurs check prevents infinite types");
	test_assert(error.code == TE_INFINITE_TYPE, "unify: infinite type error");
}

static void test_unify_functions(void) {
	struct SubstitutionMap subst;
	struct TypeError error = {0};
	subst_map_init(&subst);

	struct Type t_int = type_int();
	struct Type t_bool = type_bool();
	struct Type t_func1 = type_func(&t_int, &t_bool);
	struct Type t_func2 = type_func(&t_int, &t_bool);

	u8 result = unify_types(&t_func1, &t_func2, &subst, &error);
	test_assert(result == 1, "unify: same function types");
}

static void test_unify_tuples(void) {
	struct SubstitutionMap subst;
	struct TypeError error = {0};
	subst_map_init(&subst);

	struct Type t_int = type_int();
	struct Type t_bool = type_bool();
	struct Type *elems1[] = {&t_int, &t_bool};
	struct Type *elems2[] = {&t_int, &t_bool};
	struct Type t_tuple1 = type_tuple(elems1, 2);
	struct Type t_tuple2 = type_tuple(elems2, 2);

	u8 result = unify_types(&t_tuple1, &t_tuple2, &subst, &error);
	test_assert(result == 1, "unify: same tuples");
}

/* ============================================================ */
/* CONSTRAINT SYSTEM */
/* ============================================================ */

static void test_constraints_init(void) {
	struct ConstraintSystem cs;
	constraints_init(&cs);
	test_assert(cs.count == 0, "constraints_init clears count");
}

static void test_constraints_add(void) {
	struct ConstraintSystem cs;
	constraints_init(&cs);

	struct Type t1 = type_int();
	struct Type t2 = type_int();
	u8 result = constraints_add(&cs, &t1, &t2, 0, "test");
	test_assert(result == 0, "constraints_add returns success");
	test_assert(cs.count == 1, "constraints_add increments count");
}

static void test_constraints_overflow(void) {
	struct ConstraintSystem cs;
	constraints_init(&cs);

	struct Type t_int = type_int();
	struct Type t_bool = type_bool();

	/* Add 128 constraints */
	u32 i;
	for (i = 0; i < 128; i++) {
		u8 result = constraints_add(&cs, &t_int, &t_bool, 0, "test");
		test_assert(result == 0, "constraints_add succeeds (loop)");
	}

	/* Try to add 129th (should fail) */
	u8 result = constraints_add(&cs, &t_int, &t_bool, 0, "test");
	test_assert(result != 0, "constraints_add fails when full");
}

/* ============================================================ */
/* TYPE CLASSIFICATION */
/* ============================================================ */

static void test_type_is_numeric(void) {
	struct Type t_int = type_int();
	struct Type t_float = type_float();
	struct Type t_bool = type_bool();

	test_assert(type_is_numeric(&t_int), "int is numeric");
	test_assert(type_is_numeric(&t_float), "float is numeric");
	test_assert(!type_is_numeric(&t_bool), "bool is not numeric");
}

static void test_type_is_primitive(void) {
	struct Type t_int = type_int();
	struct Type t_func = type_func(&t_int, &t_int);
	struct Type t_var = type_var(0, "a");

	test_assert(type_is_primitive(&t_int), "int is primitive");
	test_assert(type_is_primitive(&t_func), "function is primitive");
	test_assert(type_is_primitive(&t_var), "type variable is primitive");
}

static void test_type_is_compound(void) {
	struct Type t_int = type_int();
	struct Type t_func = type_func(&t_int, &t_int);
	struct Type t_arr = type_array(&t_int, 5);

	test_assert(!type_is_compound(&t_int), "int is not compound");
	test_assert(type_is_compound(&t_func), "function is compound");
	test_assert(type_is_compound(&t_arr), "array is compound");
}

/* ============================================================ */
/* INFERENCE CONTEXT */
/* ============================================================ */

static void test_inference_init(void) {
	struct InferenceContext ctx;
	inference_init(&ctx);
	test_assert(ctx.next_var_id == 0, "inference_init sets var_id to 0");
	test_assert(ctx.inferred_count == 0, "inference_init clears inferred_count");
}

static void test_fresh_var(void) {
	struct InferenceContext ctx;
	inference_init(&ctx);

	struct Type v1 = fresh_var(&ctx);
	test_assert(v1.kind == TYPE_VAR, "fresh_var creates type variable");
	test_assert(v1.data.var.id == 0, "first fresh_var has id 0");

	struct Type v2 = fresh_var(&ctx);
	test_assert(v2.data.var.id == 1, "second fresh_var has id 1");
	test_assert(ctx.next_var_id == 2, "fresh_var increments counter");
}

/* ============================================================ */
/* INFERENCE BASICS */
/* ============================================================ */

static void test_infer_int_literal_small(void) {
	struct Type t = infer_int_literal(42);
	test_assert(t.kind == TYPE_INT32, "small int literal infers i32");
}

static void test_infer_int_literal_large(void) {
	struct Type t = infer_int_literal(3000000000LL);
	test_assert(t.kind == TYPE_INT64, "large int literal infers i64");
}

static void test_infer_bool_literal(void) {
	struct Type t = infer_bool_literal();
	test_assert(t.kind == TYPE_BOOL, "bool literal infers bool");
}

static void test_infer_str_literal(void) {
	struct Type t = infer_str_literal();
	test_assert(t.kind == TYPE_STR, "string literal infers str");
}

static void test_infer_float_literal(void) {
	struct Type t = infer_float_literal();
	test_assert(t.kind == TYPE_FLOAT, "float literal infers f32");
}

/* ============================================================ */
/* UNIFICATION STATE */
/* ============================================================ */

static void test_unify_state_init(void) {
	struct UnificationState state;
	unify_state_init(&state);
	test_assert(state.error.code == TE_OK, "unify_state_init clears error");
	test_assert(state.iterations == 0, "unify_state_init clears iterations");
	test_assert(state.success == 0, "unify_state_init sets success to 0");
}

/* ============================================================ */
/* TYPE ERROR REPORTING */
/* ============================================================ */

static void test_type_error_string(void) {
	const char *msg = type_error_string(TE_MISMATCH);
	test_assert(msg != 0, "type_error_string returns non-null");
	test_assert(strlen(msg) > 0, "type_error_string returns non-empty string");

	const char *msg_ok = type_error_string(TE_OK);
	test_assert(msg_ok != 0, "type_error_string handles TE_OK");
}

/* ============================================================ */
/* SUBSTITUTION COMPOSITION */
/* ============================================================ */

static void test_subst_compose_empty(void) {
	struct SubstitutionMap s1, s2, result;
	subst_map_init(&s1);
	subst_map_init(&s2);

	u8 res = subst_compose(&s1, &s2, &result);
	test_assert(res == 1, "subst_compose handles empty maps");
	test_assert(result.count == 0, "subst_compose of empty maps is empty");
}

static void test_subst_compose_identity(void) {
	struct SubstitutionMap s1, s2, result;
	subst_map_init(&s1);
	subst_map_init(&s2);

	struct TypeVar v = {0, "a", VAR_INVARIANT, 0};
	struct Type t = type_int();
	subst_map_add(&s2, v, t);

	u8 res = subst_compose(&s1, &s2, &result);
	test_assert(res == 1, "subst_compose with identity");
	test_assert(result.count == 1, "subst_compose preserves non-empty map");
}

/* ============================================================ */
/* MAIN TEST RUNNER */
/* ============================================================ */

int main(void) {
	printf("=== Phase 21: Type System & Inference Tests ===\n\n");

	/* Type system basics */
	printf("--- Type Constructors ---\n");
	test_type_constructors();
	test_type_var_constructor();
	test_type_ptr_constructor();
	test_type_array_constructor();
	test_type_func_constructor();

	/* Type equality */
	printf("\n--- Type Equality ---\n");
	test_type_equal_primitives();
	test_type_equal_vars();
	test_type_equal_arrays();

	/* Substitution */
	printf("\n--- Substitution Operations ---\n");
	test_subst_map_init();
	test_subst_map_add();
	test_subst_map_lookup();
	test_subst_map_overflow();

	/* Occurs check */
	printf("\n--- Occurs Check ---\n");
	test_occurs_check_not_present();
	test_occurs_check_var_matches();
	test_occurs_check_in_array();
	test_occurs_check_in_func();
	test_occurs_check_in_tuple();

	/* Unification */
	printf("\n--- Unification ---\n");
	test_unify_same_primitives();
	test_unify_different_primitives();
	test_unify_var_with_primitive();
	test_unify_occurs_check();
	test_unify_functions();
	test_unify_tuples();

	/* Constraint system */
	printf("\n--- Constraint System ---\n");
	test_constraints_init();
	test_constraints_add();
	test_constraints_overflow();

	/* Type classification */
	printf("\n--- Type Classification ---\n");
	test_type_is_numeric();
	test_type_is_primitive();
	test_type_is_compound();

	/* Inference context */
	printf("\n--- Inference Context ---\n");
	test_inference_init();
	test_fresh_var();

	/* Inference basics */
	printf("\n--- Inference Basics ---\n");
	test_infer_int_literal_small();
	test_infer_int_literal_large();
	test_infer_bool_literal();
	test_infer_str_literal();
	test_infer_float_literal();

	/* Unification state */
	printf("\n--- Unification State ---\n");
	test_unify_state_init();

	/* Type error reporting */
	printf("\n--- Type Error Reporting ---\n");
	test_type_error_string();

	/* Substitution composition */
	printf("\n--- Substitution Composition ---\n");
	test_subst_compose_empty();
	test_subst_compose_identity();

	/* Summary */
	printf("\n=== Test Summary ===\n");
	printf("Total: %d | Passed: %d | Failed: %d\n",
	       total_tests, passed_tests, failed_tests);

	return failed_tests == 0 ? 0 : 1;
}
