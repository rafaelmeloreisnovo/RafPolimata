/* CLOSURE_L2 governs the device evidence gap, not ECC_GENERAL correction capability. */
/* Hosted exhaustive roundtrip and corruption harness for 5-byte W20 pairing. */
#include <stdio.h>
#include "../rafaelia/bitraf_w20_pair40_v1.h"

static int fail(const char *scope, unsigned a, unsigned b) {
    fprintf(stderr, "PAIR40_FAIL:%s:%u:%u\n", scope, a, b);
    return 1;
}

int main(void) {
    bitraf_u32 n;
    bitraf_u32 pending = 0u;
    bitraf_u32 pairs = 0u;
    bitraf_u32 single_flips = 0u;
    bitraf_u8 bytes[5];
    bitraf_u32 w0;
    bitraf_u32 w1;
    for (n = 0u; n < RAF_W20_STATES; ++n) {
        bitraf_state state = bitraf_from_index8000((bitraf_u16)(n / 32u));
        bitraf_u32 word;
        bitraf_u32 r = raf_w20_pack(state, (bitraf_u8)(n % 32u), &word);
        if (r) return fail("INPUT", n, r);
        if ((n & 1u) == 0u) {
            pending = word;
            continue;
        }
        if (raf_w20_pair40_write(pending, word, bytes))
            return fail("WRITE", n, 0u);
        if (raf_w20_pair40_read(bytes, &w0, &w1))
            return fail("READ", n, 0u);
        if (w0 != pending || w1 != word)
            return fail("ROUNDTRIP", n, 0u);

        if (n == 1u && (bytes[0] != 0u || bytes[1] != 0u ||
                        bytes[2] != 16u || bytes[3] != 2u ||
                        bytes[4] != 0u))
            return fail("GOLDEN_00_21", n, bytes[2]);

        {
            bitraf_u32 b;
            for (b = 0u; b < 40u; ++b) {
                bitraf_u8 altered[5];
                bitraf_u32 actual0 = 0xFFFFFFFFu;
                bitraf_u32 actual1 = 0xFFFFFFFFu;
                bitraf_u32 j;
                for (j = 0u; j < 5u; ++j) altered[j] = bytes[j];
                altered[b >> 3u] ^= (bitraf_u8)(1u << (b & 7u));
                if (!raf_w20_pair40_read(altered, &actual0, &actual1))
                    return fail("FLIP_UNDETECTED", n, b);
                if (actual0 != 0xFFFFFFFFu || actual1 != 0xFFFFFFFFu)
                    return fail("FAILCLOSED_MUTATED", n, b);
                ++single_flips;
            }
        }
        ++pairs;
    }

    if (pairs != 128000u || single_flips != 5120000u)
        return fail("COUNT", pairs, single_flips);
    {
        bitraf_u8 old[5] = {7u, 7u, 7u, 7u, 7u};
        bitraf_u32 w = 0u;
        bitraf_state state = {0u, 0u, 0u, 0u, 0u};
        bitraf_u32 r;
        raf_w20_pack(state, 0u, &w);
        r = raf_w20_pair40_write(w, 0x100000u, old);
        if (!r || old[0] != 7u || old[4] != 7u)
            return fail("INVALID_PAIR_WRITE", r, old[0]);
        if (!raf_w20_pair40_write(w, w, (bitraf_u8 *)0))
            return fail("NULL_WRITE", 0u, 0u);
        if (!raf_w20_pair40_read((bitraf_u8 *)0, &w0, &w1))
            return fail("NULL_READ", 0u, 0u);
        if (!raf_w20_pair40_read(old, &w0, &w0))
            return fail("ALIAS_OUTPUT", 0u, 0u);
    }
    printf("W20_PAIR40=PASS_SCOPED\n");
    printf("PAIRS=%u BYTES=%u SINGLE_BIT_FAULTS=%u\n",
           (unsigned)pairs, (unsigned)(pairs * 5u), (unsigned)single_flips);
    printf("CLAIM_ALLOWED=false ECC_GENERAL=false DEVICE=TOKEN_VAZIO\n");
    return 0;
}
