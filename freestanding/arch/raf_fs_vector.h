#ifndef RAF_FS_VECTOR_H
#define RAF_FS_VECTOR_H

/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Fixed-vector, branchless, one-block execution primitives.
 * SCOPE: SSE2, AVX2, AVX-512F, ARMv7 NEON and AArch64 Advanced SIMD; no OS/runtime contract.
 * PRECONDITIONS: Selected compiler target enables the exact ISA profile; src/dst expose one full block.
 * REGISTER_OWNERSHIP: Inline assembly owns only the declared vector temporary; C-vector selection owns compiler temporaries.
 * CLOBBERS: Declared vector temporary plus memory for copy/zero; compiler allocates select temporaries.
 * MEMORY_ORDER: Ordinary data access; no fence implied. Use raf_fs_arch.h ordering primitives separately.
 * TAIL_SHADOW: Exactly one complete vector block; residuals require explicit mask/full accessible block; no scalar tail/shadow.
 * EVIDENCE: Source implementation; profile compile/codegen gates promote build evidence. CLOSURE_L11/CLOSURE_L12.
 */

#include "../include/raf_fs_types.h"
#include "../include/raf_fs_abi.h"
#include "raf_fs_arch.h"

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

#else
# error "raf_fs_vector.h requires SSE2/AVX2/AVX-512F, ARMv7 NEON, or AArch64 Advanced SIMD"
#endif

RAF_FS_INLINE void raf_fs_vec_select_u32_block(
    void *dst, const void *mask, const void *yes, const void *no) {
    raf_fs_native_u32v m = *(const raf_fs_native_u32v *)mask;
    raf_fs_native_u32v y = *(const raf_fs_native_u32v *)yes;
    raf_fs_native_u32v n = *(const raf_fs_native_u32v *)no;
    *(raf_fs_native_u32v *)dst = (y & m) | (n & ~m);
}

#define RAF_FS_VEC_STAGE(name) RAF_FS_INLINE void name(void *state)

#endif
