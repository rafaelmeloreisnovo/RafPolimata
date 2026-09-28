#include "rafpolimata_v1.h"

static rafp_u32 rafp_crc32c_byte(rafp_u32 crc, rafp_u8 byte) {
    crc ^= byte;
    for (rafp_u32 bit = 0; bit < 8u; bit++) {
        rafp_u32 mask = 0u - (crc & 1u);
        crc = (crc >> 1) ^ (0x82F63B78u & mask);
    }
    return crc;
}

rafp_u32 rafp_v1_crc32c(const void *data, rafp_size len, rafp_u32 seed) {
    if (!data && len != 0u) return 0u;
    const rafp_u8 *p = (const rafp_u8 *)data;
    rafp_u32 crc = ~seed;
    while (len--) crc = rafp_crc32c_byte(crc, *p++);
    return ~crc;
}

rafp_q16 rafp_v1_q16_mul(rafp_q16 a, rafp_q16 b) {
    return (rafp_q16)(((rafp_s64)a * (rafp_s64)b) >> 16);
}

rafp_q16 rafp_v1_q16_iir(rafp_q16 state, rafp_q16 input) {
    return state - (state >> 2) + (input >> 2);
}

rafp_q16 rafp_v1_q16_abs_sat(rafp_q16 value) {
    rafp_u32 x = (rafp_u32)value;
    if (x == 0x80000000u) return (rafp_q16)0x7fffffffu;
    rafp_u32 mask = 0u - (x >> 31);
    return (rafp_q16)((x ^ mask) + (mask & 1u));
}

void rafp_v1_arena_init(rafp_arena_v1 *arena, void *storage, rafp_u32 capacity) {
    if (!arena) return;
    arena->base = (rafp_u8 *)storage;
    arena->capacity = storage ? capacity : 0u;
    arena->top = 0u;
    arena->peak = 0u;
}

void *rafp_v1_arena_alloc(rafp_arena_v1 *arena, rafp_u32 size) {
    if (!arena || !arena->base) return (void *)0;
    if (size > arena->capacity) return (void *)0;
    if (size > 0xffffffffu - (RAFP_V1_ARENA_ALIGN - 1u)) return (void *)0;
    rafp_u32 aligned = (size + (RAFP_V1_ARENA_ALIGN - 1u)) & ~(RAFP_V1_ARENA_ALIGN - 1u);
    if (arena->top > arena->capacity || aligned > arena->capacity - arena->top) return (void *)0;
    rafp_u32 old = arena->top;
    arena->top = old + aligned;
    if (arena->top > arena->peak) arena->peak = arena->top;
    return arena->base + old;
}

void rafp_v1_arena_reset(rafp_arena_v1 *arena) {
    if (arena) arena->top = 0u;
}

rafp_u32 rafp_v1_arena_used(const rafp_arena_v1 *arena) {
    return arena ? arena->top : 0u;
}

rafp_u32 rafp_v1_arena_peak(const rafp_arena_v1 *arena) {
    return arena ? arena->peak : 0u;
}
