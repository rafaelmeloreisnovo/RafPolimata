/* Governance binding: CLOSURE_L9; tests do not promote T^7 convergence claims. */
#include "../rafaelia/t7_workflow_projection_v1.h"

static int test_token_vazio_not_zero(void) {
    T7WorkflowEventV1 e = {{0u,0u,0u,0u,0u,0u,0u},0u,1u,1u};
    T7WorkflowProjectionV1 p = {{0u},0u,0u,0u,0u};
    t7wf_project_v1(&e, &p);
    if (t7wf_is_present_v1(&p, T7WF_U)) return 1;
    e.present_mask = t7wf_bit_v1(T7WF_U);
    t7wf_project_v1(&e, &p);
    if (!t7wf_is_present_v1(&p, T7WF_U)) return 2;
    if (p.q[T7WF_U] != 0u) return 3;
    return 0;
}

static int test_projection_wrap_and_metadata(void) {
    T7WorkflowEventV1 e = {
        {0x10000u,1u,2u,3u,4u,5u,0x1FFFFu},
        T7WF_PRESENT_ALL,
        0x12345678u,
        7u
    };
    T7WorkflowProjectionV1 p = {{0u},0u,0u,0u,0u};
    t7wf_project_v1(&e, &p);
    if (!t7wf_is_complete_v1(&p)) return 1;
    if (p.version != T7WF_VERSION_V1) return 2;
    if (p.q[0] != 0u || p.q[6] != 0xFFFFu) return 3;
    if (p.source_ref_hash != e.source_ref_hash) return 4;
    if (p.normalization_id != e.normalization_id) return 5;
    return 0;
}

static int test_legacy_capture_matches_runtime(void) {
    T7Input in;
    in.data_hash = 0x12345678u;
    in.entropy = (q16_t)0x00001234;
    in.hw_state = 0x89ABCDEFu;

    u32 raw[T7_DIM];
    t7wf_legacy_raw_coords_v1(&in, raw);

    T7State t;
    t7_init(&t);
    u32 before[T7_DIM];
    for (u32 i=0;i<T7_DIM;++i) before[i]=t.s[i];

    t7_map_input(&t, &in);
    for (u32 i=0;i<T7_DIM;++i) {
        u32 expected = (before[i] - (before[i] >> 2) + (raw[i] >> 2)) & 0xFFFFu;
        if (t.s[i] != expected) return (int)(i + 1u);
    }

    /* Exercise the remaining canonical static paths under -Werror too.
     * This is not a convergence claim; it is compile/runtime coverage. */
    t7_step(&t, Q16_HALF, Q16_HALF);
    if (t.step != 1u) return 8;
    q16_t coherence = t7_coherence(&t);
    (void)coherence;
    return 0;
}

static int test_legacy_vs_semantic_mask(void) {
    T7Input in;
    in.data_hash = 0x12345678u;
    in.entropy = (q16_t)0x00001234;
    in.hw_state = 0x89ABCDEFu;

    u32 raw[T7_DIM];
    t7wf_legacy_raw_coords_v1(&in, raw);

    T7WorkflowEventV1 e = {{0u},T7WF_PRESENT_ALL,9u,3u};
    for (u32 i=0;i<T7_DIM;++i) e.q[i]=raw[i];

    T7WorkflowProjectionV1 p = {{0u},0u,0u,0u,0u};
    t7wf_project_v1(&e, &p);
    if (t7wf_compare_legacy_v1(&p,&in) != T7WF_PRESENT_ALL) return 1;

    e.q[T7WF_U] ^= 1u;
    t7wf_project_v1(&e, &p);
    if (t7wf_compare_legacy_v1(&p,&in) != (T7WF_PRESENT_ALL & ~t7wf_bit_v1(T7WF_U))) return 2;

    e.present_mask &= ~t7wf_bit_v1(T7WF_CHI);
    t7wf_project_v1(&e, &p);
    if (t7wf_compare_legacy_v1(&p,&in) & t7wf_bit_v1(T7WF_CHI)) return 3;
    return 0;
}

int main(void) {
    int r;
    r=test_token_vazio_not_zero(); if (r) return 10+r;
    r=test_projection_wrap_and_metadata(); if (r) return 20+r;
    r=test_legacy_capture_matches_runtime(); if (r) return 30+r;
    r=test_legacy_vs_semantic_mask(); if (r) return 40+r;
    return 0;
}
