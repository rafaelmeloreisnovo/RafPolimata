/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: pure admission decision from matrix bits, not runtime dispatch.
 * SCOPE: freestanding L0. PRECONDITIONS: disjoint caller buffers.
 * REGISTER_OWNERSHIP: compiler; CLOBBERS: NONE; MEMORY_ORDER: NONE.
 * TAIL_SHADOW: none. EVIDENCE: only a policy decision, never execution.
 */
#include "raf_fs_admit.h"
void raf_fs_admit(const raf_u32 request[RAF_CELL_COUNT], raf_u32 out[RAF_CELL_COUNT]) {
    if (!out) return;
    out[0]=RAF_ADMIT_NULL; out[1]=0u; out[2]=0u; out[3]=0u;
    out[4]=1u; out[5]=0u; out[6]=0u; out[7]=1u;
    if (!request) return;
    const raf_u32 version=request[0], module=request[1];
    const raf_u32 external=request[2], size=request[3];
    out[1]=module; out[2]=external; out[3]=size;
    if (version!=1u) { out[0]=RAF_ADMIT_BAD_ABI; return; }
    if (external!=0u) { out[0]=RAF_ADMIT_HOSTED_REQUIRED; return; }
    /* Only modules with an actual PURE_CORE implementation may be admitted.
     * Syscall and Android adapters are explicitly NOT implemented in L0.
     */
    const raf_u32 has_l0 = (module==RAF_MOD_PREFIX_PROBE) | (module==RAF_MOD_FLAG_GATE);
    out[0]=has_l0 ? RAF_ADMIT_PURE_CONTRACT : RAF_ADMIT_NOT_IMPLEMENTED;
    out[6]=has_l0;
}
