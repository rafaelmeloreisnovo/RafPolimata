#ifndef RAFBBS_LOG_H
#define RAFBBS_LOG_H
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "rafbbs_core.h"
#include "rafbbs_theme.h"

#define RAFBBS_MAX_LOG_LINES 512
#define RAFBBS_LOG_LINE 256
static char rafbbs_lines[RAFBBS_MAX_LOG_LINES][RAFBBS_LOG_LINE];

static long raf_elapsed_ms(RafContext *ctx) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long)((now.tv_sec - ctx->start.tv_sec) * 1000L + (now.tv_nsec - ctx->start.tv_nsec) / 1000000L);
}

static void raf_log(RafContext *ctx, RafStatus st, const char *module, const char *fmt, ...) {
    char detail[160];
    long ms = raf_elapsed_ms(ctx);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(detail, sizeof(detail), fmt, ap);
    va_end(ap);
    if (ctx->syslog_count < RAFBBS_MAX_LOG_LINES) {
        snprintf(rafbbs_lines[ctx->syslog_count], RAFBBS_LOG_LINE, "%02ld:%02ld.%03ld %-12s %-10s %s",
                 ms / 60000L, (ms / 1000L) % 60L, ms % 1000L, raf_status_name(st), module, detail);
        ctx->syslog_count++;
    }
    printf("%s%s%s\n", raf_status_color(st), rafbbs_lines[ctx->syslog_count - 1], RAF_ANSI_RESET);
    fflush(stdout);
}

static int raf_write_log(RafContext *ctx) {
    int i;
    FILE *f = fopen(ctx->log_path, "w");
    if (!f) return -1;
    fprintf(f, "# RafBBS Run Log\n\nrun_id=%s\npipeline=%s\nstatus=%s\nhost=%s\narch=%s\ncommit=%s\nbranch=%s\nmanifest=%s\nbin_manifest=%s\n\n[SYSLOG]\n",
            ctx->run_id, ctx->pipeline, raf_status_name(ctx->final_status), ctx->host, ctx->arch, ctx->commit, ctx->branch, ctx->manifest_path, ctx->bin_manifest_path);
    for (i = 0; i < ctx->syslog_count; i++) fprintf(f, "%s\n", rafbbs_lines[i]);
    fprintf(f, "\n[ARTIFACTS]\ninput=%s\noutput=%s\ninput_crc32=%08x\noutput_crc32=%08x\ninput_sha256=%s\noutput_sha256=%s\nhash_state=%08x\n\n[GAPS]\n%s\n",
            ctx->input, ctx->output, ctx->input_crc32, ctx->output_crc32, ctx->input_sha256, ctx->output_sha256, ctx->hash_state, ctx->gaps[0] ? ctx->gaps : "none=TOKEN_VAZIO");
    fclose(f);
    return 0;
}
#endif
