#include <stdio.h>
#include <stdint.h>
#include "raf_crc32c.h"
#include "raf_q16.h"
#include "raf_arena.h"
#include "raf_fsm.h"
#include "rafpolimata_v1.h"

#define PROPERTY_CASES 2000000u

static uint64_t rng_state = 0x9e3779b97f4a7c15ULL;

static uint32_t next32(void) {
    rng_state ^= rng_state << 7;
    rng_state ^= rng_state >> 9;
    rng_state ^= rng_state << 8;
    return (uint32_t)(rng_state ^ (rng_state >> 32));
}

static int streq(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

static int test_arena_boundaries(void) {
    static const uint32_t sizes[] = {0u,1u,7u,8u,9u,65527u,65528u,65529u,65535u,65536u,65537u,0xffffffffu};
    static Arena internal;
    _Alignas(RAFP_V1_ARENA_ALIGN) static unsigned char storage[ARENA_CAP];
    rafp_arena_v1 public_arena;

    for (unsigned i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        internal.top = internal.peak = 0u;
        rafp_v1_arena_init(&public_arena, storage, ARENA_CAP);
        void *a = arena_alloc(&internal, sizes[i]);
        void *b = rafp_v1_arena_alloc(&public_arena, sizes[i]);
        if ((a == 0) != (b == 0)) return 20;
        if (internal.top != public_arena.top) return 21;
        if (a && (((uintptr_t)a & (ARENA_ALIGN - 1u)) != 0u)) return 22;
        if (b && (((uintptr_t)b & (RAFP_V1_ARENA_ALIGN - 1u)) != 0u)) return 23;
    }
    return 0;
}

static int test_arena_aliasing_alignment(void) {
    static Arena internal;
    _Alignas(RAFP_V1_ARENA_ALIGN) static unsigned char storage[256];
    _Alignas(RAFP_V1_ARENA_ALIGN) static unsigned char misaligned[64 + RAFP_V1_ARENA_ALIGN];
    static const uint32_t sizes[] = {1u, 7u, 8u, 9u, 15u, 31u, 64u};
    rafp_arena_v1 public_arena;

    internal.top = internal.peak = 0u;
    rafp_v1_arena_init(&public_arena, storage, (rafp_u32)sizeof(storage));

    uintptr_t prev_i_end = 0u, prev_p_end = 0u;
    for (unsigned i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        uint32_t sz = sizes[i];
        void *pi = arena_alloc(&internal, sz);
        void *pp = rafp_v1_arena_alloc(&public_arena, sz);
        if (!pi || !pp) return 30;
        uintptr_t ai = (uintptr_t)pi;
        uintptr_t ap = (uintptr_t)pp;
        if ((ai & (ARENA_ALIGN - 1u)) != 0u) return 31;
        if ((ap & (RAFP_V1_ARENA_ALIGN - 1u)) != 0u) return 32;
        if (prev_i_end && ai < prev_i_end) return 33;
        if (prev_p_end && ap < prev_p_end) return 34;
        prev_i_end = ai + sz;
        prev_p_end = ap + sz;
        if (internal.top != public_arena.top) return 35;
    }

    rafp_v1_arena_init(&public_arena, misaligned + 1u, 64u);
    if (public_arena.capacity != 0u || public_arena.base != (void *)0) return 36;
    if (rafp_v1_arena_alloc(&public_arena, 1u) != (void *)0) return 37;
    return 0;
}

int main(void) {
    unsigned char buf[32] = {0};
    static const unsigned char known[] = "123456789";
    if (crc32c_buf(known, 9u, 0u) != 0xe3069283u) return 1;
    if (rafp_v1_crc32c(known, 9u, 0u) != 0xe3069283u) return 2;

    for (uint32_t i = 0; i < PROPERTY_CASES; i++) {
        uint32_t r = next32();
        uint32_t off = r & 7u;
        uint32_t len = (r >> 3) & 7u;
        uint32_t seed = next32();
        for (uint32_t j = 0; j < off + len; j++) buf[j] = (unsigned char)next32();

        uint32_t specialized = crc32c_buf(buf + off, len, seed);
        uint32_t reference = rafp_v1_crc32c(buf + off, len, seed);
        if (specialized != reference) {
            fprintf(stderr, "CRC mismatch case=%u off=%u len=%u seed=%u got=%u ref=%u\n",
                    i, off, len, seed, specialized, reference);
            return 3;
        }

        q16_t a = (q16_t)((int32_t)(next32() % 131073u) - 65536);
        q16_t b = (q16_t)((int32_t)(next32() % 131073u) - 65536);
        if (q16_mul(a, b) != rafp_v1_q16_mul(a, b)) return 4;
        if (q16_iir(a, b) != rafp_v1_q16_iir(a, b)) return 5;
        if (q16_abs(a) != rafp_v1_q16_abs_sat(a)) return 6;
    }

    if (q16_abs((q16_t)0x80000000u) != 0x7fffffff) return 7;
    if (rafp_v1_q16_abs_sat((rafp_q16)0x80000000u) != 0x7fffffff) return 8;
    if (!streq(fsm_domain_name(10u), "INVALID_STATE")) return 9;
    if (!streq(attractor_name((AttractorClass)6), "INVALID_CLASS")) return 10;

    int arena_rc = test_arena_boundaries();
    if (arena_rc) return arena_rc;
    arena_rc = test_arena_aliasing_alignment();
    if (arena_rc) return arena_rc;

    printf("MATURITY_EQUIVALENCE_PASS cases=%u crc=reference_vs_specialized q16=exact arena=boundary_alignment_nonoverlap\n", PROPERTY_CASES);
    return 0;
}
