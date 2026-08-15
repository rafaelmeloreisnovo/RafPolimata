/* rt_streaming.h — Streaming & Iterators (Stage 10.3)
 *
 * Iterator protocol: next(), has_next(), current().
 * Stream operations: map, filter, fold, reduce.
 * Infinite streams: iterate(f, x0) → x0, f(x0), f²(x0), ...
 * Composable streams: chain operations without intermediate arrays.
 * Stream sinks: collect, count, first, last, any, all.
 * Stateless stream operations, no intermediate allocation.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_RT_STREAMING_H
#define APKC_RT_STREAMING_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Iterator protocol state */
enum IteratorState {
	ITER_INIT = 0,      /* Not yet started */
	ITER_ACTIVE = 1,    /* Producing values */
	ITER_EXHAUSTED = 2  /* No more values */
};

/* Generic iterator */
struct Iterator {
	u64 *data;          /* Array data source */
	u32 length;         /* Array length */
	u32 position;       /* Current position */
	u8 state;           /* ITER_INIT, ITER_ACTIVE, ITER_EXHAUSTED */
	u64 current;        /* Current value cache */
};

/* ============================================================ */
/* ITERATOR BASICS */
/* ============================================================ */

/* Create iterator from array */
static inline struct Iterator iterator_from_array(
	u64 *data, u32 length) {

	struct Iterator it = {0};
	it.data = data;
	it.length = length;
	it.position = 0;
	it.state = (length > 0) ? ITER_ACTIVE : ITER_EXHAUSTED;
	return it;
}

/* Check if iterator has more values */
static inline u8 iterator_has_next(struct Iterator *it) {
	if (!it) return 0;
	return (it->state == ITER_ACTIVE && it->position < it->length);
}

/* Get next value */
static inline u8 iterator_next(struct Iterator *it, u64 *value_out) {
	if (!it || !iterator_has_next(it)) {
		it->state = ITER_EXHAUSTED;
		return 1;
	}

	if (it->data) {
		it->current = it->data[it->position];
		if (value_out) *value_out = it->current;
		it->position++;
		return 0;
	}

	return 1;
}

/* Get current value without advancing */
static inline u64 iterator_current(struct Iterator *it) {
	if (!it) return 0;
	return it->current;
}

/* Reset iterator to beginning */
static inline void iterator_reset(struct Iterator *it) {
	if (!it) return;
	it->position = 0;
	it->state = (it->length > 0) ? ITER_ACTIVE : ITER_EXHAUSTED;
	it->current = 0;
}

/* ============================================================ */
/* MAPPED ITERATOR */
/* ============================================================ */

struct MappedIterator {
	struct Iterator source;
	u64 (*fn)(u64);     /* Transformation function */
	u64 current;        /* Current transformed value */
};

/* Create mapped iterator */
static inline struct MappedIterator iterator_map(
	u64 *data, u32 length,
	u64 (*fn)(u64)) {

	struct MappedIterator mi = {0};
	mi.source = iterator_from_array(data, length);
	mi.fn = fn;
	return mi;
}

/* Get next mapped value */
static inline u8 iterator_map_next(
	struct MappedIterator *mi,
	u64 *value_out) {

	if (!mi || !mi->fn) return 1;

	u64 src_value;
	if (iterator_next(&mi->source, &src_value) != 0) {
		return 1;  /* Source exhausted */
	}

	mi->current = mi->fn(src_value);
	if (value_out) *value_out = mi->current;
	return 0;
}

/* ============================================================ */
/* FILTERED ITERATOR */
/* ============================================================ */

struct FilteredIterator {
	struct Iterator source;
	u8 (*predicate)(u64);  /* Filter function */
	u64 current;
};

/* Create filtered iterator */
static inline struct FilteredIterator iterator_filter(
	u64 *data, u32 length,
	u8 (*predicate)(u64)) {

	struct FilteredIterator fi = {0};
	fi.source = iterator_from_array(data, length);
	fi.predicate = predicate;
	return fi;
}

/* Get next filtered value */
static inline u8 iterator_filter_next(
	struct FilteredIterator *fi,
	u64 *value_out) {

	if (!fi || !fi->predicate) return 1;

	u64 value;
	while (iterator_next(&fi->source, &value) == 0) {
		if (fi->predicate(value)) {
			fi->current = value;
			if (value_out) *value_out = value;
			return 0;
		}
	}

	return 1;  /* No more matching values */
}

/* ============================================================ */
/* STREAM OPERATIONS (SINKS) */
/* ============================================================ */

/* Collect all iterator values into output buffer */
static inline u32 iterator_collect(
	struct Iterator *it,
	u64 *output_buf, u32 output_size) {

	if (!it || !output_buf) return 0;

	u32 count = 0;
	u64 value;

	while (count < output_size && iterator_next(it, &value) == 0) {
		output_buf[count++] = value;
	}

	return count;
}

/* Count total values in iterator */
static inline u32 iterator_count(struct Iterator *it) {
	if (!it) return 0;

	u32 count = 0;
	u64 dummy;

	while (iterator_next(it, &dummy) == 0) {
		count++;
	}

	return count;
}

/* Get first value */
static inline u8 iterator_first(
	struct Iterator *it,
	u64 *value_out) {

	if (!it) return 1;
	return iterator_next(it, value_out);
}

/* Get last value */
static inline u8 iterator_last(
	struct Iterator *it,
	u64 *value_out) {

	if (!it) return 1;

	u64 last_val = 0;
	u64 current;

	while (iterator_next(it, &current) == 0) {
		last_val = current;
	}

	if (value_out) *value_out = last_val;
	return (it->position > 0) ? 0 : 1;
}

/* Take first N values */
static inline u32 iterator_take(
	struct Iterator *it,
	u32 n,
	u64 *output_buf, u32 output_size) {

	if (!it || !output_buf || n == 0) return 0;

	u32 limit = (n < output_size) ? n : output_size;
	u32 count = 0;
	u64 value;

	while (count < limit && iterator_next(it, &value) == 0) {
		output_buf[count++] = value;
	}

	return count;
}

/* Skip first N values */
static inline void iterator_skip(struct Iterator *it, u32 n) {
	if (!it) return;

	u32 i;
	u64 dummy;

	for (i = 0; i < n && iterator_next(it, &dummy) == 0; i++) {
		/* Skip */
	}
}

/* ============================================================ */
/* AGGREGATION OPERATIONS */
/* ============================================================ */

/* Sum all values in iterator */
static inline u64 iterator_sum(struct Iterator *it) {
	if (!it) return 0;

	u64 sum = 0;
	u64 value;

	while (iterator_next(it, &value) == 0) {
		sum += value;
	}

	return sum;
}

/* Fold with accumulator function */
static inline u64 iterator_fold(
	struct Iterator *it,
	u64 init,
	u64 (*fn)(u64 acc, u64 val)) {

	if (!it || !fn) return init;

	u64 accumulator = init;
	u64 value;

	while (iterator_next(it, &value) == 0) {
		accumulator = fn(accumulator, value);
	}

	return accumulator;
}

/* Check if any value satisfies predicate */
static inline u8 iterator_any(
	struct Iterator *it,
	u8 (*predicate)(u64)) {

	if (!it || !predicate) return 0;

	u64 value;

	while (iterator_next(it, &value) == 0) {
		if (predicate(value)) {
			return 1;
		}
	}

	return 0;
}

/* Check if all values satisfy predicate */
static inline u8 iterator_all(
	struct Iterator *it,
	u8 (*predicate)(u64)) {

	if (!it || !predicate) return 1;

	u64 value;

	while (iterator_next(it, &value) == 0) {
		if (!predicate(value)) {
			return 0;
		}
	}

	return 1;
}

/* Find maximum value */
static inline u64 iterator_max(struct Iterator *it) {
	if (!it || it->length == 0) return 0;

	u64 max_val = 0;
	u64 value;

	if (iterator_next(it, &max_val) != 0) {
		return 0;
	}

	while (iterator_next(it, &value) == 0) {
		if (value > max_val) {
			max_val = value;
		}
	}

	return max_val;
}

/* Find minimum value */
static inline u64 iterator_min(struct Iterator *it) {
	if (!it || it->length == 0) return 0;

	u64 min_val = 0;
	u64 value;

	if (iterator_next(it, &min_val) != 0) {
		return 0;
	}

	while (iterator_next(it, &value) == 0) {
		if (value < min_val) {
			min_val = value;
		}
	}

	return min_val;
}

/* ============================================================ */
/* INFINITE STREAMS */
/* ============================================================ */

struct InfiniteStream {
	u64 (*fn)(u64);     /* Transformation function */
	u64 current;        /* Current value */
	u32 position;       /* Number of steps taken */
};

/* Create infinite stream: x, f(x), f(f(x)), ... */
static inline struct InfiniteStream stream_iterate(
	u64 x0,
	u64 (*fn)(u64)) {

	struct InfiniteStream s = {0};
	s.fn = fn;
	s.current = x0;
	s.position = 0;
	return s;
}

/* Get next value from infinite stream */
static inline u8 stream_iterate_next(
	struct InfiniteStream *s,
	u64 *value_out) {

	if (!s || !s->fn) return 1;

	if (value_out) *value_out = s->current;
	s->current = s->fn(s->current);
	s->position++;

	return 0;
}

/* Take first N values from infinite stream */
static inline u32 stream_iterate_take(
	struct InfiniteStream *s,
	u32 n,
	u64 *output_buf, u32 output_size) {

	if (!s || !output_buf || n == 0) return 0;

	u32 limit = (n < output_size) ? n : output_size;
	u32 count = 0;
	u64 value;

	while (count < limit && stream_iterate_next(s, &value) == 0) {
		output_buf[count++] = value;
	}

	return count;
}

/* Infinite stream of natural numbers: 0, 1, 2, ... */
static inline u64 stream_succ(u64 x) {
	return x + 1;
}

static inline struct InfiniteStream stream_naturals(void) {
	return stream_iterate(0, stream_succ);
}

/* Infinite stream of powers of 2: 1, 2, 4, 8, ... */
static inline u64 stream_double(u64 x) {
	return x * 2;
}

static inline struct InfiniteStream stream_powers_of_2(void) {
	return stream_iterate(1, stream_double);
}

#endif /* APKC_RT_STREAMING_H */
