/* Compile-only cross-ISA probe; not an ABI or physical runtime claim. */
#include "../include/raf_fs_cognitive_v1.h"
raf_u32 raf_fs_cog_probe(raf_fs_cog_state *prior,
                          const raf_fs_cog_observation *observation,
                          raf_fs_cog_state *result) {
    return raf_fs_cog_step(prior, observation, result);
}
raf_u32 raf_fs_cog_coordinates(raf_u32 slot, raf_u32 *x, raf_u32 *y, raf_u32 *z) {
    return raf_fs_cog_unpack(slot,x,y,z);
}
