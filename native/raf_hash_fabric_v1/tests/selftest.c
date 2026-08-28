#include "../include/raf_hash_fabric.h"

static rhf_u32 test_pair(const rhf_u8 left[32], const rhf_u8 right[32], rhf_u8 out[32], void *ctx) {
    rhf_u32 seed = ctx ? *(const rhf_u32*)ctx : 0u;
    rhf_u32 i;
    /* Test adapter only. The production Merkle path must use a validated hash. */
    for (i = 0u; i < 32u; ++i) {
        seed = rhf_mix32(seed, (rhf_u32)left[i], (rhf_u32)right[i]);
        out[i] = (rhf_u8)(seed >> ((i & 3u) * 8u));
    }
    return 0u;
}

int main(void) {
    rhf_u8 leaves[16][32] = {{0}};
    rhf_u8 scratch[14][32];
    rhf_u8 root[32];
    rhf_u32 seed = 0x52484631u; /* "RHF1" */
    rhf_watchdog wd = {0u,1u,10u,0u};
    rhf_state s;

    if (rhf_mix32(0u,0u,0u) != 0x4B0E6BF3u) return 10;
    if (rhf_mix32(1u,2u,3u) != 0x2808DCF3u) return 11;
    if (rhf_mix32(0xFFFFFFFFu,0x12345678u,0xABCDEF01u) != 0x67AAFA9Eu) return 12;

    if (rhf_lanes32(128u) != 4u) return 20;
    if (rhf_lanes32(256u) != 8u) return 21;
    if (rhf_lanes32(512u) != 16u) return 22;

    s = rhf_step(RHF_VOID, RHF_BOOT);
    if (s != RHF_PROBE) return 30;
    s = rhf_step(s, RHF_PROBE_OK);
    if (s != RHF_BASELINE) return 31;
    s = rhf_step(s, RHF_BEGIN_CANDIDATE);
    if (s != RHF_CANDIDATE) return 32;
    s = rhf_step(s, RHF_CANDIDATE_OK);
    if (s != RHF_VALIDATED) return 33;
    s = rhf_watchdog_tick(s, &wd, 1u);
    if (s != RHF_VALIDATED || wd.epoch != 1u) return 34;
    s = rhf_watchdog_tick(s, &wd, 10u);
    if (s != RHF_DEGRADED) return 35;

    leaves[0][0] = 1u;
    leaves[15][31] = 2u;
    if (rhf_merkle16_reduce(leaves, root, scratch, test_pair, &seed) != 0u) return 40;
    if ((root[0] | root[1] | root[2] | root[3]) == 0u) return 41;

    return 0;
}
