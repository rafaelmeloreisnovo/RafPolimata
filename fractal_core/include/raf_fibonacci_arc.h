#ifndef RAF_FIBONACCI_ARC_H
#define RAF_FIBONACCI_ARC_H

#include "raf_projective.h"

typedef struct {
    raf_u32 f0;
    raf_u32 f1;
    raf_u32 step;
    raf_u32 renormalizations;
    raf_u32 friction_flags;
} RafFibArcState;

typedef struct {
    RafPointQ16 upper;
    RafPointQ16 lower;
    RafPointQ16 reciprocal;
} RafFibArcPoints;

RAF_API void raf_fib_arc_init(RafFibArcState *state);
RAF_API void raf_fib_arc_step(RafFibArcState *state);
RAF_API void raf_fib_arc_points(const RafFibArcState *state, RafFibArcPoints *out);

#endif
