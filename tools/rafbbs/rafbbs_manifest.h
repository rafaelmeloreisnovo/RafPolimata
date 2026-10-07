#ifndef RAFBBS_MANIFEST_H
#define RAFBBS_MANIFEST_H
#include <stdio.h>
#include "rafbbs_core.h"
#include "rafbbs_log.h"
#include "rafbbs_manifest_bin.h"
#include "rafbbs_manifest_core.h"

/*
 * Hosted persistence adapter.
 * Deterministic text rendering is owned by rafbbs_manifest_core.h.
 * FILE/filesystem behavior remains outside the authorial freestanding set.
 */
static int raf_write_manifest(RafContext *ctx) {
    char text[RAFBBS_MANIFEST_TEXT_CAP];
    RafManifestText out;
    long elapsed = raf_elapsed_ms(ctx);
    FILE *f;

    raf_manifest_text_init(&out, text, (RafU32)sizeof(text));
    raf_manifest_render(
        &out,
        ctx,
        elapsed < 0L ? 0ull : (RafU64)elapsed,
        (RafU32)(elapsed >= 0L)
    );
    if (out.dropped != 0u) return -1;

    f = fopen(ctx->manifest_path, "w");
    if (!f) return -1;
    if (fwrite(text, 1u, (size_t)out.pos, f) != (size_t)out.pos) {
        fclose(f);
        return -1;
    }
    fclose(f);

    {
        RafBinManifest bm = raf_bin_manifest_make(
            (RafU32)ctx->final_status,
            0u,
            ctx->input_crc32,
            ctx->output_crc32,
            ctx->hash_state,
            (RafU32)(ctx->gaps[0] != 0)
        );
        (void)raf_write_bin_manifest_file(ctx->bin_manifest_path, &bm);
    }
    return 0;
}
#endif
