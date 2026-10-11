#ifndef RAF_FS_BINARY_VIEW_H
#define RAF_FS_BINARY_VIEW_H
/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Bound-checked byte views shared by ELF and DEX inspection.
 * SCOPE: Pure scalar fixed-width decoding; no runtime or hosted headers.
 * PRECONDITIONS: Caller owns readable bytes for the declared length; callers
 *              check ranges before using the unchecked byte-load helpers.
 * REGISTER_OWNERSHIP: Compiler scalar registers only.
 * CLOBBERS: NONE; readonly input.
 * MEMORY_ORDER: Caller owns publication and synchronization.
 * TAIL_SHADOW: Explicit byte offsets only; no copies, retries or shadow state.
 * EVIDENCE: Synthetic host and target object gate, not runtime validation.
 */
#include "raf_fs_types.h"
static inline __attribute__((always_inline)) raf_u32
raf_fs_bv_range(raf_usize length, raf_u64 offset, raf_u64 size)
{
    raf_u64 n = (raf_u64)length;
    return (raf_u32)(offset <= n && size <= n - offset);
}
static inline __attribute__((always_inline)) raf_u16
raf_fs_bv_u16(const raf_u8 *p, raf_u32 be)
{
    return (raf_u16)(be ? (((raf_u16)p[0] << 8) | p[1])
                          : ((raf_u16)p[0] | ((raf_u16)p[1] << 8)));
}
static inline __attribute__((always_inline)) raf_u32
raf_fs_bv_u32(const raf_u8 *p, raf_u32 be)
{
    if (be) return ((raf_u32)p[0]<<24) | ((raf_u32)p[1]<<16)
                    | ((raf_u32)p[2]<<8) | (raf_u32)p[3];
    return (raf_u32)p[0] | ((raf_u32)p[1]<<8)
        | ((raf_u32)p[2]<<16) | ((raf_u32)p[3]<<24);
}
static inline __attribute__((always_inline)) raf_u64
raf_fs_bv_u64(const raf_u8 *p, raf_u32 be)
{
    return be ? (((raf_u64)raf_fs_bv_u32(p,1u)<<32) |
                  (raf_u64)raf_fs_bv_u32(p+4,1u))
              : ((raf_u64)raf_fs_bv_u32(p,0u) |
                 ((raf_u64)raf_fs_bv_u32(p+4,0u)<<32));
}
#endif
