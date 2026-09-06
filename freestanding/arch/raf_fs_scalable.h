#ifndef RAF_FS_SCALABLE_H
#define RAF_FS_SCALABLE_H

/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: One-stage scalable-vector copy primitive with explicit consumed-lane state.
 * SCOPE: AArch64 SVE and RISC-V V only; no OS ABI, allocation, syscall or runtime helper.
 * PRECONDITIONS: Target enables SVE or V; state points to full struct; src/dst cover requested active lanes.
 * REGISTER_OWNERSHIP: Inline asm owns declared predicate/vector temporaries; scalar state remains caller-owned.
 * CLOBBERS: SVE p0/z0 or RVV v0 plus memory; scalar output register is compiler allocated.
 * MEMORY_ORDER: Ordinary data movement; no ordering fence implied.
 * TAIL_SHADOW: Hardware predicate/VL represents residual directly; exactly one vector stage, no scalar tail and no shadow copy.
 * EVIDENCE: Source implementation; target compile/codegen gates required. CLOSURE_L11/CLOSURE_L12.
 */

#include "../include/raf_fs_types.h"
#include "raf_fs_arch.h"

typedef struct raf_fs_scalable_u32_state {
    void *dst;
    const void *src;
    raf_usize requested_lanes;
    raf_usize consumed_lanes;
} raf_fs_scalable_u32_state;

#if defined(__aarch64__) && defined(__ARM_FEATURE_SVE)
RAF_FS_INLINE void raf_fs_scalable_copy_u32_once(void *opaque) {
    raf_fs_scalable_u32_state *s = (raf_fs_scalable_u32_state *)opaque;
    raf_usize consumed;
    __asm__ __volatile__(
        "whilelo p0.s, xzr, %x[requested]\n\t"
        "cntp %x[consumed], p0, p0.s\n\t"
        "ld1w { z0.s }, p0/z, [%[src]]\n\t"
        "st1w { z0.s }, p0, [%[dst]]"
        : [consumed] "=r"(consumed)
        : [requested] "r"(s->requested_lanes), [src] "r"(s->src), [dst] "r"(s->dst)
        : "p0", "z0", "memory");
    s->consumed_lanes = consumed;
}

#elif defined(__riscv) && defined(__riscv_vector)
RAF_FS_INLINE void raf_fs_scalable_copy_u32_once(void *opaque) {
    raf_fs_scalable_u32_state *s = (raf_fs_scalable_u32_state *)opaque;
    raf_usize consumed;
    __asm__ __volatile__(
        "vsetvli %[consumed], %[requested], e32, m1, ta, ma\n\t"
        "vle32.v v0, (%[src])\n\t"
        "vse32.v v0, (%[dst])"
        : [consumed] "=r"(consumed)
        : [requested] "r"(s->requested_lanes), [src] "r"(s->src), [dst] "r"(s->dst)
        : "v0", "memory");
    s->consumed_lanes = consumed;
}
#else
# error "raf_fs_scalable.h requires AArch64 SVE or RISC-V V"
#endif

#endif
