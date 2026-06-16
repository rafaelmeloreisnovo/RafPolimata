#ifndef RAFAELIA_COMMON_H
#define RAFAELIA_COMMON_H

/* expose clock_gettime/CLOCK_MONOTONIC under strict -std=c11 (POSIX.1-2008) */
#if defined(__linux__) || defined(__ANDROID__)
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include <stdint.h>
#include <stddef.h>

#define RAFA_STATIC_ASSERT(cond, name) typedef char static_assert_##name[(cond) ? 1 : -1]
#define RAFA_MMIO8(addr)  (*(volatile uint8_t *)(uintptr_t)(addr))
#define RAFA_MMIO16(addr) (*(volatile uint16_t *)(uintptr_t)(addr))
#define RAFA_MMIO32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))

static inline uint32_t rafa_u32_min(uint32_t a, uint32_t b) { return a < b ? a : b; }

#endif
