#include "rafbbs_freestanding.h"
#include "rafbbs_crc32.h"
#include "rafbbs_sha256.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    RafWatchdog w = raf_watchdog_start(2u);
    RafRollbackRing r;
    RafRollbackFrame a;
    RafRollbackFrame b;
    unsigned char digest[32];
    char hex[65];
    RafSha256 sha;
    memset(&r, 0, sizeof(r));
    a.step = 1u; a.status = 2u; a.input_crc32 = 3u; a.output_crc32 = 4u;
    b.step = 5u; b.status = 6u; b.input_crc32 = 7u; b.output_crc32 = 8u;
    if (raf_watchdog_step(&w) != 0u) return 1;
    if (raf_watchdog_step(&w) == 0u) return 2;
    raf_rollback_push(&r, a);
    raf_rollback_push(&r, b);
    if (raf_rollback_last(&r).step != 5u) return 3;
    raf_sha256_init(&sha);
    raf_sha256_update(&sha, (const unsigned char *)"abc", 3u);
    raf_sha256_final(&sha, digest);
    raf_sha256_hex(digest, hex);
    if (strcmp(hex, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") != 0) return 4;
    puts("PASS rafbbs failsafe/failover/rollback smoke");
    return 0;
}
