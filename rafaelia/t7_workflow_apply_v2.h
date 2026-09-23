/* t7_workflow_apply_v2.h — additive semantic application bridge for T^7.
 * Governance: CLOSURE_L9. TOKEN_VAZIO remains absence, never numeric zero.
 * This bridge does not replace Benchmark/raf_toroid.h::t7_map_input().
 */
#pragma once
#ifndef RAFAELIA_T7_WORKFLOW_APPLY_V2_H
#define RAFAELIA_T7_WORKFLOW_APPLY_V2_H

#include "t7_workflow_projection_v1.h"

#define T7WF_APPLY_VERSION_V2          2u
#define T7WF_APPLY_POLICY_PRESENT_ONLY 1u

typedef struct {
    u32 version;
    u32 policy;
    u32 applied_mask;
    u32 preserved_mask;
} T7WorkflowApplyReceiptV2;

/* PRESENT_ONLY policy:
 * - present coordinate: apply the same alpha=1/4 integer IIR used by t7_map_input
 * - absent coordinate: preserve prior T7State.s[i] exactly
 * - numeric zero with presence bit set is a real input and is applied
 */
static inline T7WorkflowApplyReceiptV2
t7wf_apply_present_v2(T7State *state, const T7WorkflowProjectionV1 *projection) {
    T7WorkflowApplyReceiptV2 r = {
        T7WF_APPLY_VERSION_V2,
        T7WF_APPLY_POLICY_PRESENT_ONLY,
        0u,
        0u
    };
    if (!state || !projection || projection->version != T7WF_VERSION_V1)
        return r;

    for (u32 i = 0; i < T7_DIM; ++i) {
        const u32 bit = t7wf_bit_v1(i);
        if (projection->present_mask & bit) {
            const u32 q = projection->q[i] & T7WF_COORD_MASK;
            state->s[i] =
                (state->s[i] - (state->s[i] >> 2) + (q >> 2)) & T7WF_COORD_MASK;
            r.applied_mask |= bit;
        } else {
            r.preserved_mask |= bit;
        }
    }
    return r;
}

#endif
