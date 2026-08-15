/* perf_optimization_hints.h — Optimization Recommendation Engine (Stage 20.3)
 *
 * Hotspot analysis: identify functions needing optimization.
 * Optimization suggestions: recommend specific improvements.
 * Impact prediction: estimate performance gain from optimization.
 * Algorithm alternatives: suggest algorithmic improvements.
 * Inlining candidates: identify functions for inlining.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_PERF_OPTIMIZATION_HINTS_H
#define APKC_PERF_OPTIMIZATION_HINTS_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Optimization suggestion */
struct OptimizationHint {
	const char *target_function;
	const char *hint_type;  /* "inline", "loop-unroll", "vectorize", "cache-align" */
	const char *description;
	u32 estimated_speedup;  /* Percentage improvement (e.g., 25 = 25% faster) */
	u8 priority;            /* 1=low, 2=medium, 3=high, 4=critical */
};

/* Optimization engine */
struct OptimizationEngine {
	struct OptimizationHint hints[64];
	u32 hint_count;
	u32 total_speedup_potential;
	u32 critical_hints;
};

/* Initialize optimization engine */
static inline void optengine_init(struct OptimizationEngine *engine) {
	if (!engine) return;
	engine->hint_count = 0;
	engine->total_speedup_potential = 0;
	engine->critical_hints = 0;
}

/* Add optimization hint */
static inline u8 optengine_add_hint(
	struct OptimizationEngine *engine,
	const char *function,
	const char *hint_type,
	const char *description,
	u32 speedup_pct,
	u8 priority) {

	if (!engine || !function || !hint_type || engine->hint_count >= 64) return 0;

	struct OptimizationHint *hint = &engine->hints[engine->hint_count];
	hint->target_function = function;
	hint->hint_type = hint_type;
	hint->description = description;
	hint->estimated_speedup = speedup_pct;
	hint->priority = priority;

	engine->hint_count++;
	engine->total_speedup_potential += speedup_pct;
	if (priority >= 3) engine->critical_hints++;
	return 1;
}

/* Get total potential speedup */
static inline u32 optengine_get_total_speedup(struct OptimizationEngine *engine) {
	if (!engine) return 0;
	return engine->total_speedup_potential;
}

#endif /* APKC_PERF_OPTIMIZATION_HINTS_H */
