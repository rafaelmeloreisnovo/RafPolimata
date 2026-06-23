#ifndef RAF_TRACE_FLAGS_H
#define RAF_TRACE_FLAGS_H

#include <stddef.h>
#include <stdint.h>

#define RAF_TRACEBUF_MAGIC 0x5241465452414345ULL
#define RAF_TRACEBUF_TEXT 1u
#define RAF_TRACEBUF_BIN 2u
#define RAF_TRACEBUF_TRUNC 0x80000000u

#ifndef RAF_TRACEBUF_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define RAF_TRACEBUF_INLINE static inline __attribute__((always_inline))
#else
#define RAF_TRACEBUF_INLINE static inline
#endif
#endif

#ifndef RAF_TRACEBUF_FEATURE_ARM64_TICK
#if defined(__aarch64__) && (defined(__GNUC__) || defined(__clang__))
#define RAF_TRACEBUF_FEATURE_ARM64_TICK 1
#else
#define RAF_TRACEBUF_FEATURE_ARM64_TICK 0
#endif
#endif

#endif
