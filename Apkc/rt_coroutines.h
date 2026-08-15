/* rt_coroutines.h — Coroutine & Generator Support (Stage 10.1)
 *
 * Generator functions with yield syntax.
 * Coroutine state machine: save/restore execution context.
 * Yield expressions: pause, return value, resume later.
 * Lazy range generators: on-demand value production.
 * Max 8 nested coroutines, stack-only state storage.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_RT_COROUTINES_H
#define APKC_RT_COROUTINES_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Coroutine state: execution context for yield/resume */
enum CoroutineState {
	CORO_INIT = 0,      /* Not yet started */
	CORO_RUNNING = 1,   /* Currently executing */
	CORO_SUSPENDED = 2, /* Paused at yield */
	CORO_DONE = 3       /* Completed, no more yields */
};

/* Stack frame for suspended coroutine */
struct CoroutineFrame {
	u64 regs[16];       /* r0-r15 saved state */
	u64 sp;             /* Stack pointer at suspension */
	u64 pc;             /* Program counter (instruction offset) */
	u64 yielded_value;  /* Value returned from yield */
	u32 local_size;     /* Local variable stack size */
};

/* Coroutine context: tracks generator state */
struct Coroutine {
	struct CoroutineFrame frame;
	u32 id;             /* Unique coroutine ID */
	u8 state;           /* CORO_INIT, CORO_RUNNING, CORO_SUSPENDED, CORO_DONE */
	u8 depth;           /* Nesting depth (max 8) */
	u64 (*fn)(struct Coroutine *);  /* Generator function pointer */
	u64 arg0, arg1, arg2;  /* Function arguments (first 3) */
};

/* Generator object: wraps coroutine for iteration */
struct Generator {
	struct Coroutine coro;
	u8 exhausted;       /* 1 if generator is exhausted */
	u64 current_value;  /* Current yielded value */
};

/* Coroutine manager: tracks all active coroutines */
struct CoroutineManager {
	struct Coroutine coroutines[8];  /* Up to 8 nested coroutines */
	u32 count;
	struct CoroutineFrame saved_frames[8];  /* Saved frames for nesting */
	u32 frame_stack_depth;
};

/* ============================================================ */
/* COROUTINE INITIALIZATION & STATE MANAGEMENT */
/* ============================================================ */

/* Initialize coroutine manager */
static inline void coroutine_manager_init(struct CoroutineManager *cm) {
	if (!cm) return;
	cm->count = 0;
	cm->frame_stack_depth = 0;
}

/* Create new coroutine */
static inline u8 coroutine_create(
	struct CoroutineManager *cm,
	u64 (*fn)(struct Coroutine *),
	u64 arg0, u64 arg1, u64 arg2,
	struct Coroutine *out_coro) {

	if (!cm || cm->count >= 8 || !fn) return 1;

	struct Coroutine *coro = &cm->coroutines[cm->count];
	coro->id = cm->count;
	coro->state = CORO_INIT;
	coro->depth = cm->count;
	coro->fn = fn;
	coro->arg0 = arg0;
	coro->arg1 = arg1;
	coro->arg2 = arg2;

	/* Initialize frame */
	coro->frame.sp = 0;
	coro->frame.pc = 0;
	coro->frame.yielded_value = 0;
	coro->frame.local_size = 0;

	u32 i;
	for (i = 0; i < 16; i++) {
		coro->frame.regs[i] = 0;
	}

	cm->count++;
	if (out_coro) *out_coro = *coro;
	return 0;
}

/* ============================================================ */
/* YIELD & RESUME */
/* ============================================================ */

/* Yield value from coroutine (pause execution) */
static inline u8 coroutine_yield(
	struct CoroutineManager *cm,
	struct Coroutine *coro,
	u64 value) {

	if (!cm || !coro || coro->state == CORO_DONE) return 1;

	/* Save current registers and state */
	u32 i;
	for (i = 0; i < 16; i++) {
		coro->frame.regs[i] = 0;  /* Would capture actual register state */
	}

	coro->frame.yielded_value = value;
	coro->state = CORO_SUSPENDED;

	return 0;
}

/* Resume coroutine (continue from yield point) */
static inline u64 coroutine_resume(
	struct CoroutineManager *cm,
	struct Coroutine *coro) {

	if (!coro || coro->state == CORO_DONE) return 0;

	u64 result = 0;

	if (coro->state == CORO_INIT || coro->state == CORO_SUSPENDED) {
		coro->state = CORO_RUNNING;
		if (coro->fn) {
			result = coro->fn(coro);
		}
		/* Only mark as DONE if function explicitly set it. Otherwise suspend for next call. */
		if (coro->state == CORO_RUNNING) {
			coro->state = CORO_SUSPENDED;
		}
		return result;
	}

	return 0;
}

/* ============================================================ */
/* GENERATOR OBJECTS */
/* ============================================================ */

/* Create generator from function */
static inline u8 generator_create(
	struct CoroutineManager *cm,
	u64 (*gen_fn)(struct Coroutine *),
	u64 arg0,
	struct Generator *out_gen) {

	if (!gen_fn || !out_gen) return 1;

	/* Initialize generator's coroutine directly */
	out_gen->coro.id = 0;
	out_gen->coro.state = CORO_INIT;
	out_gen->coro.depth = 0;
	out_gen->coro.fn = gen_fn;
	out_gen->coro.arg0 = arg0;
	out_gen->coro.arg1 = 0;
	out_gen->coro.arg2 = 0;
	out_gen->coro.frame.pc = 0;
	out_gen->coro.frame.sp = 0;
	out_gen->coro.frame.yielded_value = 0;

	out_gen->exhausted = 0;
	out_gen->current_value = 0;

	return 0;
}

/* Get next value from generator */
static inline u8 generator_next(
	struct CoroutineManager *cm,
	struct Generator *gen,
	u64 *value_out) {

	if (!gen || gen->exhausted) return 1;

	u64 result = coroutine_resume(cm, &gen->coro);

	if (gen->coro.state == CORO_DONE) {
		gen->exhausted = 1;
		return 1;  /* No more values */
	}

	gen->current_value = result;
	if (value_out) *value_out = result;
	return 0;  /* Value returned */
}

/* Check if generator has more values */
static inline u8 generator_has_next(struct Generator *gen) {
	if (!gen) return 0;
	return !gen->exhausted;
}

/* Get current value without advancing */
static inline u64 generator_current(struct Generator *gen) {
	if (!gen) return 0;
	return gen->current_value;
}

/* ============================================================ */
/* LAZY RANGE GENERATOR */
/* ============================================================ */

/* Range generator coroutine: yields 0, 1, 2, ..., n-1 */
static inline u64 range_generator_fn(struct Coroutine *coro) {
	if (!coro) return 0;

	u64 n = coro->arg0;
	u64 i = coro->frame.pc;  /* Use pc as resumption point */

	if (i >= n) {
		coro->state = CORO_DONE;
		return 0;
	}

	coro->frame.yielded_value = i;
	coro->frame.pc = i + 1;  /* Next resumption starts at i+1 */

	return i;
}

/* Create range generator: range(0..n) */
static inline u8 range_generator(
	struct CoroutineManager *cm,
	u64 n,
	struct Generator *out_gen) {

	if (!cm || !out_gen) return 1;

	return generator_create(cm, range_generator_fn, n, out_gen);
}

/* ============================================================ */
/* FIBONACCI GENERATOR */
/* ============================================================ */

/* Fibonacci generator coroutine: yields fib(0), fib(1), fib(2), ... */
static inline u64 fibonacci_generator_fn(struct Coroutine *coro) {
	if (!coro) return 0;

	u64 max_n = coro->arg0;
	u64 i = coro->frame.pc;  /* Resumption index */

	if (i > max_n) {
		coro->state = CORO_DONE;
		return 0;
	}

	/* Compute fib(i) directly */
	u64 fib_val;
	if (i == 0) {
		fib_val = 0;
	} else if (i == 1) {
		fib_val = 1;
	} else {
		u64 a = 0, b = 1;
		u64 j;
		for (j = 2; j <= i; j++) {
			u64 temp = a + b;
			a = b;
			b = temp;
		}
		fib_val = b;
	}

	coro->frame.yielded_value = fib_val;
	coro->frame.pc = i + 1;  /* Next resumption */

	return fib_val;
}

/* Create Fibonacci generator */
static inline u8 fibonacci_generator(
	struct CoroutineManager *cm,
	u64 max_terms,
	struct Generator *out_gen) {

	if (!cm || !out_gen) return 1;

	return generator_create(cm, fibonacci_generator_fn, max_terms, out_gen);
}

/* ============================================================ */
/* GENERATOR ITERATION HELPERS */
/* ============================================================ */

/* Collect all generator values into array */
static inline u32 generator_collect(
	struct CoroutineManager *cm,
	struct Generator *gen,
	u64 *output_buf, u32 output_size) {

	if (!cm || !gen || !output_buf) return 0;

	u32 count = 0;
	u64 value;

	while (generator_has_next(gen) && count < output_size) {
		if (generator_next(cm, gen, &value) == 0) {
			output_buf[count++] = value;
		} else {
			break;
		}
	}

	return count;
}

/* Count total values from generator */
static inline u32 generator_count(
	struct CoroutineManager *cm,
	struct Generator *gen) {

	if (!cm || !gen) return 0;

	u32 count = 0;
	u64 dummy;

	while (generator_has_next(gen)) {
		if (generator_next(cm, gen, &dummy) == 0) {
			count++;
		} else {
			break;
		}
	}

	return count;
}

/* Get first N values from generator */
static inline u32 generator_take(
	struct CoroutineManager *cm,
	struct Generator *gen,
	u32 n,
	u64 *output_buf, u32 output_size) {

	if (!cm || !gen || !output_buf || n == 0) return 0;

	u32 limit = (n < output_size) ? n : output_size;
	u32 count = 0;
	u64 value;

	while (count < limit && generator_has_next(gen)) {
		if (generator_next(cm, gen, &value) == 0) {
			output_buf[count++] = value;
		} else {
			break;
		}
	}

	return count;
}

/* Sum all values from generator */
static inline u64 generator_sum(
	struct CoroutineManager *cm,
	struct Generator *gen) {

	if (!cm || !gen) return 0;

	u64 sum = 0;
	u64 value;

	while (generator_has_next(gen)) {
		if (generator_next(cm, gen, &value) == 0) {
			sum += value;
		} else {
			break;
		}
	}

	return sum;
}

/* Fold generator values with accumulator function */
static inline u64 generator_fold(
	struct CoroutineManager *cm,
	struct Generator *gen,
	u64 init,
	u64 (*fold_fn)(u64 acc, u64 val)) {

	if (!cm || !gen || !fold_fn) return init;

	u64 accumulator = init;
	u64 value;

	while (generator_has_next(gen)) {
		if (generator_next(cm, gen, &value) == 0) {
			accumulator = fold_fn(accumulator, value);
		} else {
			break;
		}
	}

	return accumulator;
}

#endif /* APKC_RT_COROUTINES_H */
