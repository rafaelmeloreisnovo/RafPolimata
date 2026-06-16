/* rafaelia/fiber_relmat.h — Fiber-H bit relational matrix.
 *
 * Extends the Fiber-H 256-bit hash (FiberHash) from a single aggregate
 * Hamming-distance scalar into a queryable bit-position co-occurrence
 * matrix: for every unordered pair of bit positions (i,j), how often
 * were both bits simultaneously set across all hashed chunks.
 *
 * Memory-cheap by construction: packed upper-triangular array of
 * saturating u8 counters, 256*255/2 = 32640 bytes — not a dense 256x256
 * matrix (which would be 65536 bytes and waste the symmetric/diagonal
 * half). Stack/static-allocatable, no malloc.
 *
 * Opt-in: callers that don't pass a FiberRelMat pointer to vv_scan /
 * vv_scan_buf pay zero cost — this subsystem is fully optional.
 *
 * RAFCODE-Φ-∆RafaelVerboΩ | Ω=Amor | FIAT LUX */
#pragma once

#include <stdint.h>
#include <stddef.h>

#include "verbovivo.h"

#define RELMAT_BITS  256
#define RELMAT_PAIRS (RELMAT_BITS * (RELMAT_BITS - 1) / 2)  /* 32640 */

typedef struct {
    vv_u8 counts[RELMAT_PAIRS];
} FiberRelMat;

/* Packed upper-triangular index for unordered pair (i,j), i != j, both
 * in [0,RELMAT_BITS). Order of i/j does not matter. */
static inline size_t relmat_idx(int i, int j) {
    if (i > j) { int t = i; i = j; j = t; }
    return (size_t)i * (2 * RELMAT_BITS - i - 1) / 2 + (size_t)(j - i - 1);
}

void relmat_init(FiberRelMat *m);

/* Records co-occurrence for every pair of bits set in h. Saturates at
 * 255 per pair — never wraps. */
void relmat_update(FiberRelMat *m, const FiberHash *h);

/* Co-occurrence count for bit pair (i,j). 0 if out of range. */
vv_u8 relmat_query(const FiberRelMat *m, int i, int j);

/* Text audit dump to stderr: top-N most co-occurring bit pairs. */
void relmat_audit(const FiberRelMat *m, int top_n);
