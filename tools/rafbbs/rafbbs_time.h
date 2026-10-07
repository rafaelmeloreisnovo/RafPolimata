#ifndef RAFBBS_TIME_H
#define RAFBBS_TIME_H

/*
 * RafBBS authorial monotonic-time value layer.
 *
 * Boundary:
 * - zero hosted/system headers;
 * - zero syscall / OS ABI / allocator;
 * - caller-visible validity is separate from numeric zero;
 * - clock acquisition belongs to a host/provider adapter.
 */

#include "rafbbs_types.h"

#define RAFBBS_NS_PER_MS 1000000ull
#define RAFBBS_NS_PER_SEC 1000000000ull

typedef struct {
    RafU64 ns;
    RafU32 valid;
} RafMonoTime;

typedef struct {
    RafU64 ms;
    RafU32 valid;
} RafMonoElapsed;

static inline RafMonoTime raf_mono_invalid(void) {
    RafMonoTime out;
    out.ns = 0ull;
    out.valid = 0u;
    return out;
}

static inline RafMonoTime raf_mono_from_ns(RafU64 ns) {
    RafMonoTime out;
    out.ns = ns;
    out.valid = 1u;
    return out;
}

static inline RafMonoElapsed raf_mono_elapsed_ms(RafMonoTime start, RafMonoTime end) {
    RafMonoElapsed out;
    RafU64 valid = (RafU64)(
        (start.valid != 0u) &
        (end.valid != 0u) &
        (end.ns >= start.ns)
    );
    RafU64 mask = 0ull - valid;
    RafU64 delta = (end.ns - start.ns) & mask;
    out.ms = delta / RAFBBS_NS_PER_MS;
    out.valid = (RafU32)valid;
    return out;
}

#endif
