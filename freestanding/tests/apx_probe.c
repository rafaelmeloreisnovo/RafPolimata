#include "../arch/raf_fs_apx.h"

typedef struct raf_fs_apx_probe_state {
    raf_u64 a;
    raf_u64 b;
    raf_u64 sum;
    raf_u64 high_copy;
} raf_fs_apx_probe_state;

void raf_fs_apx_probe(void *state)
{
    raf_fs_apx_probe_state *s = (raf_fs_apx_probe_state *)state;
    raf_fs_apx_add_u64(&s->sum, s->a, s->b);
    raf_fs_apx_copy_r31_u64(&s->high_copy, s->sum);
}
