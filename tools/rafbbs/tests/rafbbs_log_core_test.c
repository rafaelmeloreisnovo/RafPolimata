#include "rafbbs_log_core.h"
static int raf_log_test_equal(const char *a, const char *b) { while (*a && *b && *a == *b) { a++; b++; } return *a == *b; }
int rafbbs_log_core_test(void) {
    char buf[128], small[8]; RafLogText out;
    raf_log_text_init(&out, buf, (RafU32)sizeof(buf));
    raf_log_line_render(&out, RAF_INFO, "boot", "ok", 1ull, 1u);
    if (!raf_log_test_equal(buf, "00:00.001 INFO         boot       ok") || out.dropped != 0u) return 1;
    raf_log_text_init(&out, buf, (RafU32)sizeof(buf));
    raf_log_line_render(&out, RAF_PASS, "x", "d", 0ull, 0u);
    if (!raf_log_test_equal(buf, "--:--.--- PASS         x          d")) return 2;
    raf_log_text_init(&out, buf, (RafU32)sizeof(buf));
    raf_log_line_render(&out, RAF_HASH, "m", "z", 7384005ull, 1u);
    if (!raf_log_test_equal(buf, "123:04.005 HASH         m          z")) return 3;
    raf_log_text_init(&out, small, (RafU32)sizeof(small));
    raf_log_line_render(&out, RAF_PASS, "x", "d", 0ull, 0u);
    if (out.dropped == 0u || small[7] != 0) return 4;
    return 0;
}
#if defined(RAFBBS_LOG_TEST_MAIN)
int main(void) { return rafbbs_log_core_test(); }
#endif
