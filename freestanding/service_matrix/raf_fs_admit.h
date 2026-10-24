/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: admission gate for bounded caller-owned low-level module frames.
 * SCOPE: PURE_CORE, no executable dispatch, JNI or platform operations.
 * PRECONDITIONS: unsigned int 32-bit, request/out buffers nonoverlapping.
 * REGISTER_OWNERSHIP: compiler; CLOBBERS: NONE; MEMORY_ORDER: NONE.
 * TAIL_SHADOW: no loops, shadow, residual or allocation.
 * EVIDENCE: policy decision only; not module execution/attestation.
 */
#ifndef RAF_FS_ADMIT_H
#define RAF_FS_ADMIT_H
#include "raf_fs_probe.h"
enum {
 RAF_MOD_PREFIX_PROBE=1, RAF_MOD_FLAG_GATE=2,
 RAF_MOD_PLATFORM_SYSCALL=3, RAF_MOD_ANDROID_BRIDGE=4
};
enum {
 RAF_CAP_LIBC=1u, RAF_CAP_HEAP=2u, RAF_CAP_SYSCALL=4u,
 RAF_CAP_JNI=8u, RAF_CAP_JVM=16u, RAF_CAP_JIT=32u,
 RAF_CAP_NETWORK=64u, RAF_CAP_OS_LOG=128u, RAF_CAP_DYNLINK=256u
};
enum {
 RAF_ADMIT_NULL=1, RAF_ADMIT_BAD_ABI=2,
 RAF_ADMIT_HOSTED_REQUIRED=3, RAF_ADMIT_NOT_IMPLEMENTED=4,
 RAF_ADMIT_PURE_CONTRACT=5
};
/* request[0]=ABI version 1; [1]=module ID; [2]=external capability bitset;
 * [3]=caller frame size bytes. Other request positions reserved and ignored.
 * out[0]=state; [1]=module; [2]=rejected capabilities; [3]=frame bytes;
 * [4]=decision ambiguity (1); [5]=verified execution (0);
 * [6]=module implementation status (0/1); [7]=ABI version.
 */
void raf_fs_admit(const raf_u32 request[RAF_CELL_COUNT], raf_u32 out[RAF_CELL_COUNT]);
#endif
