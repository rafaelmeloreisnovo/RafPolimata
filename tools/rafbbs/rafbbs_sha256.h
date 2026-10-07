#ifndef RAFBBS_SHA256_H
#define RAFBBS_SHA256_H

/* Hosted file adapter. Pure algorithm lives in rafbbs_sha256_core.h. */
#include <stdio.h>
#include "rafbbs_sha256_core.h"

static inline int raf_sha256_file(const char *path, char out[65]) {
    unsigned char buf[4096];
    unsigned char digest[32];
    RafSha256 sha;
    FILE *f = fopen(path, "rb");

    if (!f) return -1;
    raf_sha256_init(&sha);
    for (;;) {
        size_t n = fread(buf, 1u, sizeof(buf), f);
        if (n) raf_sha256_update(&sha, buf, (RafU32)n);
        if (n < sizeof(buf)) break;
    }
    if (ferror(f)) {
        fclose(f);
        return -1;
    }
    fclose(f);
    raf_sha256_final(&sha, digest);
    raf_sha256_hex(digest, out);
    return 0;
}
#endif
