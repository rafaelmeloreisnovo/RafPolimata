#include "../arch/raf_fs_vector.h"

/* Compile/codegen probe only. Production L0 remains header-only and inlined. */
typedef struct raf_fs_vector_probe_state {
    raf_u32 src[16];
    raf_u32 yes[16];
    raf_u32 no[16];
    raf_u32 mask[16];
    raf_u32 out[16];
    raf_u32 zero[16];
} raf_fs_vector_probe_state;

void raf_fs_vector_compile_probe(void *state) {
    raf_fs_vector_probe_state *s = (raf_fs_vector_probe_state *)state;
    raf_fs_vec_copy_block(s->out, s->src);
    raf_fs_vec_select_u32_block(s->out, s->mask, s->yes, s->no);
#if defined(__x86_64__) && defined(__AVX512F__)
    /* Eight active lanes prove native K-mask bounded memory without a scalar tail. */
    raf_fs_vec_masked_copy_u32(s->out, s->src, 0x00ffu);
#endif
    raf_fs_vec_zero_block(s->zero);
}
