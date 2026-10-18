#ifndef RAFAELIA_BITRAF_W20_PAIR40_V1_H
#define RAFAELIA_BITRAF_W20_PAIR40_V1_H
/*
 * Two valid W20 words packed into exactly five bytes, little-bit-order V1.
 * w0 bits 0..19; w1 bits 20..39; bytes b0..b4.
 * The inputs are validated with the original W20 parity/address decoder.
 * This is exact serialization, not compression or error correction.
 *
 * No libc, memory allocation, I/O, syscall, global state, OS endianness
 * assumption or unaligned read.
 */
#include "bitraf_w20_bridge_v1.h"
#define RAF_W20_PAIR40_BYTES 5u

/* Caller buffers must be at least five bytes and non-overlapping with inputs. */
static inline bitraf_u32 raf_w20_pair40_write(bitraf_u32 w0, bitraf_u32 w1,
                                             bitraf_u8 out[5]) {
    raf_w20_decoded temp;
    bitraf_u8 b[5];
    bitraf_u32 error;
    if (!out) return RAF_W20_ERR_OUTPUT;
    error = raf_w20_unpack(w0, &temp);
    if (error) return error;
    error = raf_w20_unpack(w1, &temp);
    if (error) return error;
    b[0] = (bitraf_u8)(w0 & 255u);
    b[1] = (bitraf_u8)((w0 >> 8u) & 255u);
    b[2] = (bitraf_u8)(((w0 >> 16u) & 15u) | ((w1 & 15u) << 4u));
    b[3] = (bitraf_u8)((w1 >> 4u) & 255u);
    b[4] = (bitraf_u8)((w1 >> 12u) & 255u);
    out[0] = b[0]; out[1] = b[1]; out[2] = b[2];
    out[3] = b[3]; out[4] = b[4];
    return RAF_W20_OK;
}

/* Write both output words only after both are fully validated. */
static inline bitraf_u32 raf_w20_pair40_read(const bitraf_u8 in[5],
                                            bitraf_u32 *out0,
                                            bitraf_u32 *out1) {
    bitraf_u32 w0;
    bitraf_u32 w1;
    bitraf_u32 error;
    raf_w20_decoded temp;
    if (!in) return RAF_W20_ERR_INPUT;
    if (!out0 || !out1 || out0 == out1) return RAF_W20_ERR_OUTPUT;
    w0 = (bitraf_u32)in[0] |
         ((bitraf_u32)in[1] << 8u) |
         (((bitraf_u32)in[2] & 15u) << 16u);
    w1 = ((bitraf_u32)in[2] >> 4u) |
         ((bitraf_u32)in[3] << 4u) |
         ((bitraf_u32)in[4] << 12u);
    error = raf_w20_unpack(w0, &temp);
    if (error) return error;
    error = raf_w20_unpack(w1, &temp);
    if (error) return error;
    *out0 = w0;
    *out1 = w1;
    return RAF_W20_OK;
}
#endif
