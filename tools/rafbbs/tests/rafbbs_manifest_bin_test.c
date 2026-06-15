#include "rafbbs_manifest_bin.h"
#include <stdio.h>
int main(void) {
    RafBinManifest m = raf_bin_manifest_make(1u, RAF_ARCH_GENERIC, 0xabu, 0xcdu, RAFBBS_HASH_TOKEN_VAZIO, 1u);
    FILE *f;
    RafBinManifest r;
    if (raf_write_bin_manifest_file("/tmp/rafbbs_manifest_fixture.bin", &m) != 0) return 1;
    f = fopen("/tmp/rafbbs_manifest_fixture.bin", "rb");
    if (!f) return 2;
    if (fread(&r, sizeof(r), 1, f) != 1) { fclose(f); return 3; }
    fclose(f);
    if (r.magic != RAFBBS_BIN_MANIFEST_MAGIC) return 4;
    if (r.hash_state != RAFBBS_HASH_TOKEN_VAZIO) return 5;
    return 0;
}
