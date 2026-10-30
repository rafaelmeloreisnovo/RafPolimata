/* ARM32 freestanding compile/link-import boundary only; not hardware execution. */
#include "../rafaelia/bitraf_evidence_planes_v1.h"
static raf_ev3_planes ev3;
bitraf_u32 raf_ev3_freestanding_probe(bitraf_u16 index) {
    bitraf_u8 mask=0u,d0=0u,d1=0u,d2=0u;
    bitraf_u16 back=0u;
    if (raf_ev3_zero(&ev3)) return 1u;
    if (raf_ev3_index_to_digits20(index,&d0,&d1,&d2)) return 2u;
    if (raf_ev3_digits20_to_index(d0,d1,d2,&back) || back!=index) return 3u;
    if (raf_ev3_observe(&ev3,index,RAF_EV3_POSITIVE)) return 4u;
    if (raf_ev3_read(&ev3,index,&mask) || mask!=RAF_EV3_POSITIVE) return 5u;
    return 0u;
}
