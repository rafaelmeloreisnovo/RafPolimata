#ifndef RAF_FS_CORE_H
#define RAF_FS_CORE_H

#include "raf_fs_types.h"
#include "../arch/raf_fs_arch.h"

/* Branchless scalar masks. cond must be 0 or 1. */
#define RAF_FS_MASK32(cond01) ((raf_u32)0u - (raf_u32)(cond01))
#define RAF_FS_MASK64(cond01) ((raf_u64)0ull - (raf_u64)(cond01))
#define RAF_FS_SELECT32(mask, yes, no) ((((raf_u32)(yes)) & (raf_u32)(mask)) | (((raf_u32)(no)) & ~(raf_u32)(mask)))
#define RAF_FS_SELECT64(mask, yes, no) ((((raf_u64)(yes)) & (raf_u64)(mask)) | (((raf_u64)(no)) & ~(raf_u64)(mask)))

/*
 * Fixed-width operations. GCC/Clang statement expressions deliberately avoid
 * the conventional `do { ... } while (0)` macro shell: there is no synthetic loop
 * even in the source representation. Residual ownership stays with the caller.
 */
#define RAF_FS_COPY_U32X1(dst, src) __extension__ ({ \
    *(raf_u32 *)(dst) = *(const raf_u32 *)(src); \
    (void)0; \
})
#define RAF_FS_COPY_U32X2(dst, src) __extension__ ({ \
    ((raf_u32 *)(dst))[0] = ((const raf_u32 *)(src))[0]; \
    ((raf_u32 *)(dst))[1] = ((const raf_u32 *)(src))[1]; \
    (void)0; \
})
#define RAF_FS_COPY_U32X4(dst, src) __extension__ ({ \
    RAF_FS_COPY_U32X2((dst), (src)); \
    RAF_FS_COPY_U32X2(((raf_u32 *)(dst)) + 2, ((const raf_u32 *)(src)) + 2); \
    (void)0; \
})
#define RAF_FS_COPY_U32X8(dst, src) __extension__ ({ \
    RAF_FS_COPY_U32X4((dst), (src)); \
    RAF_FS_COPY_U32X4(((raf_u32 *)(dst)) + 4, ((const raf_u32 *)(src)) + 4); \
    (void)0; \
})

#define RAF_FS_ZERO_U32X1(dst) __extension__ ({ ((raf_u32 *)(dst))[0] = 0u; (void)0; })
#define RAF_FS_ZERO_U32X2(dst) __extension__ ({ RAF_FS_ZERO_U32X1(dst); RAF_FS_ZERO_U32X1(((raf_u32 *)(dst)) + 1); (void)0; })
#define RAF_FS_ZERO_U32X4(dst) __extension__ ({ RAF_FS_ZERO_U32X2(dst); RAF_FS_ZERO_U32X2(((raf_u32 *)(dst)) + 2); (void)0; })
#define RAF_FS_ZERO_U32X8(dst) __extension__ ({ RAF_FS_ZERO_U32X4(dst); RAF_FS_ZERO_U32X4(((raf_u32 *)(dst)) + 4); (void)0; })

typedef struct raf_fs_residual {
    raf_u32 lane_count;
    raf_u32 lane_mask;
} raf_fs_residual;

RAF_FS_INLINE raf_fs_residual raf_fs_residual32(raf_u32 remaining) {
    raf_fs_residual r;
    raf_u32 n = remaining & 31u;
    raf_u32 nz = (raf_u32)(n != 0u);
    raf_u32 safe_n = n | (raf_u32)(nz ^ 1u);
    raf_u32 full = (raf_u32)((1ull << safe_n) - 1ull);
    r.lane_count = n;
    r.lane_mask = full & RAF_FS_MASK32(nz);
    return r;
}

/* Caller owns state; void avoids return-object traffic and external ABI symbols. */
#define RAF_FS_STAGE(name) RAF_FS_INLINE void name(void *state)

#endif
