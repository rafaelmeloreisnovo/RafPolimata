#include "../include/raf_fs_z0_token.h"

raf_u32 raf_fs_z0_probe(const raf_u8 *p, raf_usize n, raf_u8 provided) {
    raf_fs_z0_view v;
    v.data = p;
    v.size = n;
    v.provided = provided;
    return (raf_u32)raf_fs_z0_classify(v);
}
