#include "raf_bus_throughput.h"

static int expect_u32(u32 got, u32 want) {
    return got == want ? 0 : 1;
}

int main(void) {
    int fail = 0;

    static u8 src[RAF_BUS_MEM_SIZE];
    static u8 dst[RAF_BUS_MEM_SIZE];
    for (u32 i = 0u; i < RAF_BUS_MEM_SIZE; i++) src[i] = (u8)(i & 0xFFu);

    u32 seq_sum = raf_bus_mem_sequential(dst, src, RAF_BUS_MEM_SIZE);
    u32 expect_sum = 0u;
    for (u32 i = 0u; i < RAF_BUS_MEM_SIZE; i++) expect_sum += src[i];
    fail |= expect_u32(seq_sum, expect_sum);
    for (u32 i = 0u; i < RAF_BUS_MEM_SIZE; i++) fail |= (dst[i] != src[i]);

    for (u32 i = 0u; i < RAF_BUS_MEM_SIZE; i++) dst[i] = 0u;
    u32 strided_sum = raf_bus_mem_strided(dst, src, RAF_BUS_MEM_SIZE);
    fail |= expect_u32(strided_sum, expect_sum);
    for (u32 i = 0u; i < RAF_BUS_MEM_SIZE; i++) fail |= (dst[i] != src[i]);

    RafRouteInput in = {RAF_CAP_ARM64_NEON | RAF_CAP_GPU_BATCH, 0u, 64u, 128u, RAF_ROUTE_STATE_VALIDATED};
    fail |= expect_u32(raf_bus_dispatch_op(in), RAF_BACKEND_GPU_BATCH);

    return fail;
}
