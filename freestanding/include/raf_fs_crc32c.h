#ifndef RAF_FS_CRC32C_H
#define RAF_FS_CRC32C_H

/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Portable CRC32C (Castagnoli reflected polynomial) for caller-owned byte spans.
 * SCOPE: Freestanding C11 bytewise reference kernel; no libc, allocation, syscall or runtime state.
 * PRECONDITIONS: data addresses at least len readable bytes when len > 0;
 *                NULL is allowed only when len == 0; prior is the finalized CRC.
 * REGISTER_OWNERSHIP: Compiler-managed scalar registers; caller owns input and prior CRC.
 * CLOBBERS: NONE; input bytes are read only.
 * MEMORY_ORDER: No synchronization; caller owns publication.
 * TAIL_SHADOW: Explicit one-byte CRC algorithm, not a vector tail or hidden fallback;
 *              no shadow allocation or retry.
 * EVIDENCE: Source candidate; synthetic KAT and freestanding codegen gate required.
 */
#include "raf_fs_types.h"

static inline __attribute__((always_inline)) raf_u32
raf_fs_crc32c_update(const raf_u8 *data, raf_usize len, raf_u32 prior)
{
    raf_u32 crc = ~prior;
    for (raf_usize i = 0; i < len; ++i) {
        crc ^= (raf_u32)data[i];
        for (raf_u32 bit = 0u; bit < 8u; ++bit) {
            raf_u32 mask = (raf_u32)0u - (crc & 1u);
            crc = (crc >> 1) ^ (0x82F63B78u & mask);
        }
    }
    return ~crc;
}
#endif
