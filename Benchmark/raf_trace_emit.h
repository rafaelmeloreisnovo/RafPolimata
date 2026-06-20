#ifndef RAF_TRACE_EMIT_H
#define RAF_TRACE_EMIT_H

#include "Benchmark/raf_trace_event.h"
#include "Benchmark/raf_trace_hex.h"

RAF_TRACEBUF_INLINE size_t raf_trace_event_bin(raf_tracebuf_t *b, const raf_trace_event_t *e) {
    b->flags |= RAF_TRACEBUF_BIN;
    return raf_trace_bytes(b, e, sizeof(*e));
}

RAF_TRACEBUF_INLINE size_t raf_trace_event_text(raf_tracebuf_t *b, const raf_trace_event_t *e) {
    size_t w = 0u;
    b->flags |= RAF_TRACEBUF_TEXT;
    w += raf_trace_cstr(b, "RAFTRACE t=0x");
    w += raf_trace_hex64(b, e->tick);
    w += raf_trace_cstr(b, " tag=0x");
    w += raf_trace_hex32(b, e->tag);
    w += raf_trace_cstr(b, " code=0x");
    w += raf_trace_hex32(b, e->code);
    w += raf_trace_cstr(b, " value=0x");
    w += raf_trace_hex64(b, e->value);
    w += raf_trace_u8(b, (uint8_t)'\n');
    return w;
}

#endif
