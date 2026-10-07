#ifndef RAFBBS_FORMAT_CORE_H
#define RAFBBS_FORMAT_CORE_H

#include "rafbbs_log_core.h"

/*
 * Closed, typed formatter primitives for RafBBS operational text.
 *
 * Deliberately not printf-compatible: only the value families currently used
 * by RafBBS are represented. This keeps the grammar finite, typed and
 * freestanding instead of importing a general hosted formatter.
 */

static inline RafU32 raf_format_cstr_len(const char *text)
{
    RafU32 n = 0u;
    if (text == (const char *)0) return 0u;
    while (text[n] != 0) n++;
    return n;
}

static inline void raf_format_hex32_fixed8(RafLogText *out, RafU32 value)
{
    static const char hex[] = "0123456789abcdef";
    RafU32 shift = 32u;
    while (shift > 0u) {
        shift -= 4u;
        raf_log_text_putc(out, hex[(value >> shift) & 0x0fu]);
    }
}

static inline void raf_format_i32(RafLogText *out, RafI32 value)
{
    char digits[10];
    RafU32 count = 0u;
    RafU32 magnitude;

    if (value < 0) {
        raf_log_text_putc(out, '-');
        magnitude = 0u - (RafU32)value;
    } else {
        magnitude = (RafU32)value;
    }

    do {
        RafU32 rem = 0u;
        RafU64 quotient = raf_log_u64_divmod_u32((RafU64)magnitude, 10u, &rem);
        digits[count++] = (char)('0' + (char)rem);
        magnitude = (RafU32)quotient;
    } while (magnitude != 0u && count < (RafU32)sizeof(digits));

    while (count > 0u)
        raf_log_text_putc(out, digits[--count]);
}

static inline void raf_format_prefixed_text(
    RafLogText *out,
    const char *prefix,
    const char *value
) {
    raf_log_text_puts(out, prefix);
    raf_log_text_puts(out, value);
}

static inline void raf_format_prefixed_i32(
    RafLogText *out,
    const char *prefix,
    RafI32 value
) {
    raf_log_text_puts(out, prefix);
    raf_format_i32(out, value);
}

static inline void raf_format_prefixed_hex32(
    RafLogText *out,
    const char *prefix,
    RafU32 value
) {
    raf_log_text_puts(out, prefix);
    raf_format_hex32_fixed8(out, value);
}

#endif
