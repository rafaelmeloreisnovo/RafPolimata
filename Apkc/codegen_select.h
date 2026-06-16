/* codegen_select.h — deterministic multi-encoding instruction selection.
 *
 * Some ARM64 instructions admit multiple bit-distinct encodings that are
 * semantically identical (e.g. MOV Xd,Xm == ORR Xd,XZR,Xm == ADD Xd,Xm,#0
 * == SUB Xd,Xm,#0 — all leave Xd == Xm with no flag side effects). Rather
 * than hardcoding a single encoder per logical operation, an "equivalence
 * family" picks among N verified-equivalent encoders.
 *
 * Selection is a pure function of the bytes already emitted, reusing
 * phi_fst/phi_attractor (coherence.h) as the deterministic "observer":
 * same source -> same prior bytes -> same phi -> same choice, every
 * build, on every host. This is reproducibility, not randomness — the
 * variant log (see RafCtx in raf_compile.h) makes the choice auditable.
 */
#pragma once
#include "sys.h"
#include "coherence.h"

static inline u32 codegen_select(const u8 *emitted_buf, u32 emitted_len, u32 num_variants) {
    if (num_variants <= 1u) return 0u;
    u32 phi = phi_fst(emitted_buf, emitted_len);
    return phi_attractor(phi) % num_variants;
}
