/* Host arithmetic oracle; cross-target compilation is NOT hardware execution. */
#include "../include/raf_fs_cognitive_v1.h"
#define CHECK(p,n) do { if (!(p)) return (n); } while (0)
int main(void) {
    raf_fs_cog_state before, after, keep, tmp;
    raf_fs_cog_observation obs;
    raf_u32 k,x,y,z;
    CHECK(RAF_FS_COG_TOKEN_VAZIO!=RAF_FS_COG_INVALID,1);
    CHECK(RAF_FS_COG_TOKEN_VAZIO!=RAF_FS_COG_CANDIDATE,2);
    CHECK(raf_fs_cog_init(&before)==RAF_FS_COG_CANDIDATE,3);
    CHECK(before.count==0u && before.has_sample==0u && before.ema_q16==0u,4);
    CHECK(raf_fs_cog_init((raf_fs_cog_state *)0)==RAF_FS_COG_INVALID,5);
    for (k=0u;k<1000u;++k) {
        CHECK(raf_fs_cog_unpack(k,&x,&y,&z)==RAF_FS_COG_CANDIDATE,6);
        CHECK(raf_fs_cog_pack(x,y,z,&x)==RAF_FS_COG_CANDIDATE && x==k,7);
    }
    CHECK(raf_fs_cog_unpack(1000u,&x,&y,&z)==RAF_FS_COG_INVALID,8);
    CHECK(raf_fs_cog_unpack(0u,&x,&x,&z)==RAF_FS_COG_INVALID,9);
    CHECK(raf_fs_cog_pack(10u,0u,0u,&k)==RAF_FS_COG_INVALID,10);
    CHECK(raf_fs_cog_pack(0u,10u,0u,&k)==RAF_FS_COG_INVALID,11);
    CHECK(raf_fs_cog_pack(0u,0u,10u,&k)==RAF_FS_COG_INVALID,12);
    obs.slot=0u; obs.observed=0u; obs.value_q16=0u;
    obs.l9_mask=1u; obs.source_ref=10u; obs.evidence_ref=11u; obs.receipt_ref=12u;
    after.count=777u;
    CHECK(raf_fs_cog_step(&before,&obs,&after)==RAF_FS_COG_TOKEN_VAZIO,13);
    CHECK(after.count==777u && before.count==0u,14);
    obs.observed=1u;
    CHECK(raf_fs_cog_step(&before,&obs,&after)==RAF_FS_COG_CANDIDATE,15);
    CHECK(after.has_sample==1u && after.ema_q16==0u && after.count==1u,16);
    CHECK(after.next_slot==1u && after.last_slot==0u,17);
    CHECK(before.count==0u && before.has_sample==0u,18);
    keep=after; obs.slot=1u; obs.value_q16=RAF_FS_COG_Q16_ONE;
    CHECK(raf_fs_cog_step(&keep,&obs,&after)==RAF_FS_COG_CANDIDATE,19);
    CHECK(after.ema_q16==16384u && after.count==2u,20);
    tmp=after;
    CHECK(raf_fs_cog_step(&keep,&obs,&tmp)==RAF_FS_COG_CANDIDATE,21);
    CHECK(tmp.ema_q16==after.ema_q16 && tmp.last_receipt_ref==after.last_receipt_ref,22);
    obs.observed=0u; obs.slot=2u; keep=after; after.count=888u;
    CHECK(raf_fs_cog_step(&keep,&obs,&after)==RAF_FS_COG_TOKEN_VAZIO && after.count==888u,23);
    obs.observed=1u; obs.slot=2u; obs.source_ref=0u;
    CHECK(raf_fs_cog_step(&keep,&obs,&after)==RAF_FS_COG_TOKEN_VAZIO,24);
    obs.source_ref=10u; obs.receipt_ref=10u;
    CHECK(raf_fs_cog_step(&keep,&obs,&after)==RAF_FS_COG_INVALID,25);
    obs.receipt_ref=12u; obs.l9_mask=0u;
    CHECK(raf_fs_cog_step(&keep,&obs,&after)==RAF_FS_COG_TOKEN_VAZIO,26);
    obs.l9_mask=512u;
    CHECK(raf_fs_cog_step(&keep,&obs,&after)==RAF_FS_COG_INVALID,27);
    obs.l9_mask=511u; obs.value_q16=65537u;
    CHECK(raf_fs_cog_step(&keep,&obs,&after)==RAF_FS_COG_INVALID,28);
    obs.value_q16=0u; obs.slot=4u;
    CHECK(raf_fs_cog_step(&keep,&obs,&after)==RAF_FS_COG_INVALID,29);
    obs.slot=2u;
    CHECK(raf_fs_cog_step(&keep,&obs,&keep)==RAF_FS_COG_INVALID,30);
    CHECK(raf_fs_cog_step((const raf_fs_cog_state *)0,&obs,&after)==RAF_FS_COG_INVALID,31);
    CHECK(raf_fs_cog_step(&keep,(const raf_fs_cog_observation *)0,&after)==RAF_FS_COG_INVALID,32);
    CHECK(raf_fs_cog_step(&keep,&obs,(raf_fs_cog_state *)0)==RAF_FS_COG_INVALID,33);
    keep.count=0xffffffffu;
    CHECK(raf_fs_cog_step(&keep,&obs,&after)==RAF_FS_COG_EXHAUSTED,34);
    CHECK(after.count==888u,35);
    raf_fs_cog_init(&before);
    for (k=0u;k<1000u;++k) {
        obs.slot=k; obs.value_q16=0u; obs.l9_mask=1u;
        CHECK(raf_fs_cog_step(&before,&obs,&after)==RAF_FS_COG_CANDIDATE,36);
        before=after;
    }
    CHECK(before.next_slot==0u && before.count==1000u && before.last_slot==999u,37);
    return 0;
}
