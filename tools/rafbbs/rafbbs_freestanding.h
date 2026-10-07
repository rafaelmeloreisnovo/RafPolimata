#ifndef RAFBBS_FREESTANDING_H
#define RAFBBS_FREESTANDING_H

#include "rafbbs_types.h"

#define RAFBBS_NO_HEAP 1u
#define RAFBBS_NO_GC 1u
#define RAFBBS_STATIC_CAP 512u
#define RAFBBS_WATCHDOG_DEFAULT_TICKS 1000000u
#define RAFBBS_ROLLBACK_SLOTS 4u
#define RAFBBS_TOKEN_EMPTY "TOKEN_VAZIO"

typedef struct {
    RafU32 tick;
    RafU32 budget;
    RafU32 tripped;
} RafWatchdog;

typedef struct {
    RafU32 step;
    RafU32 status;
    RafU32 input_crc32;
    RafU32 output_crc32;
} RafRollbackFrame;

typedef struct {
    RafRollbackFrame frame[RAFBBS_ROLLBACK_SLOTS];
    RafU32 head;
} RafRollbackRing;

typedef struct {
    RafU32 no_heap;
    RafU32 no_gc;
    RafU32 branchless_hint;
    RafU32 syscall_free_hint;
    RafU32 lowlevel_flags;
} RafFreestandingFlags;

static inline RafWatchdog raf_watchdog_start(RafU32 budget) {
    RafWatchdog w;
    w.tick = 0u;
    w.budget = budget ? budget : RAFBBS_WATCHDOG_DEFAULT_TICKS;
    w.tripped = 0u;
    return w;
}

static inline RafU32 raf_watchdog_step(RafWatchdog *w) {
    RafU32 over;
    w->tick += 1u;
    over = (RafU32)(w->tick >= w->budget);
    w->tripped |= over;
    return w->tripped;
}

static inline void raf_rollback_push(RafRollbackRing *r, RafRollbackFrame f) {
    r->frame[r->head & (RAFBBS_ROLLBACK_SLOTS - 1u)] = f;
    r->head += 1u;
}

static inline RafRollbackFrame raf_rollback_last(const RafRollbackRing *r) {
    RafU32 idx = (r->head - 1u) & (RAFBBS_ROLLBACK_SLOTS - 1u);
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
