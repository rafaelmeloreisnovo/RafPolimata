#include "../include/raf_fs_z0_token.h"

raf_fs_z0_emit
raf_z0_probe(const raf_u8 *data, raf_usize size, raf_u8 provided, raf_usize index, raf_fs_z0_token *out)
{
    raf_fs_z0_view view = { data, size, provided };
    return raf_fs_z0_token_at(view, index, out);
}
