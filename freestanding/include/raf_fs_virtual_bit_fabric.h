#ifndef RAF_FS_VIRTUAL_BIT_FABRIC_H
#define RAF_FS_VIRTUAL_BIT_FABRIC_H
/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Virtual bit-path observation, finite caller-owned syslog, parity/watchdogs.
 * SCOPE: Abstract register-bit fabric only, NOT an instruction-set emulator, CPU,
 *        microarchitectural trace, privileged monitor or physical sensor.
 * PRECONDITIONS: Caller owns distinct readable/writable context, sequential use,
 *                data is synthetic/public; no concurrent writers or aliasing.
 * REGISTER_OWNERSHIP: NONE; all eight modeled registers are caller-owned arrays.
 * CLOBBERS: A successful step changes one modeled destination bit, event log,
 *           parity, state digest, epoch/chain and watchdog guard; FAIL changes nothing.
 * MEMORY_ORDER: C single-thread program order; no atomics or fences.
 * TAIL_SHADOW: Eight explicit nodes; max sixteen events; exhausted log is FAIL,
 *              never overwritten, sampled, silently truncated or retried.
 * EVIDENCE: Source/model/hosted test only; hardware, CPU timing and device
 *           equivalence remain TOKEN_VAZIO (CLOSURE_L12).
 * ANNOTATION: VBF_TRACE_REQUIRED; VBF_NO_HIDDEN_TAIL; VBF_FULL_AUDIT;
 *             VBF_MODEL_ONLY; source markers are reviewable contracts, never
 *             executable optimization directives by themselves.
 * RIGHTS: repository-wide license and redistribution provenance P0 unresolved.
 */
#include "raf_fs_types.h"
#define RAF_FS_VBF_NODES 8u
#define RAF_FS_VBF_TRACE_MAX 16u
#define RAF_FS_VBF_PROFILE_MCU8 1u
#define RAF_FS_VBF_PROFILE_MCU16 2u
#define RAF_FS_VBF_PROFILE_I686 3u
#define RAF_FS_VBF_PROFILE_X86_64 4u
#define RAF_FS_VBF_PROFILE_ARMV7 5u
#define RAF_FS_VBF_PROFILE_AARCH64 6u
#define RAF_FS_VBF_PROFILE_RV32 7u
#define RAF_FS_VBF_PROFILE_RV64 8u
#define RAF_FS_VBF_PROFILE_POWER64 9u
#define RAF_FS_VBF_PROFILE_S390X 10u
#define RAF_FS_VBF_PROFILE_LOONGARCH64 11u
#define RAF_FS_VBF_INPUT 1u
#define RAF_FS_VBF_ROUTE 2u
#define RAF_FS_VBF_OK 0xA1u
#define RAF_FS_VBF_BAD_INPUT 0xA2u
#define RAF_FS_VBF_BAD_STATE 0xA3u
#define RAF_FS_VBF_TRACE_FULL 0xA4u
#define RAF_FS_VBF_BAD_PARITY 0xA5u
#define RAF_FS_VBF_BAD_DIGEST 0xA6u
#define RAF_FS_VBF_BAD_CHAIN 0xA7u
#define RAF_FS_VBF_BAD_WATCHDOG 0xA8u
#define RAF_FS_VBF_NOT_INITIALIZED 0xA9u
#define RAF_FS_VBF_CHAIN_SEED 0x56524631u
#define RAF_FS_VBF_STATE_SEED 0x454C4641u

typedef struct raf_fs_vbf_event {
    raf_u32 tick, operation, from, to, source_bit, dest_bit;
    raf_u32 source_value, prior_value, next_value, chain;
} raf_fs_vbf_event;
typedef struct raf_fs_vbf {
    raf_u32 profile, width, tick, count;
    raf_u32 words[RAF_FS_VBF_NODES][2];
    raf_u32 parity[RAF_FS_VBF_NODES];
    raf_u32 checksum, chain, watchdog;
    raf_fs_vbf_event trace[RAF_FS_VBF_TRACE_MAX];
} raf_fs_vbf;

static inline __attribute__((always_inline)) raf_u32 raf_fs_vbf_width(raf_u32 profile)
{
    if (profile==RAF_FS_VBF_PROFILE_MCU8) return 8u;
    if (profile==RAF_FS_VBF_PROFILE_MCU16) return 16u;
    if (profile==RAF_FS_VBF_PROFILE_I686 || profile==RAF_FS_VBF_PROFILE_ARMV7 ||
        profile==RAF_FS_VBF_PROFILE_RV32) return 32u;
    if (profile>=RAF_FS_VBF_PROFILE_X86_64 && profile<=RAF_FS_VBF_PROFILE_LOONGARCH64 &&
        profile!=RAF_FS_VBF_PROFILE_ARMV7 && profile!=RAF_FS_VBF_PROFILE_RV32) return 64u;
    return 0u;
}
static inline __attribute__((always_inline)) raf_u32 raf_fs_vbf_parity32(raf_u32 x)
{
    x ^= x>>16; x ^= x>>8; x ^= x>>4; x ^= x>>2; x ^= x>>1;
    return x&1u;
}
static inline __attribute__((always_inline)) raf_u32 raf_fs_vbf_mix(raf_u32 x, raf_u32 v)
{
    return (x ^ (v + 0x9e3779b9u + (x<<6) + (x>>2))) * 0x01000193u;
}
static inline __attribute__((always_inline)) raf_u32 raf_fs_vbf_state_digest(const raf_fs_vbf *s)
{
    raf_u32 i, x=RAF_FS_VBF_STATE_SEED;
    for (i=0u;i<RAF_FS_VBF_NODES;++i) {
        x=raf_fs_vbf_mix(x,s->words[i][0]);
        x=raf_fs_vbf_mix(x,s->words[i][1]);
    }
    return x;
}
static inline __attribute__((always_inline)) raf_u32
raf_fs_vbf_trace_digest(raf_u32 prev, const raf_fs_vbf_event *e)
{
    raf_u32 x=raf_fs_vbf_mix(prev,e->tick);
    x=raf_fs_vbf_mix(x,e->operation);
    x=raf_fs_vbf_mix(x,e->from);
    x=raf_fs_vbf_mix(x,e->to);
    x=raf_fs_vbf_mix(x,e->source_bit);
    x=raf_fs_vbf_mix(x,e->dest_bit);
    x=raf_fs_vbf_mix(x,e->source_value);
    x=raf_fs_vbf_mix(x,e->prior_value);
    return raf_fs_vbf_mix(x,e->next_value);
}
static inline __attribute__((always_inline)) raf_u32 raf_fs_vbf_guard(const raf_fs_vbf *s)
{
    raf_u32 x=raf_fs_vbf_mix(s->checksum,s->chain);
    x=raf_fs_vbf_mix(x,s->tick);
    x=raf_fs_vbf_mix(x,s->count);
    x=raf_fs_vbf_mix(x,s->profile);
    return ~x;
}
/* Watchdog of watchdogs: check structural shape, bit parity, state digest,
 * complete event chain and guard separately; detection != cryptographic proof. */
static inline __attribute__((always_inline)) raf_u32 raf_fs_vbf_check(const raf_fs_vbf *s)
{
    raf_u32 i, p, chain=RAF_FS_VBF_CHAIN_SEED;
    if (!s) return RAF_FS_VBF_BAD_INPUT;
    if (!s->profile || !s->width) return RAF_FS_VBF_NOT_INITIALIZED;
    if (s->width!=raf_fs_vbf_width(s->profile) ||
        s->count>RAF_FS_VBF_TRACE_MAX || s->tick!=s->count) return RAF_FS_VBF_BAD_STATE;
    for(i=0u;i<RAF_FS_VBF_NODES;++i) {
        raf_u32 lo=s->words[i][0], hi=s->words[i][1];
        if ((s->width==8u && (lo&0xffffff00u)) ||
            (s->width==16u && (lo&0xffff0000u)) ||
            (s->width<=32u && hi)) return RAF_FS_VBF_BAD_STATE;
        p=raf_fs_vbf_parity32(lo)^raf_fs_vbf_parity32(hi);
        if (p!=s->parity[i]) return RAF_FS_VBF_BAD_PARITY;
    }
    if (raf_fs_vbf_state_digest(s)!=s->checksum) return RAF_FS_VBF_BAD_DIGEST;
    for(i=0u;i<s->count;++i) {
        const raf_fs_vbf_event *e=&s->trace[i];
        if (e->tick!=i+1u ||
            (e->operation!=RAF_FS_VBF_INPUT && e->operation!=RAF_FS_VBF_ROUTE) ||
            e->from>=RAF_FS_VBF_NODES || e->to>=RAF_FS_VBF_NODES ||
            e->source_bit>=s->width || e->dest_bit>=s->width ||
            e->source_value>1u || e->prior_value>1u || e->next_value>1u ||
            e->source_value!=e->next_value) return RAF_FS_VBF_BAD_CHAIN;
        chain=raf_fs_vbf_trace_digest(chain,e);
        if (e->chain!=chain) return RAF_FS_VBF_BAD_CHAIN;
    }
    if (s->chain!=chain) return RAF_FS_VBF_BAD_CHAIN;
    if (s->watchdog!=raf_fs_vbf_guard(s)) return RAF_FS_VBF_BAD_WATCHDOG;
    return RAF_FS_VBF_OK;
}
/* Initialize caller memory explicitly; do not infer architecture from host macros.
 * No platform services or imported functions. */
static inline __attribute__((always_inline)) raf_u32
raf_fs_vbf_init(raf_fs_vbf *s, raf_u32 profile)
{
    raf_u32 i,j,width=raf_fs_vbf_width(profile);
    if (!s || !width) return RAF_FS_VBF_BAD_INPUT;
    s->profile=profile; s->width=width; s->tick=0u; s->count=0u;
    for(i=0u;i<RAF_FS_VBF_NODES;++i) {
        s->words[i][0]=0u; s->words[i][1]=0u; s->parity[i]=0u;
    }
    for(j=0u;j<RAF_FS_VBF_TRACE_MAX;++j) {
        raf_fs_vbf_event *e=&s->trace[j];
        e->tick=0u; e->operation=0u; e->from=0u; e->to=0u;
        e->source_bit=0u; e->dest_bit=0u; e->source_value=0u;
        e->prior_value=0u; e->next_value=0u; e->chain=0u;
    }
    s->checksum=raf_fs_vbf_state_digest(s);
    s->chain=RAF_FS_VBF_CHAIN_SEED;
    s->watchdog=raf_fs_vbf_guard(s);
    return RAF_FS_VBF_OK;
}
/* One event/transaction, zero hidden tail: INPUT injects a typed 0/1 into
 * destination; ROUTE samples one modeled source bit and writes destination.
 * If a validity check fails, no caller-owned field is modified. */
static inline __attribute__((always_inline)) raf_u32
raf_fs_vbf_step(raf_fs_vbf *s, raf_u32 operation, raf_u32 from, raf_u32 to,
                raf_u32 source_bit, raf_u32 dest_bit, raf_u32 input_bit)
{
    raf_fs_vbf_event e;
    raf_u32 status, lane, mask, old, bit, word;
    if (!s) return RAF_FS_VBF_BAD_INPUT;
    status=raf_fs_vbf_check(s);
    if (status!=RAF_FS_VBF_OK) return status;
    if (operation!=RAF_FS_VBF_INPUT && operation!=RAF_FS_VBF_ROUTE)
        return RAF_FS_VBF_BAD_INPUT;
    if (from>=RAF_FS_VBF_NODES || to>=RAF_FS_VBF_NODES ||
        source_bit>=s->width || dest_bit>=s->width || input_bit>1u ||
        (operation==RAF_FS_VBF_ROUTE && input_bit!=0u)) return RAF_FS_VBF_BAD_INPUT;
    if (s->count>=RAF_FS_VBF_TRACE_MAX) return RAF_FS_VBF_TRACE_FULL;
    bit=(operation==RAF_FS_VBF_INPUT) ? input_bit :
        ((s->words[from][source_bit>>5]>>(source_bit&31u))&1u);
    lane=dest_bit>>5;
    mask=(raf_u32)1u<<(dest_bit&31u);
    word=s->words[to][lane];
    old=(word&mask)?1u:0u;
    e.tick=s->tick+1u; e.operation=operation; e.from=from; e.to=to;
    e.source_bit=source_bit; e.dest_bit=dest_bit;
    e.source_value=bit; e.prior_value=old; e.next_value=bit;
    e.chain=raf_fs_vbf_trace_digest(s->chain,&e);
    s->words[to][lane]=(word&~mask)|(bit?mask:0u);
    s->parity[to]^=old^bit;
    s->trace[s->count]=e;
    ++s->count; ++s->tick;
    s->chain=e.chain;
    s->checksum=raf_fs_vbf_state_digest(s);
    s->watchdog=raf_fs_vbf_guard(s);
    return RAF_FS_VBF_OK;
}
#endif
