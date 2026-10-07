#ifndef RAFBBS_LOG_H
#define RAFBBS_LOG_H
#include <stdio.h>
#include "rafbbs_core.h"
#include "rafbbs_time_posix.h"
#include "rafbbs_theme.h"
#include "rafbbs_log_core.h"
#include "rafbbs_runlog_core.h"
#include "rafbbs_format_core.h"
#define RAFBBS_MAX_LOG_LINES 512
static char rafbbs_lines[RAFBBS_MAX_LOG_LINES][RAFBBS_LOG_LINE];
static long raf_elapsed_ms(RafContext *ctx) {
    RafMonoElapsed elapsed = raf_mono_elapsed_ms(ctx->start, raf_mono_posix_now());
    if (elapsed.valid == 0u) return -1L;
    return (long)elapsed.ms;
}
static void raf_log(RafContext *ctx, RafStatus st, const char *module, const char *detail) {
    long ms = raf_elapsed_ms(ctx);
    if (ctx->syslog_count < RAFBBS_MAX_LOG_LINES) {
        RafLogText line;
        raf_log_text_init(&line, rafbbs_lines[ctx->syslog_count], (RafU32)RAFBBS_LOG_LINE);
        raf_log_line_render(&line, st, module, detail, ms < 0L ? 0ull : (RafU64)ms, (RafU32)(ms >= 0L));
        ctx->syslog_count++;
    }
    printf("%s%s%s\n", raf_status_color(st), rafbbs_lines[ctx->syslog_count - 1], RAF_ANSI_RESET);
    fflush(stdout);
}
static void raf_log_s(RafContext *ctx, RafStatus st, const char *module, const char *prefix, const char *value) {
    char detail[160];
    RafLogText out;
    raf_log_text_init(&out, detail, (RafU32)sizeof(detail));
    raf_format_prefixed_text(&out, prefix, value);
    raf_log(ctx, st, module, detail);
}
static void raf_log_i32(RafContext *ctx, RafStatus st, const char *module, const char *prefix, RafI32 value) {
    char detail[160];
    RafLogText out;
    raf_log_text_init(&out, detail, (RafU32)sizeof(detail));
    raf_format_prefixed_i32(&out, prefix, value);
    raf_log(ctx, st, module, detail);
}
static void raf_log_hex32(RafContext *ctx, RafStatus st, const char *module, const char *prefix, RafU32 value) {
    char detail[160];
    RafLogText out;
    raf_log_text_init(&out, detail, (RafU32)sizeof(detail));
    raf_format_prefixed_hex32(&out, prefix, value);
    raf_log(ctx, st, module, detail);
}
static int raf_file_write_exact(FILE *f, const char *buf, RafU32 len) {
    return fwrite(buf, 1u, (size_t)len, f) == (size_t)len ? 0 : -1;
}
static int raf_file_write_line(FILE *f, const char *text) {
    RafU32 len = raf_format_cstr_len(text);
    if (raf_file_write_exact(f, text, len) != 0) return -1;
    return raf_file_write_exact(f, "\n", 1u);
}
static int raf_write_log(RafContext *ctx) {
    int i;
    int rc = 0;
    char section[4096];
    RafLogText out;
    FILE *f = fopen(ctx->log_path, "w");
    if (!f) return -1;

    raf_log_text_init(&out, section, (RafU32)sizeof(section));
    raf_runlog_header_render(&out, ctx);
    if (out.dropped != 0u || raf_file_write_exact(f, section, out.pos) != 0)
        rc = -1;

    for (i = 0; rc == 0 && i < ctx->syslog_count; i++)
        if (raf_file_write_line(f, rafbbs_lines[i]) != 0) rc = -1;

    if (rc == 0) {
        raf_log_text_init(&out, section, (RafU32)sizeof(section));
        raf_runlog_tail_render(&out, ctx);
        if (out.dropped != 0u || raf_file_write_exact(f, section, out.pos) != 0)
            rc = -1;
    }

    if (fclose(f) != 0) rc = -1;
    return rc;
}
#endif
