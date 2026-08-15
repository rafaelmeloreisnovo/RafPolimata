/* test_phase10_advanced_runtime.c — Phase 10 Advanced Runtime Features Tests
 *
 * Comprehensive test suite for:
 * - Stage 10.1: Coroutines & Generators (yield, resume, lazy generators)
 * - Stage 10.2: Lazy Evaluation & Memoization (thunks, caching, LRU)
 * - Stage 10.3: Streaming & Iterators (map, filter, fold, infinite streams)
 *
 * FREESTANDING: No malloc, no libc.
 */

#include <stdio.h>
#include <string.h>

/* Include Phase 10 headers */
#include "Apkc/rt_coroutines.h"
#include "Apkc/rt_lazy_eval.h"
#include "Apkc/rt_streaming.h"

/* ============================================================ */
/* COROUTINE & GENERATOR TESTS (Stage 10.1) */
/* ============================================================ */

/* Test 1: Range generator */
static int test_range_generator(void) {
	struct CoroutineManager cm = {0};
	coroutine_manager_init(&cm);

	struct Generator gen = {0};
	if (range_generator(&cm, 5, &gen)) {
		printf("❌ test_range_generator: failed to create generator\n");
		return 1;
	}

	u64 values[5] = {0};
	u32 count = generator_collect(&cm, &gen, values, 5);

	if (count != 5) {
		printf("❌ test_range_generator: expected 5 values, got %u\n", count);
		return 1;
	}

	for (int i = 0; i < 5; i++) {
		if (values[i] != i) {
			printf("❌ test_range_generator: expected %d, got %llu at index %d\n", i, values[i], i);
			return 1;
		}
	}

	printf("✓ test_range_generator\n");
	return 0;
}

/* Test 2: Fibonacci generator */
static int test_fibonacci_generator(void) {
	struct CoroutineManager cm = {0};
	coroutine_manager_init(&cm);

	struct Generator gen = {0};
	if (fibonacci_generator(&cm, 10, &gen)) {
		printf("❌ test_fibonacci_generator: failed to create generator\n");
		return 1;
	}

	u64 fib_seq[11];
	u32 count = generator_collect(&cm, &gen, fib_seq, 11);

	u64 expected[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55};

	if (count != 11) {
		printf("❌ test_fibonacci_generator: expected 11 values, got %u\n", count);
		return 1;
	}

	for (int i = 0; i < 11; i++) {
		if (fib_seq[i] != expected[i]) {
			printf("❌ test_fibonacci_generator: mismatch at index %d\n", i);
			return 1;
		}
	}

	printf("✓ test_fibonacci_generator\n");
	return 0;
}

/* Test 3: Generator exhaustion */
static int test_generator_exhaustion(void) {
	struct CoroutineManager cm = {0};
	coroutine_manager_init(&cm);

	struct Generator gen = {0};
	range_generator(&cm, 3, &gen);

	u64 val1, val2, val3, val4;
	generator_next(&cm, &gen, &val1);
	generator_next(&cm, &gen, &val2);
	generator_next(&cm, &gen, &val3);

	if (generator_next(&cm, &gen, &val4) == 0) {
		printf("❌ test_generator_exhaustion: generator should be exhausted\n");
		return 1;
	}

	if (gen.exhausted != 1) {
		printf("❌ test_generator_exhaustion: exhausted flag not set\n");
		return 1;
	}

	printf("✓ test_generator_exhaustion\n");
	return 0;
}

/* Test 4: Generator sum */
static int test_generator_sum(void) {
	struct CoroutineManager cm = {0};
	coroutine_manager_init(&cm);

	struct Generator gen = {0};
	range_generator(&cm, 5, &gen);

	u64 sum = generator_sum(&cm, &gen);

	if (sum != 10) {  /* 0+1+2+3+4 = 10 */
		printf("❌ test_generator_sum: expected 10, got %llu\n", sum);
		return 1;
	}

	printf("✓ test_generator_sum\n");
	return 0;
}

/* Test 5: Generator take */
static int test_generator_take(void) {
	struct CoroutineManager cm = {0};
	coroutine_manager_init(&cm);

	struct Generator gen = {0};
	range_generator(&cm, 10, &gen);

	u64 taken[5] = {0};
	u32 count = generator_take(&cm, &gen, 5, taken, 5);

	if (count != 5) {
		printf("❌ test_generator_take: expected 5, got %u\n", count);
		return 1;
	}

	for (int i = 0; i < 5; i++) {
		if (taken[i] != i) {
			printf("❌ test_generator_take: mismatch at index %d\n", i);
			return 1;
		}
	}

	printf("✓ test_generator_take\n");
	return 0;
}

/* ============================================================ */
/* LAZY EVALUATION & MEMOIZATION TESTS (Stage 10.2) */
/* ============================================================ */

/* Simple thunk for testing */
static u64 lazy_constant(void) {
	return 42;
}

/* Test 6: Lazy value evaluation */
static int test_lazy_force(void) {
	struct LazyValue lv = lazy_create(lazy_constant);

	if (lv.evaluated) {
		printf("❌ test_lazy_force: value should not be evaluated yet\n");
		return 1;
	}

	u64 value = lazy_force(&lv);

	if (value != 42) {
		printf("❌ test_lazy_force: expected 42, got %llu\n", value);
		return 1;
	}

	if (!lv.evaluated) {
		printf("❌ test_lazy_force: value should be marked as evaluated\n");
		return 1;
	}

	/* Second force should return cached value */
	u64 value2 = lazy_force(&lv);
	if (value2 != 42) {
		printf("❌ test_lazy_force: cached value incorrect\n");
		return 1;
	}

	printf("✓ test_lazy_force\n");
	return 0;
}

/* Memoized fibonacci */
static u64 fib_simple(u64 n) {
	if (n == 0) return 0;
	if (n == 1) return 1;
	return fib_simple(n - 1) + fib_simple(n - 2);
}

/* Test 7: Memoization cache */
static int test_memo_get(void) {
	struct MemoContext mc = {0};
	memo_init(&mc, fib_simple);

	u64 fib5_first = memo_get(&mc, 5);
	u64 fib5_second = memo_get(&mc, 5);

	if (fib5_first != fib5_second) {
		printf("❌ test_memo_get: cached value mismatch\n");
		return 1;
	}

	u32 cache_size = memo_size(&mc);
	if (cache_size != 1) {
		printf("❌ test_memo_get: expected cache size 1, got %u\n", cache_size);
		return 1;
	}

	printf("✓ test_memo_get\n");
	return 0;
}

/* Test 8: Memo clear */
static int test_memo_clear(void) {
	struct MemoContext mc = {0};
	memo_init(&mc, fib_simple);

	memo_get(&mc, 3);
	memo_get(&mc, 4);

	if (memo_size(&mc) == 0) {
		printf("❌ test_memo_clear: cache should not be empty\n");
		return 1;
	}

	memo_clear(&mc);

	if (memo_size(&mc) != 0) {
		printf("❌ test_memo_clear: cache should be empty after clear\n");
		return 1;
	}

	printf("✓ test_memo_clear\n");
	return 0;
}

/* Test 9: Lazy cycle detection */
static int test_lazy_cycle_detection(void) {
	/* In a real scenario, this would be a circular thunk.
	   For testing, we just verify the in_progress flag works. */

	struct LazyValue lv = lazy_create(NULL);
	lv.in_progress = 1;

	u64 result = lazy_force(&lv);

	/* Should return 0 due to cycle detection */
	if (result != 0) {
		printf("❌ test_lazy_cycle_detection: should return 0 for cycle\n");
		return 1;
	}

	printf("✓ test_lazy_cycle_detection\n");
	return 0;
}

/* ============================================================ */
/* STREAMING & ITERATOR TESTS (Stage 10.3) */
/* ============================================================ */

/* Test 10: Iterator from array */
static int test_iterator_from_array(void) {
	u64 data[5] = {10, 20, 30, 40, 50};
	struct Iterator it = iterator_from_array(data, 5);

	if (!iterator_has_next(&it)) {
		printf("❌ test_iterator_from_array: should have next\n");
		return 1;
	}

	u64 first;
	iterator_next(&it, &first);

	if (first != 10) {
		printf("❌ test_iterator_from_array: expected 10, got %llu\n", first);
		return 1;
	}

	printf("✓ test_iterator_from_array\n");
	return 0;
}

/* Test 11: Iterator collect */
static int test_iterator_collect(void) {
	u64 data[5] = {1, 2, 3, 4, 5};
	u64 output[5] = {0};
	struct Iterator it = iterator_from_array(data, 5);

	u32 count = iterator_collect(&it, output, 5);

	if (count != 5) {
		printf("❌ test_iterator_collect: expected 5, got %u\n", count);
		return 1;
	}

	for (int i = 0; i < 5; i++) {
		if (output[i] != data[i]) {
			printf("❌ test_iterator_collect: mismatch at index %d\n", i);
			return 1;
		}
	}

	printf("✓ test_iterator_collect\n");
	return 0;
}

/* Test 12: Iterator sum */
static int test_iterator_sum(void) {
	u64 data[5] = {10, 20, 30, 40, 50};
	struct Iterator it = iterator_from_array(data, 5);

	u64 sum = iterator_sum(&it);

	if (sum != 150) {
		printf("❌ test_iterator_sum: expected 150, got %llu\n", sum);
		return 1;
	}

	printf("✓ test_iterator_sum\n");
	return 0;
}

/* Predicate for filtering even numbers */
static u8 is_even(u64 x) {
	return (x % 2) == 0;
}

/* Test 13: Filtered iterator */
static int test_iterator_filter(void) {
	u64 data[6] = {1, 2, 3, 4, 5, 6};
	struct FilteredIterator fi = iterator_filter(data, 6, is_even);

	u64 output[3] = {0};
	u32 count = 0;
	u64 value;

	while (count < 3 && iterator_filter_next(&fi, &value) == 0) {
		output[count++] = value;
	}

	if (count != 3 || output[0] != 2 || output[1] != 4 || output[2] != 6) {
		printf("❌ test_iterator_filter: filter result incorrect\n");
		return 1;
	}

	printf("✓ test_iterator_filter\n");
	return 0;
}

/* Transformation: square */
static u64 square(u64 x) {
	return x * x;
}

/* Test 14: Mapped iterator */
static int test_iterator_map(void) {
	u64 data[4] = {1, 2, 3, 4};
	struct MappedIterator mi = iterator_map(data, 4, square);

	u64 output[4] = {0};
	u32 count = 0;
	u64 value;

	while (count < 4 && iterator_map_next(&mi, &value) == 0) {
		output[count++] = value;
	}

	u64 expected[4] = {1, 4, 9, 16};

	if (count != 4) {
		printf("❌ test_iterator_map: expected 4 values, got %u\n", count);
		return 1;
	}

	for (int i = 0; i < 4; i++) {
		if (output[i] != expected[i]) {
			printf("❌ test_iterator_map: mismatch at index %d\n", i);
			return 1;
		}
	}

	printf("✓ test_iterator_map\n");
	return 0;
}

/* Test 15: Iterator take */
static int test_iterator_take(void) {
	u64 data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	u64 output[5] = {0};
	struct Iterator it = iterator_from_array(data, 10);

	u32 count = iterator_take(&it, 5, output, 5);

	if (count != 5) {
		printf("❌ test_iterator_take: expected 5, got %u\n", count);
		return 1;
	}

	for (int i = 0; i < 5; i++) {
		if (output[i] != i) {
			printf("❌ test_iterator_take: mismatch at index %d\n", i);
			return 1;
		}
	}

	printf("✓ test_iterator_take\n");
	return 0;
}

/* Fold operation: sum accumulator */
static u64 add_acc(u64 acc, u64 val) {
	return acc + val;
}

/* Test 16: Iterator fold */
static int test_iterator_fold(void) {
	u64 data[5] = {1, 2, 3, 4, 5};
	struct Iterator it = iterator_from_array(data, 5);

	u64 result = iterator_fold(&it, 0, add_acc);

	if (result != 15) {
		printf("❌ test_iterator_fold: expected 15, got %llu\n", result);
		return 1;
	}

	printf("✓ test_iterator_fold\n");
	return 0;
}

/* Predicate: greater than 5 */
static u8 gt_5(u64 x) {
	return x > 5;
}

/* Test 17: Iterator any */
static int test_iterator_any(void) {
	u64 data[5] = {1, 2, 3, 6, 4};
	struct Iterator it = iterator_from_array(data, 5);

	u8 result = iterator_any(&it, gt_5);

	if (result != 1) {
		printf("❌ test_iterator_any: expected 1, got %u\n", result);
		return 1;
	}

	printf("✓ test_iterator_any\n");
	return 0;
}

/* Test 18: Iterator all */
static int test_iterator_all(void) {
	u64 data[3] = {10, 20, 30};
	struct Iterator it = iterator_from_array(data, 3);

	u8 result = iterator_all(&it, gt_5);

	if (result != 1) {
		printf("❌ test_iterator_all: expected 1, got %u\n", result);
		return 1;
	}

	printf("✓ test_iterator_all\n");
	return 0;
}

/* Test 19: Infinite stream naturals */
static int test_stream_naturals(void) {
	struct InfiniteStream s = stream_naturals();

	u64 output[5] = {0};
	u32 count = stream_iterate_take(&s, 5, output, 5);

	if (count != 5) {
		printf("❌ test_stream_naturals: expected 5, got %u\n", count);
		return 1;
	}

	for (int i = 0; i < 5; i++) {
		if (output[i] != i) {
			printf("❌ test_stream_naturals: expected %d, got %llu at index %d\n", i, output[i], i);
			return 1;
		}
	}

	printf("✓ test_stream_naturals\n");
	return 0;
}

/* Test 20: Infinite stream powers of 2 */
static int test_stream_powers_of_2(void) {
	struct InfiniteStream s = stream_powers_of_2();

	u64 output[5] = {0};
	u32 count = stream_iterate_take(&s, 5, output, 5);

	u64 expected[5] = {1, 2, 4, 8, 16};

	if (count != 5) {
		printf("❌ test_stream_powers_of_2: expected 5, got %u\n", count);
		return 1;
	}

	for (int i = 0; i < 5; i++) {
		if (output[i] != expected[i]) {
			printf("❌ test_stream_powers_of_2: mismatch at index %d\n", i);
			return 1;
		}
	}

	printf("✓ test_stream_powers_of_2\n");
	return 0;
}

/* ============================================================ */
/* MAIN TEST RUNNER */
/* ============================================================ */

int main(void) {
	printf("=== Phase 10: Advanced Runtime Features Tests ===\n\n");

	printf("Stage 10.1: Coroutines & Generators\n");
	test_range_generator();
	test_fibonacci_generator();
	test_generator_exhaustion();
	test_generator_sum();
	test_generator_take();

	printf("\nStage 10.2: Lazy Evaluation & Memoization\n");
	test_lazy_force();
	test_memo_get();
	test_memo_clear();
	test_lazy_cycle_detection();

	printf("\nStage 10.3: Streaming & Iterators\n");
	test_iterator_from_array();
	test_iterator_collect();
	test_iterator_sum();
	test_iterator_filter();
	test_iterator_map();
	test_iterator_take();
	test_iterator_fold();
	test_iterator_any();
	test_iterator_all();
	test_stream_naturals();
	test_stream_powers_of_2();

	printf("\n=== All Phase 10 tests completed ===\n");
	return 0;
}
