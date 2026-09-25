#include "../Pkc/pkc_wordcode.h"

/* Object-only purity probe: no hosted main, no I/O, no runtime calls. */
pkc_i32 pkc_probe(
    const char *src, pkc_u32 len, pkc_u8 lang,
    PkcInsnV1 *out, pkc_u32 cap, pkc_u32 *count,
    PkcStatusV1 *status)
{
    return pkc_parse_wordcode_v1(src, len, lang, out, cap, count, status);
}
