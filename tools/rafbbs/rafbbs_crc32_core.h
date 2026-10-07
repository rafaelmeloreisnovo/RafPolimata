#ifndef RAFBBS_CRC32_CORE_H
#define RAFBBS_CRC32_CORE_H

#include "rafbbs_types.h"

/* Pure CRC32 core: no file I/O, libc, heap, syscall or external runtime. */
static inline RafU32
raf_crc32_update(RafU32 crc, const unsigned char *buf, unsigned long len)
{
    unsigned long i;
    int bit;

    crc = ~crc;
    for (i = 0u; i < len; i++) {
        crc ^= (RafU32)buf[i];
        for (bit = 0; bit < 8; bit++) {
            RafU32 mask = (RafU32)(0u - (crc & 1u));
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

#endif
