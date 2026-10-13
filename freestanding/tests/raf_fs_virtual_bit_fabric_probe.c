#include "../include/raf_fs_virtual_bit_fabric.h"
/* Target-only object proof: no runtime or syscall ABI. */
raf_u32 raf_fs_vbf_target_probe(raf_fs_vbf *f, raf_u32 profile)
{
    raf_u32 status=raf_fs_vbf_init(f,profile);
    if(status!=RAF_FS_VBF_OK) return status;
    status=raf_fs_vbf_step(f,RAF_FS_VBF_INPUT,0u,1u,0u,3u,1u);
    if(status!=RAF_FS_VBF_OK) return status;
    return raf_fs_vbf_check(f);
}
