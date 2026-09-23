/* t7_workflow_projection_v1.h — versioned semantic projection into T^7.
 *
 * Governance: CLOSURE_L9 (T^7 proof/unknown-state claims remain fail-closed).
 * Contract:
 *   TOKEN_VAZIO != numeric zero.
 * Absence is represented by present_mask, never by a numeric sentinel.
 * q[i] is only semantically valid when bit i is set.
 *
 * This adapter DOES NOT mutate T7State and DOES NOT replace t7_map_input().
 * It gives the workflow layer a typed seven-axis representation while the
 * legacy numeric mapping remains available for compatibility comparison.
 */
#pragma once
#ifndef RAFAELIA_T7_WORKFLOW_PROJECTION_V1_H
#define RAFAELIA_T7_WORKFLOW_PROJECTION_V1_H

#include "../Benchmark/raf_toroid.h"

#define T7WF_VERSION_V1       1u
#define T7WF_PRESENT_ALL      0x7Fu
#define T7WF_COORD_MASK       0xFFFFu

#define T7WF_U      0u
#define T7WF_V      1u
#define T7WF_PSI    2u
#define T7WF_CHI    3u
#define T7WF_RHO    4u
#define T7WF_DELTA  5u
#define T7WF_SIGMA  6u

typedef struct {
    u32 q[T7_DIM];
    u32 present_mask;       /* low 7 bits; bit unset = TOKEN_VAZIO */
    u32 source_ref_hash;    /* provenance pointer digest fragment, not ownership proof */
    u32 normalization_id;   /* versioned normalization rule identifier */
} T7WorkflowEventV1;

typedef struct {
    u32 q[T7_DIM];
    u32 present_mask;
    u32 version;
    u32 source_ref_hash;
    u32 normalization_id;
} T7WorkflowProjectionV1;

static inline u32 t7wf_bit_v1(u32 index) {
    return (index < T7_DIM) ? (1u << index) : 0u;
}

static inline int t7wf_is_present_v1(const T7WorkflowProjectionV1 *p, u32 index) {
    if (!p || index >= T7_DIM) return 0;
    return (p->present_mask & t7wf_bit_v1(index)) != 0u;
}

static inline int t7wf_is_complete_v1(const T7WorkflowProjectionV1 *p) {
    return p && ((p->present_mask & T7WF_PRESENT_ALL) == T7WF_PRESENT_ALL);
}

/* Pi_wf^V1: event -> typed semantic point on the Q16 grid of T^7.
 * Values wrap modulo 65536. Missing values remain missing through present_mask.
 */
static inline void t7wf_project_v1(const T7WorkflowEventV1 *event,
                                   T7WorkflowProjectionV1 *out) {
    if (!event || !out) return;
    for (u32 i = 0; i < T7_DIM; ++i)
        out->q[i] = event->q[i] & T7WF_COORD_MASK;
    out->present_mask = event->present_mask & T7WF_PRESENT_ALL;
    out->version = T7WF_VERSION_V1;
    out->source_ref_hash = event->source_ref_hash;
    out->normalization_id = event->normalization_id;
}

/* Exact raw coordinate extraction used by the current legacy t7_map_input().
 * This exists only for regression comparison; it does not assert semantics.
 */
static inline void t7wf_legacy_raw_coords_v1(const T7Input *x,
                                              u32 out[T7_DIM]) {
    if (!x || !out) return;
    const u32 h = x->data_hash;
    out[0] = h & 0xFFFFu;
    out[1] = (h >> 16) & 0xFFFFu;
    out[2] = ((u32)x->entropy) & 0xFFFFu;
    out[3] = ((u32)x->entropy >> 16) & 0xFFFFu;
    out[4] = x->hw_state & 0xFFFFu;
    out[5] = (x->hw_state >> 8) & 0xFFFFu;
    out[6] = (h ^ x->hw_state) & 0xFFFFu;
}

/* Returns a 7-bit mask of semantic coordinates that are present AND numerically
 * equal to the current legacy raw mapping. Absence never compares equal.
 */
static inline u32 t7wf_compare_legacy_v1(const T7WorkflowProjectionV1 *semantic,
                                         const T7Input *legacy_input) {
    if (!semantic || !legacy_input) return 0u;
    u32 legacy[T7_DIM];
    u32 equal_mask = 0u;
    t7wf_legacy_raw_coords_v1(legacy_input, legacy);
    for (u32 i = 0; i < T7_DIM; ++i) {
        const u32 bit = t7wf_bit_v1(i);
        if ((semantic->present_mask & bit) &&
            semantic->q[i] == legacy[i])
            equal_mask |= bit;
    }
    return equal_mask;
}

#endif
