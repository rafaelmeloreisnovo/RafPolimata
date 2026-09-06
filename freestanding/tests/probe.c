#include "../include/raf_fs_core.h"

/*
 * Compile-only probe. It deliberately exports one symbol so objdump/nm can inspect
 * generated code while the production headers themselves contribute no helper symbols.
 */
void raf_fs_compile_probe(void *state) {
    raf_u32 *p = (raf_u32 *)state;
    raf_u32 mask = RAF_FS_MASK32((raf_u32)(p[0] != 0u));
    p[1] = RAF_FS_SELECT32(mask, p[0], 7u);
    raf_fs_fence_rw();
}
