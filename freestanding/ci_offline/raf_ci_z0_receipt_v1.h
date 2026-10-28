#ifndef RAF_CI_Z0_RECEIPT_V1_H
#define RAF_CI_Z0_RECEIPT_V1_H
/* Optional Z0 sidecar: 24 bytes. Existing 40-byte CI wire/v2 unchanged.
 * [0:4] Z0R1 magic, [4:8] version, [8:12] typed byte-absence code,
 * [12:16] original byte length, [16:20] raw CRC32C (not authentication),
 * [20:24] receipt CRC32C. All caller-owned, little endian, no heap.
 */
#include "raf_ci_wire_v2.h"
#define RAF_CI_Z0_RECEIPT_BYTES 24u
RAF_CI_INLINE void raf_ci_z0_receipt_v1_encode(const unsigned char *p,unsigned int n,unsigned char out[24]) {
    unsigned int i,t=raf_ci_z0_classify(p,n),raw=(p && n<=RAF_CI_MAX_INPUT) ? raf_ci_crc32c(p,n) : 0u;
    for(i=0u;i<24u;++i)out[i]=0u;
    out[0]='Z';out[1]='0';out[2]='R';out[3]='1';
    raf_ci_le32_put(out+4,1u);
    raf_ci_le32_put(out+8,t);
    raf_ci_le32_put(out+12,n);
    raf_ci_le32_put(out+16,raw);
    raf_ci_le32_put(out+20,raf_ci_crc32c(out,20u));
}
#endif
