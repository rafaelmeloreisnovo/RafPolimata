/*
 * compiler/raf_matrix_geometry_v1.h
 * 8 positions x 3 layers x 2 contextual oppositions = 48 typed slots.
 * Historical source: RAFAELIA NOVOexport March-July 2025, notably May.
 *
 * This is a freestanding integer geometry/compilation primitive. No libc,
 * heap, dynamic allocation, global state, OS calls, or external references.
 * Matrix composition is a projection; it does not recover 48 originals.
 * No claim about quantum physics, spacetime, compression or error correction.
 */
#ifndef RAF_MATRIX_GEOMETRY_V1_H
#define RAF_MATRIX_GEOMETRY_V1_H

#define RAF_GEO_POSITIONS 8u
#define RAF_GEO_LAYERS 3u
#define RAF_GEO_OPPOSITIONS 2u
#define RAF_GEO_SLOTS 48u
#define RAF_GEO_Q16_ONE 65536u
#define RAF_GEO_Q16_SQRT3_OVER2 56756u

typedef unsigned char raf_geo_u8;
typedef unsigned long long raf_geo_u64;

/* Canonical order: position, then layer, then opposition. */
static inline int raf_geo_slot(unsigned p, unsigned l, unsigned o,
                               unsigned *slot) {
    if (!slot || p >= RAF_GEO_POSITIONS || l >= RAF_GEO_LAYERS ||
        o >= RAF_GEO_OPPOSITIONS) return 0;
    *slot = (p * RAF_GEO_LAYERS + l) * RAF_GEO_OPPOSITIONS + o;
    return 1;
}

static inline int raf_geo_mirror(unsigned p, unsigned axis,
                                 unsigned *reflected) {
    if (!reflected || p >= 8u || axis >= 3u) return 0;
    *reflected = p ^ (1u << axis);
    return 1;
}

/* Exact integer cube corners: bit 0/1/2 -> x/y/z, even/odd parity. */
static inline int raf_geo_corner(unsigned p, int xyz[3], unsigned *parity) {
    if (!xyz || !parity || p >= 8u) return 0;
    xyz[0] = (p & 1u) ? -1 : 1;
    xyz[1] = (p & 2u) ? -1 : 1;
    xyz[2] = (p & 4u) ? -1 : 1;
    *parity = ((p & 1u) ^ ((p >> 1u) & 1u) ^ ((p >> 2u) & 1u));
    return 1;
}

/* Handles up to 7-dimensional nested matrix coordinates without overflow. */
static inline int raf_geo_path(const raf_geo_u64 *dims,
                               const raf_geo_u64 *indices,
                               unsigned depth, raf_geo_u64 *linear) {
    raf_geo_u64 pos = 0u;
    const raf_geo_u64 maximum = ~(raf_geo_u64)0;
    unsigned i;
    if (!dims || !indices || !linear || depth == 0u || depth > 7u) return 0;
    for (i = 0u; i < depth; ++i) {
        if (!dims[i] || indices[i] >= dims[i] ||
            pos > (maximum - indices[i]) / dims[i]) return 0;
        pos = pos * dims[i] + indices[i];
    }
    *linear = pos;
    return 1;
}

/* Index a 10^3 or 1000^3 conceptual grid without materializing its memory. */
static inline int raf_geo_cubic_offset(unsigned side,
                                       unsigned x, unsigned y, unsigned z,
                                       raf_geo_u64 *offset) {
    raf_geo_u64 dims[3];
    raf_geo_u64 coords[3];
    if (!offset || side == 0u || side > 1000u) return 0;
    dims[0] = dims[1] = dims[2] = (raf_geo_u64)side;
    coords[0] = (raf_geo_u64)x;
    coords[1] = (raf_geo_u64)y;
    coords[2] = (raf_geo_u64)z;
    return raf_geo_path(dims, coords, 3u, offset);
}

/* Explicit Q16 geometric attenuation q=round(65536*sqrt(3)/2).
 * Quantization is not invertible; after sufficient depth it may round to 0.
 */
static inline int raf_geo_q16_scale(unsigned depth, unsigned *out) {
    raf_geo_u64 current = RAF_GEO_Q16_ONE;
    unsigned i;
    if (!out || depth > 48u) return 0;
    for (i = 0u; i < depth; ++i) {
        current = (current * RAF_GEO_Q16_SQRT3_OVER2 + 32768u) >> 16u;
    }
    *out = (unsigned)current;
    return 1;
}

/* 48 typed input bytes -> 3 layer XOR projections, 2 context projections,
 * one central projection. Caller retains source bytes for reconstruction.
 * XOR fold is lossy and is never treated as a reversible codec.
 */
static inline int raf_geo_fold48(const raf_geo_u8 *source48,
                                 raf_geo_u8 layer3[3],
                                 raf_geo_u8 opposition2[2],
                                 raf_geo_u8 *central) {
    unsigned p, l, o, slot;
    if (!source48 || !layer3 || !opposition2 || !central) return 0;
    for (l = 0u; l < 3u; ++l) layer3[l] = 0u;
    for (o = 0u; o < 2u; ++o) opposition2[o] = 0u;
    *central = 0u;
    for (p = 0u; p < 8u; ++p) {
        for (l = 0u; l < 3u; ++l) {
            for (o = 0u; o < 2u; ++o) {
                slot = (p * 3u + l) * 2u + o;
                layer3[l] ^= source48[slot];
                opposition2[o] ^= source48[slot];
                *central ^= source48[slot];
            }
        }
    }
    return 1;
}
#endif /* RAF_MATRIX_GEOMETRY_V1_H */
