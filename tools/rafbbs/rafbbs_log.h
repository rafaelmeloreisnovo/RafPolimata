#ifndef RAFBBS_LOG_H
#define RAFBBS_LOG_H
#include <stdarg.h>
#include <stdio.h>
#include "rafbbs_core.h"
#include "rafbbs_time_posix.h"
#include "rafbbs_theme.h"
#include "rafbbs_log_core.h"
#define RAFBBS_MAX_LOG_LINES 512
static char rafbbs_lines[RAFBBS_MAX_LOG_LINES][RAFBBS_LOG_LINE];
static long raf_elapsed_ms(RafContext *ctx) {
    RafMonoElapsed elapsed = raf_mono_elapsed_ms(ctx->start, raf_mono_posix_now());
    if (elapsed.valid == 0u) return -1L;
    return (long)elapsed.ms;
}
static void raf_log(RafContext *ctx, RafStatus st, const char *module, const char *fmt, ...) {
    char detail[160]; long ms = raf_elapsed_ms(ctx); va_list ap;
    va_start(ap, fmt); vsnprintf(detail, sizeof(detail), fmt, ap); va_end(ap);
    if (ctx->syslog_count < RAFBBS_MAX_LOG_LINES) {
        RafLogText line;
        raf_log_text_init(&line, rafbbs_lines[ctx->syslog_count], (RafU32)RAFBBS_LOG_LINE);
        raf_log_line_render(&line, st, module, detail, ms < 0L ? 0ull : (RafU64)ms, (RafU32)(ms >= 0L));
        ctx->syslog_count++;
    }
    printf("%s%s%s\n", raf_status_color(st), rafbbs_lines[ctx->syslog_count - 1], RAF_ANSI_RESET);
    fflush(stdout);
}
static int raf_write_log(RafContext *ctx) {
    int i; FILE *f = fopen(ctx->log_path, "w"); if (!f) return -1;
    fprintf(f, "# RafBBS Run Log\n\nrun_id=%s\npipeline=%s\nstatus=%s\nhost=%s\narch=%s\ncommit=%s\nbranch=%s\nmanifest=%s\nbin_manifest=%s\n\n[SYSLOG]\n",
            ctx->run_id, ctx->pipeline, raf_status_name(ctx->final_status), ctx->host, ctx->arch, ctx->commit, ctx->branch, ctx->manifest_path, ctx->bin_manifest_path);
    for (i = 0; i < ctx->syslog_count; i++) fprintf(f, "%s\n", rafbbs_lines[i]);
    fprintf(f, "\n[ARTIFACTS]\ninput=%s\noutput=%s\ninput_crc32=%08x\noutput_crc32=%08x\ninput_sha256=%s\noutput_sha256=%s\nhash_state=%08x\n\n[GAPS]\n%s\n",
            ctx->input, ctx->output, ctx->input_crc32, ctx->output_crc32, ctx->input_sha256, ctx->output_sha256, ctx->hash_state, ctx->gaps[0] ? ctx->gaps : "none=TOKEN_VAZIO");
    fclose(f); return 0;
}
#endif
