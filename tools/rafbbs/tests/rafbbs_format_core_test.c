#include "rafbbs_format_core.h"

static int raf_format_test_equal(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

int rafbbs_format_core_test(void)
{
    char buf[128];
    char small[5];
    RafLogText out;

    raf_log_text_init(&out, buf, (RafU32)sizeof(buf));
    raf_format_prefixed_text(&out, "name=", "rafaelia");
    if (!raf_format_test_equal(buf, "name=rafaelia") || out.dropped != 0u) return 1;

    raf_log_text_init(&out, buf, (RafU32)sizeof(buf));
    raf_format_prefixed_i32(&out, "rc=", (RafI32)-2147483647 - 1);
    if (!raf_format_test_equal(buf, "rc=-2147483648") || out.dropped != 0u) return 2;

    raf_log_text_init(&out, buf, (RafU32)sizeof(buf));
    raf_format_prefixed_i32(&out, "rc=", (RafI32)2147483647);
    if (!raf_format_test_equal(buf, "rc=2147483647")) return 3;

    raf_log_text_init(&out, buf, (RafU32)sizeof(buf));
    raf_format_prefixed_hex32(&out, "crc=", 0x0000000au);
    if (!raf_format_test_equal(buf, "crc=0000000a")) return 4;

    raf_log_text_init(&out, buf, (RafU32)sizeof(buf));
    raf_format_prefixed_hex32(&out, "", 0x89abcdefu);
    if (!raf_format_test_equal(buf, "89abcdef")) return 5;

    if (raf_format_cstr_len("abc") != 3u) return 6;
    if (raf_format_cstr_len((const char *)0) != 0u) return 7;

    raf_log_text_init(&out, small, (RafU32)sizeof(small));
    raf_format_prefixed_text(&out, "abcdef", "gh");
    if (out.dropped == 0u || small[4] != 0) return 8;

    return 0;
}

#if defined(RAFBBS_FORMAT_CORE_TEST_MAIN)
int main(void) { return rafbbs_format_core_test(); }
#endif
