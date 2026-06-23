#ifndef RAF_TRACE_BUFFER_H
#define RAF_TRACE_BUFFER_H

#include "Benchmark/raf_trace_flags.h"

typedef struct raf_tracebuf_s {
    uint8_t *p;
    size_t cap;
    size_t len;
    uint32_t flags;
} raf_tracebuf_t;

RAF_TRACEBUF_INLINE raf_tracebuf_t raf_tracebuf(void *p, size_t cap) {
    raf_tracebuf_t b;
    b.p = (uint8_t *)p;
    b.cap = cap;
    b.len = 0u;
    b.flags = 0u;
    return b;
}

RAF_TRACEBUF_INLINE uint32_t raf_trace_room(size_t len, size_t add, size_t cap) {
    return (uint32_t)((add <= cap) & (len <= (cap - add)));
}

RAF_TRACEBUF_INLINE size_t raf_trace_u8(raf_tracebuf_t *b, uint8_t x) {
    const uint32_t ok = raf_trace_room(b->len, 1u, b->cap);
    if (ok) {
        b->p[b->len] = x;
        b->len += 1u;
    } else {
        b->flags |= RAF_TRACEBUF_TRUNC;
    }
    return (size_t)ok;
}

RAF_TRACEBUF_INLINE size_t raf_trace_bytes(raf_tracebuf_t *b, const void *src, size_t n) {
    const uint8_t *s = (const uint8_t *)src;
    size_t w = 0u;
    for (size_t i = 0u; i < n; i++) w += raf_trace_u8(b, s[i]);
    return w;
}

RAF_TRACEBUF_INLINE size_t raf_trace_cstr(raf_tracebuf_t *b, const char *s) {
    size_t w = 0u;
    if (!s) return 0u;
    while (*s) {
        w += raf_trace_u8(b, (uint8_t)*s);
        s++;
    }
    return w;
}

#endif
