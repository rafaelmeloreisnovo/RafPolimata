#include "rafbbs_baremetal.h"
#include <string.h>

int main(void) {
    RafBaremetalOut out;
    RafBinManifest manifest;
    RafArchFlags arm64;
    uint32_t state;
    raf_baremetal_out_init(&out);
    raf_baremetal_write(&out, "RAF");
    if (out.pos != 3u) return 1;
    state = raf_hash_failover_state(0u, 0u);
    if ((state & RAFBBS_HASH_TOKEN_VAZIO) == 0u) return 2;
    state = raf_hash_failover_state(1u, 1u);
    if ((state & RAFBBS_HASH_TOKEN_VAZIO) != 0u) return 3;
    manifest = raf_bin_manifest_make(0u, RAF_ARCH_ARM64, 0x11u, 0x22u, state, 0u);
    if (manifest.magic != RAFBBS_BIN_MANIFEST_MAGIC) return 4;
    if (manifest.input_crc32 != 0x11u || manifest.output_crc32 != 0x22u) return 5;
    arm64 = raf_arch_flags(RAF_ARCH_ARM64);
    if (arm64.no_heap != 1u || arm64.no_syscall != 1u || arm64.simd != 1u) return 6;
    return 0;
}
