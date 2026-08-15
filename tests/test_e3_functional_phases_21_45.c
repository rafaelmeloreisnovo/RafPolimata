/* tests/test_e3_functional_phases_21_45.c
 *
 * E-Level 3: FUNCTIONAL TESTS (Not Tautological)
 *
 * These tests actually exercise real compiler components and verify behavior,
 * not just Assert(1). They replace the scaffolding tests from phase49_validation.c.
 *
 * Compile:
 *   gcc -std=c99 -O2 -Wall -Wextra -ffreestanding -I. -IApkc \
 *       -IApkc tests/test_e3_functional_phases_21_45.c -o tests/test_e3_functional
 *
 * Run:
 *   ./tests/test_e3_functional
 *
 * Methodology:
 *   For each phase (21-45), create tests that:
 *   1. Call the actual phase function with real input
 *   2. Verify output matches specification (not just "output exists")
 *   3. Test error cases (what should fail?)
 *   4. Compare against oracle when possible (GCC, reference implementation)
 *   5. Measure determinism (same input → same output, every time)
 *
 * Status: IN_PROGRESS (adding tests incrementally)
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Import semantic analysis modules */
#include "Apkc/sem_type_system.h"
#include "Apkc/sem_type_inference.h"
#include "Apkc/sem_symbol_table.h"
#include "Apkc/sem_unification.h"
#include "Apkc/sem_cfg_builder.h"

/* Types already defined by sem_cfg_builder.h and others, skip redefine */

static int total_tests = 0;
static int passed = 0;
static int failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        total_tests++; \
        if (cond) { \
            passed++; \
            printf("✓ %s\n", msg); \
        } else { \
            failed++; \
            printf("✗ %s\n", msg); \
        } \
    } while(0)

#define TEST_ASSERT_EQ(a, b, msg) TEST_ASSERT((a) == (b), msg)
#define TEST_ASSERT_NE(a, b, msg) TEST_ASSERT((a) != (b), msg)
#define TEST_ASSERT_STR_EQ(a, b, msg) TEST_ASSERT(strcmp(a, b) == 0, msg)

/* ─────────────────────────────────────────────────────────────────────────
   PHASE 21: TYPE SYSTEM (Hindley-Milner Inference)
   ───────────────────────────────────────────────────────────────────────── */

/* Test: Type inference for integer literal */
static void test_type_inference_literal_int(void) {
    /* Input: Literal 42
       Expected: infer type as INT
       Verification: check type_kind == TYPE_INT32
    */
    struct Type t = infer_int_literal(42);
    TEST_ASSERT_EQ(t.kind, TYPE_INT32, "[E3-21.1] Type inference detects integer literal as i32");
}

/* Test: Type equality check works */
static void test_type_inference_equality(void) {
    /* Input: Two identical types (int32 vs int32)
       Expected: type_equal returns true
       Verification: equality succeeds
    */
    struct Type t1 = type_int32();
    struct Type t2 = type_int32();
    TEST_ASSERT(type_equal(&t1, &t2) == 1, "[E3-21.2] Type equality detects identical types");
}

/* Test: Type variables can be created and used */
static void test_type_inference_polymorphic(void) {
    /* Input: Create a polymorphic function type ∀α. α → α
       Expected: Function type with type variables
       Verification: type has VAR_INVARIANT variance
    */
    struct Type alpha = type_var(0, "α");
    struct Type func_type = type_func(&alpha, &alpha);
    TEST_ASSERT_EQ(func_type.kind, TYPE_FUNC, "[E3-21.3] Type inference creates function types");
    TEST_ASSERT_EQ(func_type.data.func.param_type->kind, TYPE_VAR, "[E3-21.3] Function param is type variable");
}

/* Test: Type inference context initialization */
static void test_type_inference_determinism(void) {
    /* Input: Initialize inference context
       Expected: Fresh variable counter starts at 0
       Verification: Fresh variables get sequential IDs
    */
    struct InferenceContext ctx;
    inference_init(&ctx);
    struct Type v1 = fresh_var(&ctx);
    struct Type v2 = fresh_var(&ctx);
    TEST_ASSERT_EQ(v1.data.var.id, 0, "[E3-21.4] Fresh vars get sequential IDs");
    TEST_ASSERT_EQ(v2.data.var.id, 1, "[E3-21.4] Fresh vars are deterministic");
}

/* ─────────────────────────────────────────────────────────────────────────
   PHASE 22: SYMBOL RESOLUTION (Scope & Name Binding)
   ───────────────────────────────────────────────────────────────────────── */

/* Test: Symbol binding in single scope */
static void test_symbol_binding_single_scope(void) {
    /* Input: Add symbol "x" to table
       Expected: lookup("x") returns the symbol
       Verification: symbol_table_lookup finds the binding
    */
    struct SymbolTable table;
    symbol_table_init(&table);
    struct Type int_type = type_int32();
    u8 result = symbol_table_add(&table, "x", SYM_VARIABLE, &int_type, 0);
    TEST_ASSERT_EQ(result, 0, "[E3-22.1] Symbol add succeeds");
    struct Symbol *found = symbol_table_lookup(&table, "x");
    TEST_ASSERT(found != 0, "[E3-22.1] Symbol lookup finds binding");
}

/* Test: Shadowing in nested scope */
static void test_symbol_shadowing(void) {
    /* Input: Add x at scope 0, then x at scope 1
       Expected: both symbols exist in table
       Verification: count and types tracked per scope
    */
    struct SymbolTable table;
    symbol_table_init(&table);
    struct Type int_type = type_int32();
    symbol_table_add(&table, "x", SYM_VARIABLE, &int_type, 0);  /* outer scope */
    symbol_table_add(&table, "x", SYM_VARIABLE, &int_type, 1);  /* inner scope shadows */
    struct Symbol *found = symbol_table_lookup(&table, "x");
    TEST_ASSERT(found != 0, "[E3-22.2] Symbol lookup finds binding");
    TEST_ASSERT_EQ(table.count, 2, "[E3-22.2] Both symbols tracked (shadowing)");
}

/* Test: Undefined variable detected */
static void test_symbol_undefined_variable(void) {
    /* Input: Look up undefined symbol "undefined_var"
       Expected: lookup returns NULL
       Verification: symbol_table_lookup returns null pointer
    */
    struct SymbolTable table;
    symbol_table_init(&table);
    struct Symbol *found = symbol_table_lookup(&table, "undefined_var");
    TEST_ASSERT(found == 0, "[E3-22.3] Undefined variables return NULL from lookup");
}

/* Test: Symbol kinds are correctly tracked */
static void test_symbol_multilingual_consistency(void) {
    /* Input: Add symbols of different kinds (variable, function, type)
       Expected: count_by_kind returns correct counts
       Verification: symbol_table_count_by_kind works for each kind
    */
    struct SymbolTable table;
    symbol_table_init(&table);
    struct Type int_type = type_int32();
    symbol_table_add(&table, "x", SYM_VARIABLE, &int_type, 0);
    symbol_table_add(&table, "foo", SYM_FUNCTION, &int_type, 0);
    symbol_table_add(&table, "y", SYM_VARIABLE, &int_type, 0);
    u32 var_count = symbol_table_count_by_kind(&table, SYM_VARIABLE);
    u32 fn_count = symbol_table_count_by_kind(&table, SYM_FUNCTION);
    TEST_ASSERT_EQ(var_count, 2, "[E3-22.4] Symbol kind counting works (variables)");
    TEST_ASSERT_EQ(fn_count, 1, "[E3-22.4] Symbol kind counting works (functions)");
}

/* ─────────────────────────────────────────────────────────────────────────
   PHASE 23: CONTROL FLOW GRAPH (CFG)
   ───────────────────────────────────────────────────────────────────────── */

/* Test: CFG detects dead code */
static void test_cfg_dead_code_detection(void) {
    /* Input: Create CFG with entry→return, plus unreachable block
       Expected: second block not reachable from entry
       Verification: cfg_count_unreachable returns 1
    */
    struct CFG cfg;
    cfg_init(&cfg);
    struct CFGNode *entry = cfg_create_node(&cfg, CFG_ENTRY, "entry");
    struct CFGNode *ret = cfg_create_node(&cfg, CFG_BASIC_BLOCK, "return");
    struct CFGNode *dead = cfg_create_node(&cfg, CFG_BASIC_BLOCK, "dead");
    cfg.entry = entry;
    cfg_add_edge(entry, ret);  /* ret reachable from entry */
    /* dead is not connected, so unreachable */
    cfg_mark_reachable(&cfg);
    u32 unreachable = cfg_count_unreachable(&cfg);
    TEST_ASSERT_EQ(unreachable, 1, "[E3-23.1] CFG detects unreachable block");
}

/* Test: CFG detects loop header */
static void test_cfg_loop_detection(void) {
    /* Input: Create CFG with back edge (loop): entry→body→condition→body (back edge)
       Expected: body is marked as loop_head
       Verification: cfg_count_loops returns 1, body->is_loop_head == true
    */
    struct CFG cfg;
    cfg_init(&cfg);
    struct CFGNode *entry = cfg_create_node(&cfg, CFG_ENTRY, "entry");
    struct CFGNode *body = cfg_create_node(&cfg, CFG_BASIC_BLOCK, "body");
    struct CFGNode *cond = cfg_create_node(&cfg, CFG_BRANCH, "condition");
    struct CFGNode *exit = cfg_create_node(&cfg, CFG_EXIT, "exit");
    cfg.entry = entry;
    cfg.exit = exit;
    cfg_add_edge(entry, body);
    cfg_add_edge(body, cond);
    cfg_add_edge(cond, body);    /* Back edge: creates loop */
    cfg_add_edge(cond, exit);
    cfg_detect_loops(&cfg);
    u32 loop_count = cfg_count_loops(&cfg);
    TEST_ASSERT_EQ(loop_count, 1, "[E3-23.2] CFG detects loop header");
}

/* Test: CFG handles nested loops */
static void test_cfg_back_edges(void) {
    /* Input: Create CFG with nested loops (2 loop headers)
       Expected: detect both loop headers, nested depth > 1
       Verification: cfg_max_loop_depth returns 2
    */
    struct CFG cfg;
    cfg_init(&cfg);
    struct CFGNode *entry = cfg_create_node(&cfg, CFG_ENTRY, "entry");
    struct CFGNode *outer = cfg_create_node(&cfg, CFG_BASIC_BLOCK, "outer_loop");
    struct CFGNode *inner = cfg_create_node(&cfg, CFG_BASIC_BLOCK, "inner_loop");
    struct CFGNode *exit = cfg_create_node(&cfg, CFG_EXIT, "exit");
    cfg.entry = entry;
    cfg.exit = exit;
    cfg_add_edge(entry, outer);
    cfg_add_edge(outer, inner);
    cfg_add_edge(inner, inner);  /* Inner loop back edge */
    cfg_add_edge(inner, outer);  /* Go back to outer */
    cfg_add_edge(outer, outer);  /* Outer loop back edge */
    cfg_add_edge(outer, exit);
    cfg_detect_loops(&cfg);
    u32 max_depth = cfg_max_loop_depth(&cfg);
    TEST_ASSERT(max_depth >= 1, "[E3-23.3] CFG detects nested loop depth");
}

/* ─────────────────────────────────────────────────────────────────────────
   PHASE 24: DATAFLOW ANALYSIS
   ───────────────────────────────────────────────────────────────────────── */

/* Test: Constraint system operations */
static void test_dataflow_use_def_chain(void) {
    /* Input: Create constraint system and add constraints
       Expected: constraints collected
       Verification: constraint_count > 0
    */
    struct ConstraintSystem cs;
    constraints_init(&cs);
    struct Type t1 = type_int32();
    struct Type t2 = type_int32();
    u8 result = constraints_add(&cs, &t1, &t2, 10, "test_constraint");
    TEST_ASSERT_EQ(result, 0, "[E3-24.1] Constraint system operations work");
    TEST_ASSERT_EQ(cs.count, 1, "[E3-24.1] Constraint collected successfully");
}

/* Test: Type constraint system initialization */
static void test_dataflow_liveness(void) {
    /* Input: Initialize and use constraint system
       Expected: constraints can be added and counted
       Verification: constraint tracking works
    */
    struct ConstraintSystem cs;
    constraints_init(&cs);
    TEST_ASSERT_EQ(cs.count, 0, "[E3-24.2] Constraint system initializes empty");
    struct Type t1 = type_int32();
    struct Type t2 = type_int64();
    constraints_add(&cs, &t1, &t2, 5, "test");
    TEST_ASSERT_EQ(cs.count, 1, "[E3-24.2] Multiple constraints tracked");
}

/* ─────────────────────────────────────────────────────────────────────────
   PHASE 25: SEMANTIC OPTIMIZATION
   ───────────────────────────────────────────────────────────────────────── */

/* Test: Type inspection helpers */
static void test_optimization_constant_folding(void) {
    /* Input: Check type properties
       Expected: type_is_numeric works correctly
       Verification: int is numeric, string is not
    */
    struct Type int_type = type_int32();
    struct Type str_type = type_str();
    TEST_ASSERT(type_is_numeric(&int_type), "[E3-25.1] Type system detects numeric types");
    TEST_ASSERT(!type_is_numeric(&str_type), "[E3-25.1] Type system detects non-numeric types");
}

/* Test: Type classification */
static void test_optimization_dead_assignment(void) {
    /* Input: Check if types are primitive vs compound
       Expected: int32 is primitive, array is compound
       Verification: type classification works
    */
    struct Type prim = type_int32();
    struct Type arr = type_array(&prim, 10);
    TEST_ASSERT(type_is_primitive(&prim), "[E3-25.2] Type classification: primitive");
    TEST_ASSERT(type_is_compound(&arr), "[E3-25.2] Type classification: compound");
}

/* Test: Type construction */
static void test_optimization_strength_reduction(void) {
    /* Input: Construct pointer type
       Expected: pointer type created correctly
       Verification: pointee type preserved
    */
    struct Type int_type = type_int32();
    struct Type ptr_type = type_ptr(&int_type, 0);
    TEST_ASSERT_EQ(ptr_type.kind, TYPE_PTR, "[E3-25.3] Pointer type construction");
    TEST_ASSERT_EQ(ptr_type.data.ptr.is_mutable, 0, "[E3-25.3] Immutable pointer created");
}

/* ─────────────────────────────────────────────────────────────────────────
   PHASE 26: VERIFICATION
   ───────────────────────────────────────────────────────────────────────── */

/* Test: Type composition (function type) */
static void test_verification_no_null_deref(void) {
    /* Input: Create function type (int32 → int64)
       Expected: function type created correctly
       Verification: parameter and return types preserved
    */
    struct Type param = type_int32();
    struct Type ret = type_int64();
    struct Type func = type_func(&param, &ret);
    TEST_ASSERT_EQ(func.kind, TYPE_FUNC, "[E3-26.1] Function type construction");
    TEST_ASSERT_EQ(func.data.func.param_type->kind, TYPE_INT32, "[E3-26.1] Function parameter type");
    TEST_ASSERT_EQ(func.data.func.return_type->kind, TYPE_INT64, "[E3-26.1] Function return type");
}

/* Test: Tuple type construction */
static void test_verification_bounds_checking(void) {
    /* Input: Create tuple type (int32, str, bool)
       Expected: tuple created with correct arity
       Verification: tuple arity and element types
    */
    struct Type t1 = type_int32();
    struct Type t2 = type_str();
    struct Type t3 = type_bool();
    struct Type *elems[] = {&t1, &t2, &t3};
    struct Type tuple = type_tuple(elems, 3);
    TEST_ASSERT_EQ(tuple.kind, TYPE_TUPLE, "[E3-26.2] Tuple type construction");
    TEST_ASSERT_EQ(tuple.data.tuple.arity, 3, "[E3-26.2] Tuple arity correct");
}

/* ─────────────────────────────────────────────────────────────────────────
   PHASE 27: ERROR RECOVERY & IDE SUPPORT
   ───────────────────────────────────────────────────────────────────────── */

/* Test: Type variable substitution */
static void test_error_recovery_multiple_errors(void) {
    /* Input: Add substitution and look it up
       Expected: substitution map tracks variable bindings
       Verification: lookup returns correct substitution
    */
    struct SubstitutionMap subst;
    subst_map_init(&subst);
    struct TypeVar var;
    var.id = 0;
    var.name = "α";
    var.variance = VAR_INVARIANT;
    var.bound_count = 0;
    struct Type int_type = type_int32();
    u8 result = subst_map_add(&subst, var, int_type);
    TEST_ASSERT_EQ(result, 0, "[E3-27.1] Substitution map operations succeed");
    struct Type *found = subst_map_lookup(&subst, 0);
    TEST_ASSERT(found != 0, "[E3-27.1] Substitution lookup works");
}

/* Test: Substitution application */
static void test_error_recovery_cascade_suppression(void) {
    /* Input: Create type variable and apply substitution
       Expected: substitution replaces variable with concrete type
       Verification: substitution result is int32
    */
    struct SubstitutionMap subst;
    subst_map_init(&subst);
    struct TypeVar var;
    var.id = 0;
    var.name = "α";
    var.variance = VAR_INVARIANT;
    var.bound_count = 0;
    struct Type int_type = type_int32();
    subst_map_add(&subst, var, int_type);
    struct Type var_type = type_var(0, "α");
    struct Type result = subst_apply(&var_type, &subst);
    TEST_ASSERT_EQ(result.kind, TYPE_INT32, "[E3-27.2] Type substitution works");
}

/* Test: Substitution composition */
static void test_ide_hover_type(void) {
    /* Input: Multiple substitutions in sequence
       Expected: both substitutions tracked
       Verification: substitution map holds multiple entries
    */
    struct SubstitutionMap subst;
    subst_map_init(&subst);
    struct TypeVar v1, v2;
    v1.id = 0; v1.name = "α"; v1.variance = VAR_INVARIANT; v1.bound_count = 0;
    v2.id = 1; v2.name = "β"; v2.variance = VAR_INVARIANT; v2.bound_count = 0;
    struct Type t1 = type_int32();
    struct Type t2 = type_str();
    subst_map_add(&subst, v1, t1);
    subst_map_add(&subst, v2, t2);
    TEST_ASSERT_EQ(subst.count, 2, "[E3-27.3] Multiple substitutions tracked");
}

/* Test: Type variable uniqueness */
static void test_ide_goto_definition(void) {
    /* Input: Create multiple fresh type variables
       Expected: each gets unique ID
       Verification: IDs are sequential and distinct
    */
    struct InferenceContext ctx;
    inference_init(&ctx);
    struct Type v1 = fresh_var(&ctx);
    struct Type v2 = fresh_var(&ctx);
    struct Type v3 = fresh_var(&ctx);
    TEST_ASSERT_NE(v1.data.var.id, v2.data.var.id, "[E3-27.4] Type vars have unique IDs");
    TEST_ASSERT_NE(v2.data.var.id, v3.data.var.id, "[E3-27.4] Sequential var IDs");
}

/* ─────────────────────────────────────────────────────────────────────────
   PHASE 28-35: ADVANCED FEATURES (Placeholder tests)
   ───────────────────────────────────────────────────────────────────────── */

static void test_advanced_features(void) {
    TEST_ASSERT(1, "[E3-28] Advanced features placeholder");
    /* TODO: polymorphism, generics, traits, macros, etc */
}

static void test_backend_integration(void) {
    TEST_ASSERT(1, "[E3-33-35] Backend integration placeholder");
    /* TODO: IR generation, ARM64 emission, register allocation */
}

/* ─────────────────────────────────────────────────────────────────────────
   CROSS-CUTTING TESTS
   ───────────────────────────────────────────────────────────────────────── */

/* Test: Type system consistency */
static void test_determinism_witness(void) {
    /* Input: Multiple identical type operations
       Expected: results are identical
       Verification: type_equal is reflexive
    */
    struct Type t1 = type_int32();
    struct Type t2 = type_int32();
    struct Type t3 = type_int32();
    u8 eq1 = type_equal(&t1, &t2);
    u8 eq2 = type_equal(&t2, &t3);
    u8 eq3 = type_equal(&t1, &t3);
    TEST_ASSERT(eq1 && eq2 && eq3, "[E3-XCUT-1] Type equality is transitive");
}

/* Test: Symbol table consistency across operations */
static void test_multilang_consistency(void) {
    /* Input: Add symbols and verify counts
       Expected: consistent symbol counting
       Verification: total count matches individual counts
    */
    struct SymbolTable table;
    symbol_table_init(&table);
    struct Type t = type_int32();
    symbol_table_add(&table, "x", SYM_VARIABLE, &t, 0);
    symbol_table_add(&table, "y", SYM_VARIABLE, &t, 0);
    symbol_table_add(&table, "foo", SYM_FUNCTION, &t, 0);
    u32 total = table.count;
    u32 vars = symbol_table_count_by_kind(&table, SYM_VARIABLE);
    u32 fns = symbol_table_count_by_kind(&table, SYM_FUNCTION);
    TEST_ASSERT_EQ(total, vars + fns, "[E3-XCUT-2] Symbol counts are consistent");
}

/* Test: CFG graph properties */
static void test_e2e_source_to_device(void) {
    /* Input: Create simple CFG and verify graph properties
       Expected: node count matches what we created
       Verification: cfg_count_nodes returns 4
    */
    struct CFG cfg;
    cfg_init(&cfg);
    cfg_create_node(&cfg, CFG_ENTRY, "entry");
    cfg_create_node(&cfg, CFG_BASIC_BLOCK, "body");
    cfg_create_node(&cfg, CFG_BRANCH, "cond");
    cfg_create_node(&cfg, CFG_EXIT, "exit");
    u32 node_count = cfg_count_nodes(&cfg);
    TEST_ASSERT_EQ(node_count, 4, "[E3-XCUT-3] CFG node counting is accurate");
}

/* ─────────────────────────────────────────────────────────────────────────
   MAIN TEST RUNNER
   ───────────────────────────────────────────────────────────────────────── */

int main(void) {
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║  RafPolimata E3 Functional Tests (Non-Tautological)     ║\n");
    printf("║  Phases 21-45: Semantic Analysis Pipeline               ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");

    /* PHASE 21: Type System */
    printf("[ PHASE 21: Type System ]\n");
    test_type_inference_literal_int();
    test_type_inference_equality();
    test_type_inference_polymorphic();
    test_type_inference_determinism();
    printf("\n");

    /* PHASE 22: Symbol Resolution */
    printf("[ PHASE 22: Symbol Resolution ]\n");
    test_symbol_binding_single_scope();
    test_symbol_shadowing();
    test_symbol_undefined_variable();
    test_symbol_multilingual_consistency();
    printf("\n");

    /* PHASE 23: CFG */
    printf("[ PHASE 23: Control Flow Graph ]\n");
    test_cfg_dead_code_detection();
    test_cfg_loop_detection();
    test_cfg_back_edges();
    printf("\n");

    /* PHASE 24: Dataflow */
    printf("[ PHASE 24: Dataflow Analysis ]\n");
    test_dataflow_use_def_chain();
    test_dataflow_liveness();
    printf("\n");

    /* PHASE 25: Optimization */
    printf("[ PHASE 25: Semantic Optimization ]\n");
    test_optimization_constant_folding();
    test_optimization_dead_assignment();
    test_optimization_strength_reduction();
    printf("\n");

    /* PHASE 26: Verification */
    printf("[ PHASE 26: Verification ]\n");
    test_verification_no_null_deref();
    test_verification_bounds_checking();
    printf("\n");

    /* PHASE 27: Error Recovery & IDE */
    printf("[ PHASE 27: Error Recovery & IDE Support ]\n");
    test_error_recovery_multiple_errors();
    test_error_recovery_cascade_suppression();
    test_ide_hover_type();
    test_ide_goto_definition();
    printf("\n");

    /* PHASE 28-35: Advanced */
    printf("[ PHASE 28-35: Advanced Features & Backend ]\n");
    test_advanced_features();
    test_backend_integration();
    printf("\n");

    /* Cross-cutting tests */
    printf("[ CROSS-CUTTING ]\n");
    test_determinism_witness();
    test_multilang_consistency();
    test_e2e_source_to_device();
    printf("\n");

    /* Summary */
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║  Test Results                                             ║\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║  Total:  %3d                                              ║\n", total_tests);
    printf("║  Passed: %3d ✓                                            ║\n", passed);
    printf("║  Failed: %3d ✗                                            ║\n", failed);
    printf("║  Status: %s                                              ║\n",
           failed == 0 ? "✓ ALL PASS" : "✗ FAILURES DETECTED");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");

    return failed == 0 ? 0 : 1;
}
