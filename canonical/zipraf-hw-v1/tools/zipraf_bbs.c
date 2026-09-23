#include <stdio.h>
#include "../include/zipraf_hw.h"

static void banner(void) {
    puts("\x1b[36m+--------------------------------------------------+");
    puts("| \x1b[33mZIPRAF HW/FS // RAFAELIA CAPABILITY BOARD V1\x1b[36m      |");
    puts("| \x1b[32mASM + C + Rust // evidence before claim\x1b[36m            |");
    puts("+--------------------------------------------------+\x1b[0m");
}

int main(void) {
    banner();
    puts("[1] Hardware capability math");
    puts("[2] Crypto provider registry (15 algorithms)");
    puts("[3] ZIPRAF-FS mixed permissions");
    puts("[4] Receipts / provenance / rollback");
    puts("[5] Run selftest externally");
    puts("");
    printf("example: 256-bit / 32-bit = %u lanes\n", zh_vector_lanes(256u, 32u));
    printf("example: 1-byte semantic change on 64-byte granule = %u.%04ux write amplification\n",
           zh_write_amplification_q16(1u,64u) >> 16,
           ((zh_write_amplification_q16(1u,64u) & 0xffffu) * 10000u) >> 16);
    return 0;
}
