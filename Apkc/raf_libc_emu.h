#ifndef RAF_LIBC_EMU_H
#define RAF_LIBC_EMU_H

/* RAFAELIA freestanding C compatibility layer.
 * Build-plane headers/runtimes are not carried into the final artifact.
 * Storage remains static or caller-owned; heap APIs intentionally fail closed. */

typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;
typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __PTRDIFF_TYPE__ ssize_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef __cplusplus
typedef _Bool bool;
#define true 1
#define false 0
#endif

#if defined(__GNUC__) || defined(__clang__)
#define RAF_INLINE static __inline__ __attribute__((always_inline))
#define RAF_EXPORT __attribute__((visibility("default"), used))
#else
#define RAF_INLINE static inline
#define RAF_EXPORT
#endif

RAF_INLINE void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < n; ++i) d[i] = s[i];
    return dst;
}

RAF_INLINE void *memmove(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    if (d == s || n == 0u) return dst;
    if (d < s) {
        for (size_t i = 0; i < n; ++i) d[i] = s[i];
    } else {
        for (size_t i = n; i != 0u; --i) d[i - 1u] = s[i - 1u];
    }
    return dst;
}

RAF_INLINE void *memset(void *dst, int value, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t v = (uint8_t)value;
    for (size_t i = 0; i < n; ++i) d[i] = v;
    return dst;
}

RAF_INLINE int memcmp(const void *lhs, const void *rhs, size_t n) {
    const uint8_t *a = (const uint8_t *)lhs;
    const uint8_t *b = (const uint8_t *)rhs;
    for (size_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
    }
    return 0;
}

RAF_INLINE size_t strlen(const char *s) {
    size_t n = 0u;
    while (s[n] != '\0') ++n;
    return n;
}

RAF_INLINE size_t strnlen(const char *s, size_t cap) {
    size_t n = 0u;
    while (n < cap && s[n] != '\0') ++n;
    return n;
}

RAF_INLINE int strcmp(const char *a, const char *b) {
    size_t i = 0u;
    while (a[i] != '\0' && a[i] == b[i]) ++i;
    return (unsigned char)a[i] < (unsigned char)b[i] ? -1
         : (unsigned char)a[i] > (unsigned char)b[i] ? 1 : 0;
}

RAF_INLINE int strncmp(const char *a, const char *b, size_t n) {
    for (size_t i = 0u; i < n; ++i) {
        const unsigned char ac = (unsigned char)a[i];
        const unsigned char bc = (unsigned char)b[i];
        if (ac != bc) return ac < bc ? -1 : 1;
        if (ac == 0u) return 0;
    }
    return 0;
}

RAF_INLINE char *strchr(const char *s, int c) {
    const char needle = (char)c;
    for (;;) {
        if (*s == needle) return (char *)(uintptr_t)s;
        if (*s == '\0') return NULL;
        ++s;
    }
}

RAF_INLINE char *strrchr(const char *s, int c) {
    const char needle = (char)c;
    const char *last = NULL;
    for (;;) {
        if (*s == needle) last = s;
        if (*s == '\0') return (char *)(uintptr_t)last;
        ++s;
    }
}

RAF_INLINE int atoi(const char *s) {
    int sign = 1;
    int value = 0;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') ++s;
    if (*s == '-') { sign = -1; ++s; }
    else if (*s == '+') { ++s; }
    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (*s - '0');
        ++s;
    }
    return sign * value;
}

RAF_INLINE unsigned long strtoul(const char *s, char **end, int base) {
    unsigned long value = 0ul;
    int detected = base;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') ++s;
    if (detected == 0) {
        detected = 10;
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { detected = 16; s += 2; }
        else if (s[0] == '0') { detected = 8; ++s; }
    } else if (detected == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
    }
    for (;;) {
        unsigned digit;
        if (*s >= '0' && *s <= '9') digit = (unsigned)(*s - '0');
        else if (*s >= 'a' && *s <= 'z') digit = (unsigned)(*s - 'a' + 10);
        else if (*s >= 'A' && *s <= 'Z') digit = (unsigned)(*s - 'A' + 10);
        else break;
        if (digit >= (unsigned)detected) break;
        value = value * (unsigned long)detected + digit;
        ++s;
    }
    if (end != NULL) *end = (char *)(uintptr_t)s;
    return value;
}

RAF_INLINE ssize_t raf_write(int fd, const void *buf, size_t len) {
#if defined(__aarch64__)
    register long x0 __asm__("x0") = (long)fd;
    register long x1 __asm__("x1") = (long)(uintptr_t)buf;
    register long x2 __asm__("x2") = (long)len;
    register long x8 __asm__("x8") = 64;
    __asm__ volatile("svc 0" : "+r"(x0) : "r"(x1), "r"(x2), "r"(x8) : "memory", "cc");
    return (ssize_t)x0;
#elif defined(__arm__)
    register long r0 __asm__("r0") = (long)fd;
    register long r1 __asm__("r1") = (long)(uintptr_t)buf;
    register long r2 __asm__("r2") = (long)len;
    register long r7 __asm__("r7") = 4;
    __asm__ volatile("svc 0" : "+r"(r0) : "r"(r1), "r"(r2), "r"(r7) : "memory", "cc");
    return (ssize_t)r0;
#elif defined(__x86_64__)
    register long rax __asm__("rax") = 1;
    register long rdi __asm__("rdi") = (long)fd;
    register long rsi __asm__("rsi") = (long)(uintptr_t)buf;
    register long rdx __asm__("rdx") = (long)len;
    __asm__ volatile("syscall" : "+a"(rax) : "D"(rdi), "S"(rsi), "d"(rdx) : "rcx", "r11", "memory", "cc");
    return (ssize_t)rax;
#elif defined(__riscv)
    register long a0 __asm__("a0") = (long)fd;
    register long a1 __asm__("a1") = (long)(uintptr_t)buf;
    register long a2 __asm__("a2") = (long)len;
    register long a7 __asm__("a7") = 64;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");
    return (ssize_t)a0;
#else
    (void)fd; (void)buf; (void)len;
    return (ssize_t)-1;
#endif
}

RAF_INLINE int putchar(int c) {
    const char ch = (char)c;
    return raf_write(1, &ch, 1u) == 1 ? (unsigned char)ch : -1;
}

RAF_INLINE int puts(const char *s) {
    const size_t n = strlen(s);
    if (raf_write(1, s, n) != (ssize_t)n) return -1;
    return putchar('\n') < 0 ? -1 : 0;
}

/* Fail-closed names: the diagnostic points to the required architecture. */
#define malloc(...) RAF_HEAP_FORBIDDEN__use_static_or_caller_owned_buffer
#define calloc(...) RAF_HEAP_FORBIDDEN__use_static_or_caller_owned_buffer
#define realloc(...) RAF_HEAP_FORBIDDEN__use_static_or_caller_owned_buffer
#define free(...) RAF_HEAP_FORBIDDEN__no_heap_object_exists

#endif
