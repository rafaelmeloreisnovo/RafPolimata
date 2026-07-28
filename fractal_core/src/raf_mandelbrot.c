#include "raf_mandelbrot.h"

static RAF_ALWAYS_INLINE raf_q16 raf_square_q16(raf_q16 v) {
    return raf_q16_mul(v, v);
}

static RAF_ALWAYS_INLINE raf_u32 raf_mandel_in_period2_bulb(RafComplexQ16 c) {
    raf_q16 x = c.re + RAF_Q16_ONE;
    raf_q16 r2 = raf_square_q16(x) + raf_square_q16(c.im);
    return (r2 <= (RAF_Q16_ONE >> 4)) ? 1u : 0u; /* radius^2 = 1/16 */
}

static RAF_ALWAYS_INLINE raf_u32 raf_mandel_in_cardioid(RafComplexQ16 c) {
    raf_q16 x = c.re - RAF_Q16_QUARTER;
    raf_q16 y2 = raf_square_q16(c.im);
    raf_q16 q = raf_square_q16(x) + y2;
    raf_q16 left = raf_q16_mul(q, q + x);
    raf_q16 right = y2 >> 2;
    return (left <= right) ? 1u : 0u;
}

RAF_API RAF_SECTION(".text.raf.mandel.attach")
void raf_mandel_attachment(RafComplexQ16 *out) {
    out->re = RAF_MANDEL_ATTACH_RE;
    out->im = RAF_MANDEL_ATTACH_IM;
}

RAF_API RAF_SECTION(".text.raf.mandel.map")
void raf_mandel_map_period2_bulb(const RafPointQ16 *unit,
                                 raf_q16 radial_probe,
                                 RafComplexQ16 *out,
                                 raf_u32 *friction_flags) {
    raf_q16 radius = RAF_MANDEL_P2_RADIUS;
    radius = raf_q16_add_sat(radius, radial_probe, friction_flags);
    out->re = RAF_MANDEL_P2_CENTER_RE + raf_q16_mul(radius, unit->x);
    out->im = raf_q16_mul(radius, unit->y);
}

RAF_API RAF_SECTION(".text.raf.mandel.eval")
RafMandelResult raf_mandel_eval(RafComplexQ16 c, raf_u32 max_iter) {
    RafMandelResult result;
    raf_q16 zr = 0;
    raf_q16 zi = 0;
    raf_u32 i = 0u;

    result.iterations = max_iter;
    result.flags = RAF_MANDEL_NONE;
    result.friction_flags = RAF_FRICTION_NONE;

    if (raf_mandel_in_period2_bulb(c) != 0u) {
        result.flags = RAF_MANDEL_BULB_P2 | RAF_MANDEL_MAX_ITER;
        return result;
    }
    if (raf_mandel_in_cardioid(c) != 0u) {
        result.flags = RAF_MANDEL_CARDIOID | RAF_MANDEL_MAX_ITER;
        return result;
    }

    /* RAF-F003: only unresolved boundary points pay the recurrence cost. */
    result.friction_flags |= RAF_FRICTION_MANDEL_ITERATED;
    while (i < max_iter) {
        raf_q16 zr2 = raf_square_q16(zr);
        raf_q16 zi2 = raf_square_q16(zi);
        raf_q16 next_re;
        raf_q16 next_im;

        if ((zr2 + zi2) > (RAF_Q16_ONE << 2)) {
            result.iterations = i;
            result.flags = RAF_MANDEL_ESCAPED;
            return result;
        }
        next_im = (raf_q16)((((raf_s64)zr * (raf_s64)zi) >> 15) + c.im);
        next_re = zr2 - zi2 + c.re;
        zr = next_re;
        zi = next_im;
        i++;
    }

    result.flags = RAF_MANDEL_MAX_ITER;
    return result;
}
