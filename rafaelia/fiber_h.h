/* rafaelia/fiber_h.h — Fiber-H layer: 256-bit structural hash + Hamming distance.
 *
 * Standalone header — no malloc, no libc (beyond stdint/stddef/string),
 * freestanding-compatible.
 *
 * Provides:
 *   FiberHash               — 4×64-bit (256-bit) hash state
 *   fiber_hash_init()       — seed a FiberHash to zero state
 *   fiber_hash_update()     — absorb a byte buffer into a FiberHash
 *   fiber_hash_distance()   — popcount-based Hamming distance (0..256)
 *   fiber_hash_monobit()    — count set bits across all 256 bits
 *
 * Internal helpers (static inline, no external linkage):
 *   _fh_popcnt64()          — portable 64-bit popcount via __builtin_popcountll
 *   _fh_djb2_mix()          — positional djb2 mixing for four lanes
 *
 * RAFCODE-Φ-∆RafaelVerboΩ | Ω=Amor | FIAT LUX */
#pragma once
#ifndef RAFAELIA_FIBER_H_H
#define RAFAELIA_FIBER_H_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef uint8_t  _fh_u8;
typedef uint64_t _fh_u64;

/* ── FiberHash: 256-bit structural fingerprint ──────────────────────────── */
typedef struct {
    uint64_t a, b, c, d;   /* 4 × 64-bit lanes = 256 bits total */
} FiberHash;

/* ── internal: 64-bit popcount ─────────────────────────────────────────── */
static inline int _fh_popcnt64(uint64_t x) {
    return __builtin_popcountll((unsigned long long)x);
}

/* ── fiber_hash_init: zero-initialise to a known seed state ────────────── */
static inline void fiber_hash_init(FiberHash *fh) {
    if (!fh) return;
    fh->a = 5381u;
    fh->b = 5381u ^ 0xDEADBEEFu;
    fh->c = 5381u ^ 0xCAFEBABEu;
    fh->d = 5381u ^ 0xF00DFACEu;
}

/* ── fiber_hash_update: absorb len bytes of data into fh ───────────────── */
/*
 * Uses four independent djb2 lanes with positional and length mixing so
 * that:  lane-a = content,  lane-b = content×position,
 *        lane-c = content×length,  lane-d = content×(position^length).
 * This gives structural diversity across the 256 output bits.
 */
static inline void fiber_hash_update(FiberHash *fh, const uint8_t *data, size_t len) {
    if (!fh || !data) return;
    uint64_t h0 = fh->a, h1 = fh->b, h2 = fh->c, h3 = fh->d;
    for (size_t i = 0; i < len; i++) {
        uint8_t b = data[i];
        h0 = ((h0 << 5) + h0) ^ (uint64_t)b;
        h1 = ((h1 << 5) + h1) ^ (uint64_t)(b ^ (uint8_t)(i       & 0xFFu));
        h2 = ((h2 << 5) + h2) ^ (uint64_t)(b ^ (uint8_t)(len     & 0xFFu));
        h3 = ((h3 << 5) + h3) ^ (uint64_t)(b ^ (uint8_t)((i^len) & 0xFFu));
    }
    fh->a = h0; fh->b = h1; fh->c = h2; fh->d = h3;
}

/* ── fiber_hash_distance: popcount Hamming distance between two FiberHashes */
/*
 * Returns the number of differing bits across all 256 bits.
 * Range: [0, 256].  0 = identical,  256 = maximally different.
 * Null-safe: returns 0 if either pointer is NULL.
 */
static inline int fiber_hash_distance(const FiberHash *x, const FiberHash *y) {
    if (!x || !y) return 0;
    return _fh_popcnt64(x->a ^ y->a)
         + _fh_popcnt64(x->b ^ y->b)
         + _fh_popcnt64(x->c ^ y->c)
         + _fh_popcnt64(x->d ^ y->d);
}

/* ── fiber_hash_monobit: count total set bits in a FiberHash (0..256) ──── */
/*
 * Used for NIST SP 800-22 monobit balance check:
 *   ideal = 128 set bits (50/50 distribution).
 *   balance = 1 - |ones/256 - 0.5| * 2  → 1.0 = perfect, 0.0 = all-same.
 */
static inline int fiber_hash_monobit(const FiberHash *fh) {
    if (!fh) return 0;
    return _fh_popcnt64(fh->a)
         + _fh_popcnt64(fh->b)
         + _fh_popcnt64(fh->c)
         + _fh_popcnt64(fh->d);
}

/* ── fiber_hash_monobit_bytes: legacy byte-array variant ───────────────── */
/*
 * Accepts a 32-byte (256-bit) raw hash buffer — same semantics as
 * vv_monobit() in verbovivo.h.  Provided for callers that store the
 * hash as a byte array rather than a FiberHash struct.
 */
static inline int fiber_hash_monobit_bytes(const uint8_t hash[32]) {
    if (!hash) return 0;
    uint64_t w0, w1, w2, w3;
    memcpy(&w0, hash +  0, 8);
    memcpy(&w1, hash +  8, 8);
    memcpy(&w2, hash + 16, 8);
    memcpy(&w3, hash + 24, 8);
    return _fh_popcnt64(w0) + _fh_popcnt64(w1)
         + _fh_popcnt64(w2) + _fh_popcnt64(w3);
}

/* ── fiber_hash_hamming_bytes: legacy raw-byte Hamming distance ─────────── */
/*
 * Accepts two 32-byte (256-bit) byte arrays — same semantics as
 * vv_hamming_256() in verbovivo.h.
 */
static inline int fiber_hash_hamming_bytes(const uint8_t a[32], const uint8_t b[32]) {
    if (!a || !b) return 0;
    uint64_t a0, a1, a2, a3, b0, b1, b2, b3;
    memcpy(&a0, a +  0, 8); memcpy(&a1, a +  8, 8);
    memcpy(&a2, a + 16, 8); memcpy(&a3, a + 24, 8);
    memcpy(&b0, b +  0, 8); memcpy(&b1, b +  8, 8);
    memcpy(&b2, b + 16, 8); memcpy(&b3, b + 24, 8);
    return _fh_popcnt64(a0 ^ b0) + _fh_popcnt64(a1 ^ b1)
         + _fh_popcnt64(a2 ^ b2) + _fh_popcnt64(a3 ^ b3);
}

#endif /* RAFAELIA_FIBER_H_H */
