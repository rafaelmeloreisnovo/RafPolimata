#ifndef RAF_FRACTAL_CORE_H
#define RAF_FRACTAL_CORE_H

#include "raf_fibonacci_arc.h"
#include "raf_mandelbrot.h"

typedef struct {
    RafFibArcState fib;
    RafFibArcPoints arc;
    RafComplexQ16 upper_probe;
    RafComplexQ16 lower_probe;
    RafComplexQ16 reciprocal_probe;
    RafMandelResult upper_result;
    RafMandelResult lower_result;
    RafMandelResult reciprocal_result;
    raf_q16 radial_probe;
    raf_u32 max_iter;
    raf_u32 friction_flags;
} RafFractalCore;

RAF_API void raf_fractal_init(RafFractalCore *core,
                              raf_u32 max_iter,
                              raf_q16 radial_probe);
RAF_API void raf_fractal_step(RafFractalCore *core);
RAF_API void raf_fractal_sample(RafFractalCore *core);

#endif
