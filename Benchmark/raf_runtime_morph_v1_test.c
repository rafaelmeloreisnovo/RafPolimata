/* No hosted headers: host executable returns the number of failed assertions. */
#include "raf_runtime_morph_v1.h"
#define CHECK(X) failures += (u32)(!(X))

int main(void) {
    u32 failures = 0u;
    RafMorphV1Profile p = {RAF_MORPH_V1_ARCH_ARM32, 0u, 0u, 0u, 0u, 0u,
                           RAF_MORPH_V1_VALIDATED};
    RafMorphV1Journal j = {0u, 0u, 0u};
    RafMorphV1Route r = raf_morph_v1_route(p, j);
    CHECK(r.backend == RAF_MORPH_V1_GENERIC && r.fail_safe == 1u);
    CHECK(r.unknown_or_blocked == 1u && r.eligible_caps == 0u);

    p.detected_caps = RAF_MORPH_V1_CAP_ARM32;
    r = raf_morph_v1_route(p, j);
    CHECK(r.backend == 0u); /* no compiled implementation */
    p.compiled_caps = RAF_MORPH_V1_CAP_ARM32;
    r = raf_morph_v1_route(p, j);
    CHECK(r.backend == 0u); /* no callable implementation */
    p.executable_caps = RAF_MORPH_V1_CAP_ARM32;
    r = raf_morph_v1_route(p, j);
    CHECK(r.backend == 0u); /* no verified gate */
    p.verified_caps = RAF_MORPH_V1_CAP_ARM32;
    r = raf_morph_v1_route(p, j);
    CHECK(r.backend == RAF_MORPH_V1_ARM32_NEON && r.fail_safe == 0u);
    CHECK(r.fallback_backend == 0u && r.generation == 0u);

    p.arch = RAF_MORPH_V1_ARCH_ARM64;
    CHECK(raf_morph_v1_route(p, j).backend == 0u); /* wrong ISA */
    p.arch = RAF_MORPH_V1_ARCH_ARM32;
    p.degraded_caps = RAF_MORPH_V1_CAP_ARM32;
    CHECK(raf_morph_v1_route(p, j).backend == 0u);
    p.degraded_caps = 0u;
    p.phase = RAF_MORPH_V1_BASELINE;
    CHECK(raf_morph_v1_route(p, j).backend == 0u);
    p.phase = RAF_MORPH_V1_ROLLBACK;
    CHECK(raf_morph_v1_route(p, j).backend == 0u);

    p.phase = RAF_MORPH_V1_CANDIDATE;
    r = raf_morph_v1_route(p, j);
    RafMorphV1Feedback f = raf_morph_v1_feedback(p, j, r, 0);
    CHECK(f.journal.last_good_backend == 0u); /* candidate cannot self-promote */
    CHECK(f.journal.generation == 1u && f.rollback == 0u);
    CHECK(raf_morph_v1_feedback(p, f.journal, r, -1).stale_feedback == 1u);
    CHECK(raf_morph_v1_feedback(p, f.journal, r, -1).effective_backend == 0u);
    CHECK(raf_morph_v1_feedback(p, f.journal, r, -1).journal.quarantined_caps == 0u);

    j.generation = f.journal.generation;
    p.phase = RAF_MORPH_V1_VALIDATED;
    r = raf_morph_v1_route(p, j);
    f = raf_morph_v1_feedback(p, j, r, 0);
    CHECK(f.journal.last_good_backend == RAF_MORPH_V1_ARM32_NEON);
    CHECK(f.journal.generation == 2u);

    j = f.journal;
    r = raf_morph_v1_route(p, j);
    f = raf_morph_v1_feedback(p, j, r, -7);
    CHECK(f.rollback == 1u && f.effective_backend == 0u);
    CHECK((f.journal.quarantined_caps & RAF_MORPH_V1_CAP_ARM32) != 0u);
    CHECK(raf_morph_v1_route(p, f.journal).backend == 0u);
    CHECK(f.journal.generation == 3u);
    r.backend = 32u; /* adversarial forged feedback: formerly undefined shift */
    f = raf_morph_v1_feedback(p, j, r, -9);
    CHECK(f.stale_feedback == 1u && f.effective_backend == 0u);
    CHECK(f.journal.generation == j.generation && f.journal.quarantined_caps == j.quarantined_caps);
    r = raf_morph_v1_route(p, j);
    f = raf_morph_v1_feedback(p, j, r, 7); /* positive is also failure */
    CHECK(f.rollback == 1u);

    p.arch = RAF_MORPH_V1_ARCH_ARM64;
    p.detected_caps = RAF_MORPH_V1_CAP_ARM64;
    p.compiled_caps = RAF_MORPH_V1_CAP_ARM64;
    p.executable_caps = RAF_MORPH_V1_CAP_ARM64;
    p.verified_caps = RAF_MORPH_V1_CAP_ARM64;
    j.quarantined_caps = 0u;
    j.last_good_backend = 0u;
    r = raf_morph_v1_route(p, j);
    CHECK(r.backend == RAF_MORPH_V1_ARM64_NEON);

    /* Presence mask is a truth state, not forced imputation of a zero. */
    RafMorphV1T7 prior = {{100u, 200u, 300u, 400u, 500u, 600u, 700u}, 0x01u};
    RafMorphV1T7 sample = {{0u, 800u, 800u, 800u, 800u, 800u, 800u}, 0x02u};
    RafMorphV1T7 next = raf_morph_v1_t7(prior, sample);
    CHECK(next.q[0] == 100u && next.q[1] == 350u);
    CHECK(next.q[2] == 300u && next.q[6] == 700u);
    CHECK(next.present_mask == 3u);
    sample.present_mask = 0u;
    CHECK(raf_morph_v1_t7(prior, sample).present_mask == prior.present_mask);
    sample.present_mask = 0x7Fu;
    next = raf_morph_v1_t7(prior, sample);
    CHECK(next.q[0] == 75u && next.q[6] == 725u);
    CHECK(next.present_mask == 127u);
    sample.present_mask = 0xFFFFFF80u;
    next = raf_morph_v1_t7(prior, sample);
    CHECK(next.present_mask == 1u && next.q[0] == 100u);

    /* Exhaustive 7-bit semantic presence: absent lanes must be bit-identical. */
    for (u32 mask = 0u; mask < 128u; ++mask) {
        sample.present_mask = mask;
        next = raf_morph_v1_t7(prior, sample);
        for (u32 lane = 0u; lane < 7u; ++lane) {
            const u32 expected = ((mask >> lane) & 1u) ?
                ((prior.q[lane] - (prior.q[lane] >> 2) +
                  ((sample.q[lane] & 65535u) >> 2)) & 65535u) :
                (prior.q[lane] & 65535u);
            CHECK(next.q[lane] == expected);
        }
        CHECK(next.present_mask == ((prior.present_mask | mask) & 127u));
    }

    /* Metadata/abstraction must not create device evidence or physical failover. */
    return (int)failures;
}
