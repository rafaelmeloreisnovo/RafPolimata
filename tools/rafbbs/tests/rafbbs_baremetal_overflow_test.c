#include "rafbbs_baremetal.h"
int main(void) {
    RafBaremetalOut out;
    RafU32 i;
    raf_baremetal_out_init(&out);
    for (i = 0u; i < RAFBBS_BAREMETAL_OUT_CAP + 3u; i++)
        raf_baremetal_putc(&out, (RafU8)'x');
    if (out.pos != RAFBBS_BAREMETAL_OUT_CAP) return 1;
    if (out.dropped != 3u) return 2;
    return 0;
}
