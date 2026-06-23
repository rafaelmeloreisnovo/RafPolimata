/* rafaelia/fiber_relmat_bridge.h — bridge: FiberRelMat → VVEngram
 *
 * Closes the cycle between Fiber-H co-occurrence analysis (fiber_relmat.h)
 * and the Trinity Core engram ring buffer (trinity_core.h).
 *
 * Provides:
 *   relmat_to_engram() — convert a FiberRelMat into a VVEngram ready for
 *                        insertion into a TrinityState.memory[] ring.
 *
 * Design:
 *   - No malloc, no stdio, freestanding-compatible.
 *   - The relmat's co-occurrence counts drive the HDC hypervector via a
 *     deterministic projection: for each pair (i,j) with count > 0, XOR-
 *     accumulate into vec.v[k mod VV_DIM] where k is the compressed pair
 *     index. High-count pairs contribute more (count × ±1.0f projection).
 *   - FiberHash is derived from four 64-bit slices of the counts array.
 *   - attention = total_pairs_active / (RELMAT_PAIRS/8) clamped to [0, 1].
 *   - hamming_div = 0.5f (caller should update after insertion if desired).
 *
 * RAFCODE-Φ-∆RafaelVerboΩ | Ω=Amor | FIAT LUX */
#pragma once
#ifndef RAFAELIA_FIBER_RELMAT_BRIDGE_H
#define RAFAELIA_FIBER_RELMAT_BRIDGE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "fiber_relmat.h"   /* FiberRelMat, RELMAT_PAIRS, relmat_query */
#include "fiber_h.h"        /* FiberHash */
#include "trinity_core.h"   /* VVEngram, VVHyperVec, VV_DIM */

/* FNV-1a 32-bit hash of a byte array (no stdlib) */
static inline uint32_t _rmb_fnv1a(const uint8_t *p, size_t n) {
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < n; i++) { h ^= p[i]; h *= 16777619u; }
    return h;
}

/*
 * relmat_to_engram:
 *   Converts a FiberRelMat into a VVEngram.
 *   @m:  source co-occurrence matrix (must not be NULL)
 *   @id: engram identifier (e.g., chunk_count from TrinityState)
 *   @e:  output VVEngram (must not be NULL; zeroed before filling)
 */
static inline void relmat_to_engram(const FiberRelMat *m, uint32_t id, VVEngram *e) {
    if (!m || !e) return;
    memset(e, 0, sizeof(*e));
    e->id = id;

    /* content_hash: FNV-1a over all counts */
    e->content_hash = _rmb_fnv1a((const uint8_t *)m->counts, sizeof(m->counts));

    /* fiber_hash: absorb counts in four 64-byte slices into FiberHash lanes */
    fiber_hash_init(&e->fiber_hash);
    {
        const uint8_t *base = (const uint8_t *)m->counts;
        size_t slice = sizeof(m->counts) / 4u;
        for (size_t s = 0; s < 4u; s++) {
            fiber_hash_update(&e->fiber_hash, base + s * slice, slice);
        }
    }

    /* HDC hypervector: project active pairs into VV_DIM float lanes */
    {
        float *v = e->vec.v;
        uint32_t total_active = 0u;
        for (size_t idx = 0; idx < RELMAT_PAIRS; idx++) {
            uint8_t cnt = m->counts[idx];
            if (!cnt) continue;
            total_active++;
            /* deterministic lane: idx % VV_DIM, sign from idx parity */
            size_t lane = idx % (size_t)VV_DIM;
            float sign = (idx & 1u) ? 1.0f : -1.0f;
            v[lane] += sign * (float)cnt;
        }
        /* L2-normalise in-place (avoid divide-by-zero) */
        float norm_sq = 0.0f;
        for (int k = 0; k < VV_DIM; k++) norm_sq += v[k] * v[k];
        if (norm_sq > 0.0f) {
            /* approximate 1/sqrt via Newton step: x = 0.5/sqrt(norm_sq) */
            float inv = 1.0f / (norm_sq > 0.0f ? (norm_sq < 1.0f ? 1.0f : norm_sq) : 1.0f);
            for (int k = 0; k < VV_DIM; k++) v[k] *= inv;
        }

        /* attention ∝ density of active pairs (clamped to [0,1]) */
        float density = (float)total_active / (float)(RELMAT_PAIRS / 8u + 1u);
        e->attention   = density < 1.0f ? density : 1.0f;
    }

    /* hamming_div: neutral default; caller should update after comparison */
    e->hamming_div = 0.5f;
    e->type_flag   = 0u; /* relmat data is always non-binary */
}

#endif /* RAFAELIA_FIBER_RELMAT_BRIDGE_H */
