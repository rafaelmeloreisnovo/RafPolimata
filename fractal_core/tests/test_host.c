#include <assert.h>
#include <stdio.h>
#include "raf_fractal_core.h"

static int q16_abs(int v) { return (v < 0) ? -v : v; }

int main(void) {
    RafPointQ16 p;
    RafPointQ16 inf;
    RafProjectiveQ q;
    RafComplexQ16 attach;
    RafMandelResult r;
    RafFractalCore core;
    int i;

    q.num = 1;
    q.den = 1;
    (void)raf_projective_circle(q, &p);
    assert(q16_abs(p.x) <= 2);
    assert(q16_abs(p.y - RAF_Q16_ONE) <= 2);

    raf_projective_infinity(&inf);
    assert(inf.x == RAF_Q16_ONE);
    assert(inf.y == 0);

    raf_mandel_attachment(&attach);
    assert(attach.re == -RAF_Q16_3_QUARTER);
    assert(attach.im == 0);
    r = raf_mandel_eval(attach, 128u);
    assert((r.flags & (RAF_MANDEL_CARDIOID | RAF_MANDEL_BULB_P2)) != 0u);

    r = raf_mandel_eval((RafComplexQ16){ RAF_Q16_ONE, 0 }, 128u);
    assert((r.flags & RAF_MANDEL_ESCAPED) != 0u);

    raf_fractal_init(&core, 128u, RAF_Q16_EPS_PROBE);
    for (i = 0; i < 2000; ++i) {
        raf_fractal_step(&core);
    }
    assert(core.fib.step == 2000u);
    assert(core.fib.renormalizations > 0u);
    assert((core.friction_flags & RAF_FRICTION_FIB_RENORMALIZED) != 0u);
    assert(q16_abs(core.arc.upper.x - core.arc.lower.x) <= 2);
    assert(q16_abs(core.arc.upper.y + core.arc.lower.y) <= 2);

    printf("PASS step=%u renorm=%u flags=0x%08x upper_iter=%u lower_iter=%u\n",
           core.fib.step,
           core.fib.renormalizations,
           core.friction_flags,
           core.upper_result.iterations,
           core.lower_result.iterations);
    return 0;
}
