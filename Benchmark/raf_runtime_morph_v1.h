#ifndef RAF_RUNTIME_MORPH_V1_H
#define RAF_RUNTIME_MORPH_V1_H
/* RAFAELIA runtime morph v1 — additive, caller-owned, freestanding policy.
 * No allocation, hosted runtime, syscall, hardware probing, or hidden device I/O.
 * A backend may be selected only after all four independent gates intersect.
 * Numeric 42-slot state is NOT evidence of 42 mathematical attractors.
 */
#include "raf_types.h"

#define RAF_MORPH_V1_GENERIC 0u
#define RAF_MORPH_V1_ARM32_NEON 1u
#define RAF_MORPH_V1_ARM64_NEON 2u
#define RAF_MORPH_V1_CAP_ARM32 (1u << RAF_MORPH_V1_ARM32_NEON)
#define RAF_MORPH_V1_CAP_ARM64 (1u << RAF_MORPH_V1_ARM64_NEON)
#define RAF_MORPH_V1_ARCH_GENERIC 0u
#define RAF_MORPH_V1_ARCH_ARM32 1u
#define RAF_MORPH_V1_ARCH_ARM64 2u
#define RAF_MORPH_V1_BASELINE 0u
#define RAF_MORPH_V1_CANDIDATE 1u
#define RAF_MORPH_V1_VALIDATED 2u
#define RAF_MORPH_V1_ROLLBACK 3u
#define RAF_MORPH_V1_T7_PRESENT_ALL 127u

/* A named field is a custody/ABI necessity, not hot-path runtime allocation. */
typedef struct {
    u32 arch;
    u32 detected_caps;   /* trusted architecture probe belongs to an adapter */
    u32 compiled_caps;   /* code generation proven for THIS exact ISA */
    u32 executable_caps; /* callable implementation exists; no placeholders */
    u32 verified_caps;   /* same-revision gate; caller is accountable */
    u32 degraded_caps;   /* faults/thermal/invalidated instructions */
    u32 phase;
} RafMorphV1Profile;

typedef struct {
    u32 last_good_backend;
    u32 quarantined_caps;
    u32 generation;
} RafMorphV1Journal;

typedef struct {
    u32 backend;
    u32 fallback_backend;
    u32 eligible_caps;
    u32 fail_safe;
    u32 failover_possible;
    u32 unknown_or_blocked;
    u32 generation;
} RafMorphV1Route;

typedef struct {
    RafMorphV1Journal journal;
    u32 effective_backend;
    u32 rollback;
    u32 stale_feedback;
} RafMorphV1Feedback;

/* No conditional branch is required by the C value-selection semantics.
 * Machine branchlessness must be checked per compiler/target via disassembly.
 */
static inline u32 raf_morph_v1_select(u32 condition, u32 yes, u32 no) {
    u32 mask = 0u - (u32)(condition != 0u);
    return (yes & mask) | (no & ~mask);
}

static inline RafMorphV1Route raf_morph_v1_route(RafMorphV1Profile p, RafMorphV1Journal j) {
    const u32 eligible = p.detected_caps & p.compiled_caps & p.executable_caps &
                         p.verified_caps & ~(p.degraded_caps | j.quarantined_caps);
    const u32 arm32 = (u32)(p.arch == RAF_MORPH_V1_ARCH_ARM32) &
                      (u32)((eligible & RAF_MORPH_V1_CAP_ARM32) != 0u);
    const u32 arm64 = (u32)(p.arch == RAF_MORPH_V1_ARCH_ARM64) &
                      (u32)((eligible & RAF_MORPH_V1_CAP_ARM64) != 0u);
    const u32 live = (u32)(p.phase == RAF_MORPH_V1_CANDIDATE) |
                     (u32)(p.phase == RAF_MORPH_V1_VALIDATED);
    const u32 choice = raf_morph_v1_select(arm64, RAF_MORPH_V1_ARM64_NEON,
                        raf_morph_v1_select(arm32, RAF_MORPH_V1_ARM32_NEON,
                                            RAF_MORPH_V1_GENERIC));
    RafMorphV1Route r;
    r.backend = raf_morph_v1_select(live, choice, RAF_MORPH_V1_GENERIC);
    r.eligible_caps = eligible & (RAF_MORPH_V1_CAP_ARM32 | RAF_MORPH_V1_CAP_ARM64);
    /* Only an independent and still eligible previous backend can be standby. */
    const u32 good32 = (u32)(j.last_good_backend == RAF_MORPH_V1_ARM32_NEON) & arm32;
    const u32 good64 = (u32)(j.last_good_backend == RAF_MORPH_V1_ARM64_NEON) & arm64;
    const u32 distinct = (u32)(j.last_good_backend != r.backend);
    r.fallback_backend = raf_morph_v1_select(live & distinct & (good32 | good64),
                                            j.last_good_backend, RAF_MORPH_V1_GENERIC);
    r.fail_safe = (u32)(r.backend == RAF_MORPH_V1_GENERIC);
    r.failover_possible = (u32)(r.fallback_backend != RAF_MORPH_V1_GENERIC);
    r.unknown_or_blocked = r.fail_safe;
    r.generation = j.generation;
    return r;
}

/* result==0 is success; any nonzero value is failure. A stale receipt is ignored.
 * This journal is VOLATILE, not persistent rollback of I/O or an APK.
 */
static inline RafMorphV1Feedback raf_morph_v1_feedback(
        RafMorphV1Profile p, RafMorphV1Journal before,
        RafMorphV1Route route, s32 result) {
    /* Recompute the decision to reject forged/mismatched feedback, not just
     * replayed generations. Platform profile remains caller-owned evidence.
     */
    const RafMorphV1Route canonical = raf_morph_v1_route(p, before);
    const u32 fresh = (u32)(route.generation == before.generation) &
                      (u32)(route.backend == canonical.backend) &
                      (u32)(route.fallback_backend == canonical.fallback_backend) &
                      (u32)(route.eligible_caps == canonical.eligible_caps);
    const u32 failed = (u32)(result != 0);
    const u32 fault = fresh & failed;
    const u32 success = fresh & (failed ^ 1u);
    const u32 accelerated = (u32)(route.backend != RAF_MORPH_V1_GENERIC);
    const u32 valid_promotion = success & accelerated &
                               (u32)(p.phase == RAF_MORPH_V1_VALIDATED);
    RafMorphV1Feedback f;
    f.journal = before;
    f.journal.quarantined_caps |= (0u - fault) &
                                  ((1u << (route.backend & 31u)) &
                                   (RAF_MORPH_V1_CAP_ARM32 | RAF_MORPH_V1_CAP_ARM64));
    f.journal.last_good_backend = raf_morph_v1_select(valid_promotion,
                                                      route.backend,
                                                      before.last_good_backend);
    f.journal.generation += fresh;
    f.effective_backend = raf_morph_v1_select(
        fresh, raf_morph_v1_select(fault, route.fallback_backend, route.backend),
        RAF_MORPH_V1_GENERIC);
    f.rollback = fault;
    f.stale_feedback = fresh ^ 1u;
    return f;
}

/* T7 Q16-grid update; absent coordinates remain absent, not numerical zero.
 * Exact integer recurrence of the legacy q16 IIR (not real-number EMA).
 * Seven explicit lanes: no implicit scalar tail, allocation or loop.
 */
typedef struct {
    u32 q[7];
    u32 present_mask;
} RafMorphV1T7;

static inline u32 raf_morph_v1_t7_lane(u32 old_q, u32 new_q, u32 present) {
    const u32 old16 = old_q & 65535u;
    const u32 updated = (old16 - (old16 >> 2) + ((new_q & 65535u) >> 2)) & 65535u;
    return raf_morph_v1_select(present, updated, old16);
}

#define RAF_MORPH_V1_T7_LANE(N) \
    out.q[N] = raf_morph_v1_t7_lane(prior.q[N], sample.q[N], \
                                   (sample.present_mask >> (N)) & 1u)
static inline RafMorphV1T7 raf_morph_v1_t7(RafMorphV1T7 prior,
                                            RafMorphV1T7 sample) {
    RafMorphV1T7 out;
    RAF_MORPH_V1_T7_LANE(0);
    RAF_MORPH_V1_T7_LANE(1);
    RAF_MORPH_V1_T7_LANE(2);
    RAF_MORPH_V1_T7_LANE(3);
    RAF_MORPH_V1_T7_LANE(4);
    RAF_MORPH_V1_T7_LANE(5);
    RAF_MORPH_V1_T7_LANE(6);
    out.present_mask = (prior.present_mask | sample.present_mask) &
                       RAF_MORPH_V1_T7_PRESENT_ALL;
    return out;
}
#undef RAF_MORPH_V1_T7_LANE

#endif
