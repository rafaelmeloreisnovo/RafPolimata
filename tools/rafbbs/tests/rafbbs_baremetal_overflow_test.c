#include "rafbbs_baremetal.h"
int main(void) {
    RafBaremetalOut out;
    uint32_t i;
    raf_baremetal_out_init(&out);
    for (i = 0u; i < RAFBBS_BAREMETAL_OUT_CAP + 3u; i++) raf_baremetal_putc(&out, (uint8_t)'x');
    if (out.pos != RAFBBS_BAREMETAL_OUT_CAP) return 1;
    if (out.dropped != 3u) return 2;
    return 0;
}
