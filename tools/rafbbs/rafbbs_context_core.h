#ifndef RAFBBS_CONTEXT_CORE_H
#define RAFBBS_CONTEXT_CORE_H

#include "rafbbs_core.h"

typedef struct {
    const char *run_id;
    const char *pipeline;
    const char *host;
    const char *arch;
    const char *branch;
    const char *commit;
    RafMonoTime start;
} RafContextSeed;

static inline void raf_context_zero(RafContext *ctx)
{
    volatile RafU8 *bytes = (volatile RafU8 *)(void *)ctx;
    RafU32 i;
    for (i = 0u; i < (RafU32)sizeof(*ctx); ++i)
        bytes[i] = 0u;
}

static inline RafU32 raf_context_text_copy(
    char *dst, RafU32 cap, const char *src
)
{
    RafU32 i = 0u;
    if (cap == 0u)
        return (RafU32)(src != (const char *)0 && src[0] != 0);
    if (src == (const char *)0) {
        dst[0] = 0;
        return 0u;
    }
    while (src[i] != 0 && i + 1u < cap) {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = 0;
    return (RafU32)(src[i] != 0);
}

static inline RafU32 raf_context_append(
    char *dst, RafU32 cap, RafU32 *pos, const char *src
)
{
    RafU32 i = 0u;
    RafU32 dropped = 0u;
    if (src == (const char *)0)
        return 0u;
    while (src[i] != 0) {
        if (cap != 0u && *pos + 1u < cap) {
            dst[*pos] = src[i];
            *pos += 1u;
        } else {
            dropped = 1u;
        }
        ++i;
    }
    if (cap != 0u)
        dst[*pos] = 0;
    return dropped;
}

static inline RafU32 raf_context_path(
    char *dst, RafU32 cap,
    const char *prefix, const char *run_id, const char *suffix
)
{
    RafU32 pos = 0u;
    RafU32 dropped = 0u;
    if (cap != 0u)
        dst[0] = 0;
    dropped |= raf_context_append(dst, cap, &pos, prefix);
    dropped |= raf_context_append(dst, cap, &pos, run_id);
    dropped |= raf_context_append(dst, cap, &pos, suffix);
    return dropped;
}

static inline RafU32 raf_context_init_seeded(
    RafContext *ctx, const RafContextSeed *seed
)
{
    RafU32 dropped = 0u;
    raf_context_zero(ctx);
    ctx->watchdog = raf_watchdog_start(RAFBBS_WATCHDOG_DEFAULT_TICKS);
    if (seed == (const RafContextSeed *)0) {
        ctx->start = raf_mono_invalid();
        return 0u;
    }

    ctx->start = seed->start;
    dropped |= raf_context_text_copy(
        ctx->run_id, (RafU32)sizeof(ctx->run_id), seed->run_id
    );
    dropped |= raf_context_text_copy(
        ctx->pipeline, (RafU32)sizeof(ctx->pipeline), seed->pipeline
    );
    dropped |= raf_context_text_copy(
        ctx->host, (RafU32)sizeof(ctx->host), seed->host
    );
    dropped |= raf_context_text_copy(
        ctx->arch, (RafU32)sizeof(ctx->arch), seed->arch
    );
    dropped |= raf_context_text_copy(
        ctx->branch, (RafU32)sizeof(ctx->branch), seed->branch
    );
    dropped |= raf_context_text_copy(
        ctx->commit, (RafU32)sizeof(ctx->commit), seed->commit
    );
    dropped |= raf_context_path(
        ctx->log_path, (RafU32)sizeof(ctx->log_path),
        "tools/rafbbs/logs/run-", ctx->run_id, ".txt"
    );
    dropped |= raf_context_path(
        ctx->manifest_path, (RafU32)sizeof(ctx->manifest_path),
        "tools/rafbbs/logs/manifest-", ctx->run_id, ".txt"
    );
    dropped |= raf_context_path(
        ctx->bin_manifest_path, (RafU32)sizeof(ctx->bin_manifest_path),
        "tools/rafbbs/logs/manifest-", ctx->run_id, ".bin"
    );
    return dropped;
}

#endif
