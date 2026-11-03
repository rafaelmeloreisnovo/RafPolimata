/* Independent synthetics: no provider, runtime, physical clock or ART claim. */
#include "../include/raf_fs_guard_supervisor_v1.h"
#define CHECK(e,n) do { if (!(e)) return (n); } while (0)
int main(void) {
    raf_fs_guard_route p={11u,12u,RAF_FS_CAP_COG_Q16,
        {101u,201u,95u,RAF_FS_WD_ALIVE},
        {102u,202u,97u,RAF_FS_WD_ALIVE}};
    raf_fs_guard_route b={21u,22u,RAF_FS_CAP_COG_Q16,
        {103u,203u,94u,RAF_FS_WD_ALIVE},
        {104u,204u,98u,RAF_FS_WD_ALIVE}};
    raf_fs_guard_policy policy={100u,10u,99u,777u,RAF_FS_CAP_COG_Q16,1u,1u};
    raf_fs_guard_decision out={888u,889u,890u,891u};
    raf_u32 i,status;
    CHECK(RAF_FS_GUARD_TOKEN_VAZIO!=RAF_FS_GUARD_INVALID,1);
    CHECK(RAF_FS_GUARD_FAILOVER!=RAF_FS_GUARD_PRIMARY,2);
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_PRIMARY,3);
    CHECK(out.source_ref==11u && out.artifact_ref==12u && out.authorization_ref==0u,4);
    p.watchdog_a.verdict=RAF_FS_WD_FAULT;
    out.source_ref=888u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_SAFE_STOP,5);
    CHECK(out.source_ref==888u,6);
    p.watchdog_b.verdict=RAF_FS_WD_FAULT;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_FAILOVER,7);
    CHECK(out.source_ref==21u && out.artifact_ref==22u && out.authorization_ref==777u,8);
    policy.allow_failover=0u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_ROLLBACK_READY,9);
    CHECK(out.artifact_ref==99u && out.route_kind==RAF_FS_GUARD_ROLLBACK_READY,10);
    policy.allow_rollback=0u;
    out.artifact_ref=889u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_SAFE_STOP,11);
    CHECK(out.artifact_ref==889u,12);
    policy.allow_failover=1u; policy.allow_rollback=1u;
    b.watchdog_a.verdict=RAF_FS_WD_FAULT;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_SAFE_STOP,13);
    b.watchdog_b.verdict=RAF_FS_WD_FAULT;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_ROLLBACK_READY,14);
    b.watchdog_a.verdict=RAF_FS_WD_ALIVE; b.watchdog_b.verdict=RAF_FS_WD_ALIVE;
    b.watchdog_a.heartbeat_tick=80u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_TOKEN_VAZIO,15);
    b.watchdog_a.heartbeat_tick=94u;
    b.watchdog_a.observer_ref=p.watchdog_a.observer_ref;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,16);
    b.watchdog_a.observer_ref=103u;
    b.source_ref=p.source_ref;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,17);
    b.source_ref=21u;
    b.artifact_ref=p.artifact_ref;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,18);
    b.artifact_ref=22u;
    b.declared_capabilities=RAF_FS_CAP_ELF_STRUCT;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_SAFE_STOP,19);
    b.declared_capabilities=RAF_FS_CAP_COG_Q16;
    p.watchdog_a.verdict=RAF_FS_WD_UNKNOWN;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_TOKEN_VAZIO,20);
    p.watchdog_a.verdict=RAF_FS_WD_ALIVE;
    p.watchdog_b.verdict=RAF_FS_WD_ALIVE;
    p.watchdog_a.heartbeat_tick=101u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_TOKEN_VAZIO,21);
    p.watchdog_a.heartbeat_tick=95u;
    p.watchdog_b.heartbeat_tick=89u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_TOKEN_VAZIO,22);
    p.watchdog_b.heartbeat_tick=97u;
    p.watchdog_a.observer_ref=p.watchdog_b.observer_ref;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,23);
    p.watchdog_a.observer_ref=101u;
    p.watchdog_a.evidence_ref=p.watchdog_b.evidence_ref;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,24);
    p.watchdog_a.evidence_ref=201u;
    p.declared_capabilities=0u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_SAFE_STOP,25);
    p.declared_capabilities=RAF_FS_CAP_COG_Q16;
    policy.required_capabilities=0u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,26);
    policy.required_capabilities=RAF_FS_CAP_COG_Q16;
    policy.max_age_ticks=0u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,27);
    policy.max_age_ticks=10u;
    policy.allow_failover=2u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,28);
    policy.allow_failover=1u;
    CHECK(raf_fs_guard_select((const raf_fs_guard_route*)0,&b,&policy,&out)==RAF_FS_GUARD_INVALID,29);
    CHECK(raf_fs_guard_select(&p,&b,(const raf_fs_guard_policy*)0,&out)==RAF_FS_GUARD_INVALID,30);
    CHECK(raf_fs_guard_select(&p,&b,&policy,(raf_fs_guard_decision*)0)==RAF_FS_GUARD_INVALID,31);
    policy.now_tick=~(raf_u64)0u;
    p.watchdog_a.heartbeat_tick=policy.now_tick-2u;
    p.watchdog_b.heartbeat_tick=policy.now_tick-1u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_PRIMARY,32);
    policy.now_tick=100u;
    p.watchdog_a.heartbeat_tick=95u;
    p.watchdog_b.heartbeat_tick=97u;
    p.watchdog_a.verdict=RAF_FS_WD_FAULT;
    p.watchdog_b.verdict=RAF_FS_WD_FAULT;
    policy.rollback_artifact_ref=12u;
    policy.allow_failover=0u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,33);
    policy.rollback_artifact_ref=99u;
    policy.failover_authorization_ref=0u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_ROLLBACK_READY,34);
    policy.allow_failover=1u;
    CHECK(raf_fs_guard_select(&p,(const raf_fs_guard_route*)0,&policy,&out)==RAF_FS_GUARD_ROLLBACK_READY,35);
    policy.failover_authorization_ref=777u;
    CHECK(raf_fs_guard_select(&p,(const raf_fs_guard_route*)0,&policy,&out)==RAF_FS_GUARD_TOKEN_VAZIO,36);
    /* Capability flags are an admission mask, never physical/ART proof. */
    p.watchdog_a.verdict=RAF_FS_WD_ALIVE;
    p.watchdog_b.verdict=RAF_FS_WD_ALIVE;
    p.declared_capabilities=RAF_FS_CAP_ELF_STRUCT|RAF_FS_CAP_DEX_STRUCT|
                            RAF_FS_CAP_NUMA_ADVISORY;
    policy.required_capabilities=p.declared_capabilities;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_PRIMARY,40);
    policy.required_capabilities|=RAF_FS_CAP_ART_HOST_BRIDGE;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_SAFE_STOP,41);
    p.declared_capabilities|=RAF_FS_CAP_ART_HOST_BRIDGE;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_PRIMARY,42);
    policy.required_capabilities=0x20u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,43);
    policy.required_capabilities=RAF_FS_CAP_COG_Q16;
    p.declared_capabilities=RAF_FS_CAP_COG_Q16;
    policy.allow_rollback=2u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,44);
    policy.allow_rollback=1u;
    p.watchdog_a.observer_ref=0u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_TOKEN_VAZIO,45);
    p.watchdog_a.observer_ref=101u;
    p.watchdog_a.verdict=3u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_INVALID,46);
    p.watchdog_a.verdict=RAF_FS_WD_ALIVE;
    p.watchdog_b.evidence_ref=0u;
    CHECK(raf_fs_guard_select(&p,&b,&policy,&out)==RAF_FS_GUARD_TOKEN_VAZIO,47);
    p.watchdog_b.evidence_ref=202u;
    /* Falsifier sweep: independent pair consensus; no accidental promotion. */
    p.watchdog_a.verdict=RAF_FS_WD_ALIVE;
    p.watchdog_b.verdict=RAF_FS_WD_ALIVE;
    policy.failover_authorization_ref=777u;
    for (i=0u;i<256u;++i) {
        p.watchdog_a.verdict=(i&1u)?RAF_FS_WD_FAULT:RAF_FS_WD_ALIVE;
        p.watchdog_b.verdict=(i&2u)?RAF_FS_WD_FAULT:RAF_FS_WD_ALIVE;
        out.route_kind=0xFFFu;
        status=raf_fs_guard_select(&p,&b,&policy,&out);
        if ((i&3u)==1u || (i&3u)==2u) {
            CHECK(status==RAF_FS_GUARD_SAFE_STOP && out.route_kind==0xFFFu,37);
        } else if ((i&3u)==0u) {
            CHECK(status==RAF_FS_GUARD_PRIMARY,38);
        } else {
            CHECK(status==RAF_FS_GUARD_FAILOVER,39);
        }
    }
    return 0;
}
