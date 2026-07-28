#include "raf_projective.h"

/* RAF-F001: fixed 64-step divider replaces compiler/libgcc division helpers. */
static RAF_NOINLINE RAF_SECTION(".text.raf.div")
raf_u32 raf_udiv64_u32(raf_u64 num, raf_u32 den) {
    raf_u64 quotient = 0u;
    raf_u64 remainder = 0u;
    raf_u32 bit = 64u;

    while (bit != 0u) {
        bit--;
        remainder = (remainder << 1) | ((num >> bit) & 1u);
        if (remainder >= (raf_u64)den) {
            remainder -= (raf_u64)den;
            quotient |= (raf_u64)1u << bit;
        }
    }
    return (raf_u32)quotient;
}

static RAF_ALWAYS_INLINE raf_s32 raf_sdiv_scaled_q16(raf_s64 numerator,
                                                       raf_u32 denominator) {
    raf_u64 magnitude;
    raf_u32 quotient;
    raf_u32 negative = (numerator < 0) ? 1u : 0u;

    if (negative != 0u) {
        magnitude = (raf_u64)(-(numerator + 1)) + 1u;
    } else {
        magnitude = (raf_u64)numerator;
    }
    quotient = raf_udiv64_u32(magnitude, denominator);
    return (negative != 0u) ? -(raf_s32)quotient : (raf_s32)quotient;
}

static RAF_ALWAYS_INLINE raf_s32 raf_half_signed(raf_s32 v) {
    raf_u32 magnitude = raf_abs_s32(v);
    magnitude = (magnitude + 1u) >> 1;
    return (v < 0) ? -(raf_s32)magnitude : (raf_s32)magnitude;
}

static RAF_ALWAYS_INLINE void raf_projective_normalize(raf_s32 *num,
                                                        raf_s32 *den) {
    raf_u32 max_abs = raf_abs_s32(*num);
    raf_u32 den_abs = raf_abs_s32(*den);
    if (den_abs > max_abs) {
        max_abs = den_abs;
    }
    while (max_abs > RAF_PROJECTIVE_MAX) {
        *num = raf_half_signed(*num);
        *den = raf_half_signed(*den);
        max_abs = (max_abs + 1u) >> 1;
    }
}

RAF_API RAF_SECTION(".text.raf.projective")
raf_u32 raf_projective_circle(RafProjectiveQ p, RafPointQ16 *out) {
    raf_s64 n;
    raf_s64 d;
    raf_u64 n2;
    raf_u64 d2;
    raf_u32 denominator;
    raf_s64 x_numerator;
    raf_s64 y_numerator;
    raf_u32 flags = RAF_FRICTION_PROJECTIVE_DIV;

    if ((p.num == 0) && (p.den == 0)) {
        out->x = 0;
        out->y = 0;
        return RAF_FRICTION_SATURATED;
    }

    raf_projective_normalize(&p.num, &p.den);
    n = (raf_s64)p.num;
    d = (raf_s64)p.den;
    n2 = (raf_u64)(n * n);
    d2 = (raf_u64)(d * d);
    denominator = (raf_u32)(n2 + d2);

    x_numerator = ((raf_s64)n2 - (raf_s64)d2) << 16;
    y_numerator = (2LL * n * d) << 16;
    out->x = (raf_q16)raf_sdiv_scaled_q16(x_numerator, denominator);
    out->y = (raf_q16)raf_sdiv_scaled_q16(y_numerator, denominator);

    if (p.den == 0) {
        flags |= RAF_FRICTION_PROJECTIVE_INF;
    }
    return flags;
}

RAF_API RAF_SECTION(".text.raf.projective.infinity")
void raf_projective_infinity(RafPointQ16 *out) {
    out->x = RAF_Q16_ONE;
    out->y = 0;
}
