#include <stdio.h>
#include "../rafaelia/bitraf_matrix.h"

static int fail(const char *name, unsigned value) {
    fprintf(stderr, "FAIL:%s:%u\n", name, value);
    return 1;
}

int main(void) {
    unsigned i;
    unsigned core_count = 0u;
    unsigned shell_count = 0u;
    unsigned prime_count = 0u;
    bitraf_u64 max_norm2 = 0u;

    for (i = 0u; i < BITRAF_STATES_TOTAL; ++i) {
        bitraf_state s = bitraf_from_index8000((bitraf_u16)i);
        bitraf_state o;
        bitraf_state r;
        bitraf_vec3_q16 e;
        bitraf_u64 n2;
        char b20[4];
        bitraf_u16 decoded = 0u;
        bitraf_u16 core = 0u;

        if (!bitraf_state_valid(s)) return fail("valid", i);
        if (bitraf_index8000(s) != i) return fail("roundtrip8000", i);

        bitraf_to_base20_3((bitraf_u16)i, b20);
        if (bitraf_from_base20_3(b20, &decoded) || decoded != i)
            return fail("base20", i);

        o = bitraf_opposite(s);
        if (!bitraf_equal(bitraf_opposite(o), s)) return fail("opposite", i);
        if (!(bitraf_relation(s, o) & BITRAF_REL_OPPOSITE))
            return fail("opposite_relation", i);

        r = bitraf_rotate_z90(s);
        r = bitraf_rotate_z90(r);
        r = bitraf_rotate_z90(r);
        r = bitraf_rotate_z90(r);
        if (!bitraf_equal(r, s)) return fail("rotation4", i);

        e = bitraf_embed_b3_q16(s);
        n2 = bitraf_norm2_q32(e);
        if (n2 >= (bitraf_u64)BITRAF_Q16_ONE * (bitraf_u64)BITRAF_Q16_ONE)
            return fail("b3_open_ball", i);
        if (n2 > max_norm2) max_norm2 = n2;

        if (bitraf_is_core(s)) {
            bitraf_state back;
            ++core_count;
            if (bitraf_core_index4096(s, &core)) return fail("core_encode", i);
            if (core >= BITRAF_STATES_CORE) return fail("core_range", i);
            back = bitraf_from_core_index4096(core);
            if (!bitraf_equal(s, back)) return fail("core_roundtrip", i);
        } else {
            ++shell_count;
            if (!bitraf_core_index4096(s, &core)) return fail("shell_rejected", i);
        }

        if (bitraf_is_prime_u16((bitraf_u16)i)) ++prime_count;
    }

    if (core_count != BITRAF_STATES_CORE) return fail("core_count", core_count);
    if (shell_count != BITRAF_STATES_SHELL) return fail("shell_count", shell_count);
    if (core_count + shell_count != BITRAF_STATES_TOTAL)
        return fail("partition", core_count + shell_count);

    {
        bitraf_state a = {5u, 5u, 5u, 0u, 0u};
        bitraf_state n = {6u, 5u, 5u, 0u, 0u};
        bitraf_state p = {5u, 5u, 5u, 0u, 1u};
        bitraf_state v = {5u, 5u, 5u, 3u, 0u};
        if (!(bitraf_relation(a, n) & BITRAF_REL_ORTHO_NEIGHBOR)) return fail("neighbor", 0u);
        if (!(bitraf_relation(a, p) & BITRAF_REL_PARITY_TWIN)) return fail("parity_twin", 0u);
        if (!(bitraf_relation(a, v) & BITRAF_REL_VERTEX_SIBLING)) return fail("vertex_sibling", 0u);
    }

    printf("BITRAF_MATRIX_V1=PASS\n");
    printf("TOTAL=%u CORE=%u SHELL=%u BASE20=20^3 CORE_BITS=12\n",
           BITRAF_STATES_TOTAL, core_count, shell_count);
    printf("PRIMES_0_7999=%u MAX_NORM2_Q32=%llu\n",
           prime_count, (unsigned long long)max_norm2);
    printf("INVARIANTS=8000/8000 ROUNDTRIP;4096/4096 CORE;3904 SHELL;OPPOSITE^2;ROTATE^4;B3_OPEN\n");
    return 0;
}
