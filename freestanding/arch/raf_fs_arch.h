#ifndef RAF_FS_ARCH_H
#define RAF_FS_ARCH_H

/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: architecture-only relax and memory-order primitives for the scalar L0 baseline.
 * SCOPE: ISA primitives only; no OS ABI, syscall number, loader or hosted runtime contract.
 * PRECONDITIONS: selected compiler target/profile accurately describes the instruction floor.
 * REGISTER_OWNERSHIP: no fixed GPR ownership; inline assembly uses only implicit instruction state and compiler allocation.
 * CLOBBERS: compiler `memory`; x86 fences also architectural ordering; ARM/RISC-V barriers as documented by mnemonic.
 * MEMORY_ORDER: raf_fs_fence_rw/r/w provide the architecture-specific barrier selected below.
 * TAIL_SHADOW: no data tail handling and no shadow state in this file.
 * EVIDENCE: six-target source/object compile path exists; physical execution remains separately closure-gated.
 */

/*
 * Every primitive is forced inline so it contributes no external helper symbol.
 * Raw hexadecimal encodings are intentionally absent: all current baseline
 * instructions are understood by the selected assembler profiles.
 */
#if defined(__GNUC__) || defined(__clang__)
# define RAF_FS_INLINE static __inline__ __attribute__((__always_inline__, __unused__))
# define RAF_FS_COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")
#else
# error "RAFAELIA L0 currently requires GCC/Clang-compatible inline assembly"
#endif

#if defined(__x86_64__)
# define RAF_FS_ARCH_X86_64 1
# define RAF_FS_WORD_BITS 64u
/* x86_64 baseline: PAUSE and SSE2 fence instructions are available. */
RAF_FS_INLINE void raf_fs_relax(void) { __asm__ __volatile__("pause" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_rw(void) { __asm__ __volatile__("mfence" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_r(void) { __asm__ __volatile__("lfence" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_w(void) { __asm__ __volatile__("sfence" ::: "memory"); }

#elif defined(__i386__)
# define RAF_FS_ARCH_I686 1
# define RAF_FS_WORD_BITS 32u
/*
 * i686/Pentium-Pro baseline does not imply SSE/SSE2. PAUSE/MFENCE therefore
 * cannot be emitted unless the build explicitly raises the ISA floor to SSE2.
 * The baseline fallback is a compiler barrier, not a fake hardware fence.
 */
RAF_FS_INLINE void raf_fs_relax(void) {
# if defined(__SSE2__)
    __asm__ __volatile__("pause" ::: "memory");
# else
    RAF_FS_COMPILER_BARRIER();
# endif
}
RAF_FS_INLINE void raf_fs_fence_rw(void) {
# if defined(__SSE2__)
    __asm__ __volatile__("mfence" ::: "memory");
# else
    RAF_FS_COMPILER_BARRIER();
# endif
}
RAF_FS_INLINE void raf_fs_fence_r(void) { raf_fs_fence_rw(); }
RAF_FS_INLINE void raf_fs_fence_w(void) { raf_fs_fence_rw(); }

#elif defined(__aarch64__)
# define RAF_FS_ARCH_AARCH64 1
# define RAF_FS_WORD_BITS 64u
/* Inner-shareable barriers; no fixed register operands or helper calls. */
RAF_FS_INLINE void raf_fs_relax(void) { __asm__ __volatile__("yield" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_rw(void) { __asm__ __volatile__("dmb ish" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_r(void) { __asm__ __volatile__("dmb ishld" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_w(void) { __asm__ __volatile__("dmb ishst" ::: "memory"); }

#elif defined(__arm__) && (__ARM_ARCH >= 7)
# define RAF_FS_ARCH_ARMV7 1
# define RAF_FS_WORD_BITS 32u
/* ARMv7-A baseline barriers; no NEON requirement is introduced here. */
RAF_FS_INLINE void raf_fs_relax(void) { __asm__ __volatile__("yield" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_rw(void) { __asm__ __volatile__("dmb ish" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_r(void) { raf_fs_fence_rw(); }
RAF_FS_INLINE void raf_fs_fence_w(void) { __asm__ __volatile__("dmb ishst" ::: "memory"); }

#elif defined(__riscv) && (__riscv_xlen == 64)
# define RAF_FS_ARCH_RV64 1
# define RAF_FS_WORD_BITS 64u
/* No optional pause/hint extension is assumed in the RV64I baseline. */
RAF_FS_INLINE void raf_fs_relax(void) { RAF_FS_COMPILER_BARRIER(); }
RAF_FS_INLINE void raf_fs_fence_rw(void) { __asm__ __volatile__("fence rw,rw" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_r(void) { __asm__ __volatile__("fence r,r" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_w(void) { __asm__ __volatile__("fence w,w" ::: "memory"); }

#elif defined(__riscv) && (__riscv_xlen == 32)
# define RAF_FS_ARCH_RV32 1
# define RAF_FS_WORD_BITS 32u
/* No optional pause/hint extension is assumed in the RV32I baseline. */
RAF_FS_INLINE void raf_fs_relax(void) { RAF_FS_COMPILER_BARRIER(); }
RAF_FS_INLINE void raf_fs_fence_rw(void) { __asm__ __volatile__("fence rw,rw" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_r(void) { __asm__ __volatile__("fence r,r" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_w(void) { __asm__ __volatile__("fence w,w" ::: "memory"); }

#else
# error "Unsupported RAFAELIA L0 architecture"
#endif

#endif
