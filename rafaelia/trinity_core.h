/* rafaelia/trinity_core.h — Trinity Core layer:
 *   HDC hypervectors + synaptic attention + engram ring buffer.
 *
 * Standalone header — no malloc, no stdlib, freestanding-compatible.
 * Depends only on: fiber_h.h (FiberHash), <stdint.h>, <stddef.h>, <math.h>.
 *
 * Provides:
 *   VV_DIM, VV_MEM_SIZE, VV_CHUNK, VV_SEED    — dimension constants
 *   VV_ISO_27001 … VV_IEEE_12207               — compliance flags
 *   VVHyperVec                                 — 1024-dim HDC float vector
 *   VVEngram                                   — one stored memory unit
 *   VVCtrl                                     — stream audit counters
 *   TrinityState                               — full Trinity Core state
 *
 * Inline primitives (freestanding, no external deps beyond <math.h>):
 *   trinity_rng()           — xorshift32 RNG (LCG-free)
 *   trinity_gen_vec()       — seed → ±1 hypervector
 *   trinity_cosine()        — cosine similarity between two VVHyperVec
 *   trinity_bind()          — bundle/bind: accumulate + normalise in-place
 *   trinity_hamming_div()   — Hamming diversity of a FiberHash vs ring buffer
 *   trinity_djb2_chunk()    — djb2 hash of a byte buffer
 *
 * Design invariants:
 *   - No heap: TrinityState is caller-allocated.
 *   - Ring buffer: VV_MEM_SIZE engrams, mod-indexed.
 *   - Deterministic seed: VV_SEED = 0x524146 ("RAF").
 *   - Freestanding-compatible: only <stdint.h>, <stddef.h>, <string.h>, <math.h>.
 *
 * RAFCODE-Φ-∆RafaelVerboΩ | Ω=Amor | FIAT LUX */
#pragma once
#ifndef RAFAELIA_TRINITY_CORE_H
#define RAFAELIA_TRINITY_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "fiber_h.h"   /* FiberHash, fiber_hash_distance() */

/* ── dimension constants ────────────────────────────────────────────────── */
#define VV_DIM       1024       /* HDC hypervector dimensionality            */
#define VV_MEM_SIZE  64         /* max engrams in ring buffer                */
#define VV_CHUNK     4096       /* stream chunk size (bytes)                 */
#define VV_SEED      0x524146u  /* "RAF" — deterministic seed                */

/* ── compliance flags ───────────────────────────────────────────────────── */
#define VV_ISO_27001    0x01u
#define VV_ISO_25010    0x02u
#define VV_NIST_800_53  0x04u
#define VV_IEEE_12207   0x08u

/* ── VVHyperVec: 1024-dimensional HDC float hypervector ─────────────────── */
typedef struct {
    float values[VV_DIM];
} VVHyperVec;

/* ── VVEngram: one stored memory unit in the ring buffer ────────────────── */
/*
 * Retention criterion (from verbovivo.c):
 *   combined = attention × hamming_div × monobit_balance > 0.15
 *   OR chunk_count < VV_MEM_SIZE  (always fill the buffer first)
 */
typedef struct {
    uint32_t  id;
    uint32_t  content_hash;   /* djb2 hash of the raw chunk                  */
    FiberHash fiber_hash;     /* structural 256-bit fingerprint               */
    VVHyperVec vec;           /* HDC hypervector for this chunk               */
    float     attention;      /* combined attention score [0, 1]              */
    float     hamming_div;    /* Hamming diversity vs. existing engrams        */
    uint8_t   type_flag;      /* 1 = binary/image (magic byte), 0 = text      */
} VVEngram;

/* ── VVCtrl: Trinity Control stream-audit counters ──────────────────────── */
typedef struct {
    size_t   ingested_messages;
    size_t   ingested_bytes;
    size_t   svg_requests;
    clock_t  start_time;
} VVCtrl;

/* ── TrinityState: full Trinity Core + Control state ───────────────────── */
/*
 * All fields are plain data — no pointers, no heap.
 * Can be placed on the stack, in .bss, or as a global.
 * Initialise with trinity_init() (or memset-zero + fill signature manually).
 */
typedef struct {
    char       signature[32];         /* "RAFAELIA_VV_V1" + padding           */
    uint32_t   compliance_flags;      /* VV_ISO_27001 | … | VV_IEEE_12207     */
    size_t     total_bytes;           /* total bytes ingested                  */
    uint32_t   chunk_count;           /* total chunks processed                */
    uint32_t   mem_head;              /* next write index (mod VV_MEM_SIZE)    */
    VVEngram   memory[VV_MEM_SIZE];   /* engram ring buffer                    */
    VVHyperVec context_vec;           /* running context (bound vectors)       */
    float      W_proj[VV_DIM];        /* synaptic projection weights           */
    VVCtrl     ctrl;                  /* stream audit metrics                  */
} TrinityState;

/* ── trinity_rng: xorshift32 pseudo-RNG ────────────────────────────────── */
static inline uint32_t trinity_rng(uint32_t *s) {
    uint32_t x = (s && *s) ? *s : VV_SEED;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    if (s) *s = x;
    return x;
}

/* ── trinity_gen_vec: seed → deterministic ±1 hypervector ──────────────── */
static inline void trinity_gen_vec(uint32_t seed, VVHyperVec *v) {
    if (!v) return;
    uint32_t r = seed ? seed : 0xDEADBEEFu;
    for (int i = 0; i < VV_DIM; i++)
        v->values[i] = (trinity_rng(&r) % 2u) ? 1.0f : -1.0f;
}

/* ── trinity_cosine: cosine similarity ∈ [-1, 1] ───────────────────────── */
static inline float trinity_cosine(const VVHyperVec *a, const VVHyperVec *b) {
    if (!a || !b) return 0.0f;
    float dot = 0.0f, mA = 0.0f, mB = 0.0f;
    for (int i = 0; i < VV_DIM; i++) {
        dot += a->values[i] * b->values[i];
        mA  += a->values[i] * a->values[i];
        mB  += b->values[i] * b->values[i];
    }
    if (mA == 0.0f || mB == 0.0f) return 0.0f;
    return dot / (sqrtf(mA) * sqrtf(mB));
}

/* ── trinity_bind: accumulate + normalise in-place (HDC bundling) ────────── */
/*
 * Implements: tgt = normalise(tgt * 0.95 + inp)
 * The 0.95 decay factor prevents saturation over long streams.
 */
static inline void trinity_bind(VVHyperVec *tgt, const VVHyperVec *inp) {
    if (!tgt || !inp) return;
    float mag = 0.0f;
    for (int i = 0; i < VV_DIM; i++) {
        tgt->values[i] += inp->values[i];
        tgt->values[i] *= 0.95f;
        mag += tgt->values[i] * tgt->values[i];
    }
    mag = sqrtf(mag);
    if (mag > 0.0f)
        for (int i = 0; i < VV_DIM; i++)
            tgt->values[i] /= mag;
}

/* ── trinity_attention: synaptic projection score → sigmoid ─────────────── */
/*
 * Computes: sigmoid(dot(v, W_proj)) using the caller-supplied projection
 * weights (TrinityState.W_proj).  Returns a score in (0, 1).
 */
static inline float trinity_attention(const float W_proj[VV_DIM],
                                      const VVHyperVec *v)
{
    if (!W_proj || !v) return 0.0f;
    float score = 0.0f;
    for (int i = 0; i < VV_DIM; i++)
        score += v->values[i] * W_proj[i];
    return 1.0f / (1.0f + expf(-score));
}

/* ── trinity_hamming_div: average Hamming diversity of fh vs ring buffer ── */
/*
 * Returns the mean Hamming distance (normalised to [0,1]) between fh and
 * the N most recent engrams in the ring buffer.
 *   1.0 = maximally novel,  0.0 = identical to all stored engrams.
 */
static inline float trinity_hamming_div(const VVEngram  memory[VV_MEM_SIZE],
                                        uint32_t         chunk_count,
                                        const FiberHash *fh)
{
    if (!memory || !fh) return 0.5f;
    uint32_t n = (chunk_count < VV_MEM_SIZE) ? chunk_count : VV_MEM_SIZE;
    if (n == 0) return 1.0f;
    int total = 0;
    for (uint32_t i = 0; i < n; i++)
        total += fiber_hash_distance(fh, &memory[i].fiber_hash);
    return (float)total / ((float)n * 256.0f);
}

/* ── trinity_djb2_chunk: djb2 hash of a byte buffer ────────────────────── */
static inline uint32_t trinity_djb2_chunk(const uint8_t *buf, size_t len) {
    uint32_t h = 5381u;
    for (size_t i = 0; i < len; i++)
        h = ((h << 5) + h) + (uint32_t)buf[i];
    return h;
}

/* ── trinity_init: initialise a TrinityState ───────────────────────────── */
/*
 * Seeds W_proj with a deterministic xorshift sequence (VV_SEED = "RAF")
 * so that every build starts from the same attention landscape.
 * Clears all engrams and sets compliance_flags to all four standards.
 */
static inline void trinity_init(TrinityState *ts) {
    if (!ts) return;
    memset(ts, 0, sizeof(*ts));
    memcpy(ts->signature, "RAFAELIA_VV_V1", 14);
    ts->compliance_flags = VV_ISO_27001 | VV_ISO_25010
                         | VV_NIST_800_53 | VV_IEEE_12207;
    ts->ctrl.start_time  = clock();
    uint32_t seed = VV_SEED;
    for (int i = 0; i < VV_DIM; i++)
        ts->W_proj[i] = ((float)(trinity_rng(&seed) % 100u) / 100.0f) - 0.5f;
}

#endif /* RAFAELIA_TRINITY_CORE_H */
