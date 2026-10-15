/*
 * Hosted test harness only. Freestanding logic under test is
 * rafaelia/bitraf_w20_bridge_v1.h; no private data or device ABI claim.
 */
#include <stdio.h>
#include "../rafaelia/bitraf_w20_bridge_v1.h"

static int failure(const char *step, unsigned a, unsigned b) {
    fprintf(stderr, "W20_FAIL:%s:%u:%u\n", step, a, b);
    return 1;
}

int main(void) {
    bitraf_u32 records = 0u;
    bitraf_u32 single_bit_checks = 0u;
    bitraf_u32 double_undetected = 0u;
    bitraf_u32 i;
    bitraf_u32 k;
    bitraf_u32 bit;
    bitraf_u32 word = 0u;
    raf_w20_decoded decoded;

    for (i = 0u; i < BITRAF_STATES_TOTAL; ++i) {
        bitraf_state state = bitraf_from_index8000((bitraf_u16)i);
        for (k = 0u; k < RAF_W20_SUBVALUES; ++k) {
            bitraf_u32 r;
            raf_w20_decoded sentinel;
            sentinel.cell = 65535u;
            if (raf_w20_pack(state, (bitraf_u8)k, &word))
                return failure("pack", i, k);
            if (word & ~RAF_W20_WORD_MASK)
                return failure("unexpected_high_bits", i, k);
            if (raf_w20_unpack(word, &decoded))
                return failure("decode", i, k);
            if (!bitraf_equal(decoded.state, state) ||
                decoded.payload5 != k ||
                decoded.cell != bitraf_cell1000(state))
                return failure("roundtrip", i, k);
            ++records;

            for (bit = 0u; bit < 20u; ++bit) {
                sentinel.state.x = 255u;
                sentinel.payload5 = 255u;
                sentinel.cell = 65535u;
                sentinel.value = 255u;
                r = raf_w20_unpack(word ^ (1u << bit), &sentinel);
                if (!r) return failure("single_bit_undetected", i, bit);
                if (sentinel.state.x != 255u ||
                    sentinel.payload5 != 255u ||
                    sentinel.cell != 65535u ||
                    sentinel.value != 255u)
                    return failure("fail_closed_out_was_mutated", i, bit);
                ++single_bit_checks;
            }

            /* Two flips inside V8 can evade one parity equation. */
            if (k == 0u && i == 0u) {
                r = raf_w20_unpack(word ^ (1u << 2u) ^ (1u << 3u), &decoded);
                if (r || bitraf_equal(decoded.state, state) &&
                    decoded.payload5 == k)
                    return failure("double_bit_limitation_not_observed", i, k);
                ++double_undetected;
            }
        }
    }

    if (records != RAF_W20_STATES ||
        single_bit_checks != (RAF_W20_STATES * RAF_W20_WORD_BITS))
        return failure("counts", records, single_bit_checks);

    {
        bitraf_state invalid = {10u, 0u, 0u, 0u, 0u};
        bitraf_state valid = {0u, 0u, 0u, 0u, 0u};
        bitraf_u32 old = 0xA5A5A5A5u;
        if (!raf_w20_pack(invalid, 0u, &old) || old != 0xA5A5A5A5u)
            return failure("invalid_state_failclosed", old, 0u);
        if (!raf_w20_pack(valid, 32u, &old) || old != 0xA5A5A5A5u)
            return failure("invalid_payload_failclosed", old, 0u);
        if (!raf_w20_pack(valid, 0u, (bitraf_u32 *)0))
            return failure("null_pack", 0u, 0u);
        if (!raf_w20_unpack(0u, (raf_w20_decoded *)0))
            return failure("null_unpack", 0u, 0u);
        if (!raf_w20_unpack(1u << 20u, &decoded))
            return failure("reserved_high_bits", 0u, 0u);
        if (!raf_w20_unpack((1000u << 10u), &decoded))
            return failure("reserved_cell", 0u, 0u);
    }

    printf("W20_BRIDGE=PASS_SCOPED\n");
    printf("RECORDS=%u SINGLE_BIT_FAULTS=%u\n",
           (unsigned)records, (unsigned)single_bit_checks);
    printf("KNOWN_UNDETECTED_DOUBLE_FAULTS=%u\n",
           (unsigned)double_undetected);
    printf("CLAIM_ALLOWED=false PHYSICAL_ARM32=TOKEN_VAZIO\n");
    return 0;
}
