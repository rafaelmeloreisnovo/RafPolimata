#ifndef RAF_FS_GUARD_SUPERVISOR_V1_H
#define RAF_FS_GUARD_SUPERVISOR_V1_H
/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: independent dual-watchdog, fail-closed selection and rollback intent.
 * SCOPE: pure deterministic authorial L0; no clock, OS, ELF loader, DEX/ART VM,
 *        NUMA placement, provider, filesystem, heap, or network behavior.
 * PRECONDITIONS: caller supplies verified identities/observations/monotonic ticks,
 *                a disjoint decision output and explicit reversible policy.
 * REGISTER_OWNERSHIP: compiler-allocated scalar registers; no privileged access.
 * CLOBBERS: candidate decision on authorized selection only.
 * MEMORY_ORDER: one caller; no atomics, synchronization or concurrent publication.
 * TAIL_SHADOW: no hidden retry, fallback, tail, allocation or shadow state.
 * EVIDENCE: local mathematical policy and negative tests only; witness refs are
 *           opaque and do not authenticate source, independence, ART or hardware.
 * RIGHTS: authorial bounded supplement; root license remains TOKEN_VAZIO.
 */
#include "raf_fs_types.h"

#define RAF_FS_GUARD_INVALID 0xA0u
#define RAF_FS_GUARD_TOKEN_VAZIO 0xA1u
#define RAF_FS_GUARD_SAFE_STOP 0xA2u
#define RAF_FS_GUARD_PRIMARY 0xA3u
#define RAF_FS_GUARD_FAILOVER 0xA4u
#define RAF_FS_GUARD_ROLLBACK_READY 0xA5u

#define RAF_FS_WD_UNKNOWN 0u
#define RAF_FS_WD_ALIVE 1u
#define RAF_FS_WD_FAULT 2u
#define RAF_FS_WD_CONFLICT 0xA6u

/* These are declared capabilities, not independently verified runtime claims. */
#define RAF_FS_CAP_COG_Q16 0x01u
#define RAF_FS_CAP_ELF_STRUCT 0x02u
#define RAF_FS_CAP_DEX_STRUCT 0x04u
#define RAF_FS_CAP_NUMA_ADVISORY 0x08u
#define RAF_FS_CAP_ART_HOST_BRIDGE 0x10u
#define RAF_FS_CAP_DEFINED_MASK 0x1Fu

typedef struct raf_fs_guard_watch {
    raf_u64 observer_ref;
    raf_u64 evidence_ref;
    raf_u64 heartbeat_tick;
    raf_u32 verdict;
} raf_fs_guard_watch;

typedef struct raf_fs_guard_route {
    raf_u64 source_ref;
    raf_u64 artifact_ref;
    raf_u32 declared_capabilities;
    raf_fs_guard_watch watchdog_a;
    raf_fs_guard_watch watchdog_b;
} raf_fs_guard_route;

typedef struct raf_fs_guard_policy {
    raf_u64 now_tick;
    raf_u64 max_age_ticks;
    raf_u64 rollback_artifact_ref;
    raf_u64 failover_authorization_ref;
    raf_u32 required_capabilities;
    raf_u32 allow_failover;
    raf_u32 allow_rollback;
} raf_fs_guard_policy;

typedef struct raf_fs_guard_decision {
    raf_u64 source_ref;
    raf_u64 artifact_ref;
    raf_u64 authorization_ref;
    raf_u32 route_kind;
} raf_fs_guard_decision;

/* A pair is valid only with two separately identified and fresh observations.
 * Distinct identifiers are necessary, not sufficient, for observer independence.
 * Unknown/stale means TOKEN_VAZIO, never a measured fault.
 */
static inline __attribute__((always_inline)) raf_u32
raf_fs_guard_pair(const raf_fs_guard_route *route,
                  const raf_fs_guard_policy *policy)
{
    const raf_fs_guard_watch *a;
    const raf_fs_guard_watch *b;
    if (!route || !policy || !policy->max_age_ticks)
        return RAF_FS_GUARD_INVALID;
    a=&route->watchdog_a;
    b=&route->watchdog_b;
    if (a->verdict>RAF_FS_WD_FAULT || b->verdict>RAF_FS_WD_FAULT)
        return RAF_FS_GUARD_INVALID;
    if (!a->observer_ref || !b->observer_ref || !a->evidence_ref ||
        !b->evidence_ref || a->verdict==RAF_FS_WD_UNKNOWN ||
        b->verdict==RAF_FS_WD_UNKNOWN)
        return RAF_FS_GUARD_TOKEN_VAZIO;
    if (a->observer_ref==b->observer_ref ||
        a->evidence_ref==b->evidence_ref ||
        a->observer_ref==a->evidence_ref ||
        b->observer_ref==b->evidence_ref ||
        a->observer_ref==b->evidence_ref ||
        b->observer_ref==a->evidence_ref)
        return RAF_FS_GUARD_INVALID;
    if (a->heartbeat_tick>policy->now_tick ||
        b->heartbeat_tick>policy->now_tick ||
        (policy->now_tick-a->heartbeat_tick)>policy->max_age_ticks ||
        (policy->now_tick-b->heartbeat_tick)>policy->max_age_ticks)
        return RAF_FS_GUARD_TOKEN_VAZIO;
    if (a->verdict!=b->verdict) return RAF_FS_WD_CONFLICT;
    return a->verdict;
}

/* This selects an authorized *intent*. It executes no artifact, provider call,
 * heartbeat, failover, watchdog, rollback, ELF/DEX/ART or NUMA action.
 * Returns TOKEN_VAZIO/SAFE_STOP without touching the caller-owned decision.
 */
static inline __attribute__((always_inline)) raf_u32
raf_fs_guard_select(const raf_fs_guard_route *primary,
                    const raf_fs_guard_route *secondary,
                    const raf_fs_guard_policy *policy,
                    raf_fs_guard_decision *out)
{
    raf_u32 p, b;
    if (!primary || !policy || !out || !policy->max_age_ticks ||
        policy->allow_failover>1u || policy->allow_rollback>1u ||
        !policy->required_capabilities ||
        (policy->required_capabilities & ~RAF_FS_CAP_DEFINED_MASK)!=0u)
        return RAF_FS_GUARD_INVALID;
    if (!primary->source_ref || !primary->artifact_ref)
        return RAF_FS_GUARD_TOKEN_VAZIO;
    if (primary->source_ref==primary->artifact_ref ||
        (primary->declared_capabilities & ~RAF_FS_CAP_DEFINED_MASK)!=0u)
        return RAF_FS_GUARD_INVALID;
    p=raf_fs_guard_pair(primary,policy);
    if (p==RAF_FS_GUARD_INVALID || p==RAF_FS_GUARD_TOKEN_VAZIO)
        return p;
    if (p==RAF_FS_WD_CONFLICT) return RAF_FS_GUARD_SAFE_STOP;
    if (p==RAF_FS_WD_ALIVE) {
        if ((primary->declared_capabilities & policy->required_capabilities)
            != policy->required_capabilities) return RAF_FS_GUARD_SAFE_STOP;
        out->source_ref=primary->source_ref;
        out->artifact_ref=primary->artifact_ref;
        out->authorization_ref=0u;
        out->route_kind=RAF_FS_GUARD_PRIMARY;
        return RAF_FS_GUARD_PRIMARY;
    }
    /* Failover requires two independent current witnesses of PRIMARY FAULT. */
    if (p!=RAF_FS_WD_FAULT) return RAF_FS_GUARD_INVALID;
    if (policy->allow_failover && policy->failover_authorization_ref) {
        if (!secondary || !secondary->source_ref || !secondary->artifact_ref)
            return RAF_FS_GUARD_TOKEN_VAZIO;
        if (secondary->source_ref==secondary->artifact_ref ||
            secondary->source_ref==primary->source_ref ||
            secondary->artifact_ref==primary->artifact_ref ||
            (secondary->declared_capabilities & ~RAF_FS_CAP_DEFINED_MASK)!=0u)
            return RAF_FS_GUARD_INVALID;
        if (primary->watchdog_a.observer_ref==secondary->watchdog_a.observer_ref ||
            primary->watchdog_a.observer_ref==secondary->watchdog_b.observer_ref ||
            primary->watchdog_b.observer_ref==secondary->watchdog_a.observer_ref ||
            primary->watchdog_b.observer_ref==secondary->watchdog_b.observer_ref ||
            primary->watchdog_a.evidence_ref==secondary->watchdog_a.evidence_ref ||
            primary->watchdog_a.evidence_ref==secondary->watchdog_b.evidence_ref ||
            primary->watchdog_b.evidence_ref==secondary->watchdog_a.evidence_ref ||
            primary->watchdog_b.evidence_ref==secondary->watchdog_b.evidence_ref ||
            primary->watchdog_a.observer_ref==secondary->watchdog_a.evidence_ref ||
            primary->watchdog_a.observer_ref==secondary->watchdog_b.evidence_ref ||
            primary->watchdog_b.observer_ref==secondary->watchdog_a.evidence_ref ||
            primary->watchdog_b.observer_ref==secondary->watchdog_b.evidence_ref ||
            primary->watchdog_a.evidence_ref==secondary->watchdog_a.observer_ref ||
            primary->watchdog_a.evidence_ref==secondary->watchdog_b.observer_ref ||
            primary->watchdog_b.evidence_ref==secondary->watchdog_a.observer_ref ||
            primary->watchdog_b.evidence_ref==secondary->watchdog_b.observer_ref)
            return RAF_FS_GUARD_INVALID;
        b=raf_fs_guard_pair(secondary,policy);
        if (b==RAF_FS_GUARD_INVALID || b==RAF_FS_GUARD_TOKEN_VAZIO) return b;
        if (b==RAF_FS_WD_CONFLICT) return RAF_FS_GUARD_SAFE_STOP;
        if (b==RAF_FS_WD_ALIVE) {
            if ((secondary->declared_capabilities & policy->required_capabilities)
                != policy->required_capabilities) return RAF_FS_GUARD_SAFE_STOP;
            out->source_ref=secondary->source_ref;
            out->artifact_ref=secondary->artifact_ref;
            out->authorization_ref=policy->failover_authorization_ref;
            out->route_kind=RAF_FS_GUARD_FAILOVER;
            return RAF_FS_GUARD_FAILOVER;
        }
    }
    /* ROLLBACK_READY only creates an intent; previous artifact is never loaded. */
    if (policy->allow_rollback) {
        if (!policy->rollback_artifact_ref) return RAF_FS_GUARD_TOKEN_VAZIO;
        if (policy->rollback_artifact_ref==primary->artifact_ref ||
            (secondary && policy->rollback_artifact_ref==secondary->artifact_ref))
            return RAF_FS_GUARD_INVALID;
        out->source_ref=0u;
        out->artifact_ref=policy->rollback_artifact_ref;
        out->authorization_ref=0u;
        out->route_kind=RAF_FS_GUARD_ROLLBACK_READY;
        return RAF_FS_GUARD_ROLLBACK_READY;
    }
    return RAF_FS_GUARD_SAFE_STOP;
}
#endif
