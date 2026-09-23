/* Governance binding: CLOSURE_L9; application parity is not a convergence claim. */
#include "../rafaelia/t7_workflow_apply_v2.h"

static void fill_reference_input(T7Input *in) {
    in->data_hash = 0x12345678u;
    in->entropy = (q16_t)0x00001234;
    in->hw_state = 0x89ABCDEFu;
}

static void fill_projection_from_legacy(const T7Input *in,
                                        u32 present_mask,
                                        T7WorkflowProjectionV1 *p) {
    u32 raw[T7_DIM];
    t7wf_legacy_raw_coords_v1(in, raw);
    T7WorkflowEventV1 e = {{0u},present_mask,0x12345678u,1u};
    for (u32 i=0;i<T7_DIM;++i) e.q[i]=raw[i];
    t7wf_project_v1(&e,p);
}

static int test_all_present_matches_legacy(void) {
    T7Input in;
    fill_reference_input(&in);

    T7State legacy, semantic;
    t7_init(&legacy);
    t7_init(&semantic);
    t7_map_input(&legacy,&in);

    T7WorkflowProjectionV1 p = {{0u},0u,0u,0u,0u};
    fill_projection_from_legacy(&in,T7WF_PRESENT_ALL,&p);
    T7WorkflowApplyReceiptV2 r = t7wf_apply_present_v2(&semantic,&p);

    if (r.version != T7WF_APPLY_VERSION_V2) return 1;
    if (r.policy != T7WF_APPLY_POLICY_PRESENT_ONLY) return 2;
    if (r.applied_mask != T7WF_PRESENT_ALL || r.preserved_mask != 0u) return 3;
    for (u32 i=0;i<T7_DIM;++i)
        if (semantic.s[i] != legacy.s[i]) return (int)(10u+i);
    return 0;
}

static int test_absent_chi_preserves_prior_state(void) {
    T7Input in;
    fill_reference_input(&in);

    T7State semantic;
    t7_init(&semantic);
    const u32 before_chi = semantic.s[T7WF_CHI];

    T7WorkflowProjectionV1 p = {{0u},0u,0u,0u,0u};
    const u32 mask = T7WF_PRESENT_ALL & ~t7wf_bit_v1(T7WF_CHI);
    fill_projection_from_legacy(&in,mask,&p);
    T7WorkflowApplyReceiptV2 r = t7wf_apply_present_v2(&semantic,&p);

    if (r.applied_mask != mask) return 1;
    if (r.preserved_mask != t7wf_bit_v1(T7WF_CHI)) return 2;
    if (semantic.s[T7WF_CHI] != before_chi) return 3;
    return 0;
}

static int test_present_numeric_zero_is_applied(void) {
    T7Input in;
    fill_reference_input(&in);

    T7State semantic;
    t7_init(&semantic);
    const u32 before_chi = semantic.s[T7WF_CHI];

    T7WorkflowProjectionV1 p = {{0u},0u,0u,0u,0u};
    fill_projection_from_legacy(&in,T7WF_PRESENT_ALL,&p);
    if (p.q[T7WF_CHI] != 0u) return 1;
    if (!t7wf_is_present_v1(&p,T7WF_CHI)) return 2;

    T7WorkflowApplyReceiptV2 r = t7wf_apply_present_v2(&semantic,&p);
    if (!(r.applied_mask & t7wf_bit_v1(T7WF_CHI))) return 3;
    if (semantic.s[T7WF_CHI] == before_chi) return 4;
    if (semantic.s[T7WF_CHI] != 23205u) return 5;
    return 0;
}

static int test_invalid_projection_version_no_mutation(void) {
    T7State state;
    t7_init(&state);
    u32 before[T7_DIM];
    for (u32 i=0;i<T7_DIM;++i) before[i]=state.s[i];

    T7WorkflowProjectionV1 p = {{0u},T7WF_PRESENT_ALL,99u,0u,1u};
    T7WorkflowApplyReceiptV2 r = t7wf_apply_present_v2(&state,&p);
    if (r.applied_mask != 0u || r.preserved_mask != 0u) return 1;
    for (u32 i=0;i<T7_DIM;++i)
        if (state.s[i] != before[i]) return (int)(10u+i);
    return 0;
}

int main(void) {
    int r;
    r=test_all_present_matches_legacy(); if (r) return 10+r;
    r=test_absent_chi_preserves_prior_state(); if (r) return 40+r;
    r=test_present_numeric_zero_is_applied(); if (r) return 60+r;
    r=test_invalid_projection_version_no_mutation(); if (r) return 80+r;
    return 0;
}
