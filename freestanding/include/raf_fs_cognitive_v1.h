#ifndef RAF_FS_COGNITIVE_V1_H
#define RAF_FS_COGNITIVE_V1_H
/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: bounded, deterministic cognitive-state candidate for 10x10x10 cells.
 * SCOPE: authorial C11 L0 projection only; no cognition, AI training, provider or network.
 * PRECONDITIONS: raf_u32 is 32-bit; distinct caller-owned before/after state;
 *                fixed Q16 values <= 65536 and ordered observations.
 * REGISTER_OWNERSHIP: compiler allocated; no privileged registers.
 * CLOBBERS: caller-owned output on accepted transition only.
 * MEMORY_ORDER: single-writer; no atomicity or concurrent provider semantics.
 * TAIL_SHADOW: no hidden tail, heap, retry or shadow; after is explicit transaction candidate.
 * EVIDENCE: source and deterministic tests; build, device and provider claims separate.
 * RIGHTS: authorial candidate (Rafael Melo Reis); root redistribution license TOKEN_VAZIO.
 */
#include "raf_fs_types.h"

#define RAF_FS_COG_CELLS 1000u
#define RAF_FS_COG_Q16_ONE 65536u
#define RAF_FS_COG_L9_MASK 511u

/* CANDIDATE means local transition only, never evidence authenticity or PASS. */
typedef enum raf_fs_cog_status {
    RAF_FS_COG_INVALID=0,
    RAF_FS_COG_TOKEN_VAZIO=1,
    RAF_FS_COG_CANDIDATE=2,
    RAF_FS_COG_EXHAUSTED=3
} raf_fs_cog_status;

typedef struct raf_fs_cog_observation {
    raf_u32 slot;
    raf_u32 observed; /* 0=missing, 1=numeric value (including zero). */
    raf_u32 value_q16;
    raf_u32 l9_mask; /* explicit route dimensions; never an inferred relation. */
    raf_u64 source_ref;
    raf_u64 evidence_ref;
    raf_u64 receipt_ref;
} raf_fs_cog_observation;

typedef struct raf_fs_cog_state {
    raf_u32 next_slot;
    raf_u32 count;
    raf_u32 has_sample;
    raf_u32 ema_q16;
    raf_u32 last_slot;
    raf_u32 last_l9_mask;
    raf_u64 last_source_ref;
    raf_u64 last_evidence_ref;
    raf_u64 last_receipt_ref;
} raf_fs_cog_state;

/* Canonical bijection k=x+10*y+100*z, x/y/z in [0,9]. */
static inline raf_u32 raf_fs_cog_pack(raf_u32 x, raf_u32 y, raf_u32 z,
                                       raf_u32 *result) {
    if (!result || x>=10u || y>=10u || z>=10u) return RAF_FS_COG_INVALID;
    *result=x+10u*y+100u*z;
    return RAF_FS_COG_CANDIDATE;
}

static inline raf_u32 raf_fs_cog_unpack(raf_u32 slot, raf_u32 *x,
                                         raf_u32 *y, raf_u32 *z) {
    if (slot>=RAF_FS_COG_CELLS || !x || !y || !z || x==y || y==z || x==z)
        return RAF_FS_COG_INVALID;
    *x=slot%10u;
    *y=(slot/10u)%10u;
    *z=slot/100u;
    return RAF_FS_COG_CANDIDATE;
}

/* Initializes a caller-owned state; has_sample=0 != numeric zero. */
static inline raf_u32 raf_fs_cog_init(raf_fs_cog_state *dst) {
    if (!dst) return RAF_FS_COG_INVALID;
    dst->next_slot=0u;
    dst->count=0u;
    dst->has_sample=0u;
    dst->ema_q16=0u;
    dst->last_slot=RAF_FS_COG_CELLS;
    dst->last_l9_mask=0u;
    dst->last_source_ref=0u;
    dst->last_evidence_ref=0u;
    dst->last_receipt_ref=0u;
    return RAF_FS_COG_CANDIDATE;
}

/* Strict ordered transition. No output mutation for any rejected/missing input.
 * Numeric mapping: first sample preserved; subsequent EMA=(3*old+sample+2)/4.
 * This is a falsifiable arithmetic model, not self-modifying intelligence.
 */
static inline raf_u32 raf_fs_cog_step(const raf_fs_cog_state *before,
                                      const raf_fs_cog_observation *obs,
                                      raf_fs_cog_state *after) {
    if (!before || !obs || !after || before==after ||
        before->next_slot>=RAF_FS_COG_CELLS ||
        before->has_sample>1u || before->ema_q16>RAF_FS_COG_Q16_ONE ||
        (before->has_sample==0u && before->count!=0u) ||
        (before->has_sample!=0u && before->count==0u) ||
        obs->slot!=before->next_slot || obs->observed>1u ||
        obs->value_q16>RAF_FS_COG_Q16_ONE ||
        (obs->l9_mask & ~RAF_FS_COG_L9_MASK)!=0u)
        return RAF_FS_COG_INVALID;
    if (!obs->observed || !obs->l9_mask || !obs->source_ref ||
        !obs->evidence_ref || !obs->receipt_ref)
        return RAF_FS_COG_TOKEN_VAZIO;
    if (obs->source_ref==obs->evidence_ref ||
        obs->source_ref==obs->receipt_ref ||
        obs->evidence_ref==obs->receipt_ref)
        return RAF_FS_COG_INVALID;
    if (before->count==0xffffffffu) return RAF_FS_COG_EXHAUSTED;
    after->next_slot=(before->next_slot==999u)?0u:(before->next_slot+1u);
    after->count=before->count+1u;
    after->has_sample=1u;
    after->ema_q16=before->has_sample
        ? (3u*before->ema_q16+obs->value_q16+2u)/4u
        : obs->value_q16;
    after->last_slot=obs->slot;
    after->last_l9_mask=obs->l9_mask;
    after->last_source_ref=obs->source_ref;
    after->last_evidence_ref=obs->evidence_ref;
    after->last_receipt_ref=obs->receipt_ref;
    return RAF_FS_COG_CANDIDATE;
}
#endif
