#include "../include/raf_fs_z0_token.h"

static int expect(raf_fs_z0_view v, raf_fs_z0_kind k) {
    return raf_fs_z0_classify(v) == k;
}

int main(void) {
    static const raf_u8 nul = 0x00u;
    static const raf_u8 space = 0x20u;
    static const raf_u8 dot = 0x2eu;
    static const raf_u8 seq[2] = {0x20u, 0x2eu};
    int ok = 1;

    ok &= expect((raf_fs_z0_view){0, 0u, 0u}, RAF_FS_Z0_ABSENT);
    ok &= expect((raf_fs_z0_view){0, 0u, 1u}, RAF_FS_Z0_EMPTY);
    ok &= expect((raf_fs_z0_view){&space, 1u, 1u}, RAF_FS_Z0_SPACE);
    ok &= expect((raf_fs_z0_view){&nul, 1u, 1u}, RAF_FS_Z0_NUL);
    ok &= expect((raf_fs_z0_view){&dot, 1u, 1u}, RAF_FS_Z0_BYTE);
    ok &= expect((raf_fs_z0_view){seq, 2u, 1u}, RAF_FS_Z0_SEQUENCE);
    ok &= expect((raf_fs_z0_view){0, 1u, 1u}, RAF_FS_Z0_INVALID);

    return ok ? 0 : 1;
}
