#ifndef RAF_FS_NUMA_PLAN_H
#define RAF_FS_NUMA_PLAN_H
/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Deterministic NUMA capacity and distance plan from caller-supplied topology.
 * SCOPE: Pure advisory node selection; no OS probing, affinity or memory placement.
 * PRECONDITIONS: Caller supplies authentic snapshot and owns output; single-thread use.
 * REGISTER_OWNERSHIP: Compiler scalar registers only.
 * CLOBBERS: Output changes only on selected node success; topology is readonly.
 * MEMORY_ORDER: No synchronization/atomic reservation guarantees.
 * TAIL_SHADOW: Fixed maximum eight nodes, no hidden topology/device lookup.
 * EVIDENCE: Synthetic checks only; actual NUMA and physical placement TOKEN_VAZIO.
 * CLOSURE_L11: topology/authority gap governed; CLOSURE_L12: physical placement remains unproven.
 */
#include "raf_fs_types.h"
#define RAF_FS_NUMA_MAX_NODES 8u
#define RAF_FS_NUMA_SELECTED 0x91u
#define RAF_FS_NUMA_NO_FIT 0x92u
#define RAF_FS_NUMA_BAD_INPUT 0x93u
#define RAF_FS_NUMA_NO_ALLOWED 0x94u
#define RAF_FS_NUMA_TOPOLOGY_UNKNOWN 0x95u

typedef struct raf_fs_numa_snapshot {
    raf_u32 node_count;
    raf_u32 allowed_mask;
    raf_u32 origin;
    raf_u32 capacity_pages[RAF_FS_NUMA_MAX_NODES];
    raf_u32 used_pages[RAF_FS_NUMA_MAX_NODES];
    raf_u8 distance[RAF_FS_NUMA_MAX_NODES * RAF_FS_NUMA_MAX_NODES];
} raf_fs_numa_snapshot;

static inline __attribute__((always_inline)) raf_u32
raf_fs_numa_choose(const raf_fs_numa_snapshot *s, raf_u32 required_pages,
                   raf_u32 *chosen)
{
    raf_u32 i, j, best=RAF_FS_NUMA_MAX_NODES, score=256u;
    raf_u32 spare_best=0u, n;
    if (!s || !chosen || !required_pages) return RAF_FS_NUMA_BAD_INPUT;
    if (!s->node_count) return RAF_FS_NUMA_TOPOLOGY_UNKNOWN;
    if (s->node_count>RAF_FS_NUMA_MAX_NODES || s->origin>=s->node_count)
        return RAF_FS_NUMA_BAD_INPUT;
    n=s->node_count;
    if ((n<8u && (s->allowed_mask >> n)!=0u)) return RAF_FS_NUMA_BAD_INPUT;
    /* All topology constraints are checked before selection to avoid fabricated confidence. */
    for (i=0u;i<n;++i) {
        if (s->used_pages[i]>s->capacity_pages[i] ||
            s->distance[i*RAF_FS_NUMA_MAX_NODES+i]!=10u)
            return RAF_FS_NUMA_BAD_INPUT;
        for (j=0u;j<n;++j) {
            if (s->distance[i*RAF_FS_NUMA_MAX_NODES+j]<10u)
                return RAF_FS_NUMA_BAD_INPUT;
        }
    }
    if (!s->allowed_mask) return RAF_FS_NUMA_NO_ALLOWED;
    for (i=0u;i<n;++i) {
        raf_u32 spare=s->capacity_pages[i]-s->used_pages[i];
        raf_u32 dist=s->distance[s->origin*RAF_FS_NUMA_MAX_NODES+i];
        if (!(s->allowed_mask & ((raf_u32)1u<<i)) || spare<required_pages) continue;
        if (best==RAF_FS_NUMA_MAX_NODES || dist<score ||
            (dist==score && spare>spare_best)) {
            best=i; score=dist; spare_best=spare;
        }
    }
    if (best==RAF_FS_NUMA_MAX_NODES) return RAF_FS_NUMA_NO_FIT;
    *chosen=best;
    return RAF_FS_NUMA_SELECTED;
}
#endif
