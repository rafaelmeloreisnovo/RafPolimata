/* sys_host.h — freestanding x86_64 Linux syscall layer.
 * No libc. No malloc. Inline asm. Mirrors Apkc/sys.h for host builds.
 * RAFCODE-Φ-∆ | chain-of-custody: all I/O via these primitives only. */
#pragma once

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long   u64;
typedef signed   char   i8;
typedef signed   int    i32;
typedef signed   long   i64;
typedef unsigned long   sz;
typedef unsigned long   uptr;

#define NULL ((void*)0)

/* open(2) flags */
#define O_RDONLY   0
#define O_WRONLY   1
#define O_RDWR     2
#define O_CREAT    0100
#define O_TRUNC    01000
#define O_APPEND   02000
#define O_DIRECTORY 0200000

/* ── x86_64 syscall numbers ─────────────────────────────────────────── */
#define _NR_read          0
#define _NR_write         1
#define _NR_open          2
#define _NR_close         3
#define _NR_ioctl        16
#define _NR_fork         57
#define _NR_dup2         33
#define _NR_execve       59
#define _NR_wait4        61
#define _NR_getcwd       79
#define _NR_getdents64  217
#define _NR_clock_gettime 228
#define _NR_exit_group  231

/* ── inline asm wrappers ────────────────────────────────────────────── */
static __attribute__((always_inline)) i64
_sc1(i64 n, i64 a) {
    i64 r;
    __asm__ volatile("syscall"
        :"=a"(r):"a"(n),"D"(a):"memory","rcx","r11");
    return r;
}
static __attribute__((always_inline)) i64
_sc2(i64 n, i64 a, i64 b) {
    i64 r;
    __asm__ volatile("syscall"
        :"=a"(r):"a"(n),"D"(a),"S"(b):"memory","rcx","r11");
    return r;
}
static __attribute__((always_inline)) i64
_sc3(i64 n, i64 a, i64 b, i64 c) {
    i64 r;
    __asm__ volatile("syscall"
        :"=a"(r):"a"(n),"D"(a),"S"(b),"d"(c):"memory","rcx","r11");
    return r;
}
static __attribute__((always_inline)) i64
_sc4(i64 n, i64 a, i64 b, i64 c, i64 d) {
    i64 r; register i64 r10 __asm__("r10") = d;
    __asm__ volatile("syscall"
        :"=a"(r):"a"(n),"D"(a),"S"(b),"d"(c),"r"(r10):"memory","rcx","r11");
    return r;
}

static inline i64  os_read  (i32 fd,const void *b,sz n)   { return _sc3(_NR_read,(i64)fd,(i64)(uptr)b,(i64)n); }
static inline i64  os_write (i32 fd,const void *b,sz n)   { return _sc3(_NR_write,(i64)fd,(i64)(uptr)b,(i64)n); }
static inline i32  os_open  (const char *p,i32 f,i32 m)   { return (i32)_sc3(_NR_open,(i64)(uptr)p,(i64)f,(i64)m); }
static inline i32  os_close (i32 fd)                      { return (i32)_sc1(_NR_close,(i64)fd); }
static inline i32  os_ioctl (i32 fd,u64 req,void *a)      { return (i32)_sc3(_NR_ioctl,(i64)fd,(i64)req,(i64)(uptr)a); }
static inline i64  os_getdents64(i32 fd,void *b,sz n)     { return _sc3(_NR_getdents64,(i64)fd,(i64)(uptr)b,(i64)n); }
static inline i32  os_getcwd(char *b,sz n)                 { return (i32)_sc2(_NR_getcwd,(i64)(uptr)b,(i64)n); }
static inline i32  os_execve(const char *p,char*const av[],char*const ev[]) {
    return (i32)_sc3(_NR_execve,(i64)(uptr)p,(i64)(uptr)av,(i64)(uptr)ev);
}
static inline i32  os_wait4 (i32 pid,i32 *st,i32 opts)    { return (i32)_sc4(_NR_wait4,(i64)pid,(i64)(uptr)st,(i64)opts,0); }
static inline i32  os_dup2  (i32 oldfd,i32 newfd)         { return (i32)_sc2(_NR_dup2,(i64)oldfd,(i64)newfd); }
static inline __attribute__((noreturn)) void os_exit(i32 c) {
    _sc1(_NR_exit_group,(i64)c); __builtin_unreachable();
}

/* fork: no args — clobber all caller-saved regs */
static inline i32 os_fork(void) {
    i64 r;
    __asm__ volatile("syscall"
        :"=a"(r):"a"((i64)_NR_fork)
        :"memory","rcx","r11","rdx","rsi","rdi","r8","r9","r10");
    return (i32)r;
}

/* ── struct termios (linux asm-generic/termbits.h) ─────────────────── */
struct termios {
    u32 c_iflag, c_oflag, c_cflag, c_lflag;
    u8  c_line, c_cc[32];
    u32 c_ispeed, c_ospeed;
};
#define TCGETS       0x5401u
#define TCSETS       0x5402u
#define ECHO         0x00000008u
#define ICANON       0x00000002u
#define ISIG         0x00000001u
#define IXON         0x00000400u
#define VMIN         6
#define VTIME        5

/* ── struct winsize ─────────────────────────────────────────────────── */
struct winsize { u16 ws_row, ws_col, ws_xpixel, ws_ypixel; };
#define TIOCGWINSZ   0x5413u

/* ── struct dirent64 ────────────────────────────────────────────────── */
struct dirent64 { u64 d_ino; i64 d_off; u16 d_reclen; u8 d_type; char d_name[256]; };
#define DT_REG  8
#define DT_DIR  4
#define DT_LNK  10

/* ── struct timespec ────────────────────────────────────────────────── */
struct timespec { i64 tv_sec; i64 tv_nsec; };
#define CLOCK_MONOTONIC 1
static inline i32 os_clock_gettime(i32 clk, struct timespec *tp) {
    return (i32)_sc2(_NR_clock_gettime,(i64)clk,(i64)(uptr)tp);
}
