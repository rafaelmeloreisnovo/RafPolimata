#include "../include/raf_fs_registers.h"

RAF_FS_STATIC_ASSERT(register_gpr_nonzero, RAF_FS_REG_GPR_COUNT > 0u);
RAF_FS_STATIC_ASSERT(register_ptr_32_or_64, (RAF_FS_ABI_PTR_BITS == 32u) || (RAF_FS_ABI_PTR_BITS == 64u));

/* Metadata-only object: no function, no stack, no runtime or ISA execution claim. */
const raf_u32 raf_fs_register_metadata_probe[] = {
    RAF_FS_ABI_ID,
    RAF_FS_ABI_PTR_BITS,
    RAF_FS_REG_GPR_COUNT,
    RAF_FS_REG_VECTOR_COUNT,
    RAF_FS_REG_VECTOR_BITS,
    RAF_FS_REG_PRED_COUNT,
    RAF_FS_REG_MATRIX_COUNT,
    RAF_FS_REGCLASS_MASK
};
