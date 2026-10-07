#ifndef RAF_FS_VECTOR_H
#define RAF_FS_VECTOR_H

/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Fixed-vector, branchless, one-block execution primitives.
 * SCOPE: SSE2, AVX2, AVX-512F, ARMv7 NEON, AArch64 Advanced SIMD and IBM z/s390x z13 vector facility; no OS/runtime contract.
 * PRECONDITIONS: Selected compiler target enables the exact ISA profile; full-block operations expose one full block.
 * REGISTER_OWNERSHIP: Inline assembly owns only declared vector/predicate temporaries; s390x is pinned to VR16..VR19 so FPR0..15 overlap is not touched; C-vector selection owns compiler temporaries elsewhere.
 * CLOBBERS: Declared XMM/YMM/ZMM/K, NEON or s390x VR16..VR19 temporaries plus memory; compiler allocates select temporaries on non-s390x profiles.
 * MEMORY_ORDER: Ordinary data access; no fence implied. Use raf_fs_arch.h ordering primitives separately.
 * TAIL_SHADOW: Full-block paths never create scalar tail; AVX-512 residual may use K-mask bounded memory; other fixed profiles require explicit full accessible block/mask.
 * EVIDENCE: Source implementation; profile compile/codegen gates promote build evidence. CLOSURE_L11/CLOSURE_L12.
 */

#include "../include/raf_fs_types.h"
#include "../include/raf_fs_abi.h"
#ifndef RAF_FS_INLINE
# define RAF_FS_INLINE static __inline__ __attribute__((__always_inline__, __unused__))
#endif

#if defined(__x86_64__) && defined(__AVX512F__)
# define RAF_FS_NATIVE_VECTOR_BITS 512u
# define RAF_FS_NATIVE_VECTOR_BYTES 64u
# define RAF_FS_NATIVE_LANES_U32 16u
typedef raf_u32 raf_fs_native_u32v __attribute__((__vector_size__(64), __may_alias__));

RAF_FS_INLINE void raf_fs_vec_copy_block(void *dst, const void *src) {
    __asm__ __volatile__(
        "vmovdqu64 (%1), %%zmm0\n\t"
        "vmovdqu64 %%zmm0, (%0)"
        : : "r"(dst), "r"(src) : "zmm0", "memory");
}
RAF_FS_INLINE void raf_fs_vec_zero_block(void *dst) {
    __asm__ __volatile__(
        "vpxord %%zmm0, %%zmm0, %%zmm0\n\t"
        "vmovdqu64 %%zmm0, (%0)"
        : : "r"(dst) : "zmm0", "memory");
}
/* lane_mask bit i selects u32 lane i. Inactive memory lanes are not accessed. */
RAF_FS_INLINE void raf_fs_vec_masked_copy_u32(void *dst, const void *src, raf_u32 lane_mask) {
    raf_u32 mask16 = lane_mask & 0xffffu;
    __asm__ __volatile__(
        "kmovw %k2, %%k1\n\t"
        "vmovdqu32 (%1), %%zmm0%{%%k1%}%{z%}\n\t"
        "vmovdqu32 %%zmm0, (%0)%{%%k1%}"
        : : "r"(dst), "r"(src), "r"(mask16) : "k1", "zmm0", "memory");
}

#elif defined(__x86_64__) && defined(__AVX2__)
# define RAF_FS_NATIVE_VECTOR_BITS 256u
# define RAF_FS_NATIVE_VECTOR_BYTES 32u
# define RAF_FS_NATIVE_LANES_U32 8u
typedef raf_u32 raf_fs_native_u32v __attribute__((__vector_size__(32), __may_alias__));

RAF_FS_INLINE void raf_fs_vec_copy_block(void *dst, const void *src) {
    __asm__ __volatile__(
        "vmovdqu (%1), %%ymm0\n\t"
        "vmovdqu %%ymm0, (%0)"
        : : "r"(dst), "r"(src) : "ymm0", "memory");
}
RAF_FS_INLINE void raf_fs_vec_zero_block(void *dst) {
    __asm__ __volatile__(
        "vpxor %%ymm0, %%ymm0, %%ymm0\n\t"
        "vmovdqu %%ymm0, (%0)"
        : : "r"(dst) : "ymm0", "memory");
}

#elif (defined(__x86_64__) || defined(__i386__)) && defined(__SSE2__)
# define RAF_FS_NATIVE_VECTOR_BITS 128u
# define RAF_FS_NATIVE_VECTOR_BYTES 16u
# define RAF_FS_NATIVE_LANES_U32 4u
typedef raf_u32 raf_fs_native_u32v __attribute__((__vector_size__(16), __may_alias__));

RAF_FS_INLINE void raf_fs_vec_copy_block(void *dst, const void *src) {
    __asm__ __volatile__(
        "movdqu (%1), %%xmm0\n\t"
        "movdqu %%xmm0, (%0)"
        : : "r"(dst), "r"(src) : "xmm0", "memory");
}
RAF_FS_INLINE void raf_fs_vec_zero_block(void *dst) {
    __asm__ __volatile__(
        "pxor %%xmm0, %%xmm0\n\t"
        "movdqu %%xmm0, (%0)"
        : : "r"(dst) : "xmm0", "memory");
}

#elif defined(__aarch64__)
# define RAF_FS_NATIVE_VECTOR_BITS 128u
# define RAF_FS_NATIVE_VECTOR_BYTES 16u
# define RAF_FS_NATIVE_LANES_U32 4u
typedef raf_u32 raf_fs_native_u32v __attribute__((__vector_size__(16), __may_alias__));

RAF_FS_INLINE void raf_fs_vec_copy_block(void *dst, const void *src) {
    __asm__ __volatile__(
        "ldr q0, [%1]\n\t"
        "str q0, [%0]"
        : : "r"(dst), "r"(src) : "v0", "memory");
}
RAF_FS_INLINE void raf_fs_vec_zero_block(void *dst) {
    __asm__ __volatile__(
        "movi v0.16b, #0\n\t"
        "str q0, [%0]"
        : : "r"(dst) : "v0", "memory");
}

#elif defined(__arm__) && (__ARM_ARCH >= 7) && (defined(__ARM_NEON) || defined(__ARM_NEON__))
# define RAF_FS_NATIVE_VECTOR_BITS 128u
# define RAF_FS_NATIVE_VECTOR_BYTES 16u
# define RAF_FS_NATIVE_LANES_U32 4u
typedef raf_u32 raf_fs_native_u32v __attribute__((__vector_size__(16), __may_alias__));

RAF_FS_INLINE void raf_fs_vec_copy_block(void *dst, const void *src) {
    __asm__ __volatile__(
        "vld1.8 {d0-d1}, [%1]\n\t"
        "vst1.8 {d0-d1}, [%0]"
        : : "r"(dst), "r"(src) : "d0", "d1", "memory");
}
RAF_FS_INLINE void raf_fs_vec_zero_block(void *dst) {
    __asm__ __volatile__(
        "veor q0, q0, q0\n\t"
        "vst1.8 {d0-d1}, [%0]"
        : : "r"(dst) : "d0", "d1", "memory");
}

#elif defined(__s390x__) && (defined(__VEC__) || defined(__VECTOR__))
# define RAF_FS_NATIVE_VECTOR_BITS 128u
# define RAF_FS_NATIVE_VECTOR_BYTES 16u
# define RAF_FS_NATIVE_LANES_U32 4u
# define RAF_FS_S390X_HIGH_VECTOR_PROFILE 1u
typedef raf_u32 raf_fs_native_u32v __attribute__((__vector_size__(16), __may_alias__));

/*
 * z/Architecture FPR0..15 overlay the low halves of VR0..15. This profile
 * therefore owns only VR16..VR19 and never mixes floating-point operations
 * into the block executor.
 */
RAF_FS_INLINE void raf_fs_vec_copy_block(void *dst, const void *src) {
    __asm__ __volatile__(
        "vl %%v16, 0(%1)\n\t"
        "vst %%v16, 0(%0)"
        : : "a"(dst), "a"(src) : "v16", "memory");
}
RAF_FS_INLINE void raf_fs_vec_zero_block(void *dst) {
    __asm__ __volatile__(
        "vzero %%v16\n\t"
        "vst %%v16, 0(%0)"
        : : "a"(dst) : "v16", "memory");
}

#else
# error "raf_fs_vector.h requires an enabled fixed-vector profile (SSE2/AVX2/AVX-512F, ARMv7 NEON, AArch64 Advanced SIMD, or s390x z13 vector)"
#endif

#if defined(RAF_FS_S390X_HIGH_VECTOR_PROFILE)
/* VSEL chooses operand A for mask=0 and B for mask=1: A=no, B=yes. */
RAF_FS_INLINE void raf_fs_vec_select_u32_block(
    void *dst, const void *mask, const void *yes, const void *no) {
    __asm__ __volatile__(
        "vl %%v16, 0(%1)\n\t"
        "vl %%v17, 0(%2)\n\t"
        "vl %%v18, 0(%3)\n\t"
        "vsel %%v19, %%v18, %%v17, %%v16\n\t"
        "vst %%v19, 0(%0)"
        : : "a"(dst), "a"(mask), "a"(yes), "a"(no)
        : "v16", "v17", "v18", "v19", "memory");
}
#else
RAF_FS_INLINE void raf_fs_vec_select_u32_block(
    void *dst, const void *mask, const void *yes, const void *no) {
    raf_fs_native_u32v m = *(const raf_fs_native_u32v *)mask;
    raf_fs_native_u32v y = *(const raf_fs_native_u32v *)yes;
    raf_fs_native_u32v n = *(const raf_fs_native_u32v *)no;
    *(raf_fs_native_u32v *)dst = (y & m) | (n & ~m);
}
#endif

#define RAF_FS_VEC_STAGE(name) RAF_FS_INLINE void name(void *state)

#endif
