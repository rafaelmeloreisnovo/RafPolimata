#ifndef RAFBBS_MANIFEST_BIN_H
#define RAFBBS_MANIFEST_BIN_H

/*
 * Hosted persistence adapter.
 * Canonical binary layout belongs to rafbbs_manifest_bin_core.h.
 */
#include <stdio.h>
#include "rafbbs_manifest_bin_core.h"

static inline int
raf_write_bin_manifest_file(const char *path, const RafBinManifest *m)
{
    RafU8 wire[RAFBBS_BIN_MANIFEST_V1_SIZE];
    FILE *f;

    if (raf_bin_manifest_encode_v1(
            wire,
            RAFBBS_BIN_MANIFEST_V1_SIZE,
            m
        ) != 0)
        return -1;

    f = fopen(path, "wb");
    if (!f)
        return -1;

    if (fwrite(
            wire,
            1u,
            (size_t)RAFBBS_BIN_MANIFEST_V1_SIZE,
            f
        ) != (size_t)RAFBBS_BIN_MANIFEST_V1_SIZE) {
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}

#endif
