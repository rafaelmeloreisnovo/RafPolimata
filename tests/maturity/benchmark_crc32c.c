#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "raf_crc32c.h"

#define BUF_SIZE (1024u * 1024u)
#define LOOPS 16u
#define SAMPLES 31u

static unsigned char buf[BUF_SIZE + 8u];
static volatile uint32_t sink_crc;

static uint64_t now_ns(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static uint64_t cycle_counter(void) {
#if defined(__x86_64__)
    uint32_t lo, hi;
    __asm__ volatile("lfence\n\trdtsc" : "=a"(lo), "=d"(hi) :: "memory");
    return ((uint64_t)hi << 32) | lo;
#else
    return 0;
#endif
}

int main(void) {
    for (uint32_t i = 0; i < BUF_SIZE + 8u; i++) buf[i] = (unsigned char)((i * 131u + 17u) & 0xffu);

    for (uint32_t s = 0; s < SAMPLES; s++) {
        uint32_t crc = s;
        uint64_t c0 = cycle_counter();
        uint64_t t0 = now_ns();
        if (!t0) return 2;
        for (uint32_t k = 0; k < LOOPS; k++) {
            crc = crc32c_buf(buf + (k & 7u), BUF_SIZE, crc);
        }
        uint64_t t1 = now_ns();
        uint64_t c1 = cycle_counter();
        if (!t1 || t1 < t0 || c1 < c0) return 3;
        sink_crc = crc;
        printf("sample=%u ns=%llu cycles=%llu bytes=%llu crc=%u\n",
               s,
               (unsigned long long)(t1 - t0),
               (unsigned long long)(c1 - c0),
               (unsigned long long)BUF_SIZE * LOOPS,
               crc);
    }
    return sink_crc == 0xffffffffu ? 4 : 0;
}
