#include "raf_fractal_core.h"

RAF_API RAF_SECTION(".text.raf.core.sample")
void raf_fractal_sample(RafFractalCore *core) {
    raf_u32 flags = core->fib.friction_flags;

    raf_fib_arc_points(&core->fib, &core->arc);
    raf_mandel_map_period2_bulb(&core->arc.upper, core->radial_probe,
                                &core->upper_probe, &flags);
    raf_mandel_map_period2_bulb(&core->arc.lower, core->radial_probe,
                                &core->lower_probe, &flags);
    raf_mandel_map_period2_bulb(&core->arc.reciprocal, core->radial_probe,
                                &core->reciprocal_probe, &flags);

    core->upper_result = raf_mandel_eval(core->upper_probe, core->max_iter);
    core->lower_result = raf_mandel_eval(core->lower_probe, core->max_iter);
    core->reciprocal_result = raf_mandel_eval(core->reciprocal_probe,
                                              core->max_iter);
    flags |= core->upper_result.friction_flags;
    flags |= core->lower_result.friction_flags;
    flags |= core->reciprocal_result.friction_flags;
    core->friction_flags = flags;
}

RAF_API RAF_SECTION(".text.raf.core.init")
void raf_fractal_init(RafFractalCore *core,
                      raf_u32 max_iter,
                      raf_q16 radial_probe) {
    raf_fib_arc_init(&core->fib);
    core->max_iter = max_iter;
    core->radial_probe = radial_probe;
    core->friction_flags = core->fib.friction_flags;
    raf_fractal_sample(core);
}

RAF_API RAF_SECTION(".text.raf.core.step")
void raf_fractal_step(RafFractalCore *core) {
    raf_fib_arc_step(&core->fib);
    raf_fractal_sample(core);
}
