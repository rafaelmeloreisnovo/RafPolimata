#ifndef RAFBBS_CRC32_H
#define RAFBBS_CRC32_H
#include <stdint.h>
#include <stdio.h>

static inline uint32_t raf_crc32_update(uint32_t crc, const unsigned char *buf, unsigned long len) {
    unsigned long i;
    int bit;
    crc = ~crc;
    for (i = 0; i < len; i++) {
        crc ^= (uint32_t)buf[i];
        for (bit = 0; bit < 8; bit++) {
            uint32_t mask = (uint32_t)(0U - (crc & 1U));
            crc = (crc >> 1) ^ (0xEDB88320U & mask);
        }
    }
    return ~crc;
}

static inline int raf_crc32_file(const char *path, uint32_t *out) {
    unsigned char buf[4096];
    uint32_t crc = 0;
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    for (;;) {
        size_t n = fread(buf, 1, sizeof(buf), f);
        if (n) crc = raf_crc32_update(crc, buf, (unsigned long)n);
        if (n < sizeof(buf)) break;
    }
    if (ferror(f)) { fclose(f); return -1; }
    fclose(f);
    *out = crc;
    return 0;
}
#endif
