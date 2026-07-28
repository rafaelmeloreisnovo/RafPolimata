#ifndef RAF_FRACTAL_FLAGS_H
#define RAF_FRACTAL_FLAGS_H

#include "raf_fractal_base.h"

/*
 * Friction is not erased: each necessary cost becomes an observable bit.
 * Source comments use RAF-Fxxx identifiers; runtime carries the same event.
 */
enum {
    RAF_FRICTION_NONE             = 0u,
    RAF_FRICTION_PROJECTIVE_DIV   = 1u << 0, /* RAF-F001 deterministic divider */
    RAF_FRICTION_FIB_RENORMALIZED = 1u << 1, /* RAF-F002 finite state keeps ratio */
    RAF_FRICTION_MANDEL_ITERATED  = 1u << 2, /* RAF-F003 boundary needs recurrence */
    RAF_FRICTION_SATURATED        = 1u << 3, /* RAF-F004 arithmetic guard fired */
    RAF_FRICTION_ARCH_FALLBACK    = 1u << 4, /* RAF-F005 portable C path selected */
    RAF_FRICTION_PROJECTIVE_INF   = 1u << 5  /* RAF-F006 [n:0] compactified infinity */
};

static RAF_ALWAYS_INLINE raf_q16 raf_q16_add_sat(raf_q16 a, raf_q16 b,
                                                  raf_u32 *flags) {
    raf_s64 sum = (raf_s64)a + (raf_s64)b;
    if (sum > 2147483647LL) {
        *flags |= RAF_FRICTION_SATURATED;
        return (raf_q16)2147483647;
    }
    if (sum < (-2147483647LL - 1LL)) {
        *flags |= RAF_FRICTION_SATURATED;
        return (raf_q16)(-2147483647 - 1);
    }
    return (raf_q16)sum;
}

enum {
    RAF_MANDEL_NONE      = 0u,
    RAF_MANDEL_CARDIOID  = 1u << 0,
    RAF_MANDEL_BULB_P2   = 1u << 1,
    RAF_MANDEL_ESCAPED   = 1u << 2,
    RAF_MANDEL_MAX_ITER  = 1u << 3
};

#endif
