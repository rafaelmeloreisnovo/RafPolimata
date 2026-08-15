/* test_phase16_debugging_profiling.c — Phase 16 Testing (Stages 16.1–16.4)
 *
 * Comprehensive tests for debugging, profiling, and instrumentation infrastructure.
 * All 4 stages: symbol management, call graph analysis, memory profiling, runtime hooks.
 *
 * Build: gcc -std=c99 -Wall -O2 -I. -I Apkc tests/test_phase16_debugging_profiling.c -o test_phase16 && ./test_phase16
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Apkc/debug_symbol_manager.h"
#include "Apkc/prof_call_graph_analysis.h"
#include "Apkc/prof_memory_profiling.h"
#include "Apkc/instr_runtime_hooks.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Test counters */
static u32 tests_passed = 0;
static u32 tests_failed = 0;

/* Helper: assert macro */
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
/* STAGE 16.1: SYMBOL MANAGER TESTS */
/* ============================================================ */

static u8 test_debug_init(void) {
	struct DebugInfo info = {0};
	debug_init(&info, "/src", "build123", 5);

	ASSERT(info.symbol_count == 0, "symbol_count initialized");
	ASSERT(info.line_count == 0, "line_count initialized");
	ASSERT(info.dwarf_version == 5, "dwarf_version set");
	ASSERT(info.has_line_info == 0, "has_line_info cleared");
	PASS("debug_init");
}

static u8 test_debug_add_symbol(void) {
	struct DebugInfo info = {0};
	debug_init(&info, "/src", "build123", 4);

	u8 result = debug_add_symbol(
		&info, "main", SYMBOL_FUNCTION, 0x1000, 256, "main.c", 1, 0);

	ASSERT(result == 1, "add_symbol returns 1");
	ASSERT(info.symbol_count == 1, "symbol_count incremented");
	ASSERT(info.symbols[0].symbol_type == SYMBOL_FUNCTION, "symbol_type correct");
	ASSERT(info.symbols[0].address == 0x1000, "address set");
	ASSERT(info.symbols[0].size == 256, "size set");
	PASS("debug_add_symbol");
}

static u8 test_debug_add_local_variable(void) {
	struct DebugInfo info = {0};
	debug_init(&info, "/src", "build123", 4);

	u8 result = debug_add_local_variable(&info, "x", 0x100, "main.c", 10, 1);

	ASSERT(result == 1, "add_local_variable returns 1");
	ASSERT(info.symbol_count == 1, "symbol_count incremented");
	ASSERT(info.symbols[0].symbol_type == SYMBOL_VARIABLE, "symbol_type is VARIABLE");
	ASSERT(info.symbols[0].scope_depth == 1, "scope_depth set");
	PASS("debug_add_local_variable");
}

static u8 test_debug_lookup_symbol(void) {
	struct DebugInfo info = {0};
	debug_init(&info, "/src", "build123", 4);

	debug_add_symbol(&info, "printf", SYMBOL_FUNCTION, 0x2000, 512, "stdio.h", 42, 0);

	struct DebugSymbol *sym = debug_lookup_symbol(&info, "printf");

	ASSERT(sym != 0, "lookup_symbol finds symbol");
	ASSERT(sym->address == 0x2000, "found symbol has correct address");
	PASS("debug_lookup_symbol");
}

static u8 test_debug_lookup_by_address(void) {
	struct DebugInfo info = {0};
	debug_init(&info, "/src", "build123", 4);

	debug_add_symbol(&info, "func", SYMBOL_FUNCTION, 0x1000, 256, "test.c", 5, 0);

	struct DebugSymbol *sym = debug_lookup_by_address(&info, 0x1080);  /* Within range */

	ASSERT(sym != 0, "lookup_by_address finds symbol");
	ASSERT(sym->address == 0x1000, "found symbol has correct base address");
	PASS("debug_lookup_by_address");
}

static u8 test_debug_add_line_map(void) {
	struct DebugInfo info = {0};
	debug_init(&info, "/src", "build123", 4);

	u8 result = debug_add_line_map(&info, 0x1000, "main.c", 1, 0, 1);

	ASSERT(result == 1, "add_line_map returns 1");
	ASSERT(info.line_count == 1, "line_count incremented");
	ASSERT(info.has_line_info == 1, "has_line_info set");
	PASS("debug_add_line_map");
}

static u8 test_debug_lookup_source_by_offset(void) {
	struct DebugInfo info = {0};
	debug_init(&info, "/src", "build123", 4);

	debug_add_line_map(&info, 0x1000, "main.c", 10, 0, 1);
	debug_add_line_map(&info, 0x1008, "main.c", 11, 0, 0);

	struct SourceLocation *loc = debug_lookup_source_by_offset(&info, 0x1008);

	ASSERT(loc != 0, "lookup_source_by_offset finds location");
	ASSERT(loc->line_number == 11, "found location has correct line");
	PASS("debug_lookup_source_by_offset");
}

static u8 test_debug_count_symbols_by_type(void) {
	struct DebugInfo info = {0};
	debug_init(&info, "/src", "build123", 4);

	debug_add_symbol(&info, "main", SYMBOL_FUNCTION, 0x1000, 256, "main.c", 1, 0);
	debug_add_symbol(&info, "foo", SYMBOL_FUNCTION, 0x2000, 128, "foo.c", 5, 0);
	debug_add_local_variable(&info, "x", 0x100, "main.c", 10, 1);

	u32 func_count = debug_count_symbols_by_type(&info, SYMBOL_FUNCTION);
	u32 var_count = debug_count_symbols_by_type(&info, SYMBOL_VARIABLE);

	ASSERT(func_count == 2, "count_symbols_by_type counts FUNCTION correctly");
	ASSERT(var_count == 1, "count_symbols_by_type counts VARIABLE correctly");
	PASS("debug_count_symbols_by_type");
}

/* ============================================================ */
/* STAGE 16.2: CALL GRAPH ANALYSIS TESTS */
/* ============================================================ */

static u8 test_callgraph_init(void) {
	struct CallGraph cg = {0};
	callgraph_init(&cg);

	ASSERT(cg.function_count == 0, "function_count initialized");
	ASSERT(cg.edge_count == 0, "edge_count initialized");
	ASSERT(cg.total_functions_called == 0, "total_functions_called initialized");
	PASS("callgraph_init");
}

static u8 test_callgraph_add_function(void) {
	struct CallGraph cg = {0};
	callgraph_init(&cg);

	u8 result = callgraph_add_function(&cg, "main");

	ASSERT(result == 1, "add_function returns 1");
	ASSERT(cg.function_count == 1, "function_count incremented");
	ASSERT(cg.functions[0].is_leaf == 1, "new function marked as leaf");
	PASS("callgraph_add_function");
}

static u8 test_callgraph_add_call(void) {
	struct CallGraph cg = {0};
	callgraph_init(&cg);

	callgraph_add_function(&cg, "main");
	callgraph_add_function(&cg, "foo");

	u8 result = callgraph_add_call(&cg, 0, 1, 5, 1000);

	ASSERT(result == 1, "add_call returns 1");
	ASSERT(cg.edge_count == 1, "edge_count incremented");
	ASSERT(cg.functions[0].is_leaf == 0, "caller no longer leaf");
	ASSERT(cg.functions[0].callee_count == 1, "callee_count incremented");
	PASS("callgraph_add_call");
}

static u8 test_callgraph_has_cycle(void) {
	struct CallGraph cg = {0};
	callgraph_init(&cg);

	callgraph_add_function(&cg, "fib");
	callgraph_add_call(&cg, 0, 0, 1, 100);  /* Direct recursion */

	u8 has_cycle = callgraph_has_cycle_at_function(&cg, 0);

	ASSERT(has_cycle == 1, "has_cycle detects direct recursion");
	PASS("callgraph_has_cycle");
}

static u8 test_callgraph_find_hottest_function(void) {
	struct CallGraph cg = {0};
	callgraph_init(&cg);

	callgraph_add_function(&cg, "foo");
	callgraph_add_function(&cg, "bar");
	cg.functions[0].call_count = 10;
	cg.functions[1].call_count = 25;

	struct FunctionNode *hottest = callgraph_find_hottest_function(&cg);

	ASSERT(hottest != 0, "find_hottest_function returns non-null");
	ASSERT(hottest->call_count == 25, "hottest function has highest call_count");
	PASS("callgraph_find_hottest_function");
}

static u8 test_callgraph_count_leaf_functions(void) {
	struct CallGraph cg = {0};
	callgraph_init(&cg);

	callgraph_add_function(&cg, "main");
	callgraph_add_function(&cg, "foo");
	callgraph_add_function(&cg, "bar");
	callgraph_add_call(&cg, 0, 1, 1, 100);

	u32 leaf_count = callgraph_count_leaf_functions(&cg);

	ASSERT(leaf_count == 2, "count_leaf_functions correct (bar and foo)");
	PASS("callgraph_count_leaf_functions");
}

static u8 test_callgraph_count_recursive_functions(void) {
	struct CallGraph cg = {0};
	callgraph_init(&cg);

	callgraph_add_function(&cg, "fib");
	callgraph_add_call(&cg, 0, 0, 1, 100);  /* Direct recursion */

	u32 recursive_count = callgraph_count_recursive_functions(&cg);

	ASSERT(recursive_count == 1, "count_recursive_functions returns 1");
	PASS("callgraph_count_recursive_functions");
}

/* ============================================================ */
/* STAGE 16.3: MEMORY PROFILING TESTS */
/* ============================================================ */

static u8 test_memprof_init(void) {
	struct MemoryProfile prof = {0};
	memprof_init(&prof);

	ASSERT(prof.alloc_count == 0, "alloc_count initialized");
	ASSERT(prof.site_count == 0, "site_count initialized");
	ASSERT(prof.current_allocated == 0, "current_allocated initialized");
	ASSERT(prof.peak_allocated == 0, "peak_allocated initialized");
	PASS("memprof_init");
}

static u8 test_memprof_record_alloc(void) {
	struct MemoryProfile prof = {0};
	memprof_init(&prof);

	u8 result = memprof_record_alloc(&prof, 0x1000, 1024, "malloc", 42);

	ASSERT(result == 1, "record_alloc returns 1");
	ASSERT(prof.alloc_count == 1, "alloc_count incremented");
	ASSERT(prof.current_allocated == 1024, "current_allocated updated");
	ASSERT(prof.peak_allocated == 1024, "peak_allocated updated");
	ASSERT(prof.current_live_allocs == 1, "current_live_allocs incremented");
	PASS("memprof_record_alloc");
}

static u8 test_memprof_record_free(void) {
	struct MemoryProfile prof = {0};
	memprof_init(&prof);

	memprof_record_alloc(&prof, 0x1000, 512, "malloc", 42);
	u8 result = memprof_record_free(&prof, 0x1000);

	ASSERT(result == 1, "record_free returns 1");
	ASSERT(prof.current_allocated == 0, "current_allocated reset");
	ASSERT(prof.current_live_allocs == 0, "current_live_allocs decremented");
	ASSERT(prof.total_frees == 1, "total_frees incremented");
	PASS("memprof_record_free");
}

static u8 test_memprof_find_leaks(void) {
	struct MemoryProfile prof = {0};
	memprof_init(&prof);

	memprof_record_alloc(&prof, 0x1000, 256, "malloc", 1);
	memprof_record_alloc(&prof, 0x2000, 512, "malloc", 2);

	u64 leak_addrs[10];
	u32 leak_count = memprof_find_leaks(&prof, leak_addrs, 10);

	ASSERT(leak_count == 2, "find_leaks returns correct count");
	PASS("memprof_find_leaks");
}

static u8 test_memprof_get_leaked_bytes(void) {
	struct MemoryProfile prof = {0};
	memprof_init(&prof);

	memprof_record_alloc(&prof, 0x1000, 256, "malloc", 1);
	memprof_record_alloc(&prof, 0x2000, 512, "malloc", 2);
	memprof_record_free(&prof, 0x1000);

	u64 leaked = memprof_get_leaked_bytes(&prof);

	ASSERT(leaked == 512, "get_leaked_bytes correct");
	PASS("memprof_get_leaked_bytes");
}

static u8 test_memprof_get_fragmentation_ratio(void) {
	struct MemoryProfile prof = {0};
	memprof_init(&prof);

	memprof_record_alloc(&prof, 0x1000, 1000, "malloc", 1);
	memprof_record_alloc(&prof, 0x2000, 500, "malloc", 2);
	memprof_record_free(&prof, 0x1000);

	u32 frag = memprof_get_fragmentation_ratio(&prof);

	ASSERT(frag <= 100, "fragmentation_ratio is valid percent");
	PASS("memprof_get_fragmentation_ratio");
}

/* ============================================================ */
/* STAGE 16.4: RUNTIME HOOKS TESTS */
/* ============================================================ */

/* Global callback test counter */
static u32 test_callback_count = 0;

/* Test callback function */
static void test_hook_callback(struct HookEvent *event) {
	if (event) {
		test_callback_count++;
	}
}

static u8 test_instr_init(void) {
	struct InstrumentationContext ctx = {0};
	instr_init(&ctx);

	ASSERT(ctx.hook_count == 0, "hook_count initialized");
	ASSERT(ctx.instrumentation_enabled == 0, "instrumentation_enabled off");
	ASSERT(ctx.current_call_depth == 0, "current_call_depth initialized");
	PASS("instr_init");
}

static u8 test_instr_register_hook(void) {
	struct InstrumentationContext ctx = {0};
	instr_init(&ctx);

	u32 hook_id = instr_register_hook(
		&ctx, HOOK_FUNCTION_ENTRY, 0, test_hook_callback);

	ASSERT(hook_id > 0, "register_hook returns valid hook_id");
	ASSERT(ctx.hook_count == 1, "hook_count incremented");
	PASS("instr_register_hook");
}

static u8 test_instr_fire_event(void) {
	struct InstrumentationContext ctx = {0};
	instr_init(&ctx);
	ctx.instrumentation_enabled = 1;

	instr_register_hook(&ctx, HOOK_FUNCTION_ENTRY, 0, test_hook_callback);
	test_callback_count = 0;

	instr_fire_event(&ctx, HOOK_FUNCTION_ENTRY, 1, 0x1000);

	ASSERT(test_callback_count == 1, "fire_event invokes callback");
	ASSERT(ctx.event_buffer.total_events_logged == 1, "event logged");
	PASS("instr_fire_event");
}

static u8 test_instr_function_entry(void) {
	struct InstrumentationContext ctx = {0};
	instr_init(&ctx);
	ctx.instrumentation_enabled = 1;

	instr_function_entry(&ctx, 1);

	ASSERT(ctx.current_call_depth == 1, "current_call_depth incremented");
	ASSERT(ctx.max_call_depth == 1, "max_call_depth updated");
	PASS("instr_function_entry");
}

static u8 test_instr_function_exit(void) {
	struct InstrumentationContext ctx = {0};
	instr_init(&ctx);
	ctx.instrumentation_enabled = 1;

	instr_function_entry(&ctx, 1);
	instr_function_exit(&ctx, 1, 42);

	ASSERT(ctx.current_call_depth == 0, "current_call_depth decremented");
	ASSERT(ctx.event_buffer.total_events_logged == 2, "entry+exit logged");
	PASS("instr_function_exit");
}

static u8 test_instr_memory_hooks(void) {
	struct InstrumentationContext ctx = {0};
	instr_init(&ctx);
	ctx.instrumentation_enabled = 1;

	instr_memory_alloc(&ctx, 0x1000, 256);
	instr_memory_free(&ctx, 0x1000);

	ASSERT(ctx.event_buffer.total_events_logged == 2, "alloc+free logged");
	PASS("instr_memory_hooks");
}

static u8 test_instr_get_hook_hit_count(void) {
	struct InstrumentationContext ctx = {0};
	instr_init(&ctx);
	ctx.instrumentation_enabled = 1;

	u32 hook_id = instr_register_hook(&ctx, HOOK_FUNCTION_ENTRY, 0, test_hook_callback);

	instr_fire_event(&ctx, HOOK_FUNCTION_ENTRY, 1, 0);
	instr_fire_event(&ctx, HOOK_FUNCTION_ENTRY, 1, 0);

	u32 hit_count = instr_get_hook_hit_count(&ctx, hook_id);

	ASSERT(hit_count == 2, "get_hook_hit_count returns correct value");
	PASS("instr_get_hook_hit_count");
}

static u8 test_instr_count_hooks_by_type(void) {
	struct InstrumentationContext ctx = {0};
	instr_init(&ctx);

	instr_register_hook(&ctx, HOOK_FUNCTION_ENTRY, 0, test_hook_callback);
	instr_register_hook(&ctx, HOOK_FUNCTION_ENTRY, 0, test_hook_callback);
	instr_register_hook(&ctx, HOOK_MEMORY_ALLOC, 0, test_hook_callback);

	u32 entry_hooks = instr_count_hooks_by_type(&ctx, HOOK_FUNCTION_ENTRY);
	u32 alloc_hooks = instr_count_hooks_by_type(&ctx, HOOK_MEMORY_ALLOC);

	ASSERT(entry_hooks == 2, "count_hooks_by_type counts FUNCTION_ENTRY");
	ASSERT(alloc_hooks == 1, "count_hooks_by_type counts MEMORY_ALLOC");
	PASS("instr_count_hooks_by_type");
}

/* ============================================================ */
/* TEST RUNNER */
/* ============================================================ */

int main(void) {
	printf("=== Phase 16: Debugging & Profiling Tests ===\n\n");

	/* Stage 16.1: Symbol Manager (8 tests) */
	printf("Stage 16.1: Symbol Manager\n");
	test_debug_init();
	test_debug_add_symbol();
	test_debug_add_local_variable();
	test_debug_lookup_symbol();
	test_debug_lookup_by_address();
	test_debug_add_line_map();
	test_debug_lookup_source_by_offset();
	test_debug_count_symbols_by_type();

	/* Stage 16.2: Call Graph Analysis (7 tests) */
	printf("\nStage 16.2: Call Graph Analysis\n");
	test_callgraph_init();
	test_callgraph_add_function();
	test_callgraph_add_call();
	test_callgraph_has_cycle();
	test_callgraph_find_hottest_function();
	test_callgraph_count_leaf_functions();
	test_callgraph_count_recursive_functions();

	/* Stage 16.3: Memory Profiling (7 tests) */
	printf("\nStage 16.3: Memory Profiling\n");
	test_memprof_init();
	test_memprof_record_alloc();
	test_memprof_record_free();
	test_memprof_find_leaks();
	test_memprof_get_leaked_bytes();
	test_memprof_get_fragmentation_ratio();

	/* Stage 16.4: Runtime Hooks (7 tests) */
	printf("\nStage 16.4: Runtime Instrumentation Hooks\n");
	test_instr_init();
	test_instr_register_hook();
	test_instr_fire_event();
	test_instr_function_entry();
	test_instr_function_exit();
	test_instr_memory_hooks();
	test_instr_get_hook_hit_count();
	test_instr_count_hooks_by_type();

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
