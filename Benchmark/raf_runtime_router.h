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

/* TOKEN_VAZIO — canonical zero/no-op return value for unimplemented backends */
#define TOKEN_VAZIO 0u

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

/* ---------------------------------------------------------------------------
 * ROLLBACK hook — optional callback invoked when any backend returns error.
 * Set via raf_route_set_rollback_hook(); NULL by default (no notification).
 * The hook receives the route_state at time of rollback activation.
 * ---------------------------------------------------------------------------*/
static void (*_raf_rollback_hook)(u32 state) = (void*)0;

static inline void raf_route_set_rollback_hook(void (*fn)(u32 state)) {
    _raf_rollback_hook = fn;
}

/* ---------------------------------------------------------------------------
 * SYSCALL_DIRECT backend — fast-path using raw Linux syscall(2).
 * Activated when arch==ARM64 || arch==X86_64 and no NEON/SIMD is available.
 * Falls back to TOKEN_VAZIO on non-Linux hosts (no SYS_write defined).
 *
 * Writes `len` bytes from `buf` to stdout (fd=1) via SYS_write.
 * Returns bytes written (>= 0) on success, negative errno on failure.
 * Returns TOKEN_VAZIO (0) on non-Linux platforms.
 * ---------------------------------------------------------------------------*/
#if defined(__linux__) && (defined(RAF_ARCH_A64) || defined(RAF_ARCH_X64))
#  ifndef _GNU_SOURCE
#    define _GNU_SOURCE 1
#  endif
#  include <sys/syscall.h>
#  include <unistd.h>
static inline s64 _raf_route_syscall_direct(const void *buf, u32 len) {
    return (s64)syscall(SYS_write, (long)1 /*stdout*/, buf, (long)len);
}
#else
/* Non-Linux or unsupported arch: syscall_direct is TOKEN_VAZIO */
static inline s64 _raf_route_syscall_direct(const void *buf, u32 len) {
    (void)buf; (void)len;
    return (s64)TOKEN_VAZIO;
}
#endif /* __linux__ && (ARM64 || X86_64) */

/* ---------------------------------------------------------------------------
 * RAF_BACKEND_GPU_BATCH: TOKEN_VAZIO — Vulkan/OpenCL compute, não implementado neste host
 * RAF_BACKEND_STORAGE_BUFFER: TOKEN_VAZIO — mmap DMA bulk, requer hardware BCM/DMA
 * ---------------------------------------------------------------------------*/

static inline u32 raf_mask_select(u32 current, u32 mask, u32 bit, u32 value) {
    u32 m = 0u - ((mask & bit) != 0u);
    return (current & ~m) | (value & m);
}

/* ---------------------------------------------------------------------------
 * raf_runtime_route — main routing function.
 *
 * ROLLBACK detection: if the caller detects route_result < 0 (any backend
 * returned error), it should call raf_route_activate_rollback() to transition
 * the route state to RAF_ROUTE_STATE_ROLLBACK and fire the registered hook.
 * ---------------------------------------------------------------------------*/
static inline void raf_route_activate_rollback(RafRouteInput *in, s32 route_result) {
    if (route_result < 0) {
        in->state = RAF_ROUTE_STATE_ROLLBACK;
        if (_raf_rollback_hook) {
            _raf_rollback_hook(in->state);
        }
    }
}

static inline RafRouteDecision raf_runtime_route(RafRouteInput in) {
    u32 valid = (in.state == RAF_ROUTE_STATE_VALIDATED);
    u32 candidate = (in.state == RAF_ROUTE_STATE_CANDIDATE);
    u32 live = valid | candidate;
    u32 usable = in.caps & ~in.degraded;
    u32 gpu_ready = ((in.batch >= in.min_batch_gpu) & ((usable & RAF_CAP_GPU_BATCH) != 0u));
    u32 route = RAF_BACKEND_GENERIC_C;
    route = raf_mask_select(route, usable, RAF_CAP_STORAGE_BUFFER, RAF_BACKEND_STORAGE_BUFFER);
    route = raf_mask_select(route, usable, RAF_CAP_SYSCALL_DIRECT, RAF_BACKEND_SYSCALL_DIRECT);
    route = raf_mask_select(route, usable, RAF_CAP_ARM32_NEON, RAF_BACKEND_ARM32_NEON);
    route = raf_mask_select(route, usable, RAF_CAP_ARM64_NEON, RAF_BACKEND_ARM64_NEON);
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
