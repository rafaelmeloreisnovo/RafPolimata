#include "../include/raf_fs_core.h"

/*
 * Compile-only probe. It deliberately exports one symbol so objdump/nm can inspect
 * generated code while the production headers themselves contribute no helper symbols.
 * It exercises fixed-width copy, branchless select, residual-mask generation and fence.
 */
typedef struct raf_fs_probe_state {
    raf_u32 in[8];
    raf_u32 out[8];
    raf_u32 remaining;
    raf_u32 mask;
} raf_fs_probe_state;

void raf_fs_compile_probe(void *state) {
    raf_fs_probe_state *s = (raf_fs_probe_state *)state;
    raf_u32 select_mask = RAF_FS_MASK32((raf_u32)(s->in[0] != 0u));
    s->out[0] = RAF_FS_SELECT32(select_mask, s->in[0], 7u);
    RAF_FS_COPY_U32X8(s->out, s->in);
    s->mask = raf_fs_residual32(s->remaining).lane_mask;
    raf_fs_fence_rw();
}
