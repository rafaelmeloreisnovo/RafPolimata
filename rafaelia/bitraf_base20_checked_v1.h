#ifndef RAFAELIA_BITRAF_BASE20_CHECKED_V1_H
#define RAFAELIA_BITRAF_BASE20_CHECKED_V1_H
/* Strict additive base20 boundary. Does NOT map FIAT64/base60/ZIPRAF.
 * encode: out has >=4 bytes; decode: in has exactly 3 readable bytes.
 * Errors leave caller output unchanged. No libc/heap/syscalls.
 */
#include "bitraf_matrix.h"
#define RAF_B20C_OK 0u
#define RAF_B20C_ERR_INPUT 1u
#define RAF_B20C_ERR_OUTPUT 2u

static inline bitraf_u8 raf_b20c_encode(bitraf_u32 index, char out[4]) {
    if (!out) return RAF_B20C_ERR_OUTPUT;
    if (index >= BITRAF_STATES_TOTAL) return RAF_B20C_ERR_INPUT;
    bitraf_to_base20_3((bitraf_u16)index, out);
    return RAF_B20C_OK;
}
static inline bitraf_u8 raf_b20c_decode(const char *in, bitraf_u32 len,
                                        bitraf_u16 *out) {
    bitraf_u32 i;
    bitraf_u16 tmp;
    if (!out) return RAF_B20C_ERR_OUTPUT;
    if (!in || len != BITRAF_BASE20_DIGITS) return RAF_B20C_ERR_INPUT;
    for (i=0u; i<BITRAF_BASE20_DIGITS; ++i) {
        char c=in[i];
        if (!((c>='0' && c<='9') || (c>='A' && c<='J')))
            return RAF_B20C_ERR_INPUT;
    }
    if (bitraf_from_base20_3(in, &tmp)) return RAF_B20C_ERR_INPUT;
    *out=tmp;
    return RAF_B20C_OK;
}
static inline bitraf_u8 raf_b20c_from_state(bitraf_state s, char out[4]) {
    if (!out) return RAF_B20C_ERR_OUTPUT;
    if (!bitraf_state_valid(s)) return RAF_B20C_ERR_INPUT;
    return raf_b20c_encode((bitraf_u32)bitraf_index8000(s),out);
}
#endif
