#include "rafbbs_freestanding.h"
#include "rafbbs_baremetal.h"
#include "rafbbs_crc32_core.h"
#include "rafbbs_sha256_core.h"

int rafbbs_zero_dependency_test(void)
{
    static const unsigned char abc[] = {'a','b','c'};
    static const unsigned char digits[] = {'1','2','3','4','5','6','7','8','9'};
    static const char expected[] =
        "ba7816bf8f01cfea414140de5dae2223"
        "b00361a396177a9cb410ff61f20015ad";
    RafSha256 sha;
    unsigned char digest[32];
    char hex[65];
    RafBaremetalOut out;
    RafWatchdog watchdog = raf_watchdog_start(1u);
    RafU32 i;

    raf_sha256_init(&sha);
    raf_sha256_update(&sha, abc, 3u);
    raf_sha256_final(&sha, digest);
    raf_sha256_hex(digest, hex);

    for (i = 0u; i < 64u; i++)
        if (hex[i] != expected[i]) return 1;

    if (raf_crc32_update(0u, digits, 9u) != 0xcbf43926u) return 2;

    raf_baremetal_out_init(&out);
    raf_baremetal_write(&out, "RAF");
    if (out.pos != 3u) return 3;

    return (int)raf_watchdog_step(&watchdog) - 1;
}
