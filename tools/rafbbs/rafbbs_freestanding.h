#ifndef RAFBBS_FREESTANDING_H
#define RAFBBS_FREESTANDING_H
#include <stdint.h>

#define RAFBBS_NO_HEAP 1u
#define RAFBBS_NO_GC 1u
#define RAFBBS_STATIC_CAP 512u
#define RAFBBS_WATCHDOG_DEFAULT_TICKS 1000000u
#define RAFBBS_ROLLBACK_SLOTS 4u
#define RAFBBS_TOKEN_EMPTY "TOKEN_VAZIO"

typedef struct {
    uint32_t tick;
    uint32_t budget;
    uint32_t tripped;
} RafWatchdog;

typedef struct {
    uint32_t step;
    uint32_t status;
    uint32_t input_crc32;
    uint32_t output_crc32;
} RafRollbackFrame;

typedef struct {
    RafRollbackFrame frame[RAFBBS_ROLLBACK_SLOTS];
    uint32_t head;
} RafRollbackRing;

typedef struct {
    uint32_t no_heap;
    uint32_t no_gc;
    uint32_t branchless_hint;
    uint32_t syscall_free_hint;
    uint32_t lowlevel_flags;
} RafFreestandingFlags;

static inline RafWatchdog raf_watchdog_start(uint32_t budget) {
    RafWatchdog w;
    w.tick = 0u;
    w.budget = budget ? budget : RAFBBS_WATCHDOG_DEFAULT_TICKS;
    w.tripped = 0u;
    return w;
}

static inline uint32_t raf_watchdog_step(RafWatchdog *w) {
    uint32_t over;
    w->tick += 1u;
    over = (uint32_t)(w->tick >= w->budget);
    w->tripped |= over;
    return w->tripped;
}

static inline void raf_rollback_push(RafRollbackRing *r, RafRollbackFrame f) {
    r->frame[r->head & (RAFBBS_ROLLBACK_SLOTS - 1u)] = f;
    r->head += 1u;
}

static inline RafRollbackFrame raf_rollback_last(const RafRollbackRing *r) {
    uint32_t idx = (r->head - 1u) & (RAFBBS_ROLLBACK_SLOTS - 1u);
    return r->frame[idx];
}

static inline RafFreestandingFlags raf_freestanding_flags(void) {
    RafFreestandingFlags f;
    f.no_heap = RAFBBS_NO_HEAP;
    f.no_gc = RAFBBS_NO_GC;
    f.branchless_hint = 1u;
    f.syscall_free_hint = 1u;
    f.lowlevel_flags = 0xB8500001u;
    return f;
}
#endif
