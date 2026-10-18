/* CLOSURE_L11: test the type separation; unresolved sources stay unresolved. */
/* Host test checks candidate semantics only; no physical runtime claim. */
#include "../include/raf_fs_typed_gap.h"
#define CHECK(e,n) do { if (!(e)) return n; } while (0)
int main(void) {
    raf_fs_gap_lifecycle a, b;
    CHECK(RAF_FS_ZERO_NUMERIC!=RAF_FS_TOKEN_VAZIO,1);
    CHECK(RAF_FS_ZERO_RESIDUE!=RAF_FS_ZERO_NUMERIC,2);
    CHECK(RAF_FS_EMPTY_SET!=RAF_FS_NULL_VALUE,3);
    CHECK(RAF_FS_UNDEFINED!=RAF_FS_NO_REAL_SOLUTION,4);
    CHECK(RAF_FS_MISSING_OBSERVATION!=RAF_FS_NOT_APPLICABLE,5);
    CHECK(RAF_FS_APPROX_ZERO!=RAF_FS_ZERO_NUMERIC,6);
    for (raf_u32 i=1u;i<=10u;++i) CHECK(raf_fs_empty_valid(i),7);
    CHECK(!raf_fs_empty_valid(0u),8);
    CHECK(!raf_fs_empty_valid(11u),9);
    CHECK(!raf_fs_empty_valid(0xffffffffu),10);
    CHECK(raf_fs_gap_init(&a,7u,RAF_FS_TOKEN_VAZIO)==RAF_FS_GAP_OPEN,11);
    CHECK(a.evidence_ref==0u && a.receipt_ref==0u,12);
    CHECK(raf_fs_gap_bind(&a,&b,13u,17u)==RAF_FS_GAP_CANDIDATE,13);
    CHECK(raf_fs_gap_candidate_valid(&b),14);
    CHECK(a.phase==RAF_FS_GAP_OPEN && a.receipt_ref==0u,15);
    CHECK(b.identity==7u && b.empty_type==RAF_FS_TOKEN_VAZIO,16);
    CHECK(raf_fs_gap_bind(&a,&a,13u,17u)==RAF_FS_GAP_INVALID,17);
    CHECK(raf_fs_gap_bind(&a,&b,0u,17u)==RAF_FS_GAP_INVALID,18);
    CHECK(raf_fs_gap_bind(&a,&b,13u,0u)==RAF_FS_GAP_INVALID,19);
    CHECK(raf_fs_gap_bind(&a,&b,13u,13u)==RAF_FS_GAP_INVALID,20);
    CHECK(raf_fs_gap_bind(&a,&b,13u,17u)==RAF_FS_GAP_CANDIDATE,21);
    CHECK(raf_fs_gap_bind(&b,&a,19u,23u)==RAF_FS_GAP_INVALID,22);
    b.evidence_ref=0u; CHECK(!raf_fs_gap_candidate_valid(&b),23);
    b.evidence_ref=17u; CHECK(!raf_fs_gap_candidate_valid(&b),24);
    b.evidence_ref=13u; b.receipt_ref=0u; CHECK(!raf_fs_gap_candidate_valid(&b),25);
    b.receipt_ref=17u; b.phase=RAF_FS_GAP_OPEN;
    CHECK(!raf_fs_gap_candidate_valid(&b),26);
    b.phase=RAF_FS_GAP_CANDIDATE; b.empty_type=RAF_FS_ZERO_NUMERIC;
    CHECK(!raf_fs_gap_candidate_valid(&b),27);
    CHECK(raf_fs_gap_init(&a,7u,RAF_FS_ZERO_NUMERIC)==RAF_FS_GAP_INVALID,28);
    CHECK(raf_fs_gap_init(&a,0u,RAF_FS_TOKEN_VAZIO)==RAF_FS_GAP_INVALID,29);
    CHECK(raf_fs_gap_init((raf_fs_gap_lifecycle *)0,9u,RAF_FS_TOKEN_VAZIO)==RAF_FS_GAP_INVALID,30);
    CHECK(raf_fs_gap_bind((const raf_fs_gap_lifecycle *)0,&b,1u,2u)==RAF_FS_GAP_INVALID,31);
    CHECK(raf_fs_gap_bind(&a,(raf_fs_gap_lifecycle *)0,1u,2u)==RAF_FS_GAP_INVALID,32);
    CHECK(!raf_fs_gap_candidate_valid((const raf_fs_gap_lifecycle *)0),33);
    return 0;
}
