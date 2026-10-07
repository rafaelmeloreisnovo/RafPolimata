#include "rafbbs_exec_core.h"

static int raf_exec_test_equal(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

int rafbbs_exec_core_test(void)
{
    static const char *const good_argv[] = {
        "cc", "-std=c11", "-Wall", "file.c", "-o", "/tmp/file", (const char *)0
    };
    static const char *const empty_arg[] = {
        "cc", "", (const char *)0
    };
    static const char *const missing_term[] = {
        "tool", "arg", "not-null"
    };
    RafExecSpec good = {good_argv, 6u};
    RafExecSpec bad_empty = {empty_arg, 2u};
    RafExecSpec bad_term = {missing_term, 2u};
    RafLogText out;
    char buf[128];
    char small[8];

    if (raf_exec_spec_valid(&good) == 0u) return 1;
    if (raf_exec_spec_valid(&bad_empty) != 0u) return 2;
    if (raf_exec_spec_valid(&bad_term) != 0u) return 3;

    raf_log_text_init(&out, buf, (RafU32)sizeof(buf));
    raf_exec_render(&out, &good);
    if (out.dropped != 0u ||
        !raf_exec_test_equal(
            buf, "cc -std=c11 -Wall file.c -o /tmp/file"
        ))
        return 4;

    raf_log_text_init(&out, small, (RafU32)sizeof(small));
    raf_exec_render(&out, &good);
    if (out.dropped == 0u || small[7] != 0) return 5;

    {
        RafExecSpec zero = {(const char *const *)0, 0u};
        raf_log_text_init(&out, buf, (RafU32)sizeof(buf));
        raf_exec_render(&out, &zero);
        if (buf[0] != 0 || out.pos != 0u) return 6;
    }

    return 0;
}

#if defined(RAFBBS_EXEC_CORE_TEST_MAIN)
int main(void) { return rafbbs_exec_core_test(); }
#endif
