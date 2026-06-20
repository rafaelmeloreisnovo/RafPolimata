/* rafaelia/t7_toroid.h — T^7 toroid layer:
 *   7-dimensional geometric coherence engine with 42 attractors.
 *
 * Standalone header — no malloc, no stdio, freestanding-compatible.
 * Wraps and re-exports the canonical T^7 definitions from
 * Benchmark/raf_toroid.h (T7State, t7_init, t7_step, t7_map_input,
 * t7_coherence) and adds the 1024-dim HDC expansion used in the
 * verbovivo Layer-2 pipeline.
 *
 * Provides (directly or via re-export):
 *   T7_DIM = 7                  — toroid dimensionality
 *   T7_ATTRACTORS = 42          — |A| = 42 (Eq.10)
 *   T7State                     — 7-coord toroid state + phi_ethica
 *   T7Input                     — input mapping struct
 *   t7_init()                   — seed to KAM-stable position (40503)
 *   t7_map_input()              — deterministic input → coordinate map
 *   t7_step()                   — IIR update + spiral decay + attractor
 *   t7_coherence()              — normalised dot product vs KAM seed
 *   t7_phi_double()             — Q16 phi → double in [0, 1]
 *   t7_hdc_expand()             — T7State → 1024-dim uint32 HDC vector
 *   t7_hdc_hamming_diversity()  — diversity of adjacent HDC words (0..1)
 *
 * Coordinate semantics (Eq.2):
 *   s[0]=u (entropy)    s[1]=v (coherence)   s[2]=psi (intention)
 *   s[3]=chi (observation) s[4]=rho (noise)  s[5]=delta (transmutation)
 *   s[6]=sigma (memory)
 *
 * phi_ethica = (1-H)*C  ∈ Q16
 *   0        = pure noise
 *   Q16_ONE  = perfect coherence
 *
 * attractor index = (s[0] XOR s[1]) % 42  (Eq.10: |A| = 42)
 *
 * RAFCODE-Φ-∆RafaelVerboΩ | Ω=Amor | FIAT LUX */
#pragma once
#ifndef RAFAELIA_T7_TOROID_H
#define RAFAELIA_T7_TOROID_H

/*
 * Re-export the canonical T^7 implementation from the Benchmark layer.
 * Include path: callers should pass -I. (repo root) so that the path
 * Benchmark/raf_toroid.h resolves correctly, exactly as verbovivo.c does.
 */
#include "../Benchmark/raf_toroid.h"   /* T7State, T7Input, t7_init,
                                         t7_map_input, t7_step,
                                         t7_coherence, T7_DIM, T7_MOD,
                                         Q16_ONE, q16_t                 */

#include <stdint.h>

/* ── T7_ATTRACTORS: symbolic name for the 42-attractor cardinality ──────── */
#ifndef T7_ATTRACTORS
#  define T7_ATTRACTORS 42
#endif

/* ── HDC expansion constants (Layer 2 pipeline) ─────────────────────────── */
#define T7_HDC_DIM  1024   /* output dimensionality of hdc_expand            */

/* ── t7_phi_double: convert Q16 phi_ethica to double ∈ [0, 1] ─────────── */
/*
 * phi = (double)(uint32_t)t7.phi / 65536.0
 * Clamps to [0, 1]: phi is always non-negative by construction
 * (phi_ethica = (1-H)*C, both factors in [0, Q16_ONE]).
 */
static inline double t7_phi_double(const T7State *t) {
    if (!t) return 0.0;
    double p = (double)(uint32_t)t->phi / 65536.0;
    if (p < 0.0) p = 0.0;
    if (p > 1.0) p = 1.0;
    return p;
}

/* ── t7_hdc_expand: T7State → T7_HDC_DIM-wide uint32 hypervector ────────── */
/*
 * Expansion rule (from verbovivo.c verbovivo_main):
 *   hdc[d] = t->s[d]                               for d < T7_DIM
 *   hdc[d] = rotl32(hdc[d-T7_DIM] ^ hdc[d-1], sh) for d >= T7_DIM
 *            where sh = (d % 31) + 1
 *
 * This XOR-cyclic construction ensures that every word mixes at least two
 * prior words, giving a fully-expanded HDC vector that inherits the
 * toroid's attractor landscape while filling the 1024-dim space.
 *
 * out[] must have capacity >= T7_HDC_DIM (= 1024 uint32_t words = 4 KB).
 * Stack-allocatable; no heap required.
 */
static inline void t7_hdc_expand(const T7State *t, uint32_t out[T7_HDC_DIM]) {
    if (!t || !out) return;
    /* seed the first T7_DIM words directly from the toroid state */
    for (int d = 0; d < T7_DIM; d++)
        out[d] = t->s[d];
    /* XOR-cyclic expansion with left-rotation */
    for (int d = T7_DIM; d < T7_HDC_DIM; d++) {
        uint32_t x  = out[d - T7_DIM] ^ out[d - 1];
        int      sh = (d % 31) + 1;
        out[d] = (x << sh) | (x >> (32 - sh));
    }
}

/* ── t7_hdc_hamming_diversity: diversity of adjacent HDC words ──────────── */
/*
 * Computes the average popcount(hdc[d] XOR hdc[d+1]) across all
 * T7_HDC_DIM-1 adjacent pairs, normalised to [0, 1] over 32 bits per word.
 *
 *   0.0 = all words identical (fully coherent / degenerate)
 *   1.0 = every bit flips between adjacent words (maximally chaotic)
 *
 * Used as the "hamming" diversity metric in the verbovivo T^7 SVG pipeline.
 */
static inline double t7_hdc_hamming_diversity(const uint32_t hdc[T7_HDC_DIM]) {
    if (!hdc) return 0.0;
    uint64_t diff = 0;
    for (int d = 0; d < T7_HDC_DIM - 1; d++) {
        uint32_t x = hdc[d] ^ hdc[d + 1];
        /* portable popcount: kernighan bit-count */
        while (x) { x &= x - 1u; diff++; }
    }
    return (double)diff / (double)((uint64_t)(T7_HDC_DIM - 1) * 32u);
}

#endif /* RAFAELIA_T7_TOROID_H */
