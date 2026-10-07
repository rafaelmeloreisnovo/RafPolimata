#ifndef RAFBBS_LOG_CORE_H
#define RAFBBS_LOG_CORE_H

#include "rafbbs_types.h"
#include "rafbbs_status.h"

#define RAFBBS_LOG_LINE 256u

typedef struct {
    char *buf;
    RafU32 cap;
    RafU32 pos;
    RafU32 dropped;
} RafLogText;

static inline void raf_log_text_init(RafLogText *out, char *buf, RafU32 cap) {
    out->buf = buf; out->cap = cap; out->pos = 0u; out->dropped = 0u;
    if (cap > 0u) buf[0] = 0;
}
static inline void raf_log_text_putc(RafLogText *out, char value) {
    if (out->cap > 0u && out->pos + 1u < out->cap) {
        out->buf[out->pos++] = value; out->buf[out->pos] = 0;
    } else out->dropped++;
}
static inline void raf_log_text_puts(RafLogText *out, const char *text) {
    if (text == (const char *)0) return;
    while (*text) raf_log_text_putc(out, *text++);
}
static inline void raf_log_text_pad_right(RafLogText *out, const char *text, RafU32 width) {
    RafU32 count = 0u;
    if (text != (const char *)0) while (*text) { raf_log_text_putc(out, *text++); count++; }
    while (count < width) { raf_log_text_putc(out, ' '); count++; }
}
/* Authorial bitwise division avoids 64-bit runtime helpers on 32-bit targets. */
static inline RafU64 raf_log_u64_divmod_u32(RafU64 value, RafU32 divisor, RafU32 *remainder) {
    RafU64 quotient = 0ull, rem = 0ull; RafU32 bit = 64u;
    while (bit > 0u) {
        bit--; rem = (rem << 1u) | ((value >> bit) & 1ull);
        if (rem >= (RafU64)divisor) { rem -= (RafU64)divisor; quotient |= (1ull << bit); }
    }
    *remainder = (RafU32)rem; return quotient;
}
static inline void raf_log_text_u64_min(RafLogText *out, RafU64 value, RafU32 width) {
    char digits[20]; RafU32 count = 0u;
    do { RafU32 rem = 0u; value = raf_log_u64_divmod_u32(value, 10u, &rem); digits[count++] = (char)('0' + (char)rem); }
    while (value != 0ull && count < (RafU32)sizeof(digits));
    while (count < width) { raf_log_text_putc(out, '0'); width--; }
    while (count > 0u) raf_log_text_putc(out, digits[--count]);
}
static inline void raf_log_line_render(RafLogText *out, RafStatus status, const char *module, const char *detail, RafU64 elapsed_ms, RafU32 elapsed_valid) {
    if (elapsed_valid == 0u) raf_log_text_puts(out, "--:--.---");
    else {
        RafU32 ms_rem = 0u, sec_rem = 0u;
        RafU64 total_seconds = raf_log_u64_divmod_u32(elapsed_ms, 1000u, &ms_rem);
        RafU64 minutes = raf_log_u64_divmod_u32(total_seconds, 60u, &sec_rem);
        raf_log_text_u64_min(out, minutes, 2u); raf_log_text_putc(out, ':');
        raf_log_text_u64_min(out, (RafU64)sec_rem, 2u); raf_log_text_putc(out, '.');
        raf_log_text_u64_min(out, (RafU64)ms_rem, 3u);
    }
    raf_log_text_putc(out, ' '); raf_log_text_pad_right(out, raf_status_name(status), 12u);
    raf_log_text_putc(out, ' '); raf_log_text_pad_right(out, module, 10u);
    raf_log_text_putc(out, ' '); raf_log_text_puts(out, detail);
}
#endif
