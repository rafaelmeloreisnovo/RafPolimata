#ifndef RAF_FS_ARCH_H
#define RAF_FS_ARCH_H

/*
 * Architecture-only primitives. No OS contract lives here.
 * Every primitive is forced inline so it contributes no external helper symbol.
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
RAF_FS_INLINE void raf_fs_relax(void) { __asm__ __volatile__("pause" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_rw(void) { __asm__ __volatile__("mfence" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_r(void) { __asm__ __volatile__("lfence" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_w(void) { __asm__ __volatile__("sfence" ::: "memory"); }

#elif defined(__i386__)
# define RAF_FS_ARCH_I686 1
# define RAF_FS_WORD_BITS 32u
/* i686 baseline does not imply SSE2, so MFENCE is not a legal baseline assumption. */
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
RAF_FS_INLINE void raf_fs_relax(void) { __asm__ __volatile__("yield" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_rw(void) { __asm__ __volatile__("dmb ish" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_r(void) { __asm__ __volatile__("dmb ishld" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_w(void) { __asm__ __volatile__("dmb ishst" ::: "memory"); }

#elif defined(__arm__) && (__ARM_ARCH >= 7)
# define RAF_FS_ARCH_ARMV7 1
# define RAF_FS_WORD_BITS 32u
RAF_FS_INLINE void raf_fs_relax(void) { __asm__ __volatile__("yield" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_rw(void) { __asm__ __volatile__("dmb ish" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_r(void) { raf_fs_fence_rw(); }
RAF_FS_INLINE void raf_fs_fence_w(void) { __asm__ __volatile__("dmb ishst" ::: "memory"); }

#elif defined(__riscv) && (__riscv_xlen == 64)
# define RAF_FS_ARCH_RV64 1
# define RAF_FS_WORD_BITS 64u
RAF_FS_INLINE void raf_fs_relax(void) { RAF_FS_COMPILER_BARRIER(); }
RAF_FS_INLINE void raf_fs_fence_rw(void) { __asm__ __volatile__("fence rw,rw" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_r(void) { __asm__ __volatile__("fence r,r" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_w(void) { __asm__ __volatile__("fence w,w" ::: "memory"); }

#elif defined(__riscv) && (__riscv_xlen == 32)
# define RAF_FS_ARCH_RV32 1
# define RAF_FS_WORD_BITS 32u
RAF_FS_INLINE void raf_fs_relax(void) { RAF_FS_COMPILER_BARRIER(); }
RAF_FS_INLINE void raf_fs_fence_rw(void) { __asm__ __volatile__("fence rw,rw" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_r(void) { __asm__ __volatile__("fence r,r" ::: "memory"); }
RAF_FS_INLINE void raf_fs_fence_w(void) { __asm__ __volatile__("fence w,w" ::: "memory"); }

#else
# error "Unsupported RAFAELIA L0 architecture"
#endif

#endif
