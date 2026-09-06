#ifndef RAF_FS_MATRIX_H
#define RAF_FS_MATRIX_H

/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Minimal matrix/tile-register primitives proving direct AMX TMM and Arm SME ZA ownership.
 * SCOPE: x86_64 AMX-TILE and AArch64 SME only; no OS enablement, allocation, syscall or runtime wrapper.
 * PRECONDITIONS: External environment has enabled and configured the architectural matrix state; AMX tile configuration is valid; SME ZA is enabled/owned by the caller.
 * REGISTER_OWNERSHIP: Primitive owns only TMM0 or ZA for the duration of the inlined stage; environment/context ownership remains caller responsibility.
 * CLOBBERS: AMX: tmm0. SME: ZA architectural state. No memory clobber for register-only zero operations.
 * MEMORY_ORDER: None; these primitives modify register state only.
 * TAIL_SHADOW: No loop, tail, compatibility buffer or shadow matrix is created.
 * EVIDENCE: Source implementation; compile/codegen gate required before BUILD/CODEGEN_PROVEN. Runtime/device remains CLOSURE_L12.
 */

#include "raf_fs_arch.h"

#if defined(__x86_64__) && defined(__AMX_TILE__)
RAF_FS_INLINE void raf_fs_matrix_zero_primary(void *state) {
    (void)state;
    __asm__ __volatile__("tilezero %%tmm0" ::: "tmm0");
}
# define RAF_FS_MATRIX_KIND_AMX 1u
# define RAF_FS_MATRIX_PRIMARY_REGS 8u

#elif defined(__aarch64__) && defined(__ARM_FEATURE_SME)
RAF_FS_INLINE void raf_fs_matrix_zero_primary(void *state) {
    (void)state;
    /* Caller owns SME streaming/ZA enablement; L0 does not toggle hidden state. */
    __asm__ __volatile__("zero {za}" ::: "za");
}
# define RAF_FS_MATRIX_KIND_SME 2u
# define RAF_FS_MATRIX_PRIMARY_REGS 1u

#else
# error "raf_fs_matrix.h requires x86_64 AMX-TILE or AArch64 SME"
#endif

#define RAF_FS_MATRIX_STAGE(name) RAF_FS_INLINE void name(void *state)

#endif
