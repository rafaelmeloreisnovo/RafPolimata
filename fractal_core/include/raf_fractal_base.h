#ifndef RAF_FRACTAL_BASE_H
#define RAF_FRACTAL_BASE_H

/* Freestanding primitive layer: no libc, no stdint, no compiler runtime API. */
typedef unsigned char      raf_u8;
typedef unsigned short     raf_u16;
typedef unsigned int       raf_u32;
typedef unsigned long long raf_u64;
typedef signed char        raf_s8;
typedef signed short       raf_s16;
typedef signed int         raf_s32;
typedef signed long long   raf_s64;
typedef raf_s32            raf_q16;

#define RAF_Q16_ONE       ((raf_q16)65536)
#define RAF_Q16_HALF      ((raf_q16)32768)
#define RAF_Q16_QUARTER   ((raf_q16)16384)
#define RAF_Q16_EIGHTH    ((raf_q16)8192)
#define RAF_Q16_3_QUARTER ((raf_q16)49152)
#define RAF_Q16_EPS_PROBE ((raf_q16)64)      /* 1/1024 */
#define RAF_PROJECTIVE_MAX ((raf_u32)32767)
#define RAF_FIB_STATE_MAX  ((raf_u32)1073741823) /* 2^30-1 */

#if defined(__GNUC__) || defined(__clang__)
# define RAF_ALWAYS_INLINE __attribute__((always_inline)) inline
# define RAF_NOINLINE      __attribute__((noinline))
# define RAF_HIDDEN        __attribute__((visibility("hidden")))
# define RAF_USED          __attribute__((used))
# define RAF_SECTION(x)    __attribute__((section(x)))
# define RAF_PURE          __attribute__((pure))
# define RAF_CONST         __attribute__((const))
#else
# define RAF_ALWAYS_INLINE inline
# define RAF_NOINLINE
# define RAF_HIDDEN
# define RAF_USED
# define RAF_SECTION(x)
# define RAF_PURE
# define RAF_CONST
#endif

#define RAF_API RAF_HIDDEN RAF_USED

/* Build-time feature gates: compile-time selection, no runtime dispatch. */
#ifndef RAF_USE_ARCH_MUL
# define RAF_USE_ARCH_MUL 0
#endif

#if RAF_USE_ARCH_MUL
RAF_HIDDEN raf_q16 raf_q16_mul_arch(raf_q16 a, raf_q16 b);
static RAF_ALWAYS_INLINE raf_q16 raf_q16_mul(raf_q16 a, raf_q16 b) {
    return raf_q16_mul_arch(a, b);
}
#else
static RAF_ALWAYS_INLINE raf_q16 raf_q16_mul(raf_q16 a, raf_q16 b) {
    return (raf_q16)(((raf_s64)a * (raf_s64)b) >> 16);
}
#endif

static RAF_ALWAYS_INLINE raf_u32 raf_abs_s32(raf_s32 v) {
    raf_s32 m = v >> 31;
    return (raf_u32)((v + m) ^ m);
}

#endif
