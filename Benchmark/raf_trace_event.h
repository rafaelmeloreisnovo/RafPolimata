#ifndef RAF_TRACE_EVENT_H
#define RAF_TRACE_EVENT_H

#include "Benchmark/raf_trace_flags.h"

typedef struct raf_trace_event_s {
    uint64_t magic;
    uint64_t tick;
    uint32_t tag;
    uint32_t code;
    uint64_t value;
} raf_trace_event_t;

RAF_TRACEBUF_INLINE void raf_trace_barrier(void) {
#if defined(__GNUC__) || defined(__clang__)
    __asm__ volatile("" ::: "memory");
#endif
}

RAF_TRACEBUF_INLINE uint64_t raf_trace_tick(void) {
#if RAF_TRACEBUF_FEATURE_ARM64_TICK
    uint64_t v;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(v));
    return v;
#else
    return 0u;
#endif
}

RAF_TRACEBUF_INLINE raf_trace_event_t raf_trace_event(uint32_t tag, uint32_t code, uint64_t value) {
    raf_trace_event_t e;
    raf_trace_barrier();
    e.magic = RAF_TRACEBUF_MAGIC;
    e.tick = raf_trace_tick();
    e.tag = tag;
    e.code = code;
    e.value = value;
    raf_trace_barrier();
    return e;
}

#endif
