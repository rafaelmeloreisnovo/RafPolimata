#include "rafbbs_freestanding.h"
int rafbbs_freestanding_core_test(void) {
    RafWatchdog w = raf_watchdog_start(1u);
    RafRollbackRing r = {{{0u, 0u, 0u, 0u}}, 0u};
    RafRollbackFrame f = {1u, 2u, 3u, 4u};
    RafFreestandingFlags flags = raf_freestanding_flags();
    raf_rollback_push(&r, f);
    return (int)(raf_watchdog_step(&w) + raf_rollback_last(&r).step + flags.no_heap + flags.no_gc);
}
