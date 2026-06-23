#ifndef RAF_TRACE_SINK_H
#define RAF_TRACE_SINK_H

#include "Benchmark/raf_trace_emit.h"

typedef size_t (*raf_trace_sink_fn)(void *ctx, const uint8_t *data, size_t len);

typedef struct raf_trace_sink_s {
    raf_trace_sink_fn write;
    void *ctx;
    uint32_t flags;
} raf_trace_sink_t;

RAF_TRACEBUF_INLINE raf_trace_sink_t raf_trace_sink(raf_trace_sink_fn write, void *ctx) {
    raf_trace_sink_t s;
    s.write = write;
    s.ctx = ctx;
    s.flags = 0u;
    return s;
}

RAF_TRACEBUF_INLINE size_t raf_trace_sink_write(raf_trace_sink_t *s, const raf_tracebuf_t *b) {
    if (!s || !s->write || !b || !b->p) return 0u;
    return s->write(s->ctx, b->p, b->len);
}

RAF_TRACEBUF_INLINE size_t raf_trace_sink_event_text(
    raf_trace_sink_t *s,
    raf_tracebuf_t *b,
    const raf_trace_event_t *e
) {
    b->len = 0u;
    b->flags = 0u;
    (void)raf_trace_event_text(b, e);
    if (b->flags & RAF_TRACEBUF_TRUNC) return 0u;
    return raf_trace_sink_write(s, b);
}

RAF_TRACEBUF_INLINE size_t raf_trace_sink_event_bin(
    raf_trace_sink_t *s,
    raf_tracebuf_t *b,
    const raf_trace_event_t *e
) {
    b->len = 0u;
    b->flags = 0u;
    (void)raf_trace_event_bin(b, e);
    if (b->flags & RAF_TRACEBUF_TRUNC) return 0u;
    return raf_trace_sink_write(s, b);
}

#endif
