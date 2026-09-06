#ifndef RAF_FS_CAPS_H
#define RAF_FS_CAPS_H

#include "raf_fs_types.h"

#define RAF_FS_VECTOR_NONE      0u
#define RAF_FS_VECTOR_FIXED     1u
#define RAF_FS_VECTOR_SCALABLE  2u

#if defined(__x86_64__)
# define RAF_FS_GPR_BITS 64u
# if defined(__AVX512F__)
#  define RAF_FS_SIMD_BITS 512u
# elif defined(__AVX2__)
#  define RAF_FS_SIMD_BITS 256u
# else
#  define RAF_FS_SIMD_BITS 128u /* SSE2 baseline for the x86-64 profile. */
# endif
# define RAF_FS_VECTOR_KIND RAF_FS_VECTOR_FIXED

#elif defined(__i386__)
# define RAF_FS_GPR_BITS 32u
# if defined(__AVX512F__)
#  define RAF_FS_SIMD_BITS 512u
# elif defined(__AVX2__)
#  define RAF_FS_SIMD_BITS 256u
# elif defined(__SSE2__) || defined(__SSE__)
#  define RAF_FS_SIMD_BITS 128u
# else
#  define RAF_FS_SIMD_BITS 0u
# endif
# define RAF_FS_VECTOR_KIND ((RAF_FS_SIMD_BITS) ? RAF_FS_VECTOR_FIXED : RAF_FS_VECTOR_NONE)

#elif defined(__aarch64__)
# define RAF_FS_GPR_BITS 64u
# if defined(__ARM_FEATURE_SVE)
#  define RAF_FS_SIMD_BITS 0u
#  define RAF_FS_VECTOR_KIND RAF_FS_VECTOR_SCALABLE
# else
#  define RAF_FS_SIMD_BITS 128u /* Advanced SIMD baseline. */
#  define RAF_FS_VECTOR_KIND RAF_FS_VECTOR_FIXED
# endif

#elif defined(__arm__) && (__ARM_ARCH >= 7)
# define RAF_FS_GPR_BITS 32u
# if defined(__ARM_NEON) || defined(__ARM_NEON__)
#  define RAF_FS_SIMD_BITS 128u
#  define RAF_FS_VECTOR_KIND RAF_FS_VECTOR_FIXED
# else
#  define RAF_FS_SIMD_BITS 0u
#  define RAF_FS_VECTOR_KIND RAF_FS_VECTOR_NONE
# endif

#elif defined(__riscv)
# define RAF_FS_GPR_BITS __riscv_xlen
# if defined(__riscv_vector)
#  if defined(__riscv_v_fixed_vlen)
#   define RAF_FS_SIMD_BITS __riscv_v_fixed_vlen
#   define RAF_FS_VECTOR_KIND RAF_FS_VECTOR_FIXED
#  else
#   define RAF_FS_SIMD_BITS 0u
#   define RAF_FS_VECTOR_KIND RAF_FS_VECTOR_SCALABLE
#  endif
# else
#  define RAF_FS_SIMD_BITS 0u
#  define RAF_FS_VECTOR_KIND RAF_FS_VECTOR_NONE
# endif
#else
# error "Unsupported RAFAELIA capability target"
#endif

#define RAF_FS_FIXED_LANES(bits_per_element) \
    ((RAF_FS_SIMD_BITS) ? (RAF_FS_SIMD_BITS / (bits_per_element)) : 1u)

#define RAF_FS_LANES_U8   RAF_FS_FIXED_LANES(8u)
#define RAF_FS_LANES_U16  RAF_FS_FIXED_LANES(16u)
#define RAF_FS_LANES_U32  RAF_FS_FIXED_LANES(32u)
#define RAF_FS_LANES_U64  RAF_FS_FIXED_LANES(64u)

#endif
