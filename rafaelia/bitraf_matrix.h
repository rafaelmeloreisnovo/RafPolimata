#ifndef RAFAELIA_BITRAF_MATRIX_H
#define RAFAELIA_BITRAF_MATRIX_H

/*
 * BITRAF Matrix 8000/4096 V1
 *
 * Exact state space:
 *   visible lattice 10 x 10 x 10
 *   hidden tetrahedral vertex 4
 *   parity state 2
 *   total = 10^3 * 4 * 2 = 8000 = 20^3
 *
 * Binary core:
 *   inner lattice 8 x 8 x 8 (coordinates 1..8)
 *   hidden tetrahedral vertex 4
 *   parity state 2
 *   total = 8^3 * 4 * 2 = 4096 = 2^12
 *
 * The core is freestanding-friendly: fixed-width local types, no heap, no I/O.
 */

typedef unsigned char      bitraf_u8;
typedef unsigned short     bitraf_u16;
typedef unsigned int       bitraf_u32;
typedef unsigned long long bitraf_u64;
typedef signed short       bitraf_i16;
typedef signed int         bitraf_i32;

#define BITRAF_AXIS_VISIBLE      10u
#define BITRAF_AXIS_CORE          8u
#define BITRAF_VERTICES           4u
#define BITRAF_PARITIES           2u
#define BITRAF_VISIBLE_CELLS   1000u
#define BITRAF_CORE_CELLS       512u
#define BITRAF_SHELL_CELLS      488u
#define BITRAF_STATES_TOTAL    8000u
#define BITRAF_STATES_CORE     4096u
#define BITRAF_STATES_SHELL    3904u
#define BITRAF_BASE20_DIGITS      3u
#define BITRAF_Q16_ONE        65536

#define BITRAF_REL_NONE             0u
#define BITRAF_REL_SAME             (1u << 0)
#define BITRAF_REL_ORTHO_NEIGHBOR   (1u << 1)
#define BITRAF_REL_PARITY_TWIN      (1u << 2)
#define BITRAF_REL_VERTEX_SIBLING   (1u << 3)
#define BITRAF_REL_OPPOSITE         (1u << 4)
#define BITRAF_REL_ROTATE_Z90       (1u << 5)
#define BITRAF_REL_ROTATE_Z180      (1u << 6)
#define BITRAF_REL_ROTATE_Z270      (1u << 7)

typedef struct bitraf_state {
    bitraf_u8 x;
    bitraf_u8 y;
    bitraf_u8 z;
    bitraf_u8 vertex;
    bitraf_u8 parity;
} bitraf_state;

typedef struct bitraf_vec3_q16 {
    bitraf_i32 x;
    bitraf_i32 y;
    bitraf_i32 z;
} bitraf_vec3_q16;

typedef struct bitraf_hex_axial {
    bitraf_i16 q;
    bitraf_i16 r;
} bitraf_hex_axial;

static inline bitraf_u8 bitraf_state_valid(bitraf_state s) {
    return (bitraf_u8)(s.x < 10u && s.y < 10u && s.z < 10u &&
                       s.vertex < 4u && s.parity < 2u);
}

static inline bitraf_u16 bitraf_cell1000(bitraf_state s) {
    return (bitraf_u16)(((bitraf_u16)s.x * 10u + (bitraf_u16)s.y) * 10u +
                        (bitraf_u16)s.z);
}

static inline bitraf_u16 bitraf_index8000(bitraf_state s) {
    return (bitraf_u16)((bitraf_cell1000(s) * 4u + (bitraf_u16)s.vertex) * 2u +
                        (bitraf_u16)s.parity);
}

static inline bitraf_state bitraf_from_index8000(bitraf_u16 index) {
    bitraf_state s;
    bitraf_u16 cell;
    s.parity = (bitraf_u8)(index & 1u);
    index = (bitraf_u16)(index >> 1u);
    s.vertex = (bitraf_u8)(index & 3u);
    cell = (bitraf_u16)(index >> 2u);
    s.z = (bitraf_u8)(cell % 10u);
    cell = (bitraf_u16)(cell / 10u);
    s.y = (bitraf_u8)(cell % 10u);
    s.x = (bitraf_u8)(cell / 10u);
    return s;
}

static inline bitraf_u8 bitraf_is_core(bitraf_state s) {
    return (bitraf_u8)(bitraf_state_valid(s) &&
                       s.x >= 1u && s.x <= 8u &&
                       s.y >= 1u && s.y <= 8u &&
                       s.z >= 1u && s.z <= 8u);
}

/* Return 0 on success and 1 if the state belongs to the outer shell. */
static inline bitraf_u8 bitraf_core_index4096(bitraf_state s, bitraf_u16 *out) {
    bitraf_u16 cell;
    if (!out || !bitraf_is_core(s)) return 1u;
    cell = (bitraf_u16)((((bitraf_u16)s.x - 1u) * 8u +
                          ((bitraf_u16)s.y - 1u)) * 8u +
                         ((bitraf_u16)s.z - 1u));
    *out = (bitraf_u16)((cell * 4u + (bitraf_u16)s.vertex) * 2u +
                        (bitraf_u16)s.parity);
    return 0u;
}

static inline bitraf_state bitraf_from_core_index4096(bitraf_u16 index) {
    bitraf_state s;
    bitraf_u16 cell;
    s.parity = (bitraf_u8)(index & 1u);
    index = (bitraf_u16)(index >> 1u);
    s.vertex = (bitraf_u8)(index & 3u);
    cell = (bitraf_u16)(index >> 2u);
    s.z = (bitraf_u8)((cell & 7u) + 1u);
    cell = (bitraf_u16)(cell >> 3u);
    s.y = (bitraf_u8)((cell & 7u) + 1u);
    s.x = (bitraf_u8)((cell >> 3u) + 1u);
    return s;
}

static inline bitraf_state bitraf_opposite(bitraf_state s) {
    bitraf_state o;
    o.x = (bitraf_u8)(9u - s.x);
    o.y = (bitraf_u8)(9u - s.y);
    o.z = (bitraf_u8)(9u - s.z);
    o.vertex = (bitraf_u8)(3u - s.vertex);
    o.parity = (bitraf_u8)(s.parity ^ 1u);
    return o;
}

static inline bitraf_state bitraf_rotate_z90(bitraf_state s) {
    bitraf_state r = s;
    r.x = (bitraf_u8)(9u - s.y);
    r.y = s.x;
    return r;
}

static inline bitraf_state bitraf_rotate_z180(bitraf_state s) {
    return bitraf_rotate_z90(bitraf_rotate_z90(s));
}

static inline bitraf_state bitraf_rotate_z270(bitraf_state s) {
    return bitraf_rotate_z90(bitraf_rotate_z180(s));
}

static inline bitraf_u8 bitraf_equal(bitraf_state a, bitraf_state b) {
    return (bitraf_u8)(a.x == b.x && a.y == b.y && a.z == b.z &&
                       a.vertex == b.vertex && a.parity == b.parity);
}

static inline bitraf_u16 bitraf_absdiff_u8(bitraf_u8 a, bitraf_u8 b) {
    return (bitraf_u16)(a > b ? a - b : b - a);
}

static inline bitraf_u16 bitraf_relation(bitraf_state a, bitraf_state b) {
    bitraf_u16 rel = BITRAF_REL_NONE;
    bitraf_u16 manhattan;
    if (!bitraf_state_valid(a) || !bitraf_state_valid(b)) return rel;
    if (bitraf_equal(a, b)) rel |= BITRAF_REL_SAME;

    manhattan = (bitraf_u16)(bitraf_absdiff_u8(a.x, b.x) +
                             bitraf_absdiff_u8(a.y, b.y) +
                             bitraf_absdiff_u8(a.z, b.z));
    if (manhattan == 1u && a.vertex == b.vertex && a.parity == b.parity)
        rel |= BITRAF_REL_ORTHO_NEIGHBOR;
    if (a.x == b.x && a.y == b.y && a.z == b.z &&
        a.vertex == b.vertex && a.parity != b.parity)
        rel |= BITRAF_REL_PARITY_TWIN;
    if (a.x == b.x && a.y == b.y && a.z == b.z &&
        a.vertex != b.vertex && a.parity == b.parity)
        rel |= BITRAF_REL_VERTEX_SIBLING;
    if (bitraf_equal(bitraf_opposite(a), b)) rel |= BITRAF_REL_OPPOSITE;
    if (bitraf_equal(bitraf_rotate_z90(a), b)) rel |= BITRAF_REL_ROTATE_Z90;
    if (bitraf_equal(bitraf_rotate_z180(a), b)) rel |= BITRAF_REL_ROTATE_Z180;
    if (bitraf_equal(bitraf_rotate_z270(a), b)) rel |= BITRAF_REL_ROTATE_Z270;
    return rel;
}

/* Eight oriented median rays / octants around the center of the 10^3 lattice. */
static inline bitraf_u8 bitraf_octant(bitraf_state s) {
    return (bitraf_u8)(((s.x >= 5u) ? 4u : 0u) |
                       ((s.y >= 5u) ? 2u : 0u) |
                       ((s.z >= 5u) ? 1u : 0u));
}

/* Deterministic square projection: a 10x10 plane index plus depth. */
static inline bitraf_u16 bitraf_square_u(bitraf_state s) {
    return (bitraf_u16)((bitraf_u16)s.x * 10u + (bitraf_u16)s.y);
}
static inline bitraf_u8 bitraf_square_v(bitraf_state s) { return s.z; }

/* Deterministic axial projection for hex-grid visualization (not an isometry). */
static inline bitraf_hex_axial bitraf_hex_project(bitraf_state s) {
    bitraf_hex_axial h;
    h.q = (bitraf_i16)((bitraf_i16)s.x - (bitraf_i16)s.y);
    h.r = (bitraf_i16)((bitraf_i16)s.z -
                       (bitraf_i16)(((bitraf_u16)s.x + (bitraf_u16)s.y) / 2u));
    return h;
}

/*
 * Bounded Q16 embedding in the open unit ball B^3.
 * Visible coordinates provide the cube; the 4 hidden states use tetrahedral
 * offsets; parity reverses only the hidden tetrahedral offset.
 * Max norm is below 1.0 by construction. This is a carrier geometry, not a
 * physical claim and not a Poincare isometry.
 */
static inline bitraf_vec3_q16 bitraf_embed_b3_q16(bitraf_state s) {
    static const bitraf_i16 tetra[4][3] = {
        { 1,  1,  1},
        { 1, -1, -1},
        {-1,  1, -1},
        {-1, -1,  1}
    };
    bitraf_i32 sign = s.parity ? -1 : 1;
    bitraf_vec3_q16 v;
    v.x = ((bitraf_i32)(2 * (bitraf_i32)s.x - 9) * 3072) +
          sign * (bitraf_i32)tetra[s.vertex][0] * 512;
    v.y = ((bitraf_i32)(2 * (bitraf_i32)s.y - 9) * 3072) +
          sign * (bitraf_i32)tetra[s.vertex][1] * 512;
    v.z = ((bitraf_i32)(2 * (bitraf_i32)s.z - 9) * 3072) +
          sign * (bitraf_i32)tetra[s.vertex][2] * 512;
    return v;
}

static inline bitraf_u64 bitraf_norm2_q32(bitraf_vec3_q16 v) {
    bitraf_i32 x = v.x;
    bitraf_i32 y = v.y;
    bitraf_i32 z = v.z;
    return (bitraf_u64)((bitraf_u64)(x < 0 ? -x : x) * (bitraf_u64)(x < 0 ? -x : x)) +
           (bitraf_u64)((bitraf_u64)(y < 0 ? -y : y) * (bitraf_u64)(y < 0 ? -y : y)) +
           (bitraf_u64)((bitraf_u64)(z < 0 ? -z : z) * (bitraf_u64)(z < 0 ? -z : z));
}

static inline bitraf_u8 bitraf_popcount16(bitraf_u16 v) {
    bitraf_u8 c = 0u;
    while (v) {
        c = (bitraf_u8)(c + (bitraf_u8)(v & 1u));
        v = (bitraf_u16)(v >> 1u);
    }
    return c;
}

static inline bitraf_u8 bitraf_numeric_parity(bitraf_state s) {
    return (bitraf_u8)(bitraf_popcount16(bitraf_index8000(s)) & 1u);
}

static inline bitraf_u8 bitraf_is_prime_u16(bitraf_u16 n) {
    bitraf_u16 i;
    if (n < 2u) return 0u;
    if (n == 2u || n == 3u) return 1u;
    if ((n % 2u) == 0u || (n % 3u) == 0u) return 0u;
    i = 5u;
    while ((bitraf_u32)i * (bitraf_u32)i <= (bitraf_u32)n) {
        if ((n % i) == 0u || (n % (bitraf_u16)(i + 2u)) == 0u) return 0u;
        i = (bitraf_u16)(i + 6u);
    }
    return 1u;
}

static inline char bitraf_base20_digit(bitraf_u8 d) {
    if (d < 10u) return (char)('0' + (int)d);
    return (char)('A' + (int)d - 10);
}

static inline bitraf_u8 bitraf_base20_value(char c, bitraf_u8 *out) {
    if (!out) return 1u;
    if (c >= '0' && c <= '9') { *out = (bitraf_u8)(c - '0'); return 0u; }
    if (c >= 'A' && c <= 'J') { *out = (bitraf_u8)(10 + c - 'A'); return 0u; }
    if (c >= 'a' && c <= 'j') { *out = (bitraf_u8)(10 + c - 'a'); return 0u; }
    return 1u;
}

static inline void bitraf_to_base20_3(bitraf_u16 index, char out[4]) {
    out[2] = bitraf_base20_digit((bitraf_u8)(index % 20u));
    index = (bitraf_u16)(index / 20u);
    out[1] = bitraf_base20_digit((bitraf_u8)(index % 20u));
    index = (bitraf_u16)(index / 20u);
    out[0] = bitraf_base20_digit((bitraf_u8)(index % 20u));
    out[3] = '\0';
}

static inline bitraf_u8 bitraf_from_base20_3(const char in[3], bitraf_u16 *out) {
    bitraf_u8 a, b, c;
    bitraf_u16 value;
    if (!in || !out) return 1u;
    if (bitraf_base20_value(in[0], &a) ||
        bitraf_base20_value(in[1], &b) ||
        bitraf_base20_value(in[2], &c)) return 1u;
    value = (bitraf_u16)(((bitraf_u16)a * 20u + (bitraf_u16)b) * 20u +
                         (bitraf_u16)c);
    if (value >= BITRAF_STATES_TOTAL) return 1u;
    *out = value;
    return 0u;
}

#endif /* RAFAELIA_BITRAF_MATRIX_H */
