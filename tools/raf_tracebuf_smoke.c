#include "Benchmark/raf_tracebuf.h"

typedef struct sink_mem_s {
    uint8_t *p;
    size_t cap;
    size_t len;
} sink_mem_t;

static size_t sink_mem_write(void *ctx, const uint8_t *data, size_t len) {
    sink_mem_t *m = (sink_mem_t *)ctx;
    size_t n = 0u;
    if (!m || !data) return 0u;
    while (n < len && m->len < m->cap) {
        m->p[m->len++] = data[n++];
    }
    return n;
}

static int starts(const uint8_t *p, const char *s) {
    size_t i = 0u;
    while (s[i]) {
        if (p[i] != (uint8_t)s[i]) return 0;
        i++;
    }
    return 1;
}

int main(void) {
    uint8_t work[160];
    uint8_t out[160];
    sink_mem_t mem;
    raf_tracebuf_t b;
    raf_trace_sink_t s;
    raf_trace_event_t e;

    mem.p = out;
    mem.cap = sizeof(out);
    mem.len = 0u;

    b = raf_tracebuf(work, sizeof(work));
    s = raf_trace_sink(sink_mem_write, &mem);
    e = raf_trace_event(0x52414601u, 0x0000002au, 0x1122334455667788ULL);

    if (e.magic != RAF_TRACEBUF_MAGIC) return 1;
    if (raf_trace_sink_event_text(&s, &b, &e) == 0u) return 2;
    if (!starts(out, "RAFTRACE t=0x")) return 3;

    mem.len = 0u;
    if (raf_trace_sink_event_bin(&s, &b, &e) != sizeof(e)) return 4;

    b = raf_tracebuf(work, 4u);
    if (raf_trace_sink_event_text(&s, &b, &e) != 0u) return 5;
    if ((b.flags & RAF_TRACEBUF_TRUNC) == 0u) return 6;

    return 0;
}
