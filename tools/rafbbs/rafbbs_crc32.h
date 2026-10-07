#ifndef RAFBBS_CRC32_H
#define RAFBBS_CRC32_H

/* Hosted file adapter. Pure algorithm lives in rafbbs_crc32_core.h. */
#include <stdio.h>
#include "rafbbs_crc32_core.h"

static inline int raf_crc32_file(const char *path, RafU32 *out) {
    unsigned char buf[4096];
    RafU32 crc = 0u;
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    for (;;) {
        size_t n = fread(buf, 1u, sizeof(buf), f);
        if (n) crc = raf_crc32_update(crc, buf, (unsigned long)n);
        if (n < sizeof(buf)) break;
    }
    if (ferror(f)) {
        fclose(f);
        return -1;
    }
    fclose(f);
    *out = crc;
    return 0;
}
#endif
