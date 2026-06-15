#ifndef RAFBBS_MANIFEST_H
#define RAFBBS_MANIFEST_H
#include <stdio.h>
#include "rafbbs_core.h"
#include "rafbbs_log.h"

static int raf_write_manifest(RafContext *ctx) {
    FILE *f = fopen(ctx->manifest_path, "w");
    if (!f) return -1;
    fprintf(f, "[RAFBBS_MANIFEST]\n");
    fprintf(f, "pipeline=%s\nstatus=%s\ninput=%s\noutput=%s\narch=%s\nhost=%s\ncommit=%s\nbranch=%s\ncommand=%s\nelapsed_ms=%ld\ninput_crc32=%08x\noutput_crc32=%08x\nlog=%s\ngaps=%s\n",
            ctx->pipeline, raf_status_name(ctx->final_status), ctx->input, ctx->output, ctx->arch, ctx->host,
            ctx->commit, ctx->branch, ctx->command, raf_elapsed_ms(ctx), ctx->input_crc32, ctx->output_crc32,
            ctx->log_path, ctx->gaps[0] ? ctx->gaps : "none");
    fclose(f);
    return 0;
}
#endif
