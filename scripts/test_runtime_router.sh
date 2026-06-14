#!/usr/bin/env sh
set -eu
cc="${CC:-gcc}"
build_dir="build_runtime_router"
mkdir -p "$build_dir"
cat > "$build_dir/test_runtime_router.c" <<'C_EOF'
#include "Benchmark/raf_runtime_router.h"

static int expect_u32(u32 got, u32 want) {
    return got == want ? 0 : 1;
}

int main(void) {
    int fail = 0;
    RafRouteInput base = {0u, 0u, 64u, 1u, RAF_ROUTE_STATE_BASELINE};
    RafRouteDecision d0 = raf_runtime_route(base);
    fail |= expect_u32(d0.backend, RAF_BACKEND_GENERIC_C);
    fail |= expect_u32(d0.fail_safe, 1u);

    RafRouteInput a64 = {RAF_CAP_ARM64_NEON, 0u, 64u, 16u, RAF_ROUTE_STATE_VALIDATED};
    RafRouteDecision d1 = raf_runtime_route(a64);
    fail |= expect_u32(d1.backend, RAF_BACKEND_ARM64_NEON);
    fail |= expect_u32(d1.failover, 0u);

    RafRouteInput gpu = {RAF_CAP_ARM64_NEON | RAF_CAP_GPU_BATCH, 0u, 64u, 128u, RAF_ROUTE_STATE_VALIDATED};
    RafRouteDecision d2 = raf_runtime_route(gpu);
    fail |= expect_u32(d2.backend, RAF_BACKEND_GPU_BATCH);

    RafRouteInput degraded = {RAF_CAP_ARM64_NEON | RAF_CAP_GPU_BATCH, RAF_CAP_GPU_BATCH, 64u, 128u, RAF_ROUTE_STATE_VALIDATED};
    RafRouteDecision d3 = raf_runtime_route(degraded);
    fail |= expect_u32(d3.backend, RAF_BACKEND_ARM64_NEON);
    fail |= expect_u32(d3.failover, 1u);

    RafRouteInput rollback = {RAF_CAP_ARM64_NEON | RAF_CAP_GPU_BATCH, 0u, 64u, 128u, RAF_ROUTE_STATE_ROLLBACK};
    RafRouteDecision d4 = raf_runtime_route(rollback);
    fail |= expect_u32(d4.backend, RAF_BACKEND_GENERIC_C);
    fail |= expect_u32(d4.rollback, 1u);
    return fail;
}
C_EOF
"$cc" -std=c11 -Wall -Wextra -Werror -I. "$build_dir/test_runtime_router.c" -o "$build_dir/test_runtime_router"
"$build_dir/test_runtime_router"
echo "runtime_router=PASS"
