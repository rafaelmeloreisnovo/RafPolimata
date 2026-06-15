#ifndef RAFBBS_PIPELINE_H
#define RAFBBS_PIPELINE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rafbbs_crc32.h"
#include "rafbbs_manifest.h"

typedef struct {
    const char *id;
    const char *title;
    const char *description;
    int requires_arm;
    int requires_android;
    int writes_artifacts;
    RafStatus (*run)(RafContext *ctx);
} RafPipeline;

static RafStatus raf_run_cmd(RafContext *ctx, const char *module, const char *cmd, int optional) {
    int rc;
    snprintf(ctx->command, sizeof(ctx->command), "%s", cmd);
    raf_log(ctx, RAF_STEP, module, "comando=%s", cmd);
    rc = system(cmd);
    if (rc == 0) { raf_log(ctx, RAF_PASS, module, "comando finalizado rc=0"); return RAF_PASS; }
    if (optional) { ctx->limited = 1; raf_log(ctx, RAF_SKIP, module, "comando opcional indisponivel rc=%d", rc); return RAF_SKIP; }
    ctx->failed = 1; raf_log(ctx, RAF_FAIL, module, "comando falhou rc=%d", rc); return RAF_FAIL;
}

static RafStatus raf_pipe_encoders(RafContext *ctx) {
    RafStatus s;
    snprintf(ctx->input, sizeof(ctx->input), "%s", "tests/test_arm64_encoders.py");
    s = raf_run_cmd(ctx, "encoder", "python3 tests/test_arm64_encoders.py", 0);
    if (s == RAF_FAIL) return RAF_FAIL;
    if (raf_is_arm_host()) {
        s = raf_run_cmd(ctx, "encoder_c", "cc -std=c11 -Wall -Wextra -Werror -I Apkc tests/test_arm64_encoders.c -o /tmp/test_arm64_encoders && /tmp/test_arm64_encoders", 0);
        if (s == RAF_FAIL) return RAF_FAIL;
    } else {
        ctx->limited = 1;
        snprintf(ctx->gaps, sizeof(ctx->gaps), "%s", "c_arm_host=SKIP;android_logcat=TOKEN_VAZIO");
        raf_log(ctx, RAF_SKIP, "encoder_c", "teste C ARM exige host ARM");
        raf_log(ctx, RAF_TOKEN_VAZIO, "android", "logcat ausente neste host");
    }
    if (raf_crc32_file(ctx->input, &ctx->input_crc32) == 0) raf_log(ctx, RAF_HASH, "proof", "input_crc32=%08x", ctx->input_crc32);
    return ctx->limited ? RAF_PASS_LIMITED : RAF_PASS;
}

static RafStatus raf_pipe_roundtrip(RafContext *ctx) {
    snprintf(ctx->input, sizeof(ctx->input), "%s", "Apkc/hello.s.txt");
    if (raf_run_cmd(ctx, "roundtrip", "sh tests/test_asm_roundtrip.sh", 0) == RAF_FAIL) return RAF_FAIL;
    if (raf_crc32_file(ctx->input, &ctx->input_crc32) == 0) raf_log(ctx, RAF_HASH, "proof", "input_crc32=%08x", ctx->input_crc32);
    ctx->limited = 1;
    snprintf(ctx->gaps, sizeof(ctx->gaps), "%s", "android_runtime=TOKEN_VAZIO;logcat=TOKEN_VAZIO");
    raf_log(ctx, RAF_TOKEN_VAZIO, "android", "runtime/logcat nao executados nesta rotina host");
    return RAF_PASS_LIMITED;
}

static RafStatus raf_pipe_apkc_validate(RafContext *ctx) {
    snprintf(ctx->input, sizeof(ctx->input), "%s", "scripts/apkc_validate.sh");
    if (raf_run_cmd(ctx, "apkc", "sh scripts/apkc_validate.sh", 0) == RAF_FAIL) return RAF_FAIL;
    if (raf_crc32_file(ctx->input, &ctx->input_crc32) == 0) raf_log(ctx, RAF_HASH, "proof", "input_crc32=%08x", ctx->input_crc32);
    ctx->limited = 1;
    snprintf(ctx->gaps, sizeof(ctx->gaps), "%s", "apk_generation=TOKEN_VAZIO;apk_runtime=TOKEN_VAZIO");
    raf_log(ctx, RAF_TOKEN_VAZIO, "apkc", "validacao basica passou; geracao/runtime APK exigem evidencia adicional");
    return RAF_PASS_LIMITED;
}

static RafStatus raf_pipe_placeholder(RafContext *ctx) {
    ctx->limited = 1;
    snprintf(ctx->gaps, sizeof(ctx->gaps), "%s", "pipeline=TOKEN_VAZIO");
    raf_log(ctx, RAF_TOKEN_VAZIO, "pipeline", "rotina registrada; integracao futura pendente");
    return RAF_TOKEN_VAZIO;
}

static RafPipeline raf_pipelines[] = {
    {"encoders", "Testar encoders ARM64", "Executa golden cases Python e teste C em host ARM.", 0, 0, 0, raf_pipe_encoders},
    {"roundtrip", "Assembler roundtrip", "Executa tests/test_asm_roundtrip.sh.", 0, 0, 1, raf_pipe_roundtrip},
    {"apkc_validate", "Validar APKC", "Executa scripts/apkc_validate.sh.", 0, 0, 1, raf_pipe_apkc_validate},
    {"proof_chain", "Cadeia de prova", "Reservado para capture_android_proof_chain.sh.", 0, 1, 1, raf_pipe_placeholder},
    {"lang_matrix", "Matriz de linguagens", "Reservado para cobertura de linguagens.", 0, 0, 0, raf_pipe_placeholder},
    {"verbovivo", "Verbovivo", "Reservado para Verbovivo.", 0, 0, 0, raf_pipe_placeholder},
    {"export_manifest", "Exportar manifesto", "Exporta manifesto da ultima execucao.", 0, 0, 1, raf_pipe_placeholder}
};
static const int raf_pipeline_count = (int)(sizeof(raf_pipelines) / sizeof(raf_pipelines[0]));

static RafPipeline *raf_find_pipeline(const char *id) {
    int i;
    for (i = 0; i < raf_pipeline_count; i++) if (strcmp(raf_pipelines[i].id, id) == 0) return &raf_pipelines[i];
    return NULL;
}
#endif
