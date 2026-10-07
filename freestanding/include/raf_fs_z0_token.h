#ifndef RAF_FS_Z0_TOKEN_H
#define RAF_FS_Z0_TOKEN_H

/* RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Deterministic Z0 byte observation/tokenization with no learned or contextual machinery.
 * SCOPE: Freestanding byte view only; no vocabulary, normalization, embedding, attention, learned weights,
 *        synthetic BOS/EOS tokens, history, allocator, libc, syscall, OS/provider API or semantic inference.
 * PRECONDITIONS: Caller owns input bytes and length for the duration of each call.
 * REGISTER_OWNERSHIP: Compiler-allocated scalar registers only; caller owns all memory.
 * CLOBBERS: NONE beyond caller-provided output object on successful token emission.
 * MEMORY_ORDER: NONE; ordinary scalar reads/writes only.
 * TAIL_SHADOW: No residual loop, hidden retry or shadow state. One token_at call observes at most one byte.
 * EVIDENCE: Source contract until executed compile/semantic gates bind an exact revision; runtime/device remains CLOSURE_L12.
 */

typedef unsigned char raf_z0_u8;
typedef unsigned long raf_z0_size;

#define RAF_Z0_CONTEXT_LEFT            0u
#define RAF_Z0_CONTEXT_RIGHT           0u
#define RAF_Z0_ATTENTION_EDGE_COUNT    0u
#define RAF_Z0_LEARNED_WEIGHT_COUNT    0u
#define RAF_Z0_VOCAB_ENTRY_COUNT       0u
#define RAF_Z0_NORMALIZATION_PASSES    0u
#define RAF_Z0_SYNTHETIC_TOKEN_COUNT   0u
#define RAF_Z0_HISTORY_BYTES           0u

#define RAF_Z0_STATE_ABSENT   ((raf_z0_u8)0x11u)
#define RAF_Z0_STATE_EMPTY    ((raf_z0_u8)0x12u)
#define RAF_Z0_STATE_PRESENT  ((raf_z0_u8)0x13u)
#define RAF_Z0_STATE_INVALID  ((raf_z0_u8)0x1fu)

#define RAF_Z0_TOKEN_BYTE     ((raf_z0_u8)0x21u)

#define RAF_Z0_EMIT_TOKEN     ((raf_z0_u8)0x31u)
#define RAF_Z0_NO_TOKEN       ((raf_z0_u8)0x32u)
#define RAF_Z0_INPUT_ABSENT   ((raf_z0_u8)0x33u)
#define RAF_Z0_INVALID_VIEW   ((raf_z0_u8)0x3fu)

typedef struct raf_z0_view {
    const raf_z0_u8 *data;
    raf_z0_size len;
} raf_z0_view;

typedef struct raf_z0_token {
    raf_z0_size offset;
    raf_z0_u8 byte;
    raf_z0_u8 kind;
} raf_z0_token;

static __inline__ __attribute__((always_inline)) raf_z0_u8
raf_z0_classify(raf_z0_view view)
{
    if (view.data == (const raf_z0_u8 *)0) {
        return view.len == 0u ? RAF_Z0_STATE_ABSENT : RAF_Z0_STATE_INVALID;
    }
    return view.len == 0u ? RAF_Z0_STATE_EMPTY : RAF_Z0_STATE_PRESENT;
}

static __inline__ __attribute__((always_inline)) raf_z0_size
raf_z0_token_count(raf_z0_view view)
{
    return view.data == (const raf_z0_u8 *)0 ? 0u : view.len;
}

static __inline__ __attribute__((always_inline)) raf_z0_u8
raf_z0_token_at(raf_z0_view view, raf_z0_size index, raf_z0_token *out)
{
    if (out == (raf_z0_token *)0) {
        return RAF_Z0_INVALID_VIEW;
    }
    if (view.data == (const raf_z0_u8 *)0) {
        return view.len == 0u ? RAF_Z0_INPUT_ABSENT : RAF_Z0_INVALID_VIEW;
    }
    if (index >= view.len) {
        return RAF_Z0_NO_TOKEN;
    }

    out->offset = index;
    out->byte = view.data[index];
    out->kind = RAF_Z0_TOKEN_BYTE;
    return RAF_Z0_EMIT_TOKEN;
}

#endif
