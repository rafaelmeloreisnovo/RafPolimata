/* Hosted falsifier for freestanding compiler/raf_matrix_geometry_v1.h.
 * No Android or physical quantum claim. Returns nonzero on any failure.
 */
#include "../compiler/raf_matrix_geometry_v1.h"

static int test_all(void) {
    unsigned p, axis, reflected, back, slot, parity, scale, i;
    int xyz[3];
    raf_geo_u64 idx = 0u;
    raf_geo_u64 dims[3] = {8u, 3u, 2u};
    raf_geo_u64 good[3] = {7u, 2u, 1u};
    raf_geo_u64 bad[3] = {8u, 2u, 1u};
    raf_geo_u64 overflow_dims[3] = {~(raf_geo_u64)0, 2u, 3u};
    raf_geo_u64 overflow_idx[3] = {~(raf_geo_u64)0 - 1u, 1u, 1u};
    raf_geo_u8 values[48], layer[3], ctx[2], central;
    raf_geo_u8 expected_layer[3] = {0u, 0u, 0u};
    raf_geo_u8 expected_ctx[2] = {0u, 0u};
    raf_geo_u8 expected_center = 0u;
    unsigned l, o;

    for (p = 0u; p < 8u; ++p) {
        if (!raf_geo_corner(p, xyz, &parity)) return 1;
        if (xyz[0] != ((p & 1u) ? -1 : 1)) return 2;
        if (xyz[1] != ((p & 2u) ? -1 : 1)) return 3;
        if (xyz[2] != ((p & 4u) ? -1 : 1)) return 4;
        if (parity != (((p & 1u) ^ ((p >> 1u) & 1u) ^
                         ((p >> 2u) & 1u)))) return 5;
        for (axis = 0u; axis < 3u; ++axis) {
            if (!raf_geo_mirror(p, axis, &reflected)) return 6;
            if (!raf_geo_mirror(reflected, axis, &back) || back != p) return 7;
        }
        for (l = 0u; l < 3u; ++l) {
            for (o = 0u; o < 2u; ++o) {
                if (!raf_geo_slot(p, l, o, &slot)) return 8;
                if (slot != (p*3u + l)*2u + o) return 9;
                values[slot] = (raf_geo_u8)(slot * 5u + 3u);
                expected_layer[l] ^= values[slot];
                expected_ctx[o] ^= values[slot];
                expected_center ^= values[slot];
            }
        }
    }
    if (raf_geo_mirror(8u, 0u, &reflected) ||
        raf_geo_mirror(0u, 3u, &reflected) ||
        raf_geo_mirror(0u, 0u, 0)) return 10;
    if (raf_geo_slot(8u, 0u, 0u, &slot) ||
        raf_geo_slot(0u, 3u, 0u, &slot) ||
        raf_geo_slot(0u, 0u, 2u, &slot)) return 11;
    if (raf_geo_corner(8u, xyz, &parity)) return 12;
    if (!raf_geo_path(dims, good, 3u, &idx) || idx != 47u) return 13;
    if (raf_geo_path(dims, bad, 3u, &idx) ||
        raf_geo_path(overflow_dims, overflow_idx, 3u, &idx)) return 14;
    if (!raf_geo_cubic_offset(10u, 1u, 2u, 3u, &idx) ||
        idx != 123u) return 15;
    if (!raf_geo_cubic_offset(1000u, 999u, 999u, 999u, &idx) ||
        idx != 999999999u) return 16;
    if (raf_geo_cubic_offset(1000u, 1000u, 0u, 0u, &idx) ||
        raf_geo_cubic_offset(1001u, 0u, 0u, 0u, &idx)) return 17;
    if (!raf_geo_q16_scale(0u, &scale) || scale != 65536u ||
        !raf_geo_q16_scale(1u, &scale) || scale != 56756u ||
        raf_geo_q16_scale(49u, &scale)) return 18;
    if (!raf_geo_fold48(values, layer, ctx, &central) ||
        central != expected_center) return 19;
    for (i = 0u; i < 3u; ++i)
        if (layer[i] != expected_layer[i]) return 20;
    for (i = 0u; i < 2u; ++i)
        if (ctx[i] != expected_ctx[i]) return 21;
    if (central != (raf_geo_u8)(ctx[0] ^ ctx[1])) return 22;
    values[0] ^= 1u;
    if (!raf_geo_fold48(values, layer, ctx, &central) ||
        central != (raf_geo_u8)(expected_center ^ 1u)) return 23;
    if (raf_geo_fold48(0, layer, ctx, &central) ||
        raf_geo_fold48(values, 0, ctx, &central) ||
        raf_geo_fold48(values, layer, 0, &central) ||
        raf_geo_fold48(values, layer, ctx, 0)) return 24;
    return 0;
}
int main(void) { return test_all(); }
