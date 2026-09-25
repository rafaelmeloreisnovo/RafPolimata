/*
 * ZIPRAF Bit Layer Reference Vectors — Phase A V1.
 *
 * Authority: rafaelmeloreisnovo/RafPolimata
 * Contract: ZIPRAF-BIT-LAYER-PROGRESSIVE-RASTER-V1
 * Gap governance: CLOSURE_L1
 * Scope: bit-plane reconstruction and deterministic layer arrival only.
 *
 * No includes. No libc. No libm. No heap. No syscall.
 *
 * IMPORTANT:
 *   This file does NOT define the historical block mask M or geometry G(M).
 *   Geometry-dependent behavior remains TOKEN_VAZIO until separately proven.
 */
#ifndef ZIPRAF_BIT_LAYER_REF_V1_H
#define ZIPRAF_BIT_LAYER_REF_V1_H 1

typedef unsigned char zbl_u8;
typedef unsigned int  zbl_u32;
typedef signed int    zbl_i32;

typedef char zbl_assert_u8_is_1[(sizeof(zbl_u8) == 1u) ? 1 : -1];
typedef char zbl_assert_u32_is_4[(sizeof(zbl_u32) == 4u) ? 1 : -1];

#define ZBL_REF_VERSION 1u

#define ZBL_OK                  0
#define ZBL_E_ARGUMENT         -1
#define ZBL_E_VERSION          -2
#define ZBL_E_WIDTH            -3
#define ZBL_E_Q                -4
#define ZBL_E_LAYER            -5
#define ZBL_E_DUPLICATE_CONFLICT -6
#define ZBL_E_INCOMPLETE       -7

typedef struct {
    zbl_u8 seen_mask;
    zbl_u8 value;
    zbl_u8 conflict_mask;
    zbl_u8 reserved;
} ZblLayerAccumulatorV1;

static inline zbl_i32 zbl_ref_width_valid(zbl_u32 width)
{
    return (width == 30u || width == 60u || width == 120u) ? 1 : 0;
}

static inline zbl_i32 zbl_ref_q_valid(zbl_u8 q)
{
    return (q >= 1u && q <= 8u) ? 1 : 0;
}

static inline zbl_i32 zbl_ref_header_validate(
    zbl_u32 version,
    zbl_u32 width,
    zbl_u8 q)
{
    if (version != ZBL_REF_VERSION) return ZBL_E_VERSION;
    if (!zbl_ref_width_valid(width)) return ZBL_E_WIDTH;
    if (!zbl_ref_q_valid(q)) return ZBL_E_Q;
    return ZBL_OK;
}

/* Layer mask is NOT the canonical block mask M. */
static inline zbl_u8 zbl_ref_required_layer_mask(zbl_u8 q)
{
    if (!zbl_ref_q_valid(q)) return 0u;
    return (zbl_u8)(0xffu << (8u - (zbl_u32)q));
}

static inline zbl_i32 zbl_ref_decompose_u8(zbl_u8 value, zbl_u8 planes[8])
{
    zbl_u32 k;
    if (!planes) return ZBL_E_ARGUMENT;
    for (k = 0u; k < 8u; ++k) {
        planes[k] = (zbl_u8)((value >> k) & 1u);
    }
    return ZBL_OK;
}

static inline zbl_i32 zbl_ref_reconstruct_u8(
    const zbl_u8 planes[8],
    zbl_u8 *out)
{
    zbl_u32 k;
    zbl_u32 value = 0u;
    if (!planes || !out) return ZBL_E_ARGUMENT;
    for (k = 0u; k < 8u; ++k) {
        if (planes[k] > 1u) return ZBL_E_LAYER;
        value |= ((zbl_u32)planes[k]) << k;
    }
    *out = (zbl_u8)value;
    return ZBL_OK;
}

static inline zbl_i32 zbl_ref_reconstruct_msb_q(
    const zbl_u8 planes[8],
    zbl_u8 q,
    zbl_u8 *out)
{
    zbl_u8 full;
    zbl_u8 mask;
    zbl_i32 rc;
    if (!out) return ZBL_E_ARGUMENT;
    if (!zbl_ref_q_valid(q)) return ZBL_E_Q;
    rc = zbl_ref_reconstruct_u8(planes, &full);
    if (rc != ZBL_OK) return rc;
    mask = zbl_ref_required_layer_mask(q);
    *out = (zbl_u8)(full & mask);
    return ZBL_OK;
}

static inline void zbl_ref_accumulator_init(ZblLayerAccumulatorV1 *acc)
{
    if (!acc) return;
    acc->seen_mask = 0u;
    acc->value = 0u;
    acc->conflict_mask = 0u;
    acc->reserved = 0u;
}

static inline zbl_i32 zbl_ref_accumulator_put(
    ZblLayerAccumulatorV1 *acc,
    zbl_u8 layer_k,
    zbl_u8 bit)
{
    zbl_u8 mask;
    zbl_u8 old_bit;
    if (!acc) return ZBL_E_ARGUMENT;
    if (layer_k > 7u || bit > 1u) return ZBL_E_LAYER;

    mask = (zbl_u8)(1u << layer_k);
    if ((acc->seen_mask & mask) != 0u) {
        old_bit = (zbl_u8)((acc->value >> layer_k) & 1u);
        if (old_bit != bit) {
            acc->conflict_mask = (zbl_u8)(acc->conflict_mask | mask);
            return ZBL_E_DUPLICATE_CONFLICT;
        }
        return ZBL_OK;
    }

    acc->seen_mask = (zbl_u8)(acc->seen_mask | mask);
    if (bit != 0u) {
        acc->value = (zbl_u8)(acc->value | mask);
    }
    return ZBL_OK;
}

static inline zbl_i32 zbl_ref_accumulator_finalize(
    const ZblLayerAccumulatorV1 *acc,
    zbl_u8 required_layer_mask,
    zbl_u8 *out)
{
    if (!acc || !out) return ZBL_E_ARGUMENT;
    if (acc->conflict_mask != 0u) return ZBL_E_DUPLICATE_CONFLICT;
    if ((acc->seen_mask & required_layer_mask) != required_layer_mask) {
        return ZBL_E_INCOMPLETE;
    }
    *out = (zbl_u8)(acc->value & required_layer_mask);
    return ZBL_OK;
}

#endif /* ZIPRAF_BIT_LAYER_REF_V1_H */
