#include "rafbbs_manifest_bin_core.h"

int rafbbs_manifest_bin_core_test(void)
{
    RafBinManifest src;
    RafBinManifest dst;
    RafU8 wire[RAFBBS_BIN_MANIFEST_V1_SIZE];
    RafU32 i;

    src = raf_bin_manifest_make(
        0x01020304u,
        RAF_ARCH_ARM64,
        0x11223344u,
        0xaabbccddu,
        RAFBBS_HASH_SHA256_OK | RAFBBS_HASH_CRC32_OK,
        1u
    );

    for (i = 0u; i < 32u; i++) {
        src.input_sha256[i] = (RafU8)i;
        src.output_sha256[i] = (RafU8)(31u - i);
    }

    if (raf_bin_manifest_encode_v1(
            wire,
            RAFBBS_BIN_MANIFEST_V1_SIZE,
            &src
        ) != 0)
        return 1;

    /* Canonical LE bytes, including historical LE magic representation. */
    if (wire[0] != 0x4du || wire[1] != 0x46u ||
        wire[2] != 0x42u || wire[3] != 0x52u)
        return 2;
    if (wire[16] != 0x44u || wire[17] != 0x33u ||
        wire[18] != 0x22u || wire[19] != 0x11u)
        return 3;
    if (wire[20] != 0xddu || wire[21] != 0xccu ||
        wire[22] != 0xbbu || wire[23] != 0xaau)
        return 4;
    if (wire[24] != 0u || wire[55] != 31u)
        return 5;
    if (wire[56] != 31u || wire[87] != 0u)
        return 6;

    if (raf_bin_manifest_decode_v1(
            &dst,
            wire,
            RAFBBS_BIN_MANIFEST_V1_SIZE
        ) != 0)
        return 7;

    if (dst.magic != src.magic ||
        dst.version != src.version ||
        dst.status != src.status ||
        dst.arch != src.arch ||
        dst.input_crc32 != src.input_crc32 ||
        dst.output_crc32 != src.output_crc32 ||
        dst.hash_state != src.hash_state ||
        dst.gaps != src.gaps)
        return 8;

    for (i = 0u; i < 32u; i++) {
        if (dst.input_sha256[i] != src.input_sha256[i])
            return 9;
        if (dst.output_sha256[i] != src.output_sha256[i])
            return 10;
    }

    if (raf_bin_manifest_encode_v1(
            wire,
            RAFBBS_BIN_MANIFEST_V1_SIZE - 1u,
            &src
        ) == 0)
        return 11;

    if (raf_bin_manifest_decode_v1(
            &dst,
            wire,
            RAFBBS_BIN_MANIFEST_V1_SIZE - 1u
        ) == 0)
        return 12;

    return 0;
}

#ifdef RAFBBS_MANIFEST_BIN_CORE_TEST_MAIN
int main(void)
{
    return rafbbs_manifest_bin_core_test();
}
#endif
