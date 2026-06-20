#ifndef RAF_TRACEBUF_H
#define RAF_TRACEBUF_H

#include <stddef.h>
#include <stdint.h>

#define RAF_TRACEBUF_MAGIC 0x5241465452414345ULL
#define RAF_TRACEBUF_TEXT 1u
#define RAF_TRACEBUF_BIN 2u
#define RAF_TRACEBUF_TRUNC 0x80000000u

#if defined(__GNUC__) || defined(__clang__)
#define RAF_TRACEBUF_INLINE static inline __attribute__((always_inline))
#else
#define RAF_TRACEBUF_INLINE static inline
#endif

typedef struct raf_tracebuf_s {
    uint8_t *p;
    size_t cap;
    size_t len;
    uint32_t flags;
} raf_tracebuf_t;

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
#if defined(__aarch64__) && (defined(__GNUC__) || defined(__clang__))
    uint64_t v;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(v));
    return v;
#else
    return 0u;
#endif
}

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

RAF_TRACEBUF_INLINE char raf_trace_hex4(uint8_t x) {
    static const char lut[16] = "0123456789abcdef";
    return lut[x & 15u];
}

RAF_TRACEBUF_INLINE size_t raf_trace_hex32(raf_tracebuf_t *b, uint32_t v) {
    size_t w = 0u;
    for (unsigned i = 0u; i < 8u; i++) {
        const unsigned sh = (7u - i) * 4u;
        w += raf_trace_u8(b, (uint8_t)raf_trace_hex4((uint8_t)(v >> sh)));
    }
    return w;
}

RAF_TRACEBUF_INLINE size_t raf_trace_hex64(raf_tracebuf_t *b, uint64_t v) {
    size_t w = 0u;
    for (unsigned i = 0u; i < 16u; i++) {
        const unsigned sh = (15u - i) * 4u;
        w += raf_trace_u8(b, (uint8_t)raf_trace_hex4((uint8_t)(v >> sh)));
    }
    return w;
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
