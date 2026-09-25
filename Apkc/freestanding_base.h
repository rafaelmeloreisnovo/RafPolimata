/*
 * freestanding_base.h — canonical zero-include scalar/endian substrate.
 *
 * This header is intentionally independent from sys.h and mem.h.
 * It provides only fixed-width contract types derived from C implementation
 * widths plus byte-wise endian helpers. No libc, libm, heap, syscall or I/O.
 *
 * CLOSURE_L2: cross-target width/runtime evidence remains target-gated.
 */
#pragma once

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef signed char         i8;
typedef signed short        i16;
typedef signed int          i32;
typedef signed long long    i64;
typedef __SIZE_TYPE__       sz;

typedef char raf_base_assert_u8 [(sizeof(u8)  == 1u) ? 1 : -1];
typedef char raf_base_assert_u16[(sizeof(u16) == 2u) ? 1 : -1];
typedef char raf_base_assert_u32[(sizeof(u32) == 4u) ? 1 : -1];
typedef char raf_base_assert_u64[(sizeof(u64) == 8u) ? 1 : -1];

/* Pure memory/arithmetic helpers shared by encoders and format writers. */
static inline void *m_set(void *d, u8 v, sz n) {
    u8 *p = (u8*)d;
    u64 vv = (u64)v * 0x0101010101010101ULL;
    sz i = 0;
    for (; i + 8 <= n; i += 8) *(u64*)(p+i) = vv;
    for (; i < n; i++) p[i] = v;
    return d;
}

static inline void *m_cpy(void *d, const void *s, sz n) {
    u8 *dp = (u8*)d;
    const u8 *sp = (const u8*)s;
    sz i = 0;
    for (; i + 8 <= n; i += 8) *(u64*)(dp+i) = *(const u64*)(sp+i);
    for (; i < n; i++) dp[i] = sp[i];
    return d;
}

static inline u32 u32_aln(u32 v, u32 a) {
    return (v + a - 1u) & ~(a - 1u);
}

static inline void w16(u8 *p, u16 v) {
    p[0] = (u8)v;
    p[1] = (u8)(v >> 8);
}

static inline void w32(u8 *p, u32 v) {
    p[0] = (u8)v;
    p[1] = (u8)(v >> 8);
    p[2] = (u8)(v >> 16);
    p[3] = (u8)(v >> 24);
}

static inline void w64(u8 *p, u64 v) {
    w32(p, (u32)v);
    w32(p + 4, (u32)(v >> 32));
}

static inline u16 r16(const u8 *p) {
    return (u16)((u16)p[0] | (u16)((u16)p[1] << 8));
}

static inline u32 r32(const u8 *p) {
    return (u32)p[0]
        | ((u32)p[1] << 8)
        | ((u32)p[2] << 16)
        | ((u32)p[3] << 24);
}
