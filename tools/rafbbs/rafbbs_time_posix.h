#ifndef RAFBBS_TIME_POSIX_H
#define RAFBBS_TIME_POSIX_H

/*
 * Hosted POSIX monotonic-clock adapter.
 *
 * This file is intentionally outside the authorial freestanding set:
 * clock_gettime/CLOCK_MONOTONIC are provider/OS services.  The adapter only
 * converts that observation into the authorial RafMonoTime value contract.
 */

#include <time.h>
#include "rafbbs_time.h"

static inline RafMonoTime raf_mono_posix_now(void) {
    struct timespec ts;
    RafMonoTime out = raf_mono_invalid();
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return out;
    out = raf_mono_from_ns(
        ((RafU64)ts.tv_sec * RAFBBS_NS_PER_SEC) + (RafU64)ts.tv_nsec
    );
    return out;
}

#endif
