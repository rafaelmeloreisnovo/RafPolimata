#include "../arch/raf_fs_matrix.h"

/* Compile/codegen probe only; production primitive remains forced inline. */
void raf_fs_matrix_compile_probe(void *state) {
    raf_fs_matrix_zero_primary(state);
}
