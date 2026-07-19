#include "omega_core.h"

#define RAF_OMEGA_FNV_OFFSET 2166136261u
#define RAF_OMEGA_FNV_PRIME 16777619u
#define RAF_OMEGA_CELL_BASE RAF_OMEGA_HEADER_WORDS

static raf_omega_u32 raf_omega_mix(raf_omega_u32 x)
{
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

void raf_omega_init(raf_omega_state *s, raf_omega_u32 seed)
{
    raf_omega_u32 i;
    for (i = 0u; i < (raf_omega_u32)RAF_OMEGA_WORDS; ++i) {
        s->q[i] = 0u;
    }
    s->q[0] = raf_omega_mix(seed ^ RAF_OMEGA_FNV_OFFSET);
    s->q[1] = seed;
    s->q[2] = RAF_OMEGA_CELLS;
    s->q[3] = RAF_OMEGA_AXES;
    s->q[4] = 1u;
    s->q[5] = 0u;
    s->q[6] = 0u;
    s->q[7] = 0u;
    for (i = 0u; i < (raf_omega_u32)RAF_OMEGA_AXES; ++i) {
        s->q[8u + i] = 0u;
    }
    s->q[7] = raf_omega_digest(s);
}

void raf_omega_set_friction(raf_omega_state *s, raf_omega_u32 axis, raf_omega_u32 q8)
{
    s->q[8u + (axis & 7u)] = q8 & 255u;
    s->q[7] = raf_omega_digest(s);
}

raf_omega_u32 raf_omega_step(raf_omega_state *s, raf_omega_u32 token, raf_omega_u32 axis)
{
    const raf_omega_u32 a = axis & 7u;
    raf_omega_u32 idx = raf_omega_mix(token ^ s->q[0] ^ s->q[5]) & 1023u;
    const raf_omega_u32 mixed_seed = idx >= 1000u ? 1000u : 0u;
    raf_omega_u32 p;
    raf_omega_u32 mixed;
    raf_omega_u32 loss;
    raf_omega_u32 next;

    idx -= mixed_seed;
    p = RAF_OMEGA_CELL_BASE + idx;
    mixed = raf_omega_mix(s->q[p] ^ token ^ s->q[0] ^ (a << 24));
    loss = ((mixed & 65535u) * s->q[8u + a]) >> 8;
    next = mixed - loss;

    s->q[p] = next;
    s->q[0] = raf_omega_mix(s->q[0] ^ next ^ idx);
    s->q[1] ^= token;
    s->q[2] += (next ^ (next >> 16)) & 255u;
    s->q[3] = (s->q[3] << 5) | (s->q[3] >> 27);
    s->q[3] ^= next + a;
    s->q[4] += 1u;
    s->q[5] = idx;
    s->q[6] = a;
    s->q[7] = raf_omega_digest(s);
    return next;
}

raf_omega_u32 raf_omega_digest(const raf_omega_state *s)
{
    raf_omega_u32 h = RAF_OMEGA_FNV_OFFSET;
    raf_omega_u32 i;
    for (i = 0u; i < (raf_omega_u32)RAF_OMEGA_WORDS; ++i) {
        raf_omega_u32 x = (i == 7u) ? 0u : s->q[i];
        h ^= x;
        h *= RAF_OMEGA_FNV_PRIME;
    }
    return h;
}

raf_omega_u32 raf_omega_validate(const raf_omega_state *s)
{
    raf_omega_u32 i;
    raf_omega_u32 ok = 1u;
    ok &= (s->q[2] >= (raf_omega_u32)RAF_OMEGA_CELLS);
    ok &= (s->q[3] != 0u);
    ok &= (s->q[4] != 0u);
    for (i = 0u; i < (raf_omega_u32)RAF_OMEGA_AXES; ++i) {
        ok &= (s->q[8u + i] <= 255u);
    }
    ok &= (s->q[7] == raf_omega_digest(s));
    return ok;
}
