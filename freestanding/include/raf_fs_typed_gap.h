#ifndef RAF_FS_TYPED_GAP_H
#define RAF_FS_TYPED_GAP_H
/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: represent useful missingness and evidence candidate lineage.
 * SCOPE: deterministic C11 freestanding, no provider or legal verification.
 * PRECONDITIONS: caller owns disjoint before/after records and source identifiers.
 * REGISTER_OWNERSHIP: compiler allocated; no privileged registers.
 * CLOBBERS: NONE except caller-owned destination record.
 * MEMORY_ORDER: single writer, no atomics or concurrency guarantee.
 * TAIL_SHADOW: no hidden allocation, retry, scalar tail or shadow state.
 * EVIDENCE: source+tests only; CANDIDATE never denotes VERIFIED or PASS.
 */
#include "raf_fs_types.h"
typedef enum raf_fs_empty_type {
    RAF_FS_ZERO_NUMERIC=1, RAF_FS_ZERO_RESIDUE=2,
    RAF_FS_EMPTY_SET=3, RAF_FS_NULL_VALUE=4,
    RAF_FS_TOKEN_VAZIO=5, RAF_FS_UNDEFINED=6,
    RAF_FS_NO_REAL_SOLUTION=7, RAF_FS_MISSING_OBSERVATION=8,
    RAF_FS_NOT_APPLICABLE=9, RAF_FS_APPROX_ZERO=10
} raf_fs_empty_type;
typedef enum raf_fs_gap_phase {
    RAF_FS_GAP_INVALID=0, RAF_FS_GAP_OPEN=1,
    RAF_FS_GAP_CANDIDATE=2
} raf_fs_gap_phase;
typedef struct raf_fs_gap_lifecycle {
    raf_u64 identity;
    raf_u64 evidence_ref;
    raf_u64 receipt_ref;
    raf_u32 empty_type;
    raf_u32 phase;
} raf_fs_gap_lifecycle;
static inline raf_u32 raf_fs_empty_valid(raf_u32 type) {
    return (raf_u32)(type>=1u && type<=10u);
}
static inline raf_u32 raf_fs_gap_init(raf_fs_gap_lifecycle *dst,
                                      raf_u64 identity, raf_u32 type) {
    if (!dst) return RAF_FS_GAP_INVALID;
    dst->identity=identity;
    dst->empty_type=type;
    dst->evidence_ref=0u;
    dst->receipt_ref=0u;
    dst->phase=(identity!=0u && type==(raf_u32)RAF_FS_TOKEN_VAZIO)
              ? RAF_FS_GAP_OPEN : RAF_FS_GAP_INVALID;
    return dst->phase;
}
/* Bind only a candidate with nonzero distinct opaque references.
 * The original stays unchanged. No source/provider authenticity asserted. */
static inline raf_u32 raf_fs_gap_bind(const raf_fs_gap_lifecycle *before,
                                     raf_fs_gap_lifecycle *after,
                                     raf_u64 evidence_ref,
                                     raf_u64 receipt_ref) {
    if (!before || !after || before==after || before->phase!=RAF_FS_GAP_OPEN ||
        before->empty_type!=(raf_u32)RAF_FS_TOKEN_VAZIO ||
        before->identity==0u || before->evidence_ref!=0u ||
        before->receipt_ref!=0u || evidence_ref==0u || receipt_ref==0u ||
        evidence_ref==receipt_ref)
        return RAF_FS_GAP_INVALID;
    after->identity=before->identity;
    after->empty_type=before->empty_type;
    after->evidence_ref=evidence_ref;
    after->receipt_ref=receipt_ref;
    after->phase=RAF_FS_GAP_CANDIDATE;
    return after->phase;
}
static inline raf_u32 raf_fs_gap_candidate_valid(const raf_fs_gap_lifecycle *v) {
    if (!v || v->phase!=RAF_FS_GAP_CANDIDATE ||
        v->empty_type!=(raf_u32)RAF_FS_TOKEN_VAZIO || v->identity==0u ||
        v->evidence_ref==0u || v->receipt_ref==0u ||
        v->evidence_ref==v->receipt_ref)
        return 0u;
    return 1u;
}
#endif
