#ifndef RAFAELIA_BITRAF_EVIDENCE_PLANES_V1_H
#define RAFAELIA_BITRAF_EVIDENCE_PLANES_V1_H
/* Additive experimental 3-plane P/N/A evidence overlay.
 * AUTHORIAL CONTRACT: original bitraf_state, index8000 and core_index4096
 * remain unchanged. The 2 x 4096-bit pages below are SERIALIZATION PAGES,
 * NOT the historical core4096/shell3904 geometric partition.
 * Logical bitplanes: P=4, N=2, A=1. All 8 masks are preserved; 0=unknown.
 * Host/device timestamp, provenance, actor and append-only receipts must be
 * stored separately; this accumulator is not by itself an append-only ledger.
 * The operations only add bits (never clear) and fail closed on invalid input.
 * No libc, heap, I/O, syscalls, OS ABI, compression, hashing or ECC.
 */
#include "bitraf_matrix.h"
#define RAF_EV3_PAGES 2u
#define RAF_EV3_PAGE_BITS 4096u
#define RAF_EV3_PLANE_BYTES 512u
#define RAF_EV3_PAGES_BYTES 3072u
#define RAF_EV3_POSITIVE 4u
#define RAF_EV3_NEGATIVE 2u
#define RAF_EV3_ABSENCE 1u
#define RAF_EV3_UNKNOWN 0u
#define RAF_EV3_MASK 7u
#define RAF_EV3_OK 0u
#define RAF_EV3_ERR_INPUT 1u
#define RAF_EV3_ERR_OUTPUT 2u

typedef char raf_ev3_u8_must_be_byte[(sizeof(bitraf_u8)==1u)?1:-1];
typedef char raf_ev3_total_must_fit_two_pages[(BITRAF_STATES_TOTAL<=RAF_EV3_PAGES*RAF_EV3_PAGE_BITS)?1:-1];
typedef struct raf_ev3_planes {
    bitraf_u8 pages[RAF_EV3_PAGES][3][RAF_EV3_PLANE_BYTES];
} raf_ev3_planes;

/* Initialization is an explicit caller responsibility: zero-initialize the
 * storage before first use. Never use an uninitialized instance. */
static inline bitraf_u32 raf_ev3_zero(raf_ev3_planes *out) {
    bitraf_u32 i,j,k;
    if (!out) return RAF_EV3_ERR_OUTPUT;
    for(i=0u;i<RAF_EV3_PAGES;i++)
        for(j=0u;j<3u;j++)
            for(k=0u;k<RAF_EV3_PLANE_BYTES;k++) out->pages[i][j][k]=0u;
    return RAF_EV3_OK;
}

/* Each record can accumulate positive/negative/verified-absence information.
 * Unknown is never converted to verified absence. Prior bits are never removed. */
static inline bitraf_u32 raf_ev3_observe(raf_ev3_planes *db,bitraf_u16 index,
                                        bitraf_u8 evidence_mask) {
    bitraf_u32 pg, byte, shift, i;
    bitraf_u8 bit;
    if (!db || (bitraf_u32)index >= BITRAF_STATES_TOTAL ||
        evidence_mask==0u || (evidence_mask&~RAF_EV3_MASK)!=0u)
        return RAF_EV3_ERR_INPUT;
    pg=(bitraf_u32)index/RAF_EV3_PAGE_BITS;
    byte=((bitraf_u32)index%RAF_EV3_PAGE_BITS)>>3u;
    shift=(bitraf_u32)index&7u;
    bit=(bitraf_u8)(1u<<shift);
    for(i=0u;i<3u;i++)
        if ((evidence_mask & (bitraf_u8)(1u<<i))!=0u)
            db->pages[pg][i][byte]=(bitraf_u8)(db->pages[pg][i][byte]|bit);
    return RAF_EV3_OK;
}
static inline bitraf_u32 raf_ev3_read(const raf_ev3_planes *db,bitraf_u16 index,
                                     bitraf_u8 *out_mask) {
    bitraf_u32 pg, byte, shift, i;
    bitraf_u8 mask=0u;
    if (!out_mask) return RAF_EV3_ERR_OUTPUT;
    if (!db || (bitraf_u32)index>=BITRAF_STATES_TOTAL) return RAF_EV3_ERR_INPUT;
    pg=(bitraf_u32)index/RAF_EV3_PAGE_BITS;
    byte=((bitraf_u32)index%RAF_EV3_PAGE_BITS)>>3u;
    shift=(bitraf_u32)index&7u;
    for(i=0u;i<3u;i++)
        mask=(bitraf_u8)(mask | ((((db->pages[pg][i][byte]>>shift)&1u))<<i));
    *out_mask=mask;
    return RAF_EV3_OK;
}
/* Format conversion ONLY: index8000 <-> three base-20 digits.
 * This does NOT reinterpret (x,y,z,vertex,parity) as 20^3 geometry. */
static inline bitraf_u32 raf_ev3_index_to_digits20(bitraf_u16 index,
        bitraf_u8 *d0,bitraf_u8 *d1,bitraf_u8 *d2) {
    if(!d0||!d1||!d2) return RAF_EV3_ERR_OUTPUT;
    if((bitraf_u32)index>=BITRAF_STATES_TOTAL) return RAF_EV3_ERR_INPUT;
    *d0=(bitraf_u8)(index/400u);
    *d1=(bitraf_u8)((index/20u)%20u);
    *d2=(bitraf_u8)(index%20u);
    return RAF_EV3_OK;
}
static inline bitraf_u32 raf_ev3_digits20_to_index(bitraf_u8 d0,bitraf_u8 d1,
        bitraf_u8 d2,bitraf_u16 *out_index) {
    if(!out_index) return RAF_EV3_ERR_OUTPUT;
    if(d0>=20u||d1>=20u||d2>=20u) return RAF_EV3_ERR_INPUT;
    *out_index=(bitraf_u16)((bitraf_u16)d0*400u+(bitraf_u16)d1*20u+d2);
    return RAF_EV3_OK;
}
/* Resolve historical core coordinate without aliasing it to serialization page.
 * Returns 1 for a valid shell state, 2 for invalid input. Output unchanged
 * on any failure, as the original core_index4096 contract requires. */
static inline bitraf_u32 raf_ev3_historical_core_index(bitraf_state s,
        bitraf_u16 *out_core_index) {
    if (!out_core_index) return RAF_EV3_ERR_OUTPUT;
    if (!bitraf_state_valid(s)) return RAF_EV3_ERR_INPUT;
    if (!bitraf_is_core(s)) return RAF_EV3_ERR_INPUT;
    return (bitraf_u32)bitraf_core_index4096(s,out_core_index);
}
#endif
