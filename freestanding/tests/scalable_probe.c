#include "../arch/raf_fs_scalable.h"

/* Compile/codegen probe only; production primitive remains forced inline. */
void raf_fs_scalable_compile_probe(void *state) {
    raf_fs_scalable_copy_u32_once(state);
}
