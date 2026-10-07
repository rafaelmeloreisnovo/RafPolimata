#ifndef RAF_FS_Z0_TOKEN_H
#define RAF_FS_Z0_TOKEN_H

/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Deterministic Z0 presence classification plus byte token emission with no learned or contextual machinery.
 * SCOPE: Freestanding caller-owned byte view only; no vocabulary, normalization, embedding, attention, learned weights,
 *        synthetic BOS/EOS tokens, history, allocator, libc, syscall, OS/provider API or semantic inference.
 * PRECONDITIONS: provided is 0 or 1; when provided=1 and size>0, data points to at least size readable caller-owned bytes.
 * REGISTER_OWNERSHIP: Compiler-allocated scalar registers only; caller owns all input/output memory.
 * CLOBBERS: NONE beyond caller-provided output token on successful emission.
 * MEMORY_ORDER: NONE; ordinary scalar reads/writes only.
 * TAIL_SHADOW: No residual loop, hidden retry, normalization, contextual carry or shadow state. token_at observes at most one byte.
 * EVIDENCE: Source contract until executed compile/semantic gates bind an exact revision; runtime/device remains CLOSURE_L12.
 */

#include "raf_fs_types.h"

#define RAF_Z0_CONTEXT_LEFT            0u
#define RAF_Z0_CONTEXT_RIGHT           0u
#define RAF_Z0_ATTENTION_EDGE_COUNT    0u
#define RAF_Z0_LEARNED_WEIGHT_COUNT    0u
#define RAF_Z0_VOCAB_ENTRY_COUNT       0u
#define RAF_Z0_NORMALIZATION_PASSES    0u
#define RAF_Z0_SYNTHETIC_TOKEN_COUNT   0u
#define RAF_Z0_HISTORY_BYTES           0u

typedef enum raf_fs_z0_kind {
    RAF_FS_Z0_ABSENT = 0,
    RAF_FS_Z0_EMPTY = 1,
    RAF_FS_Z0_SPACE = 2,
    RAF_FS_Z0_NUL = 3,
    RAF_FS_Z0_BYTE = 4,
    RAF_FS_Z0_SEQUENCE = 5,
    RAF_FS_Z0_INVALID = 6
} raf_fs_z0_kind;

typedef enum raf_fs_z0_emit {
    RAF_FS_Z0_EMIT_TOKEN = 0x31,
    RAF_FS_Z0_NO_TOKEN = 0x32,
    RAF_FS_Z0_INPUT_ABSENT = 0x33,
    RAF_FS_Z0_INVALID_VIEW = 0x3f
} raf_fs_z0_emit;

typedef struct raf_fs_z0_view {
    const raf_u8 *data;
    raf_usize size;
    raf_u8 provided;
} raf_fs_z0_view;

typedef struct raf_fs_z0_token {
    raf_usize offset;
    raf_u8 byte;
    raf_fs_z0_kind kind;
} raf_fs_z0_token;

static inline __attribute__((always_inline)) raf_fs_z0_kind
raf_fs_z0_classify(raf_fs_z0_view view)
{
    if (view.provided == 0u) {
        return RAF_FS_Z0_ABSENT;
    }
    if (view.size == 0u) {
        return RAF_FS_Z0_EMPTY;
    }
    if (view.data == (const raf_u8 *)0) {
        return RAF_FS_Z0_INVALID;
    }
    if (view.size != 1u) {
        return RAF_FS_Z0_SEQUENCE;
    }
    if (view.data[0] == 0u) {
        return RAF_FS_Z0_NUL;
    }
    if (view.data[0] == 0x20u) {
        return RAF_FS_Z0_SPACE;
    }
    return RAF_FS_Z0_BYTE;
}

static inline __attribute__((always_inline)) raf_usize
raf_fs_z0_token_count(raf_fs_z0_view view)
{
    if (view.provided == 0u) {
        return 0u;
    }
    if (view.size != 0u && view.data == (const raf_u8 *)0) {
        return 0u;
    }
    return view.size;
}

static inline __attribute__((always_inline)) raf_fs_z0_emit
raf_fs_z0_token_at(raf_fs_z0_view view, raf_usize index, raf_fs_z0_token *out)
{
    raf_u8 byte;

    if (out == (raf_fs_z0_token *)0) {
        return RAF_FS_Z0_INVALID_VIEW;
    }
    if (view.provided == 0u) {
        return RAF_FS_Z0_INPUT_ABSENT;
    }
    if (view.size != 0u && view.data == (const raf_u8 *)0) {
        return RAF_FS_Z0_INVALID_VIEW;
    }
    if (index >= view.size) {
        return RAF_FS_Z0_NO_TOKEN;
    }

    byte = view.data[index];
    out->offset = index;
    out->byte = byte;
    out->kind = byte == 0u ? RAF_FS_Z0_NUL : (byte == 0x20u ? RAF_FS_Z0_SPACE : RAF_FS_Z0_BYTE);
    return RAF_FS_Z0_EMIT_TOKEN;
}

#endif
