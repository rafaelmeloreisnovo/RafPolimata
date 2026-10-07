#include "rafbbs_time.h"

int rafbbs_time_core_test(void) {
    RafMonoTime start = raf_mono_from_ns(1000000000ull);
    RafMonoTime end = raf_mono_from_ns(2500000000ull);
    RafMonoTime invalid = raf_mono_invalid();
    RafMonoElapsed elapsed = raf_mono_elapsed_ms(start, end);
    RafMonoElapsed reversed = raf_mono_elapsed_ms(end, start);
    RafMonoElapsed missing = raf_mono_elapsed_ms(invalid, end);

    if (elapsed.valid != 1u) return 1;
    if (elapsed.ms != 1500ull) return 2;
    if (reversed.valid != 0u) return 3;
    if (reversed.ms != 0ull) return 4;
    if (missing.valid != 0u) return 5;
    if (missing.ms != 0ull) return 6;
    return 0;
}

#ifdef RAFBBS_TIME_TEST_MAIN
int main(void) {
    return rafbbs_time_core_test();
}
#endif
