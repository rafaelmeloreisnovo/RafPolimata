#include "../include/raf_fs_z0_token.h"

raf_z0_u8
raf_z0_probe(const raf_z0_u8 *data, raf_z0_size len, raf_z0_size index, raf_z0_token *out)
{
    raf_z0_view view = { data, len };
    return raf_z0_token_at(view, index, out);
}
