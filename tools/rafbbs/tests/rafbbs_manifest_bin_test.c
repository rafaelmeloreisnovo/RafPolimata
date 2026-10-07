#include "rafbbs_manifest_bin.h"
#include <stdio.h>

int main(void)
{
    RafBinManifest src;
    RafBinManifest dst;
    RafU8 wire[RAFBBS_BIN_MANIFEST_V1_SIZE];
    FILE *f;
    size_t n;

    src = raf_bin_manifest_make(
        1u,
        RAF_ARCH_GENERIC,
        0xabu,
        0xcdu,
        RAFBBS_HASH_TOKEN_VAZIO,
        1u
    );

    if (raf_write_bin_manifest_file(
            "/tmp/rafbbs_manifest_fixture.bin",
            &src
        ) != 0)
        return 1;

    f = fopen("/tmp/rafbbs_manifest_fixture.bin", "rb");
    if (!f)
        return 2;

    n = fread(
        wire,
        1u,
        (size_t)RAFBBS_BIN_MANIFEST_V1_SIZE,
        f
    );
    if (n != (size_t)RAFBBS_BIN_MANIFEST_V1_SIZE) {
        fclose(f);
        return 3;
    }
    if (fgetc(f) != EOF) {
        fclose(f);
        return 4;
    }
    fclose(f);

    if (raf_bin_manifest_decode_v1(
            &dst,
            wire,
            RAFBBS_BIN_MANIFEST_V1_SIZE
        ) != 0)
        return 5;

    if (dst.magic != RAFBBS_BIN_MANIFEST_MAGIC)
        return 6;
    if (dst.hash_state != RAFBBS_HASH_TOKEN_VAZIO)
        return 7;
    if (dst.input_crc32 != 0xabu || dst.output_crc32 != 0xcdu)
        return 8;

    return 0;
}
