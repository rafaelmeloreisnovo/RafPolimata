/* coherence.h — freestanding geometric coherence metric (phi_fst).
 *
 * phi_fst = (1 - H_norm) * C_norm  in Q16 fixed-point [0, 65536].
 *
 *   H_norm  : normalized byte-entropy proxy (unique_bytes / 256)
 *   C_norm  : KAM-coherence — normalized dot product of the first 7
 *             byte frequencies against the KAM-7 seed {40503, ...}
 *             (same seed used by Benchmark/raf_toroid.h T^7 engine)
 *
 * phi_fst = 0 (65536 unique bytes, pure noise / maximum entropy)
 * phi_fst = 65536 (all bytes identical, perfect coherence)
 *
 * Interpretation mirrors the T^7 phi_ethica invariant but is computed
 * without floating-point or libm — pure integer arithmetic, safe in any
 * freestanding environment. */
#pragma once
#include "sys.h"

static inline u32 phi_fst(const u8 *buf, u32 n) {
    if (!n) return 0u;

    /* byte frequency histogram — 256 × u32 = 1024B on stack */
    u32 freq[256];
    for (int i = 0; i < 256; i++) freq[i] = 0u;
    for (u32 i = 0; i < n; i++) freq[buf[i]]++;

    /* H_norm: fraction of distinct byte values used */
    u32 unique = 0u;
    for (int i = 0; i < 256; i++) if (freq[i]) unique++;
    u32 H = (unique * 0x10000u) / 256u;   /* Q16 ∈ [0, 65536] */

    /* C_norm: KAM-7 coherence — dot(freq[0..6], KAM7) / ||freq||^2 */
    static const u32 KAM7[7] = {
        40503u, 40503u, 40503u, 40503u, 40503u, 40503u, 40503u
    };
    u64 dot = 0u, ns = 0u;
    for (int i = 0; i < 7; i++) {
        dot += (u64)freq[i] * KAM7[i];
        ns  += (u64)freq[i] * freq[i];
    }
    u32 C = 0u;
    if (ns) {
        C = (u32)((dot * 0x10000u) / (ns | 1u));
        if (C > 0x10000u) C = 0x10000u;
    }

    /* phi = (1 - H) * C  (Q16 × Q16 → Q16) */
    u32 oneMinH = (H < 0x10000u) ? (0x10000u - H) : 0u;
    return (u32)(((u64)oneMinH * C) >> 16);
}

/* Attractor index: maps phi onto one of 42 T^7 attractor slots */
static inline u32 phi_attractor(u32 phi) {
    return (phi ^ (phi >> 7)) % 42u;
}
