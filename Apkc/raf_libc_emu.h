#ifndef RAF_LIBC_EMU_H
#define RAF_LIBC_EMU_H

/* RAFAELIA freestanding C compatibility layer.
 *
 * Runtime contract:
 * - no heap and no hosted libc dependency;
 * - storage is static or caller-owned;
 * - functions below are bounded by their explicit arguments;
 * - strtoul overflow saturates at ULONG_MAX because errno is intentionally
 *   absent from the strict runtime.
 */

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

#ifndef ULONG_MAX
#define ULONG_MAX __ULONG_MAX__
#endif

#ifndef NULL
# ifdef __cplusplus
#  define NULL nullptr
# else
#  define NULL ((void *)0)
# endif
#endif

#ifndef __cplusplus
typedef _Bool bool;
#define true 1
#define false 0
#endif

#if defined(__GNUC__) || defined(__clang__)
#define RAF_INLINE static __inline__ __attribute__((always_inline))
#define RAF_EXPORT __attribute__((visibility("default"), used))
#define RAF_NORETURN __attribute__((noreturn))
#else
#define RAF_INLINE static inline
#define RAF_EXPORT
#define RAF_NORETURN
#endif

#if defined(__cplusplus)
static_assert(sizeof(uint8_t) == 1u, "uint8_t width");
static_assert(sizeof(uint16_t) == 2u, "uint16_t width");
static_assert(sizeof(uint32_t) == 4u, "uint32_t width");
static_assert(sizeof(uint64_t) == 8u, "uint64_t width");
#define RAF_MUTABLE_CHAR_PTR(p) const_cast<char *>(p)
#else
_Static_assert(sizeof(uint8_t) == 1u, "uint8_t width");
_Static_assert(sizeof(uint16_t) == 2u, "uint16_t width");
_Static_assert(sizeof(uint32_t) == 4u, "uint32_t width");
_Static_assert(sizeof(uint64_t) == 8u, "uint64_t width");
#define RAF_MUTABLE_CHAR_PTR(p) ((char *)(p))
#endif

RAF_INLINE void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0u; i < n; ++i) d[i] = s[i];
    return dst;
}

RAF_INLINE void *memmove(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    if (d == s || n == 0u) return dst;

    /* Relational comparison of pointers to unrelated objects is undefined in C.
     * Integer addresses provide the ordering needed by this low-level routine. */
    if ((uintptr_t)d < (uintptr_t)s) {
        for (size_t i = 0u; i < n; ++i) d[i] = s[i];
    } else {
        for (size_t i = n; i != 0u; --i) d[i - 1u] = s[i - 1u];
    }
    return dst;
}

RAF_INLINE void *memset(void *dst, int value, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t v = (uint8_t)value;
    for (size_t i = 0u; i < n; ++i) d[i] = v;
    return dst;
}

RAF_INLINE int memcmp(const void *lhs, const void *rhs, size_t n) {
    const uint8_t *a = (const uint8_t *)lhs;
    const uint8_t *b = (const uint8_t *)rhs;
    for (size_t i = 0u; i < n; ++i) {
        if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
    }
    return 0;
}

RAF_INLINE void *memchr(const void *src, int value, size_t n) {
    const uint8_t *s = (const uint8_t *)src;
    const uint8_t needle = (uint8_t)value;
    for (size_t i = 0u; i < n; ++i) {
        if (s[i] == needle) return (void *)(uintptr_t)(s + i);
    }
    return NULL;
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

RAF_INLINE char *strcpy(char *dst, const char *src) {
    size_t i = 0u;
    do {
        dst[i] = src[i];
    } while (src[i++] != '\0');
    return dst;
}

RAF_INLINE char *strncpy(char *dst, const char *src, size_t n) {
    size_t i = 0u;
    while (i < n && src[i] != '\0') {
        dst[i] = src[i];
        ++i;
    }
    while (i < n) dst[i++] = '\0';
    return dst;
}

RAF_INLINE char *strchr(const char *s, int c) {
    const char needle = (char)c;
    for (;;) {
        if (*s == needle) return RAF_MUTABLE_CHAR_PTR(s);
        if (*s == '\0') return NULL;
        ++s;
    }
}

RAF_INLINE char *strrchr(const char *s, int c) {
    const char needle = (char)c;
    const char *last = NULL;
    for (;;) {
        if (*s == needle) last = s;
        if (*s == '\0') return last == NULL ? NULL : RAF_MUTABLE_CHAR_PTR(last);
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

RAF_INLINE int raf_digit_value(char c) {
    if (c >= '0' && c <= '9') return (int)(c - '0');
    if (c >= 'a' && c <= 'z') return (int)(c - 'a' + 10);
    if (c >= 'A' && c <= 'Z') return (int)(c - 'A' + 10);
    return -1;
}

RAF_INLINE unsigned long strtoul(const char *s, char **end, int base) {
    const char *original = s;
    unsigned long value = 0ul;
    unsigned digits = 0u;
    int negative = 0;
    int overflow = 0;

    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') ++s;
    if (*s == '+' || *s == '-') {
        negative = (*s == '-');
        ++s;
    }

    if (base != 0 && (base < 2 || base > 36)) {
        if (end != NULL) *end = RAF_MUTABLE_CHAR_PTR(original);
        return 0ul;
    }

    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X') &&
        raf_digit_value(s[2]) >= 0 && raf_digit_value(s[2]) < 16) {
        base = 16;
        s += 2;
    } else if (base == 0) {
        base = s[0] == '0' ? 8 : 10;
    }

    for (;;) {
        const int decoded = raf_digit_value(*s);
        if (decoded < 0 || decoded >= base) break;
        const unsigned long digit = (unsigned long)decoded;
        if (value > (ULONG_MAX - digit) / (unsigned long)base) {
            value = ULONG_MAX;
            overflow = 1;
        } else if (!overflow) {
            value = value * (unsigned long)base + digit;
        }
        ++digits;
        ++s;
    }

    if (digits == 0u) {
        if (end != NULL) *end = RAF_MUTABLE_CHAR_PTR(original);
        return 0ul;
    }
    if (end != NULL) *end = RAF_MUTABLE_CHAR_PTR(s);
    if (negative && !overflow) value = 0ul - value;
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

/* Fail-closed names: diagnostics point to the required architecture. */
#define malloc(...) RAF_HEAP_FORBIDDEN__use_static_or_caller_owned_buffer
#define calloc(...) RAF_HEAP_FORBIDDEN__use_static_or_caller_owned_buffer
#define realloc(...) RAF_HEAP_FORBIDDEN__use_static_or_caller_owned_buffer
#define free(...) RAF_HEAP_FORBIDDEN__no_heap_object_exists

#endif
