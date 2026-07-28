#ifndef RAF_MANDELBROT_H
#define RAF_MANDELBROT_H

#include "raf_projective.h"

typedef struct {
    raf_q16 re;
    raf_q16 im;
} RafComplexQ16;

typedef struct {
    raf_u32 iterations;
    raf_u32 flags;
    raf_u32 friction_flags;
} RafMandelResult;

/* Exact contact of the main cardioid with the largest circular period-2 bulb. */
#define RAF_MANDEL_ATTACH_RE ((raf_q16)(-RAF_Q16_3_QUARTER))
#define RAF_MANDEL_ATTACH_IM ((raf_q16)0)
#define RAF_MANDEL_P2_CENTER_RE ((raf_q16)(-RAF_Q16_ONE))
#define RAF_MANDEL_P2_RADIUS RAF_Q16_QUARTER

RAF_API void raf_mandel_attachment(RafComplexQ16 *out);
RAF_API void raf_mandel_map_period2_bulb(const RafPointQ16 *unit,
                                         raf_q16 radial_probe,
                                         RafComplexQ16 *out,
                                         raf_u32 *friction_flags);
RAF_API RafMandelResult raf_mandel_eval(RafComplexQ16 c, raf_u32 max_iter);

#endif
