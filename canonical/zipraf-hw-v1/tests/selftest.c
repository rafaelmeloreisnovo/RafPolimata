#include <stdio.h>
#include "../include/zipraf_hw.h"

static int check(int ok, const char *name) {
    if (!ok) { fprintf(stderr, "FAIL %s\n", name); return 1; }
    printf("PASS %s\n", name);
    return 0;
}

int main(void) {
    int rc = 0;
    zh_u32 selected = 0u;
    struct zh_capability cap = { ZH_ARCH_X86_64, ZH_FEAT_SCALAR | ZH_FEAT_SSE2 | ZH_FEAT_AVX2, 64u, 64u, 64u, 8u };
    struct zh_measurement m[3] = {
        { 10u, ZH_FEAT_SCALAR, 1000u, 1200u, 1024u, ZH_PASS },
        { 20u, ZH_FEAT_AVX2,    500u,  700u, 1024u, ZH_PASS },
        { 30u, ZH_FEAT_AVX512,  100u,  120u, 1024u, ZH_PASS }
    };

    rc |= check(zh_patch_u64(0xaaaaULL, 0x5555ULL, 0x00ffULL) == 0xaa55ULL, "masked_patch");
    rc |= check(zh_vector_lanes(256u, 32u) == 8u, "vector_lanes");
    rc |= check(zh_block_fit(64u, 16u) == 4u, "block_fit");
    rc |= check(zh_write_amplification_q16(1u, 64u) == (64u << 16), "write_amplification");
    rc |= check(zh_select_measured_backend(&cap, m, 3u, &selected) == 0 && selected == 20u, "measured_backend_select");
    return rc ? 1 : 0;
}
