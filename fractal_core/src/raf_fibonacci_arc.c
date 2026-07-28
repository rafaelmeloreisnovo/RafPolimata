#include "raf_fibonacci_arc.h"

RAF_API RAF_SECTION(".text.raf.fib.init")
void raf_fib_arc_init(RafFibArcState *state) {
    state->f0 = 1u;
    state->f1 = 1u;
    state->step = 0u;
    state->renormalizations = 0u;
    state->friction_flags = RAF_FRICTION_NONE;
#if !RAF_USE_ARCH_MUL
    state->friction_flags |= RAF_FRICTION_ARCH_FALLBACK;
#endif
}

RAF_API RAF_SECTION(".text.raf.fib.step")
void raf_fib_arc_step(RafFibArcState *state) {
    raf_u32 a = state->f0;
    raf_u32 b = state->f1;
    raf_u32 next;

    /* RAF-F002: projective rescaling preserves the ratio and prevents overflow. */
    while (a > (RAF_FIB_STATE_MAX - b)) {
        a = (a + 1u) >> 1;
        b = (b + 1u) >> 1;
        if (a == 0u) {
            a = 1u;
        }
        if (b == 0u) {
            b = 1u;
        }
        state->renormalizations++;
        state->friction_flags |= RAF_FRICTION_FIB_RENORMALIZED;
    }

    next = a + b;
    state->f0 = b;
    state->f1 = next;
    state->step++;
}

RAF_API RAF_SECTION(".text.raf.fib.points")
void raf_fib_arc_points(const RafFibArcState *state, RafFibArcPoints *out) {
    RafProjectiveQ p;

    /* Upper/lower arms: reciprocal Fibonacci growth with opposite orientation. */
    p.num = (raf_s32)state->f1;
    p.den = (raf_s32)state->f0;
    (void)raf_projective_circle(p, &out->upper);

    p.num = -(raf_s32)state->f1;
    (void)raf_projective_circle(p, &out->lower);

    /* Reciprocal arm completes the complementary semicircle. */
    p.num = (raf_s32)state->f0;
    p.den = (raf_s32)state->f1;
    (void)raf_projective_circle(p, &out->reciprocal);
}
