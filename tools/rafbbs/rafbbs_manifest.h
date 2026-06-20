#ifndef RAFBBS_MANIFEST_H
#define RAFBBS_MANIFEST_H
#include <stdio.h>
#include "rafbbs_core.h"
#include "rafbbs_log.h"
#include "rafbbs_manifest_bin.h"

static int raf_write_manifest(RafContext *ctx) {
    FILE *f = fopen(ctx->manifest_path, "w");
    if (!f) return -1;
    fprintf(f, "[RAFBBS_MANIFEST]\n");
    fprintf(f, "pipeline=%s\nstatus=%s\ninput=%s\noutput=%s\narch=%s\nhost=%s\ncommit=%s\nbranch=%s\ncommand=%s\nelapsed_ms=%ld\ninput_crc32=%08x\noutput_crc32=%08x\nhash_state=%08x\ninput_sha256=%s\noutput_sha256=%s\nlog=%s\nbin_manifest=%s\ngaps=%s\n",
            ctx->pipeline, raf_status_name(ctx->final_status), ctx->input, ctx->output, ctx->arch, ctx->host,
            ctx->commit, ctx->branch, ctx->command, raf_elapsed_ms(ctx), ctx->input_crc32, ctx->output_crc32, ctx->hash_state, ctx->input_sha256, ctx->output_sha256,
            ctx->log_path, ctx->bin_manifest_path, ctx->gaps[0] ? ctx->gaps : "none");
    fclose(f);
    {
        RafBinManifest bm = raf_bin_manifest_make((uint32_t)ctx->final_status, 0u, ctx->input_crc32, ctx->output_crc32, ctx->hash_state, (uint32_t)(ctx->gaps[0] != 0));
        (void)raf_write_bin_manifest_file(ctx->bin_manifest_path, &bm);
    }
    return 0;
}
#endif
