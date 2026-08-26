/* raf_fluent_event.h — RAFAELIA Fluent-compatible event ABI v1.
 *
 * Purpose:
 *   Encode one Fluent Forward "Message mode" event directly as MessagePack:
 *     [tag, time, record]
 *
 * Properties:
 *   - no libc headers
 *   - no syscall-layer dependency
 *   - no heap
 *   - no external MessagePack library
 *   - caller-owned output buffer
 *   - deterministic field order
 *   - portable across ARM32/ARM64 and host validation builds
 *
 * Unknown build/runtime evidence is governed by CLOSURE_L2.
 * This is a transport/event ABI, not a compiler IR and not an artifact format.
 */
#pragma once

/* Compiler-provided primitive types avoid libc/stdint/sys.h dependencies and
 * keep this codec embeddable in the Stage0 core without namespace collisions. */
typedef __UINT8_TYPE__  raf_u8;
typedef __UINT16_TYPE__ raf_u16;
typedef __UINT32_TYPE__ raf_u32;
typedef __UINT64_TYPE__ raf_u64;
typedef __INT32_TYPE__  raf_i32;
typedef __SIZE_TYPE__   raf_sz;

typedef struct {
    const char *tag;
    const char *event;
    const char *component;
    const char *arch;
    const char *artifact_kind;
    const char *state;
    const char *source_sha256;
    const char *artifact_sha256;
    raf_u64 time_unix_s;
    raf_u64 seq;
    raf_u32 claim_allowed;
} RafFluentEventV1;

typedef struct {
    raf_u8 *p;
    raf_sz cap;
    raf_sz pos;
    raf_i32 err;
} RafMsgPackBuf;

static inline void raf_mp_u8(RafMsgPackBuf *b, raf_u8 v) {
    if (!b || b->err) return;
    if (b->pos >= b->cap) { b->err = -1; return; }
    b->p[b->pos++] = v;
}

static inline void raf_mp_be16(RafMsgPackBuf *b, raf_u16 v) {
    raf_mp_u8(b, (raf_u8)(v >> 8));
    raf_mp_u8(b, (raf_u8)v);
}

static inline void raf_mp_be32(RafMsgPackBuf *b, raf_u32 v) {
    raf_mp_u8(b, (raf_u8)(v >> 24));
    raf_mp_u8(b, (raf_u8)(v >> 16));
    raf_mp_u8(b, (raf_u8)(v >> 8));
    raf_mp_u8(b, (raf_u8)v);
}

static inline void raf_mp_be64(RafMsgPackBuf *b, raf_u64 v) {
    raf_mp_u8(b, (raf_u8)(v >> 56));
    raf_mp_u8(b, (raf_u8)(v >> 48));
    raf_mp_u8(b, (raf_u8)(v >> 40));
    raf_mp_u8(b, (raf_u8)(v >> 32));
    raf_mp_u8(b, (raf_u8)(v >> 24));
    raf_mp_u8(b, (raf_u8)(v >> 16));
    raf_mp_u8(b, (raf_u8)(v >> 8));
    raf_mp_u8(b, (raf_u8)v);
}

static inline raf_sz raf_cstr_len_65535(const char *s) {
    raf_sz n = 0;
    if (!s) return 0;
    while (s[n] && n < 65535u) n++;
    return n;
}

static inline void raf_mp_str(RafMsgPackBuf *b, const char *s) {
    raf_sz n = raf_cstr_len_65535(s);
    if (n <= 31u) {
        raf_mp_u8(b, (raf_u8)(0xA0u | (raf_u8)n));
    } else if (n <= 255u) {
        raf_mp_u8(b, 0xD9u);
        raf_mp_u8(b, (raf_u8)n);
    } else {
        raf_mp_u8(b, 0xDAu);
        raf_mp_be16(b, (raf_u16)n);
    }
    for (raf_sz i = 0; i < n; i++) raf_mp_u8(b, (raf_u8)s[i]);
}

static inline void raf_mp_u64(RafMsgPackBuf *b, raf_u64 v) {
    if (v <= 0x7Fu) {
        raf_mp_u8(b, (raf_u8)v);
    } else if (v <= 0xFFu) {
        raf_mp_u8(b, 0xCCu);
        raf_mp_u8(b, (raf_u8)v);
    } else if (v <= 0xFFFFu) {
        raf_mp_u8(b, 0xCDu);
        raf_mp_be16(b, (raf_u16)v);
    } else if (v <= 0xFFFFFFFFULL) {
        raf_mp_u8(b, 0xCEu);
        raf_mp_be32(b, (raf_u32)v);
    } else {
        raf_mp_u8(b, 0xCFu);
        raf_mp_be64(b, v);
    }
}

static inline void raf_mp_bool(RafMsgPackBuf *b, raf_u32 v) {
    raf_mp_u8(b, v ? 0xC3u : 0xC2u);
}

static inline void raf_mp_kv_str(RafMsgPackBuf *b, const char *k, const char *v) {
    raf_mp_str(b, k);
    raf_mp_str(b, v ? v : "TOKEN_VAZIO");
}

static inline void raf_mp_kv_u64(RafMsgPackBuf *b, const char *k, raf_u64 v) {
    raf_mp_str(b, k);
    raf_mp_u64(b, v);
}

static inline void raf_mp_kv_bool(RafMsgPackBuf *b, const char *k, raf_u32 v) {
    raf_mp_str(b, k);
    raf_mp_bool(b, v);
}

/*
 * Encodes a single Fluent Forward Message-mode event:
 *   [tag, time, {
 *      schema,event,component,arch,artifact_kind,state,
 *      source_sha256,artifact_sha256,seq,claim_allowed
 *   }]
 *
 * Return: encoded byte count; 0 on invalid input or capacity failure.
 */
static inline raf_sz raf_fluent_encode_event_v1(
    raf_u8 *out, raf_sz cap, const RafFluentEventV1 *e)
{
    if (!out || !cap || !e || !e->tag) return 0;

    RafMsgPackBuf b;
    b.p = out;
    b.cap = cap;
    b.pos = 0;
    b.err = 0;

    /* fixarray(3): tag, time, record */
    raf_mp_u8(&b, 0x93u);
    raf_mp_str(&b, e->tag);
    raf_mp_u64(&b, e->time_unix_s);

    /* fixmap(10), deterministic order */
    raf_mp_u8(&b, 0x8Au);
    raf_mp_kv_str(&b, "schema", "RAFAELIA_FLUENT_EVENT/v1");
    raf_mp_kv_str(&b, "event", e->event);
    raf_mp_kv_str(&b, "component", e->component);
    raf_mp_kv_str(&b, "arch", e->arch);
    raf_mp_kv_str(&b, "artifact_kind", e->artifact_kind);
    raf_mp_kv_str(&b, "state", e->state);
    raf_mp_kv_str(&b, "source_sha256", e->source_sha256);
    raf_mp_kv_str(&b, "artifact_sha256", e->artifact_sha256);
    raf_mp_kv_u64(&b, "seq", e->seq);
    raf_mp_kv_bool(&b, "claim_allowed", e->claim_allowed);

    return b.err ? 0 : b.pos;
}
