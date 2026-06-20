#ifndef RAFBBS_MANIFEST_BIN_H
#define RAFBBS_MANIFEST_BIN_H
#include <stdio.h>
#include "rafbbs_baremetal.h"

static inline int raf_write_bin_manifest_file(const char *path, const RafBinManifest *m) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    if (fwrite(m, sizeof(*m), 1, f) != 1) { fclose(f); return -1; }
    fclose(f);
    return 0;
}
#endif
