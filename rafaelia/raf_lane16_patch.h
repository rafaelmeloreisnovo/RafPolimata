#ifndef RAFAELIA_LANE16_PATCH_H
#define RAFAELIA_LANE16_PATCH_H

/*
 * Static deterministic plan: up to 16 independent lanes and masked 64-bit
 * replacement. This exposes independence to ISA/SIMD/GPU backends; it does
 * not claim that a CPU will physically issue sixteen instructions in one
 * cycle. Actual issue width is measured, never inferred.
 */

#define RAF_LANE16_COUNT 16u

#define RAF_LANE_CPU0      0u
#define RAF_LANE_CPU1      1u
#define RAF_LANE_CPU2      2u
#define RAF_LANE_CPU3      3u
#define RAF_LANE_CPU4      4u
#define RAF_LANE_CPU5      5u
#define RAF_LANE_CPU6      6u
#define RAF_LANE_CPU7      7u
#define RAF_LANE_SIMD      8u
#define RAF_LANE_NEON      9u
#define RAF_LANE_GPU      10u
#define RAF_LANE_CRC32_SW 11u
#define RAF_LANE_CACHE_L1 12u
#define RAF_LANE_CACHE_L2 13u
#define RAF_LANE_RAM_BUF  14u
#define RAF_LANE_STORAGE  15u

typedef struct {
    unsigned long long mask;
    unsigned long long value;
} rafaelia_bit_patch64_t;

typedef struct {
    unsigned int word_index;
    rafaelia_bit_patch64_t patch;
} rafaelia_overlay64_t;

typedef struct {
    unsigned short active_mask;
    unsigned short done_mask;
    unsigned short dependency_mask[RAF_LANE16_COUNT];
    unsigned short resource_mask[RAF_LANE16_COUNT];
} rafaelia_lane16_plan_t;

static inline unsigned long long
rafaelia_patch64(unsigned long long current, rafaelia_bit_patch64_t patch) {
    return current ^ ((current ^ patch.value) & patch.mask);
}

static inline unsigned long long
rafaelia_patch64_changed_bits(unsigned long long before,
                              unsigned long long after) {
    return before ^ after;
}

static inline int
rafaelia_overlay64_apply(unsigned long long *words,
                         unsigned int word_count,
                         rafaelia_overlay64_t overlay) {
    unsigned long long before;
    if (words == (unsigned long long *)0 || overlay.word_index >= word_count) {
        return 0;
    }
    before = words[overlay.word_index];
    words[overlay.word_index] = rafaelia_patch64(before, overlay.patch);
    return 1;
}

static inline void rafaelia_lane16_init(rafaelia_lane16_plan_t *plan) {
    unsigned int i;
    if (plan == (rafaelia_lane16_plan_t *)0) {
        return;
    }
    plan->active_mask = 0u;
    plan->done_mask = 0u;
    for (i = 0u; i < RAF_LANE16_COUNT; ++i) {
        plan->dependency_mask[i] = 0u;
        plan->resource_mask[i] = 0u;
    }
}

static inline int
rafaelia_lane16_configure(rafaelia_lane16_plan_t *plan,
                          unsigned int lane,
                          unsigned short dependencies,
                          unsigned short resources) {
    if (plan == (rafaelia_lane16_plan_t *)0 || lane >= RAF_LANE16_COUNT) {
        return 0;
    }
    plan->active_mask = (unsigned short)(plan->active_mask | (unsigned short)(1u << lane));
    plan->dependency_mask[lane] = dependencies;
    plan->resource_mask[lane] = resources;
    return 1;
}

static inline unsigned short
rafaelia_lane16_ready_mask(const rafaelia_lane16_plan_t *plan) {
    unsigned int lane;
    unsigned short ready = 0u;
    if (plan == (const rafaelia_lane16_plan_t *)0) {
        return 0u;
    }
    for (lane = 0u; lane < RAF_LANE16_COUNT; ++lane) {
        unsigned short bit = (unsigned short)(1u << lane);
        unsigned short active = (unsigned short)(plan->active_mask & bit);
        unsigned short pending = (unsigned short)(bit & (unsigned short)~plan->done_mask);
        unsigned short deps = plan->dependency_mask[lane];
        unsigned short deps_done = (unsigned short)(deps & plan->done_mask);
        if (active != 0u && pending != 0u && deps_done == deps) {
            ready = (unsigned short)(ready | bit);
        }
    }
    return ready;
}

/* Selects a deterministic subset of ready lanes whose declared resource masks
 * do not collide. Lower lane id wins a conflict. Resource mask zero means the
 * backend declared no exclusive resource for that lane. */
static inline unsigned short
rafaelia_lane16_dispatch_mask(const rafaelia_lane16_plan_t *plan) {
    unsigned int lane;
    unsigned short ready;
    unsigned short selected = 0u;
    unsigned short used_resources = 0u;
    if (plan == (const rafaelia_lane16_plan_t *)0) {
        return 0u;
    }
    ready = rafaelia_lane16_ready_mask(plan);
    for (lane = 0u; lane < RAF_LANE16_COUNT; ++lane) {
        unsigned short bit = (unsigned short)(1u << lane);
        unsigned short resources = plan->resource_mask[lane];
        if ((ready & bit) != 0u &&
            (resources == 0u || (resources & used_resources) == 0u)) {
            selected = (unsigned short)(selected | bit);
            used_resources = (unsigned short)(used_resources | resources);
        }
    }
    return selected;
}

static inline int
rafaelia_lane16_complete(rafaelia_lane16_plan_t *plan, unsigned int lane) {
    unsigned short bit;
    if (plan == (rafaelia_lane16_plan_t *)0 || lane >= RAF_LANE16_COUNT) {
        return 0;
    }
    bit = (unsigned short)(1u << lane);
    if ((plan->active_mask & bit) == 0u) {
        return 0;
    }
    plan->done_mask = (unsigned short)(plan->done_mask | bit);
    return 1;
}

#endif
