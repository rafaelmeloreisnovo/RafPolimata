/*
 * voynich_toroidal_v2.c
 * Deterministic simulation harness. It does not model or decode MS 408.
 * No historical/linguistic claim is emitted by this program.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIDE 10u
#define CELLS (SIDE * SIDE * SIDE)
#define PAGES 240u

typedef struct {
    uint16_t page_id;
    uint8_t has_content;
    uint8_t rgb[3];
    uint32_t energy_q16;
} Cell;

static Cell torus[CELLS];

static uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static uint32_t index3(uint32_t x, uint32_t y, uint32_t z) {
    return (x * SIDE + y) * SIDE + z;
}

static void init_torus(uint32_t seed) {
    uint16_t order[CELLS];
    uint32_t state = seed ? seed : 1u;
    for (uint32_t i = 0; i < CELLS; ++i) {
        order[i] = (uint16_t)i;
        memset(&torus[i], 0, sizeof(torus[i]));
    }
    for (uint32_t i = CELLS - 1u; i > 0u; --i) {
        uint32_t j = xorshift32(&state) % (i + 1u);
        uint16_t t = order[i]; order[i] = order[j]; order[j] = t;
    }
    for (uint32_t p = 0; p < PAGES; ++p) {
        Cell *c = &torus[order[p]];
        c->has_content = 1u;
        c->page_id = (uint16_t)(p + 1u);
        c->rgb[0] = (uint8_t)xorshift32(&state);
        c->rgb[1] = (uint8_t)xorshift32(&state);
        c->rgb[2] = (uint8_t)xorshift32(&state);
        c->energy_q16 = xorshift32(&state) & 0xffffu;
    }
}

static uint32_t fnv1a32(uint32_t h, uint32_t v) {
    for (unsigned i = 0; i < 4; ++i) {
        h ^= (v >> (i * 8u)) & 0xffu;
        h *= 16777619u;
    }
    return h;
}

/* Convention only: digits are cyclic movements over x,y,z; 0 is a pause. */
static uint32_t walk(const char *name, const char *sequence, uint32_t steps) {
    uint32_t x = 0, y = 0, z = 0, h = 2166136261u;
    const size_t n = strlen(sequence);
    printf("mode=%s sequence=%s convention=cyclic_xyz_zero_pause\n", name, sequence);
    for (uint32_t step = 0; step < steps; ++step) {
        const unsigned digit = (unsigned)(sequence[step % n] - '0');
        const uint32_t idx = index3(x, y, z);
        const Cell *c = &torus[idx];
        h = fnv1a32(h, idx);
        h = fnv1a32(h, c->page_id);
        printf("step=%u xyz=%u,%u,%u digit=%u page=%u present=%u rgb=%u,%u,%u energy_q16=%u\n",
               step, x, y, z, digit, c->page_id, c->has_content,
               c->rgb[0], c->rgb[1], c->rgb[2], c->energy_q16);
        if (digit != 0u) {
            switch (step % 3u) {
                case 0u: x = (x + digit) % SIDE; break;
                case 1u: y = (y + digit) % SIDE; break;
                default: z = (z + digit) % SIDE; break;
            }
        }
    }
    printf("mode=%s walk_fnv1a32=%08x\n", name, h);
    return h;
}

int main(int argc, char **argv) {
    uint32_t seed = 144000u;
    uint32_t steps = 20u;
    if (argc > 1) seed = (uint32_t)strtoul(argv[1], NULL, 0);
    if (argc > 2) steps = (uint32_t)strtoul(argv[2], NULL, 0);
    init_torus(seed);
    printf("status=SIMULATION_ONLY claim_allowed=false seed=%u pages=%u cells=%u\n",
           seed, PAGES, CELLS);
    walk("RAW", "123", steps);
    walk("JPEG", "0123", steps);
    walk("GIF", "01123", steps);
    walk("EXEC", "0001123", steps);
    return 0;
}
