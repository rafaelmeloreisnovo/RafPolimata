#include "rafbbs_recent_core.h"

static int raf_recent_test_equal(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

int rafbbs_recent_core_test(void)
{
    static const char *const names[] = {
        "run-20261007-000012.txt",
        "run-20261007-000001.txt",
        "manifest-20261007-000099.txt",
        "run-20261007-000006.txt",
        "run-20261007-000004.txt",
        "run-20261007-000010.txt",
        "run-20261007-000003.txt",
        "run-20261007-000008.txt",
        "run-20261007-000002.txt",
        "run-20261007-000011.txt",
        "run-20261007-000005.txt",
        "run-20261007-000007.txt",
        "run-20261007-000009.txt",
        "run-20261007-000012.bin"
    };
    RafRecentCatalog catalog;
    RafLogText out;
    char rendered[512];
    RafU32 i;

    raf_recent_init(&catalog);
    for (i = 0u; i < (RafU32)(sizeof(names) / sizeof(names[0])); i++)
        (void)raf_recent_offer(&catalog, names[i], "run-", ".txt");

    if (catalog.count != 10u || catalog.dropped != 0u) return 1;
    if (!raf_recent_test_equal(
            catalog.names[0], "run-20261007-000003.txt"
        )) return 2;
    if (!raf_recent_test_equal(
            catalog.names[9], "run-20261007-000012.txt"
        )) return 3;

    for (i = 1u; i < catalog.count; i++)
        if (raf_recent_cmp(catalog.names[i - 1u], catalog.names[i]) >= 0)
            return 4;

    raf_log_text_init(&out, rendered, (RafU32)sizeof(rendered));
    raf_recent_render(&out, &catalog);
    if (out.dropped != 0u) return 5;
    if (raf_recent_has_prefix(rendered, "run-20261007-000003.txt\n") == 0u)
        return 6;

    raf_recent_init(&catalog);
    (void)raf_recent_offer(&catalog, "manifest-1.txt", "manifest-", ".txt");
    (void)raf_recent_offer(&catalog, "manifest-2.bin", "manifest-", ".txt");
    if (catalog.count != 1u ||
        !raf_recent_test_equal(catalog.names[0], "manifest-1.txt"))
        return 7;

    {
        char long_name[RAFBBS_RECENT_NAME_CAP + 8u];
        for (i = 0u; i + 1u < (RafU32)sizeof(long_name); i++)
            long_name[i] = 'x';
        long_name[0] = 'r';
        long_name[1] = 'u';
        long_name[2] = 'n';
        long_name[3] = '-';
        long_name[sizeof(long_name) - 5u] = '.';
        long_name[sizeof(long_name) - 4u] = 't';
        long_name[sizeof(long_name) - 3u] = 'x';
        long_name[sizeof(long_name) - 2u] = 't';
        long_name[sizeof(long_name) - 1u] = 0;
        (void)raf_recent_offer(&catalog, long_name, "run-", ".txt");
        if (catalog.dropped == 0u) return 8;
    }

    return 0;
}

#if defined(RAFBBS_RECENT_CORE_TEST_MAIN)
int main(void) { return rafbbs_recent_core_test(); }
#endif
