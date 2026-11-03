/* Cross-ISA object probe; not device or provider execution. */
#include "../include/raf_fs_guard_supervisor_v1.h"
raf_u32 raf_fs_supervisor_probe(const raf_fs_guard_route *a,
                                const raf_fs_guard_route *b,
                                const raf_fs_guard_policy *policy,
                                raf_fs_guard_decision *out) {
    return raf_fs_guard_select(a,b,policy,out);
}
