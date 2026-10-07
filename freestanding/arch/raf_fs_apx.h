#ifndef RAF_FS_APX_H
#define RAF_FS_APX_H

/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Bounded Intel APX extended-GPR primitives proving R16..R31 addressability.
 * SCOPE: x86-64 APX_F only; integer EGPR ownership, no OS ABI or runtime enablement.
 * PRECONDITIONS: compiler/assembler target has APX_F enabled; caller owns readable scalar inputs and writable destinations.
 * REGISTER_OWNERSHIP: primitive owns R16/R17 or R31 only for the inline-assembly interval.
 * CLOBBERS: declared EGPRs, condition codes for add, and destination memory.
 * MEMORY_ORDER: ordinary scalar load/store; no fence implied.
 * TAIL_SHADOW: no loop, residual tail, retry or shadow state.
 * EVIDENCE: source + exact APX object/codegen gate; physical CPUID/XSTATE/runtime remains CLOSURE_L12.
 */

#include "../include/raf_fs_types.h"

#if !defined(__x86_64__) || !(defined(__APX_F__) || defined(__EGPR__))
# error "raf_fs_apx.h requires x86-64 with Intel APX extended GPR support"
#endif

#ifndef RAF_FS_APX_INLINE
# define RAF_FS_APX_INLINE static __inline__ __attribute__((__always_inline__, __unused__))
#endif

#define RAF_FS_APX_FIRST_EGPR 16u
#define RAF_FS_APX_LAST_EGPR  31u
#define RAF_FS_APX_EGPR_COUNT 16u

RAF_FS_APX_INLINE void
raf_fs_apx_add_u64(void *dst, raf_u64 a, raf_u64 b)
{
    __asm__ __volatile__(
        "movq %1, %%r16\n\t"
        "movq %2, %%r17\n\t"
        "addq %%r17, %%r16\n\t"
        "movq %%r16, (%0)"
        : : "r"(dst), "r"(a), "r"(b)
        : "r16", "r17", "cc", "memory");
}

RAF_FS_APX_INLINE void
raf_fs_apx_copy_r31_u64(void *dst, raf_u64 value)
{
    __asm__ __volatile__(
        "movq %1, %%r31\n\t"
        "movq %%r31, (%0)"
        : : "r"(dst), "r"(value)
        : "r31", "memory");
}

#endif
