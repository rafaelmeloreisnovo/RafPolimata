#ifndef RAF_PROJECTIVE_H
#define RAF_PROJECTIVE_H

#include "raf_fractal_flags.h"

typedef struct {
    raf_q16 x;
    raf_q16 y;
} RafPointQ16;

typedef struct {
    raf_s32 num;
    raf_s32 den;
} RafProjectiveQ;

/*
 * Rotated rational parameterisation of S^1:
 *   x = (n^2-d^2)/(n^2+d^2)
 *   y = 2nd/(n^2+d^2)
 * The projective point [1:0] (infinity of the real line) becomes (1,0).
 */
RAF_API raf_u32 raf_projective_circle(RafProjectiveQ p, RafPointQ16 *out);

/* Exact finite image of the projective infinity point. */
RAF_API void raf_projective_infinity(RafPointQ16 *out);

#endif
