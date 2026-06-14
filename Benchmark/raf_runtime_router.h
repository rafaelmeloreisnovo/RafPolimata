/* raf_runtime_router.h — runtime backend selector with fail-safe/failover.
 * Zero heap, no malloc, no GC, no libc dependency in the hot path.
 */
#pragma once
#include "raf_types.h"

#define RAF_BACKEND_GENERIC_C     0u
#define RAF_BACKEND_ARM32_NEON    1u
#define RAF_BACKEND_ARM64_NEON    2u
#define RAF_BACKEND_GPU_BATCH     3u
#define RAF_BACKEND_SYSCALL_DIRECT 4u
#define RAF_BACKEND_STORAGE_BUFFER 5u

#define RAF_CAP_ARM32_NEON        (1u << RAF_BACKEND_ARM32_NEON)
#define RAF_CAP_ARM64_NEON        (1u << RAF_BACKEND_ARM64_NEON)
#define RAF_CAP_GPU_BATCH         (1u << RAF_BACKEND_GPU_BATCH)
#define RAF_CAP_SYSCALL_DIRECT    (1u << RAF_BACKEND_SYSCALL_DIRECT)
#define RAF_CAP_STORAGE_BUFFER    (1u << RAF_BACKEND_STORAGE_BUFFER)

#define RAF_ROUTE_STATE_VOID      0u
#define RAF_ROUTE_STATE_BASELINE  1u
#define RAF_ROUTE_STATE_CANDIDATE 2u
#define RAF_ROUTE_STATE_VALIDATED 3u
#define RAF_ROUTE_STATE_ROLLBACK  4u

typedef struct {
    u32 caps;
    u32 degraded;
    u32 min_batch_gpu;
    u32 batch;
    u32 state;
} RafRouteInput;

typedef struct {
    u32 backend;
    u32 fallback;
    u32 fail_safe;
    u32 failover;
    u32 rollback;
    u32 mitigation;
} RafRouteDecision;

static inline u32 raf_mask_pick(u32 mask, u32 bit, u32 value) {
    u32 m = 0u - ((mask & bit) != 0u);
    return value & m;
}

static inline RafRouteDecision raf_runtime_route(RafRouteInput in) {
    u32 valid = (in.state == RAF_ROUTE_STATE_VALIDATED);
    u32 candidate = (in.state == RAF_ROUTE_STATE_CANDIDATE);
    u32 live = valid | candidate;
    u32 usable = in.caps & ~in.degraded;
    u32 gpu_ready = ((in.batch >= in.min_batch_gpu) & ((usable & RAF_CAP_GPU_BATCH) != 0u));
    u32 route = RAF_BACKEND_GENERIC_C;
    route |= raf_mask_pick(usable, RAF_CAP_STORAGE_BUFFER, RAF_BACKEND_STORAGE_BUFFER);
    route |= raf_mask_pick(usable, RAF_CAP_SYSCALL_DIRECT, RAF_BACKEND_SYSCALL_DIRECT);
    route |= raf_mask_pick(usable, RAF_CAP_ARM32_NEON, RAF_BACKEND_ARM32_NEON);
    route |= raf_mask_pick(usable, RAF_CAP_ARM64_NEON, RAF_BACKEND_ARM64_NEON);
    route = gpu_ready ? RAF_BACKEND_GPU_BATCH : route;
    route = live ? route : RAF_BACKEND_GENERIC_C;

    RafRouteDecision out;
    out.backend = route;
    out.fallback = RAF_BACKEND_GENERIC_C;
    out.fail_safe = (route == RAF_BACKEND_GENERIC_C);
    out.failover = ((in.caps & in.degraded) != 0u);
    out.rollback = (in.state == RAF_ROUTE_STATE_ROLLBACK) | (in.state == RAF_ROUTE_STATE_VOID);
    out.mitigation = (in.degraded != 0u) | (gpu_ready == 0u && ((in.caps & RAF_CAP_GPU_BATCH) != 0u));
    return out;
}
