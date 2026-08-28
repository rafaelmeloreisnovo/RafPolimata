#include "../include/raf_hash_fabric.h"

#define S(x) ((rhf_u8)(x))

static const rhf_u8 rhf_fsm[RHF_STATE_COUNT][RHF_EVENT_COUNT] = {
/* VOID */      {S(RHF_PROBE),S(RHF_VOID),S(RHF_SAFE),S(RHF_VOID),S(RHF_VOID),S(RHF_VOID),S(RHF_SAFE),S(RHF_VOID),S(RHF_SAFE),S(RHF_VOID),S(RHF_VOID)},
/* PROBE */     {S(RHF_PROBE),S(RHF_BASELINE),S(RHF_SAFE),S(RHF_PROBE),S(RHF_PROBE),S(RHF_PROBE),S(RHF_SAFE),S(RHF_PROBE),S(RHF_SAFE),S(RHF_PROBE),S(RHF_PROBE)},
/* BASELINE */  {S(RHF_BASELINE),S(RHF_BASELINE),S(RHF_SAFE),S(RHF_CANDIDATE),S(RHF_BASELINE),S(RHF_BASELINE),S(RHF_DEGRADED),S(RHF_BASELINE),S(RHF_DEGRADED),S(RHF_BASELINE),S(RHF_BASELINE)},
/* CANDIDATE */ {S(RHF_CANDIDATE),S(RHF_CANDIDATE),S(RHF_SAFE),S(RHF_CANDIDATE),S(RHF_VALIDATED),S(RHF_BASELINE),S(RHF_DEGRADED),S(RHF_CANDIDATE),S(RHF_DEGRADED),S(RHF_CANDIDATE),S(RHF_CANDIDATE)},
/* VALIDATED */ {S(RHF_VALIDATED),S(RHF_VALIDATED),S(RHF_SAFE),S(RHF_CANDIDATE),S(RHF_VALIDATED),S(RHF_BASELINE),S(RHF_DEGRADED),S(RHF_VALIDATED),S(RHF_DEGRADED),S(RHF_VALIDATED),S(RHF_VALIDATED)},
/* DEGRADED */  {S(RHF_DEGRADED),S(RHF_DEGRADED),S(RHF_SAFE),S(RHF_CANDIDATE),S(RHF_FAILOVER),S(RHF_ROLLBACK),S(RHF_FAILOVER),S(RHF_FAILOVER),S(RHF_ROLLBACK),S(RHF_BASELINE),S(RHF_FAILBACK)},
/* FAILOVER */  {S(RHF_FAILOVER),S(RHF_FAILOVER),S(RHF_SAFE),S(RHF_CANDIDATE),S(RHF_FAILOVER),S(RHF_ROLLBACK),S(RHF_ROLLBACK),S(RHF_FAILBACK),S(RHF_ROLLBACK),S(RHF_BASELINE),S(RHF_VALIDATED)},
/* ROLLBACK */  {S(RHF_ROLLBACK),S(RHF_ROLLBACK),S(RHF_SAFE),S(RHF_ROLLBACK),S(RHF_ROLLBACK),S(RHF_SAFE),S(RHF_SAFE),S(RHF_ROLLBACK),S(RHF_SAFE),S(RHF_BASELINE),S(RHF_ROLLBACK)},
/* FAILBACK */  {S(RHF_FAILBACK),S(RHF_FAILBACK),S(RHF_SAFE),S(RHF_CANDIDATE),S(RHF_FAILBACK),S(RHF_ROLLBACK),S(RHF_ROLLBACK),S(RHF_FAILBACK),S(RHF_ROLLBACK),S(RHF_BASELINE),S(RHF_VALIDATED)},
/* SAFE */      {S(RHF_PROBE),S(RHF_SAFE),S(RHF_SAFE),S(RHF_SAFE),S(RHF_SAFE),S(RHF_SAFE),S(RHF_SAFE),S(RHF_SAFE),S(RHF_SAFE),S(RHF_SAFE),S(RHF_SAFE)}
};

rhf_u8 rhf_lanes32(rhf_u16 vector_bits) {
    return (rhf_u8)(vector_bits == 512u ? 16u :
                    vector_bits == 256u ? 8u :
                    vector_bits == 128u ? 4u : 1u);
}

rhf_u8 rhf_control_q_fraction_bits(void) {
    return (rhf_u8)(sizeof(void*) >= 8u ? 32u : 16u);
}

rhf_state rhf_step(rhf_state state, rhf_event event) {
    if ((rhf_u32)state >= (rhf_u32)RHF_STATE_COUNT ||
        (rhf_u32)event >= (rhf_u32)RHF_EVENT_COUNT) {
        return RHF_SAFE;
    }
    return (rhf_state)rhf_fsm[(rhf_u32)state][(rhf_u32)event];
}

rhf_state rhf_watchdog_tick(rhf_state state, rhf_watchdog *wd, rhf_u32 ticks) {
    if (!wd) return RHF_SAFE;
    wd->elapsed_ticks += ticks;
    if (wd->heartbeat) {
        wd->heartbeat = 0u;
        wd->elapsed_ticks = 0u;
        wd->epoch += 1u;
        return rhf_step(state, RHF_WATCHDOG_OK);
    }
    if (wd->deadline_ticks && wd->elapsed_ticks >= wd->deadline_ticks) {
        return rhf_step(state, RHF_WATCHDOG_FAIL);
    }
    return state;
}

#define RHF_PAIR(L,R,O) do { \
    rhf_u32 _e = hash_pair((L),(R),(O),ctx); \
    if (_e) return _e; \
} while (0)

rhf_u32 rhf_merkle16_reduce(
    const rhf_u8 leaves[RHF_MERKLE16_LEAVES][RHF_DIGEST32_BYTES],
    rhf_u8 root[RHF_DIGEST32_BYTES],
    rhf_u8 scratch[RHF_MERKLE16_SCRATCH][RHF_DIGEST32_BYTES],
    rhf_hash_pair_fn hash_pair,
    void *ctx) {
    if (!leaves || !root || !scratch || !hash_pair) return 1u;

    RHF_PAIR(leaves[0],  leaves[1],  scratch[0]);
    RHF_PAIR(leaves[2],  leaves[3],  scratch[1]);
    RHF_PAIR(leaves[4],  leaves[5],  scratch[2]);
    RHF_PAIR(leaves[6],  leaves[7],  scratch[3]);
    RHF_PAIR(leaves[8],  leaves[9],  scratch[4]);
    RHF_PAIR(leaves[10], leaves[11], scratch[5]);
    RHF_PAIR(leaves[12], leaves[13], scratch[6]);
    RHF_PAIR(leaves[14], leaves[15], scratch[7]);

    RHF_PAIR(scratch[0], scratch[1], scratch[8]);
    RHF_PAIR(scratch[2], scratch[3], scratch[9]);
    RHF_PAIR(scratch[4], scratch[5], scratch[10]);
    RHF_PAIR(scratch[6], scratch[7], scratch[11]);

    RHF_PAIR(scratch[8],  scratch[9],  scratch[12]);
    RHF_PAIR(scratch[10], scratch[11], scratch[13]);

    RHF_PAIR(scratch[12], scratch[13], root);
    return 0u;
}

static rhf_u32 rotl32(rhf_u32 x, rhf_u32 n) {
    return (x << n) | (x >> (32u - n));
}

rhf_u32 rhf_mix32(rhf_u32 a, rhf_u32 b, rhf_u32 c) {
    a += b + 0x9E3779B9u;
    c ^= a;
    c = rotl32(c, 16u);
    b += c;
    a ^= b;
    a = rotl32(a, 12u);
    return a ^ c ^ rotl32(b, 7u);
}
