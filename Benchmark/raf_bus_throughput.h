/* raf_bus_throughput.h — throughput-per-bus probes (closes checklist S23).
 *
 * Measures throughput across two distinct "bus" paths already present
 * in this codebase:
 *   - memory bus  : sequential vs strided buffer copy (cache/DRAM bus)
 *   - dispatch bus: raf_runtime_route() function-pointer dispatch
 *     (the existing runtime backend selector bus from raf_runtime_router.h)
 *
 * Zero heap, no malloc — buffers are caller-allocated static/stack.
 */
#pragma once
#include "raf_types.h"
#include "raf_runtime_router.h"

#define RAF_BUS_MEM_SIZE (64u * 1024u)  /* 64 KiB working set */

/* Sequential copy over buf (MEM_SIZE bytes). Returns a checksum so the
 * compiler cannot eliminate the loop. */
static inline u32 raf_bus_mem_sequential(u8 *dst, const u8 *src, u32 n) {
    u32 sum = 0u;
    for (u32 i = 0u; i < n; i++) {
        dst[i] = src[i];
        sum += dst[i];
    }
    return sum;
}

/* Strided copy: stride of 64 bytes (typical cache line), wrapping. */
static inline u32 raf_bus_mem_strided(u8 *dst, const u8 *src, u32 n) {
    u32 sum = 0u;
    u32 stride = 64u;
    for (u32 off = 0u; off < stride; off++) {
        for (u32 i = off; i < n; i += stride) {
            dst[i] = src[i];
            sum += dst[i];
        }
    }
    return sum;
}

/* One dispatch-bus operation: route through raf_runtime_route(). */
static inline u32 raf_bus_dispatch_op(RafRouteInput in) {
    RafRouteDecision out = raf_runtime_route(in);
    return out.backend;
}
