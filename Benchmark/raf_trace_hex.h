#ifndef RAF_TRACE_HEX_H
#define RAF_TRACE_HEX_H

#include "Benchmark/raf_trace_buffer.h"

RAF_TRACEBUF_INLINE char raf_trace_hex4(uint8_t x) {
    /* Indexed 16-byte alphabet: no terminating NUL is needed or stored.
     * An initializer string would be 17 bytes and fails GCC -Werror. */
    static const char lut[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
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

#endif
