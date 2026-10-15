#ifndef RAFAELIA_BITRAF_W20_BRIDGE_V1_H
#define RAFAELIA_BITRAF_W20_BRIDGE_V1_H

/*
 * Experimental, reversibly typed bridge:
 *   BITRAF8000 state S=(x,y,z,vertex2,phase1), with payload5 in 0..31,
 *   maps bijectively to 256000 valid W20 records.
 *
 * This is an additive experimental profile, not an alteration of the
 * canonical BITRAF8000 state definition or evidence of physical hardware.
 *
 * Logical W20 [19:10]=A10, [9:2]=V8, [1]=pA, [0]=pV
 * A10 = 100*x + 10*y + z
 * V8  = (payload5 << 3) | (vertex << 1) | phase
 * pA = XOR of address bits; pV = XOR of value bits
 * Non-zero high bits in the containing unsigned-32 storage are forbidden.
 * Two even-bit flips in one field may escape detection: NOT ECC/SECDED.
 *
 * Core is freestanding C, no library, heap, I/O or syscall.
 */
#include "bitraf_matrix.h"

typedef char raf_w20_requires_u32[(sizeof(bitraf_u32) == 4u) ? 1 : -1];
typedef char raf_w20_requires_u8[(sizeof(bitraf_u8) == 1u) ? 1 : -1];

#define RAF_W20_WORD_BITS     20u
#define RAF_W20_STATES       256000u
#define RAF_W20_SUBVALUES    32u
#define RAF_W20_WORD_MASK    0x000FFFFFu
#define RAF_W20_ADDR_MASK    0x000003FFu
#define RAF_W20_VALUE_MASK   0x000000FFu

#define RAF_W20_OK                0u
#define RAF_W20_ERR_ADDRESS       1u
#define RAF_W20_ERR_VALUE         2u
#define RAF_W20_ERR_RESERVED      4u
#define RAF_W20_ERR_OUTPUT        8u
#define RAF_W20_ERR_INPUT       16u

typedef struct raf_w20_decoded {
    bitraf_state state;
    bitraf_u8 payload5;
    bitraf_u16 cell;
    bitraf_u8 value;
} raf_w20_decoded;

static inline bitraf_u8 raf_w20_odd_parity(bitraf_u32 value) {
    value ^= value >> 16u;
    value ^= value >> 8u;
    value ^= value >> 4u;
    value &= 15u;
    return (bitraf_u8)((0x6996u >> value) & 1u);
}

/* Return nonzero typed error; do not modify output on failure. */
static inline bitraf_u32 raf_w20_pack(bitraf_state state,
                                     bitraf_u8 payload5,
                                     bitraf_u32 *out_word) {
    bitraf_u32 cell;
    bitraf_u32 value;
    bitraf_u32 word;
    if (!out_word) return RAF_W20_ERR_OUTPUT;
    if (!bitraf_state_valid(state) || payload5 >= RAF_W20_SUBVALUES)
        return RAF_W20_ERR_INPUT;
    cell = (bitraf_u32)bitraf_cell1000(state);
    value = ((bitraf_u32)payload5 << 3u) |
            ((bitraf_u32)state.vertex << 1u) |
            (bitraf_u32)state.parity;
    word = (cell << 10u) | (value << 2u) |
           ((bitraf_u32)raf_w20_odd_parity(cell) << 1u) |
           (bitraf_u32)raf_w20_odd_parity(value);
    *out_word = word;
    return RAF_W20_OK;
}

/* Reject bad parity, out-of-range addresses and high storage bits.
 * Error precedence is a bitmask. Caller output remains unchanged on FAIL. */
static inline bitraf_u32 raf_w20_unpack(bitraf_u32 word,
                                       raf_w20_decoded *out) {
    bitraf_u32 cell;
    bitraf_u32 value;
    bitraf_u32 error = 0u;
    bitraf_state state;
    raf_w20_decoded parsed;
    if (!out) return RAF_W20_ERR_OUTPUT;
    if (word & ~RAF_W20_WORD_MASK) error |= RAF_W20_ERR_RESERVED;
    cell = (word >> 10u) & RAF_W20_ADDR_MASK;
    value = (word >> 2u) & RAF_W20_VALUE_MASK;
    if (cell >= BITRAF_VISIBLE_CELLS) error |= RAF_W20_ERR_ADDRESS;
    if (raf_w20_odd_parity(cell) != ((word >> 1u) & 1u))
        error |= RAF_W20_ERR_ADDRESS;
    if (raf_w20_odd_parity(value) != (word & 1u))
        error |= RAF_W20_ERR_VALUE;
    if (error != 0u) return error;
    state.z = (bitraf_u8)(cell % 10u);
    state.y = (bitraf_u8)((cell / 10u) % 10u);
    state.x = (bitraf_u8)(cell / 100u);
    state.vertex = (bitraf_u8)((value >> 1u) & 3u);
    state.parity = (bitraf_u8)(value & 1u);
    parsed.state = state;
    parsed.payload5 = (bitraf_u8)(value >> 3u);
    parsed.cell = (bitraf_u16)cell;
    parsed.value = (bitraf_u8)value;
    *out = parsed;
    return RAF_W20_OK;
}

#endif /* RAFAELIA_BITRAF_W20_BRIDGE_V1_H */
