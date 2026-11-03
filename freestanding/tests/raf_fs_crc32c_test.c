#include "raf_fs_crc32c.h"

static const raf_u8 known[] = "123456789";
static const raf_u8 zeros[] = {0u, 0u, 0u};

int main(void)
{
    if (raf_fs_crc32c_update(known, 9u, 0u) != 0xe3069283u) return 1;
    if (raf_fs_crc32c_update((const raf_u8 *)0, 0u, 0u) != 0u) return 2;
    if (raf_fs_crc32c_update((const raf_u8 *)0, 0u, 0x12345678u) != 0x12345678u) return 3;
    if (raf_fs_crc32c_update(known + 4, 5u, raf_fs_crc32c_update(known, 4u, 0u))
        != 0xe3069283u) return 4;
    if (raf_fs_crc32c_update(zeros, 3u, 0u) != 0x6064a37au) return 5;
    return 0;
}
