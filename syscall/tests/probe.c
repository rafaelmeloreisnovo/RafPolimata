#include "../linux/raf_linux_syscall.h"

/* Compile-only raw ABI probe. The syscall number is caller-owned. */
raf_sc_word raf_linux_syscall_probe(raf_sc_word nr, raf_sc_word ptr, raf_sc_word len) {
    return RAF_LINUX_SYSCALL3(nr, 1, ptr, len);
}
