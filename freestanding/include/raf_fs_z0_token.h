#ifndef RAF_FS_Z0_TOKEN_H
#define RAF_FS_Z0_TOKEN_H

/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: classify the minimum observable input-presence states without context, attention, learned weights or normalization.
 * SCOPE: deterministic caller-owned byte view only; not a language-model tokenizer and not a claim about metaphysical nothingness.
 * PRECONDITIONS: when provided=1 and size>0, data points to at least size caller-owned readable bytes; NULL+size>0 is classified INVALID before dereference.
 * REGISTER_OWNERSHIP: compiler-owned scalar temporaries only; input/output state is caller-owned.
 * CLOBBERS: NONE beyond the returned value.
 * MEMORY_ORDER: NONE; read-only byte observation only.
 * TAIL_SHADOW: no loop, retry, fallback, hidden state, normalization or contextual carry.
 * EVIDENCE: source contract + freestanding compile/no-unresolved-helper probe + hosted semantic selftest; device evidence remains separate.
 */

#include "raf_fs_types.h"

typedef enum raf_fs_z0_kind {
    RAF_FS_Z0_ABSENT = 0,
    RAF_FS_Z0_EMPTY = 1,
    RAF_FS_Z0_SPACE = 2,
    RAF_FS_Z0_NUL = 3,
    RAF_FS_Z0_BYTE = 4,
    RAF_FS_Z0_SEQUENCE = 5,
    RAF_FS_Z0_INVALID = 6
} raf_fs_z0_kind;

typedef struct raf_fs_z0_view {
    const raf_u8 *data;
    raf_usize size;
    raf_u8 provided;
} raf_fs_z0_view;

static inline __attribute__((always_inline)) raf_fs_z0_kind
raf_fs_z0_classify(raf_fs_z0_view v) {
    if (v.provided == 0u) return RAF_FS_Z0_ABSENT;
    if (v.size == 0u) return RAF_FS_Z0_EMPTY;
    if (v.data == (const raf_u8 *)0) return RAF_FS_Z0_INVALID;
    if (v.size != 1u) return RAF_FS_Z0_SEQUENCE;
    if (v.data[0] == 0u) return RAF_FS_Z0_NUL;
    if (v.data[0] == 0x20u) return RAF_FS_Z0_SPACE;
    return RAF_FS_Z0_BYTE;
}

#endif
