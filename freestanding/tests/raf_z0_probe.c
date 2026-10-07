#include "../include/raf_fs_z0_token.h"

raf_fs_z0_emit
raf_z0_probe(const raf_u8 *data, raf_usize size, raf_u8 provided, raf_usize index, raf_fs_z0_token *out)
{
    raf_fs_z0_view view = { data, size, provided };
    return raf_fs_z0_token_at(view, index, out);
}

raf_u8
raf_z0_projection_probe(raf_fs_z0_view a, raf_fs_z0_view b, raf_u8 observation_mask)
{
    return raf_fs_z0_equiv(a, b, observation_mask);
}
