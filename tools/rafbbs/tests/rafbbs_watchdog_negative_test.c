#include "rafbbs_freestanding.h"
int main(void) {
    RafWatchdog w = raf_watchdog_start(1u);
    if (raf_watchdog_step(&w) == 0u) return 1;
    return 0;
}
