#include "../canonical/zipraf-hw-v1/include/zipraf_bit_layer_ref_v1.h"

zbl_i32 zipraf_bit_layer_ref_probe(
    zbl_u8 value,
    zbl_u8 q,
    zbl_u8 *out)
{
    zbl_u8 planes[8];
    zbl_i32 rc = zbl_ref_decompose_u8(value, planes);
    if (rc != ZBL_OK) return rc;
    return zbl_ref_reconstruct_msb_q(planes, q, out);
}
