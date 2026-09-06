#ifndef RAF_LINUX_SYSCALL_H
#define RAF_LINUX_SYSCALL_H

/*
 * Optional Linux binding. This file is NOT included by freestanding/.
 * Syscall numbers are caller-supplied: architecture + Linux ABI owns them.
 * No errno translation, libc wrapper, TLS, cancellation point or hidden retry.
 */
#if defined(__GNUC__) || defined(__clang__)
# define RAF_SC_INLINE static __inline__ __attribute__((__always_inline__, __unused__))
#else
# error "RAFAELIA raw syscall binding requires GCC/Clang-compatible inline assembly"
#endif

#if defined(__x86_64__)
typedef long raf_sc_word;
RAF_SC_INLINE raf_sc_word raf_linux_syscall6(raf_sc_word nr, raf_sc_word a0, raf_sc_word a1, raf_sc_word a2, raf_sc_word a3, raf_sc_word a4, raf_sc_word a5) {
    register raf_sc_word r10 __asm__("r10") = a3;
    register raf_sc_word r8  __asm__("r8")  = a4;
    register raf_sc_word r9  __asm__("r9")  = a5;
    register raf_sc_word rax __asm__("rax") = nr;
    register raf_sc_word rdi __asm__("rdi") = a0;
    register raf_sc_word rsi __asm__("rsi") = a1;
    register raf_sc_word rdx __asm__("rdx") = a2;
    __asm__ __volatile__("syscall" : "+a"(rax) : "D"(rdi), "S"(rsi), "d"(rdx), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
    return rax;
}

#elif defined(__i386__)
typedef long raf_sc_word;
RAF_SC_INLINE raf_sc_word raf_linux_syscall6(raf_sc_word nr, raf_sc_word a0, raf_sc_word a1, raf_sc_word a2, raf_sc_word a3, raf_sc_word a4, raf_sc_word a5) {
    register raf_sc_word eax __asm__("eax") = nr;
    register raf_sc_word ebx __asm__("ebx") = a0;
    register raf_sc_word ecx __asm__("ecx") = a1;
    register raf_sc_word edx __asm__("edx") = a2;
    register raf_sc_word esi __asm__("esi") = a3;
    register raf_sc_word edi __asm__("edi") = a4;
    register raf_sc_word ebp __asm__("ebp") = a5;
    __asm__ __volatile__("int $0x80" : "+a"(eax) : "b"(ebx), "c"(ecx), "d"(edx), "S"(esi), "D"(edi), "r"(ebp) : "memory", "cc");
    return eax;
}

#elif defined(__aarch64__)
typedef long raf_sc_word;
RAF_SC_INLINE raf_sc_word raf_linux_syscall6(raf_sc_word nr, raf_sc_word a0, raf_sc_word a1, raf_sc_word a2, raf_sc_word a3, raf_sc_word a4, raf_sc_word a5) {
    register raf_sc_word x0 __asm__("x0") = a0;
    register raf_sc_word x1 __asm__("x1") = a1;
    register raf_sc_word x2 __asm__("x2") = a2;
    register raf_sc_word x3 __asm__("x3") = a3;
    register raf_sc_word x4 __asm__("x4") = a4;
    register raf_sc_word x5 __asm__("x5") = a5;
    register raf_sc_word x8 __asm__("x8") = nr;
    __asm__ __volatile__("svc #0" : "+r"(x0) : "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x8) : "memory", "cc");
    return x0;
}

#elif defined(__arm__) && (__ARM_ARCH >= 7)
typedef long raf_sc_word;
RAF_SC_INLINE raf_sc_word raf_linux_syscall6(raf_sc_word nr, raf_sc_word a0, raf_sc_word a1, raf_sc_word a2, raf_sc_word a3, raf_sc_word a4, raf_sc_word a5) {
    register raf_sc_word r0 __asm__("r0") = a0;
    register raf_sc_word r1 __asm__("r1") = a1;
    register raf_sc_word r2 __asm__("r2") = a2;
    register raf_sc_word r3 __asm__("r3") = a3;
    register raf_sc_word r4 __asm__("r4") = a4;
    register raf_sc_word r5 __asm__("r5") = a5;
    register raf_sc_word r7 __asm__("r7") = nr;
    __asm__ __volatile__("svc #0" : "+r"(r0) : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r7) : "memory", "cc");
    return r0;
}

#elif defined(__riscv)
typedef long raf_sc_word;
RAF_SC_INLINE raf_sc_word raf_linux_syscall6(raf_sc_word nr, raf_sc_word a0, raf_sc_word a1, raf_sc_word a2, raf_sc_word a3, raf_sc_word a4, raf_sc_word a5) {
    register raf_sc_word x10 __asm__("a0") = a0;
    register raf_sc_word x11 __asm__("a1") = a1;
    register raf_sc_word x12 __asm__("a2") = a2;
    register raf_sc_word x13 __asm__("a3") = a3;
    register raf_sc_word x14 __asm__("a4") = a4;
    register raf_sc_word x15 __asm__("a5") = a5;
    register raf_sc_word x17 __asm__("a7") = nr;
    __asm__ __volatile__("ecall" : "+r"(x10) : "r"(x11), "r"(x12), "r"(x13), "r"(x14), "r"(x15), "r"(x17) : "memory");
    return x10;
}
#else
# error "Unsupported Linux raw syscall target"
#endif

#define RAF_LINUX_SYSCALL0(nr) raf_linux_syscall6((nr),0,0,0,0,0,0)
#define RAF_LINUX_SYSCALL1(nr,a0) raf_linux_syscall6((nr),(a0),0,0,0,0,0)
#define RAF_LINUX_SYSCALL2(nr,a0,a1) raf_linux_syscall6((nr),(a0),(a1),0,0,0,0)
#define RAF_LINUX_SYSCALL3(nr,a0,a1,a2) raf_linux_syscall6((nr),(a0),(a1),(a2),0,0,0)
#define RAF_LINUX_SYSCALL4(nr,a0,a1,a2,a3) raf_linux_syscall6((nr),(a0),(a1),(a2),(a3),0,0)
#define RAF_LINUX_SYSCALL5(nr,a0,a1,a2,a3,a4) raf_linux_syscall6((nr),(a0),(a1),(a2),(a3),(a4),0)
#define RAF_LINUX_SYSCALL6(nr,a0,a1,a2,a3,a4,a5) raf_linux_syscall6((nr),(a0),(a1),(a2),(a3),(a4),(a5))

#endif
