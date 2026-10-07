#ifndef RAFBBS_RUNLOG_CORE_H
#define RAFBBS_RUNLOG_CORE_H

#include "rafbbs_core.h"
#include "rafbbs_log_core.h"

/*
 * Deterministic run-log byte rendering.
 *
 * This core owns formatting only. The caller owns storage and any persistence
 * adapter. No FILE, filesystem, terminal, allocator, clock or provider API is
 * reachable from this module.
 */

static inline RafU32 raf_runlog_cstr_len(const char *text)
{
    RafU32 n = 0u;
    if (text == (const char *)0) return 0u;
    while (text[n] != 0) n++;
    return n;
}

static inline void raf_runlog_hex32(RafLogText *out, RafU32 value)
{
    static const char hex[] = "0123456789abcdef";
    RafU32 shift = 32u;
    while (shift > 0u) {
        shift -= 4u;
        raf_log_text_putc(out, hex[(value >> shift) & 0x0fu]);
    }
}

static inline void raf_runlog_key_value(
    RafLogText *out,
    const char *key,
    const char *value
) {
    raf_log_text_puts(out, key);
    raf_log_text_putc(out, '=');
    raf_log_text_puts(out, value);
    raf_log_text_putc(out, '\n');
}

static inline void raf_runlog_header_render(
    RafLogText *out,
    const RafContext *ctx
) {
    raf_log_text_puts(out, "# RafBBS Run Log\n\n");
    raf_runlog_key_value(out, "run_id", ctx->run_id);
    raf_runlog_key_value(out, "pipeline", ctx->pipeline);
    raf_runlog_key_value(out, "status", raf_status_name(ctx->final_status));
    raf_runlog_key_value(out, "host", ctx->host);
    raf_runlog_key_value(out, "arch", ctx->arch);
    raf_runlog_key_value(out, "commit", ctx->commit);
    raf_runlog_key_value(out, "branch", ctx->branch);
    raf_runlog_key_value(out, "manifest", ctx->manifest_path);
    raf_runlog_key_value(out, "bin_manifest", ctx->bin_manifest_path);
    raf_log_text_puts(out, "\n[SYSLOG]\n");
}

static inline void raf_runlog_tail_render(
    RafLogText *out,
    const RafContext *ctx
) {
    raf_log_text_puts(out, "\n[ARTIFACTS]\n");
    raf_runlog_key_value(out, "input", ctx->input);
    raf_runlog_key_value(out, "output", ctx->output);

    raf_log_text_puts(out, "input_crc32=");
    raf_runlog_hex32(out, ctx->input_crc32);
    raf_log_text_putc(out, '\n');

    raf_log_text_puts(out, "output_crc32=");
    raf_runlog_hex32(out, ctx->output_crc32);
    raf_log_text_putc(out, '\n');

    raf_runlog_key_value(out, "input_sha256", ctx->input_sha256);
    raf_runlog_key_value(out, "output_sha256", ctx->output_sha256);

    raf_log_text_puts(out, "hash_state=");
    raf_runlog_hex32(out, ctx->hash_state);
    raf_log_text_putc(out, '\n');

    raf_log_text_puts(out, "\n[GAPS]\n");
    if (ctx->gaps[0] != 0)
        raf_log_text_puts(out, ctx->gaps);
    else
        raf_log_text_puts(out, "none=TOKEN_VAZIO");
    raf_log_text_putc(out, '\n');
}

#endif
