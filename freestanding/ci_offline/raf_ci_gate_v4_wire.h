#ifndef RAF_CI_GATE_V4_WIRE_H
#define RAF_CI_GATE_V4_WIRE_H
/* Optional V4 source topology sidecar; V2 wire40 and Z0 sidecar24 unchanged.
 * LE bytes: RCG4 [0:4], version [4:8], state [8:12], urgency [12:16],
 * gaps [16:20], typed voids [20:24], FNV32 [24:28], CRC32C [28:32].
 * CRC32C is corruption detection, NOT signature/authenticated provenance.
 */
#include "raf_ci_gate_v4.h"
#define RAF_CI_G4_WIRE_BYTES 32u
RAF_CI_INLINE void raf_ci_g4_wire_encode(raf_ci_gate_v4 r,unsigned char out[RAF_CI_G4_WIRE_BYTES]) {
    unsigned int i;
    for(i=0u;i<RAF_CI_G4_WIRE_BYTES;++i)out[i]=0u;
    out[0]='R';out[1]='C';out[2]='G';out[3]='4';
    raf_ci_le32_put(out+4,4u);
    raf_ci_le32_put(out+8,r.state);
    raf_ci_le32_put(out+12,r.urgency);
    raf_ci_le32_put(out+16,r.gaps);
    raf_ci_le32_put(out+20,r.voids);
    raf_ci_le32_put(out+24,r.source_fingerprint);
    raf_ci_le32_put(out+28,raf_ci_crc32c(out,28u));
}
#endif
