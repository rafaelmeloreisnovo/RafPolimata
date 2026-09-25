#include "../canonical/zipraf-hw-v1/include/zipraf_bit_layer_ref_v1.h"

static int check_sample(
    zbl_u8 value,
    zbl_u8 e1,
    zbl_u8 e2,
    zbl_u8 e4,
    zbl_u8 e8)
{
    zbl_u8 p[8];
    zbl_u8 out = 0u;
    if (zbl_ref_decompose_u8(value, p) != ZBL_OK) return 1;
    if (zbl_ref_reconstruct_u8(p, &out) != ZBL_OK || out != value) return 2;
    if (zbl_ref_reconstruct_msb_q(p, 1u, &out) != ZBL_OK || out != e1) return 3;
    if (zbl_ref_reconstruct_msb_q(p, 2u, &out) != ZBL_OK || out != e2) return 4;
    if (zbl_ref_reconstruct_msb_q(p, 4u, &out) != ZBL_OK || out != e4) return 5;
    if (zbl_ref_reconstruct_msb_q(p, 8u, &out) != ZBL_OK || out != e8) return 6;
    return 0;
}

static int check_order(const zbl_u8 order[8])
{
    static const zbl_u8 p[8] = {1u,0u,1u,0u,0u,1u,0u,1u};
    ZblLayerAccumulatorV1 a;
    zbl_u8 out = 0u;
    zbl_u32 i;

    zbl_ref_accumulator_init(&a);
    for (i = 0u; i < 8u; ++i) {
        zbl_u8 k = order[i];
        if (zbl_ref_accumulator_put(&a, k, p[k]) != ZBL_OK) return 1;
    }
    if (zbl_ref_accumulator_finalize(&a, 0xffu, &out) != ZBL_OK) return 2;
    return out == 165u ? 0 : 3;
}

int main(void)
{
    static const zbl_u8 order_a[8] = {7u,0u,5u,2u,6u,1u,4u,3u};
    static const zbl_u8 order_b[8] = {0u,1u,2u,3u,4u,5u,6u,7u};
    ZblLayerAccumulatorV1 a;
    zbl_u8 out = 0u;

    if (zbl_ref_header_validate(1u, 30u, 8u) != ZBL_OK) return 10;
    if (zbl_ref_header_validate(1u, 60u, 4u) != ZBL_OK) return 11;
    if (zbl_ref_header_validate(1u, 120u, 1u) != ZBL_OK) return 12;
    if (zbl_ref_header_validate(2u, 30u, 8u) != ZBL_E_VERSION) return 13;
    if (zbl_ref_header_validate(1u, 31u, 8u) != ZBL_E_WIDTH) return 14;
    if (zbl_ref_header_validate(1u, 30u, 0u) != ZBL_E_Q) return 15;

    if (check_sample(0u,   0u,   0u,   0u,   0u) != 0) return 20;
    if (check_sample(1u,   0u,   0u,   0u,   1u) != 0) return 21;
    if (check_sample(85u,  0u,  64u,  80u,  85u) != 0) return 22;
    if (check_sample(128u, 128u,128u,128u,128u) != 0) return 23;
    if (check_sample(165u, 128u,128u,160u,165u) != 0) return 24;
    if (check_sample(255u, 128u,192u,240u,255u) != 0) return 25;

    if (check_order(order_a) != 0) return 30;
    if (check_order(order_b) != 0) return 31;

    zbl_ref_accumulator_init(&a);
    if (zbl_ref_accumulator_put(&a, 7u, 1u) != ZBL_OK) return 40;
    if (zbl_ref_accumulator_put(&a, 7u, 1u) != ZBL_OK) return 41;
    if (zbl_ref_accumulator_put(&a, 7u, 0u) != ZBL_E_DUPLICATE_CONFLICT) return 42;
    if (zbl_ref_accumulator_finalize(&a, 0x80u, &out) != ZBL_E_DUPLICATE_CONFLICT) return 43;

    zbl_ref_accumulator_init(&a);
    if (zbl_ref_accumulator_put(&a, 7u, 1u) != ZBL_OK) return 44;
    if (zbl_ref_accumulator_finalize(&a, 0xc0u, &out) != ZBL_E_INCOMPLETE) return 45;

    return 0;
}
