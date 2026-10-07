#include "rafbbs_runid_core.h"

static int raf_runid_eq(const char *a, const char *b)
{
    RafU32 i = 0u;
    while (a[i] != 0 && b[i] != 0) {
        if (a[i] != b[i])
            return 0;
        ++i;
    }
    return a[i] == b[i];
}

int rafbbs_runid_core_test(void)
{
    char out[RAFBBS_RUN_ID_CAP];
    char short_out[8];
    RafCivilStamp stamp;

    stamp.year = 2026;
    stamp.month = 10u;
    stamp.day = 7u;
    stamp.hour = 9u;
    stamp.minute = 21u;
    stamp.second = 5u;
    stamp.valid = 1u;
    if (!raf_runid_render(out, (RafU32)sizeof(out), &stamp) ||
        !raf_runid_eq(out, "20261007-092105"))
        return 1;

    stamp.year = 2024;
    stamp.month = 2u;
    stamp.day = 29u;
    if (!raf_runid_render(out, (RafU32)sizeof(out), &stamp))
        return 2;

    stamp.year = 2025;
    if (raf_runid_render(out, (RafU32)sizeof(out), &stamp) != 0u)
        return 3;

    stamp.year = 2000;
    if (!raf_civil_is_leap(stamp.year))
        return 4;
    stamp.year = 1900;
    if (raf_civil_is_leap(stamp.year))
        return 5;

    stamp.year = 2026;
    stamp.month = 13u;
    stamp.day = 1u;
    if (raf_runid_render(out, (RafU32)sizeof(out), &stamp) != 0u)
        return 6;

    stamp.month = 10u;
    if (raf_runid_render(short_out, (RafU32)sizeof(short_out), &stamp) != 0u)
        return 7;

    stamp.valid = 0u;
    if (raf_runid_render(out, (RafU32)sizeof(out), &stamp) != 0u)
        return 8;

    return 0;
}

#ifdef RAFBBS_RUNID_CORE_TEST_MAIN
int main(void)
{
    return rafbbs_runid_core_test();
}
#endif
