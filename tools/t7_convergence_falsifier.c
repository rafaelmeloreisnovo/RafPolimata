/* L9 T^7 convergence falsifier.
 * Tests the strong claim "the attractor converges to a fixed point" against
 * the actual integer-Q16 transition implemented in Benchmark/raf_toroid.h.
 *
 * This does NOT claim the whole T^7 system is divergent. It distinguishes:
 *   - range invariant: attractor in Z_42 (provable by construction), from
 *   - fixed-point convergence: attractor eventually constant (falsified here
 *     for the canonical init with constant H_in=0, C_in=1.0).
 */
#include <stdio.h>
#include "../Benchmark/raf_toroid.h"

int main(void) {
    T7State t;
    t7_init(&t);

    unsigned changes_after_stable = 0;
    unsigned distinct_mask_lo = 0u;
    unsigned stable_steps = 0;
    unsigned prev = t.attractor;
    int saw_integer_iir_limit = 0;

    for (unsigned i = 0; i < 1024u; ++i) {
        t7_step(&t, 0, Q16_ONE);
        if (t.attractor >= 42u) {
            fprintf(stderr, "L9 FAIL range invariant attractor=%u step=%u\n", t.attractor, i);
            return 2;
        }

        /* Canonical integer IIR reaches H=3, C=65536 and then remains there. */
        if (t.H == 3 && t.C == Q16_ONE) {
            saw_integer_iir_limit = 1;
            ++stable_steps;
            if (t.attractor != prev) ++changes_after_stable;
            if (t.attractor < 32u) distinct_mask_lo |= 1u << t.attractor;
        }
        prev = t.attractor;
    }

    if (!saw_integer_iir_limit || stable_steps < 100u) {
        fprintf(stderr, "L9 FAIL expected integer-IIR asymptotic regime not reached H=%d C=%d\n",
                (int)t.H, (int)t.C);
        return 3;
    }
    if (t.phi != 65533) {
        fprintf(stderr, "L9 FAIL unexpected asymptotic phi=%d expected=65533\n", (int)t.phi);
        return 4;
    }

    /* In the stable H/C regime omega=floor(phi*6/65536)=5, so a trajectory
     * that were eventually fixed would require collapse back to the same
     * phase value every step. But phase_acc changes every step; the observed
     * trajectory must therefore continue changing. This executable falsifier
     * requires many actual post-stabilization changes, not one anecdotal step. */
    if (changes_after_stable < 100u) {
        fprintf(stderr, "L9 FAIL counterexample not reproduced changes=%u\n", changes_after_stable);
        return 5;
    }

    printf("L9_RANGE_INVARIANT_PASS attractor=%u\n", t.attractor);
    printf("L9_INTEGER_IIR_LIMIT_PASS H=%d C=%d phi=%d omega=%u\n",
           (int)t.H, (int)t.C, (int)t.phi,
           (unsigned)(((u64)(u32)t.phi * 6u) >> 16));
    printf("L9_STRONG_FIXED_POINT_CONVERGENCE_FALSIFIED stable_steps=%u changes=%u distinct_mask_lo=0x%08x\n",
           stable_steps, changes_after_stable, distinct_mask_lo);
    printf("L9_NEXT_THEOREM=range_invariance_plus_eventual_periodicity_not_fixed_point claim_allowed=false\n");
    return 0;
}
