/* codegen_select.h — deterministic multi-encoding instruction selection.
 *
 * Multiple bit-distinct encodings may be used only after semantic equivalence
 * has been proven. The chosen encoding is a pure function of bytes already
 * emitted. The Omega bridge enriches the key with deterministic transition,
 * entropy-proxy, fold occupancy, path and 42-attractor metrics.
 *
 * Same source -> same emitted prefix -> same choice on every build.
 * This is reproducibility, not randomness.
 */
#pragma once
#include "sys.h"
#include "omega_classifier.h"

static inline u32 codegen_select(const u8 *emitted_buf,
                                 u32 emitted_len,
                                 u32 num_variants) {
    return (u32)raf_omega_codegen_index(
        (const raf_omega_u8 *)emitted_buf,
        (raf_omega_u32)emitted_len,
        (raf_omega_u32)num_variants);
}

static inline RafOmegaMetrics codegen_classify(const u8 *emitted_buf,
                                                u32 emitted_len) {
    return raf_omega_classify((const raf_omega_u8 *)emitted_buf,
                              (raf_omega_u32)emitted_len);
}
