#ifndef RAF_FS_ABI_H
#define RAF_FS_ABI_H

/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Compile-time internal ABI/profile geometry for OS-agnostic L0.
 * SCOPE: ISA/register metadata and caller-owned `void *state`; never host/OS ABI glue.
 * PRECONDITIONS: GCC/Clang target macros identify the selected ISA/profile.
 * REGISTER_OWNERSHIP: Caller owns architectural state; L0 owns only declared inline-asm temporaries.
 * CLOBBERS: None in this metadata header.
 * MEMORY_ORDER: None; ordering primitives live in arch/raf_fs_arch.h.
 * TAIL_SHADOW: No tail fallback and no shadow state are introduced here.
 * EVIDENCE: Source metadata; build/codegen/runtime promotion requires separate gates. CLOSURE_L11/CLOSURE_L12.
 */

#include "raf_fs_types.h"

#define RAF_FS_ABI_X86_64   1u
#define RAF_FS_ABI_I686     2u
#define RAF_FS_ABI_ARMV7    3u
#define RAF_FS_ABI_AARCH64  4u
#define RAF_FS_ABI_RV32     5u
#define RAF_FS_ABI_RV64     6u

#if defined(__x86_64__)
# define RAF_FS_ABI_ID RAF_FS_ABI_X86_64
# define RAF_FS_ABI_PTR_BITS 64u
# define RAF_FS_ABI_GPR_COUNT 16u
# if defined(__AVX512F__)
#  define RAF_FS_ABI_VECTOR_BITS 512u
#  define RAF_FS_ABI_VECTOR_REGS 32u
#  define RAF_FS_ABI_PRED_REGS 8u
# elif defined(__AVX2__)
#  define RAF_FS_ABI_VECTOR_BITS 256u
#  define RAF_FS_ABI_VECTOR_REGS 16u
#  define RAF_FS_ABI_PRED_REGS 0u
# else
#  define RAF_FS_ABI_VECTOR_BITS 128u
#  define RAF_FS_ABI_VECTOR_REGS 16u
#  define RAF_FS_ABI_PRED_REGS 0u
# endif
# if defined(__AMX_TILE__)
#  define RAF_FS_ABI_MATRIX_REGS 8u
# else
#  define RAF_FS_ABI_MATRIX_REGS 0u
# endif

#elif defined(__i386__)
# define RAF_FS_ABI_ID RAF_FS_ABI_I686
# define RAF_FS_ABI_PTR_BITS 32u
# define RAF_FS_ABI_GPR_COUNT 8u
# if defined(__AVX512F__)
#  define RAF_FS_ABI_VECTOR_BITS 512u
#  define RAF_FS_ABI_VECTOR_REGS 8u
#  define RAF_FS_ABI_PRED_REGS 8u
# elif defined(__AVX2__)
#  define RAF_FS_ABI_VECTOR_BITS 256u
#  define RAF_FS_ABI_VECTOR_REGS 8u
#  define RAF_FS_ABI_PRED_REGS 0u
# elif defined(__SSE__)
#  define RAF_FS_ABI_VECTOR_BITS 128u
#  define RAF_FS_ABI_VECTOR_REGS 8u
#  define RAF_FS_ABI_PRED_REGS 0u
# else
#  define RAF_FS_ABI_VECTOR_BITS 0u
#  define RAF_FS_ABI_VECTOR_REGS 0u
#  define RAF_FS_ABI_PRED_REGS 0u
# endif
# define RAF_FS_ABI_MATRIX_REGS 0u

#elif defined(__aarch64__)
# define RAF_FS_ABI_ID RAF_FS_ABI_AARCH64
# define RAF_FS_ABI_PTR_BITS 64u
# define RAF_FS_ABI_GPR_COUNT 31u
# if defined(__ARM_FEATURE_SVE)
#  define RAF_FS_ABI_VECTOR_BITS 0u
#  define RAF_FS_ABI_VECTOR_REGS 32u
#  define RAF_FS_ABI_PRED_REGS 16u
# else
#  define RAF_FS_ABI_VECTOR_BITS 128u
#  define RAF_FS_ABI_VECTOR_REGS 32u
#  define RAF_FS_ABI_PRED_REGS 0u
# endif
# if defined(__ARM_FEATURE_SME)
#  define RAF_FS_ABI_MATRIX_REGS 1u
# else
#  define RAF_FS_ABI_MATRIX_REGS 0u
# endif

#elif defined(__arm__) && (__ARM_ARCH >= 7)
# define RAF_FS_ABI_ID RAF_FS_ABI_ARMV7
# define RAF_FS_ABI_PTR_BITS 32u
# define RAF_FS_ABI_GPR_COUNT 16u
# if defined(__ARM_NEON) || defined(__ARM_NEON__)
#  define RAF_FS_ABI_VECTOR_BITS 128u
#  define RAF_FS_ABI_VECTOR_REGS 16u
# else
#  define RAF_FS_ABI_VECTOR_BITS 0u
#  define RAF_FS_ABI_VECTOR_REGS 0u
# endif
# define RAF_FS_ABI_PRED_REGS 0u
# define RAF_FS_ABI_MATRIX_REGS 0u

#elif defined(__riscv) && (__riscv_xlen == 32)
# define RAF_FS_ABI_ID RAF_FS_ABI_RV32
# define RAF_FS_ABI_PTR_BITS 32u
# define RAF_FS_ABI_GPR_COUNT 32u
# if defined(__riscv_vector)
#  define RAF_FS_ABI_VECTOR_BITS 0u
#  define RAF_FS_ABI_VECTOR_REGS 32u
#  define RAF_FS_ABI_PRED_REGS 1u
# else
#  define RAF_FS_ABI_VECTOR_BITS 0u
#  define RAF_FS_ABI_VECTOR_REGS 0u
#  define RAF_FS_ABI_PRED_REGS 0u
# endif
# define RAF_FS_ABI_MATRIX_REGS 0u

#elif defined(__riscv) && (__riscv_xlen == 64)
# define RAF_FS_ABI_ID RAF_FS_ABI_RV64
# define RAF_FS_ABI_PTR_BITS 64u
# define RAF_FS_ABI_GPR_COUNT 32u
# if defined(__riscv_vector)
#  define RAF_FS_ABI_VECTOR_BITS 0u
#  define RAF_FS_ABI_VECTOR_REGS 32u
#  define RAF_FS_ABI_PRED_REGS 1u
# else
#  define RAF_FS_ABI_VECTOR_BITS 0u
#  define RAF_FS_ABI_VECTOR_REGS 0u
#  define RAF_FS_ABI_PRED_REGS 0u
# endif
# define RAF_FS_ABI_MATRIX_REGS 0u
#else
# error "Unsupported RAFAELIA freestanding ABI profile"
#endif

#define RAF_FS_ABI_STAGE(name) static __inline__ __attribute__((__always_inline__, __unused__)) void name(void *state)
#define RAF_FS_ABI_CONST_STAGE(name) static __inline__ __attribute__((__always_inline__, __unused__)) void name(void *dst, const void *src)

#endif
