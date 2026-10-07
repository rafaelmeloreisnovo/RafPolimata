#ifndef RAFBBS_MANIFEST_CORE_H
#define RAFBBS_MANIFEST_CORE_H

/*
 * RafBBS deterministic text-manifest core.
 *
 * Boundary:
 * - caller-owned output buffer;
 * - zero hosted headers, libc, heap, syscall or provider API;
 * - filesystem persistence remains in rafbbs_manifest.h;
 * - overflow is explicit through dropped != 0 and never reported as PASS.
 */

#include "rafbbs_core.h"

#define RAFBBS_MANIFEST_TEXT_CAP 4096u

typedef struct {
    char *buf;
    RafU32 cap;
    RafU32 pos;
    RafU32 dropped;
} RafManifestText;

static inline void raf_manifest_text_init(RafManifestText *o, char *buf, RafU32 cap)
{
    o->buf = buf;
    o->cap = cap;
    o->pos = 0u;
    o->dropped = 0u;
    if (buf != (char *)0 && cap != 0u)
        buf[0] = 0;
}

static inline void raf_manifest_putc(RafManifestText *o, char c)
{
    RafU32 room = (RafU32)(
        o->buf != (char *)0 &&
        o->cap != 0u &&
        (o->pos + 1u) < o->cap
    );

    if (room != 0u) {
        o->buf[o->pos++] = c;
        o->buf[o->pos] = 0;
    } else {
        o->dropped += 1u;
    }
}

static inline void raf_manifest_write(RafManifestText *o, const char *s)
{
    const char *p = s != (const char *)0 ? s : RAFBBS_TOKEN_EMPTY;
    while (*p != 0) {
        raf_manifest_putc(o, *p);
        p++;
    }
}

static inline void raf_manifest_u64_dec(RafManifestText *o, RafU64 value)
{
    char digit[20];
    RafU32 n = 0u;

    if (value == 0ull) {
        raf_manifest_putc(o, '0');
        return;
    }

    while (value != 0ull) {
        digit[n++] = (char)('0' + (char)(value % 10ull));
        value /= 10ull;
    }

    while (n != 0u)
        raf_manifest_putc(o, digit[--n]);
}

static inline void raf_manifest_u32_hex8(RafManifestText *o, RafU32 value)
{
    static const char hex[] = "0123456789abcdef";
    RafU32 i;

    for (i = 0u; i < 8u; i++) {
        RafU32 shift = 28u - (i * 4u);
        raf_manifest_putc(o, hex[(value >> shift) & 0x0fu]);
    }
}

static inline void raf_manifest_kv(RafManifestText *o, const char *key, const char *value)
{
    raf_manifest_write(o, key);
    raf_manifest_putc(o, '=');
    raf_manifest_write(o, value);
    raf_manifest_putc(o, '\n');
}

static inline void raf_manifest_render(
    RafManifestText *o,
    const RafContext *ctx,
    RafU64 elapsed_ms,
    RafU32 elapsed_valid
)
{
    raf_manifest_write(o, "[RAFBBS_MANIFEST]\n");
    raf_manifest_kv(o, "pipeline", ctx->pipeline);
    raf_manifest_kv(o, "status", raf_status_name(ctx->final_status));
    raf_manifest_kv(o, "input", ctx->input);
    raf_manifest_kv(o, "output", ctx->output);
    raf_manifest_kv(o, "arch", ctx->arch);
    raf_manifest_kv(o, "host", ctx->host);
    raf_manifest_kv(o, "commit", ctx->commit);
    raf_manifest_kv(o, "branch", ctx->branch);
    raf_manifest_kv(o, "command", ctx->command);

    raf_manifest_write(o, "elapsed_ms=");
    if (elapsed_valid != 0u)
        raf_manifest_u64_dec(o, elapsed_ms);
    else
        raf_manifest_write(o, "-1");
    raf_manifest_putc(o, '\n');

    raf_manifest_write(o, "input_crc32=");
    raf_manifest_u32_hex8(o, ctx->input_crc32);
    raf_manifest_putc(o, '\n');

    raf_manifest_write(o, "output_crc32=");
    raf_manifest_u32_hex8(o, ctx->output_crc32);
    raf_manifest_putc(o, '\n');

    raf_manifest_write(o, "hash_state=");
    raf_manifest_u32_hex8(o, ctx->hash_state);
    raf_manifest_putc(o, '\n');

    raf_manifest_kv(o, "input_sha256", ctx->input_sha256);
    raf_manifest_kv(o, "output_sha256", ctx->output_sha256);
    raf_manifest_kv(o, "log", ctx->log_path);
    raf_manifest_kv(o, "bin_manifest", ctx->bin_manifest_path);
    raf_manifest_kv(o, "gaps", ctx->gaps[0] != 0 ? ctx->gaps : "none");
}

#endif
