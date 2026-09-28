/* raf_sys.h — raw Linux syscall ABI, no-libc userspace
 * BOUNDARY: OS-bound Linux/Android adapter; NOT physical bare-metal and NOT
 * RAFAELIA freestanding L0. Friction reduced: no stdio buffer, libc errno
 * TLS, heap allocator or CRT ownership in this adapter.
 * ARM64 syscall ABI: x8=nr, x0-x5=args, svc #0, ret em x0
 * x86-64 syscall ABI: rax=nr, rdi,rsi,rdx,r10,r8,r9=args, syscall, ret rax
 * ARM32 syscall ABI: r7=nr, r0-r6=args, svc #0, ret r0                    */
#pragma once
#include "raf_types.h"

#ifdef RAF_ARCH_A32
/* Linux ARM EABI syscall 263 is clock_gettime32: two signed 32-bit longs. */
typedef struct { s32 sec; s32 nsec; } Timespec;
_Static_assert(sizeof(Timespec) == 8, "ARM32 clock_gettime32 ABI requires 8-byte timespec");
#else
typedef struct { s64 sec; s64 nsec; } Timespec;
_Static_assert(sizeof(Timespec) == 16, "64-bit Linux timespec requires two 64-bit longs");
#endif
#define CLOCK_MONO 1
static u32 RAF_TIMER_FAULT = 0;

/* ── ARM64 ──────────────────────────────────────────────────────────────── */
#ifdef RAF_ARCH_A64
#define SYS_write          64
#define SYS_exit           93
#define SYS_clock_gettime  113

static __attribute__((always_inline)) inline
s64 _sc3(s64 nr, s64 a, s64 b, s64 c) {
    register s64 x0 __asm__("x0") = a;
    register s64 x1 __asm__("x1") = b;
    register s64 x2 __asm__("x2") = c;
    register s64 x8 __asm__("x8") = nr;
    __asm__ volatile("svc #0"
        : "+r"(x0) : "r"(x1),"r"(x2),"r"(x8) : "memory","cc");
    return x0;
}
static __attribute__((always_inline)) inline
s64 _sc1(s64 nr, s64 a) {
    register s64 x0 __asm__("x0") = a;
    register s64 x8 __asm__("x8") = nr;
    __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8) : "memory","cc");
    return x0;
}
/* timer: cntvct_el0 — sem syscall, sem context switch, ~5 ciclos           */
static __attribute__((always_inline)) inline u64 raf_tsc(void) {
    u64 v;
    __asm__ volatile("isb\n\tmrs %0, cntvct_el0" : "=r"(v) :: "memory");
    return v;
}
static __attribute__((always_inline)) inline u64 raf_tsc_freq(void) {
    u64 v;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(v));
    return v; /* Hz reported by the current execution environment. */
}
#endif /* RAF_ARCH_A64 */

/* ── x86-64 ─────────────────────────────────────────────────────────────── */
#ifdef RAF_ARCH_X64
#define SYS_write          1
#define SYS_exit           60
#define SYS_clock_gettime  228

static __attribute__((always_inline)) inline
s64 _sc3(s64 nr, s64 a, s64 b, s64 c) {
    s64 r;
    __asm__ volatile("syscall"
        : "=a"(r)
        : "a"(nr),"D"(a),"S"(b),"d"(c)
        : "rcx","r11","memory");
    return r;
}
static __attribute__((always_inline)) inline
s64 _sc1(s64 nr, s64 a) {
    s64 r;
    __asm__ volatile("syscall"
        : "=a"(r) : "a"(nr),"D"(a) : "rcx","r11","memory");
    return r;
}
static __attribute__((always_inline)) inline u64 raf_tsc(void) {
    u32 lo, hi;
    /* lfence garante ordem — serializa loads antes do rdtsc */
    __asm__ volatile("lfence\n\trdtsc" : "=a"(lo),"=d"(hi) :: "memory");
    return ((u64)hi << 32) | lo;
}
static __attribute__((always_inline)) inline u64 raf_tsc_freq(void) {
    return 0; /* calcular via clock_gettime delta */
}
#endif /* RAF_ARCH_X64 */

/* ── ARM32 ──────────────────────────────────────────────────────────────── */
#ifdef RAF_ARCH_A32
#define SYS_write          4
#define SYS_exit           1
#define SYS_clock_gettime  263

static __attribute__((always_inline)) inline
s64 _sc3(s64 nr, s64 a, s64 b, s64 c) {
    register s32 r0 __asm__("r0") = (s32)a;
    register s32 r1 __asm__("r1") = (s32)b;
    register s32 r2 __asm__("r2") = (s32)c;
    register s32 r7 __asm__("r7") = (s32)nr;
    __asm__ volatile("svc #0"
        : "+r"(r0) : "r"(r1),"r"(r2),"r"(r7) : "memory","cc");
    return r0;
}
static __attribute__((always_inline)) inline
s64 _sc1(s64 nr, s64 a) {
    register s32 r0 __asm__("r0") = (s32)a;
    register s32 r7 __asm__("r7") = (s32)nr;
    __asm__ volatile("svc #0" : "+r"(r0) : "r"(r7) : "memory","cc");
    return r0;
}
/* ARM32/Termux: clock_gettime(CLOCK_MONOTONIC) through the raw Linux syscall.
 * PMCCNTR is commonly unavailable to Android userland, so the benchmark uses
 * this OS-bound timer. The syscall cost is part of the measurement envelope.
 * A syscall failure yields zero instead of reading uninitialized state.      */
static __attribute__((always_inline)) inline u64 raf_tsc(void) {
    Timespec ts = {0, 0};
    s64 rc = _sc3(SYS_clock_gettime, CLOCK_MONO, (s64)(usize)&ts, 0);
    if (rc < 0 || ts.sec < 0 || ts.nsec < 0 || ts.nsec >= 1000000000) {
        RAF_TIMER_FAULT = 1;
        return 0;
    }
    return (u64)(u32)ts.sec * 1000000000ULL + (u64)(u32)ts.nsec;
}
static __attribute__((always_inline)) inline u64 raf_tsc_freq(void) {
    return 1000000000ULL; /* tsc já em nanos: 1e9 ticks/s */
}
#endif /* RAF_ARCH_A32 */

/* ── Interface unificada ─────────────────────────────────────────────────── */

static inline void raf_write(const void *buf, usize n) {
    _sc3(SYS_write, FD_STDOUT, (s64)(usize)buf, (s64)n);
}
static inline void raf_exit(s32 code) {
    _sc1(SYS_exit, code);
    __builtin_unreachable();
}
static inline s64 raf_clock(Timespec *ts) {
    if (!ts) return -1;
    s64 rc = _sc3(SYS_clock_gettime, CLOCK_MONO, (s64)(usize)ts, 0);
    if (rc < 0) RAF_TIMER_FAULT = 1;
    return rc;
}
static inline u32 raf_timer_ok(void) { return RAF_TIMER_FAULT == 0; }

/* Inline decimal output: no implicit newline and no dynamic buffer. */
static void raf_putu64(u64 v) {
    char buf[20]; s32 i = 20;
    if (!v) buf[--i] = '0';
    else {
        while (v) {
#ifdef RAF_ARCH_A32
            u64 q = raf_udiv64_bounded(v, 10);
            u32 digit = (u32)(v - q * 10);
            v = q;
#else
            u32 digit = (u32)(v % 10);
            v /= 10;
#endif
            buf[--i] = (char)('0' + digit);
        }
    }
    raf_write(buf + i, (usize)(20 - i));
}
static void raf_puts(const char *s) {
    usize n = 0;
    while (s[n]) n++;
    raf_write(s, n);
}
