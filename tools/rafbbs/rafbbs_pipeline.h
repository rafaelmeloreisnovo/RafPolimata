#ifndef RAFBBS_PIPELINE_H
#define RAFBBS_PIPELINE_H
#include <stdio.h>
#include "rafbbs_pipeline_core.h"
#include "rafbbs_crc32.h"
#include "rafbbs_manifest.h"
#include "rafbbs_sha256.h"
#include "rafbbs_host.h"

static RafStatus raf_run_cmd(RafContext *ctx, const char *module, const char *cmd, int optional) {
    int rc;
    RafRollbackFrame frame;
    frame.step = (RafU32)ctx->syslog_count;
    frame.status = RAF_STEP;
    frame.input_crc32 = ctx->input_crc32;
    frame.output_crc32 = ctx->output_crc32;
    raf_rollback_push(&ctx->rollback, frame);
    if (raf_watchdog_step(&ctx->watchdog)) { ctx->failed = 1; raf_log(ctx, RAF_FAIL, module, "watchdog expirou antes do comando"); return RAF_FAIL; }
    snprintf(ctx->command, sizeof(ctx->command), "%s", cmd);
    raf_log(ctx, RAF_STEP, module, "comando=%s", cmd);
    #if defined(RAFBBS_FREESTANDING_MODE)
    (void)cmd;
    ctx->limited = 1;
    raf_log(ctx, RAF_TOKEN_VAZIO, module, "modo freestanding: comando externo nao executado");
    return RAF_TOKEN_VAZIO;
#else
    rc = raf_host_exec(cmd);
#endif
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
    if (raf_sha256_file(ctx->input, ctx->input_sha256) == 0) raf_log(ctx, RAF_HASH, "proof", "input_sha256=%s", ctx->input_sha256);
    return ctx->limited ? RAF_PASS_LIMITED : RAF_PASS;
}

static RafStatus raf_pipe_roundtrip(RafContext *ctx) {
    snprintf(ctx->input, sizeof(ctx->input), "%s", "Apkc/hello.s.txt");
    if (raf_run_cmd(ctx, "roundtrip", "sh tests/test_asm_roundtrip.sh", 0) == RAF_FAIL) return RAF_FAIL;
    if (raf_crc32_file(ctx->input, &ctx->input_crc32) == 0) raf_log(ctx, RAF_HASH, "proof", "input_crc32=%08x", ctx->input_crc32);
    if (raf_sha256_file(ctx->input, ctx->input_sha256) == 0) raf_log(ctx, RAF_HASH, "proof", "input_sha256=%s", ctx->input_sha256);
    ctx->limited = 1;
    snprintf(ctx->gaps, sizeof(ctx->gaps), "%s", "android_runtime=TOKEN_VAZIO;logcat=TOKEN_VAZIO");
    raf_log(ctx, RAF_TOKEN_VAZIO, "android", "runtime/logcat nao executados nesta rotina host");
    return RAF_PASS_LIMITED;
}

static RafStatus raf_pipe_apkc_validate(RafContext *ctx) {
    snprintf(ctx->input, sizeof(ctx->input), "%s", "scripts/apkc_validate.sh");
    if (raf_run_cmd(ctx, "apkc", "sh scripts/apkc_validate.sh", 0) == RAF_FAIL) return RAF_FAIL;
    if (raf_crc32_file(ctx->input, &ctx->input_crc32) == 0) raf_log(ctx, RAF_HASH, "proof", "input_crc32=%08x", ctx->input_crc32);
    if (raf_sha256_file(ctx->input, ctx->input_sha256) == 0) raf_log(ctx, RAF_HASH, "proof", "input_sha256=%s", ctx->input_sha256);
    ctx->limited = 1;
    snprintf(ctx->gaps, sizeof(ctx->gaps), "%s", "apk_generation=TOKEN_VAZIO;apk_runtime=TOKEN_VAZIO");
    raf_log(ctx, RAF_TOKEN_VAZIO, "apkc", "validacao basica passou; geracao/runtime APK exigem evidencia adicional");
    return RAF_PASS_LIMITED;
}

static RafStatus raf_pipe_proof_chain(RafContext *ctx) {
    snprintf(ctx->input, sizeof(ctx->input), "%s", "scripts/capture_android_proof_chain.sh");
    if (raf_run_cmd(ctx, "proof", "bash scripts/capture_android_proof_chain.sh", 1) == RAF_FAIL) return RAF_FAIL;
    ctx->limited = 1;
    snprintf(ctx->gaps, sizeof(ctx->gaps), "%s", "android_device_or_adb=TOKEN_VAZIO;human_audit=AUDIT");
    raf_log(ctx, RAF_AUDIT, "proof", "cadeia full-chain depende de dispositivo/prova humana");
    if (raf_crc32_file(ctx->input, &ctx->input_crc32) == 0) raf_log(ctx, RAF_HASH, "proof", "input_crc32=%08x", ctx->input_crc32);
    if (raf_sha256_file(ctx->input, ctx->input_sha256) == 0) raf_log(ctx, RAF_HASH, "proof", "input_sha256=%s", ctx->input_sha256);
    return RAF_PASS_LIMITED;
}

static RafStatus raf_pipe_placeholder(RafContext *ctx) {
    ctx->limited = 1;
    snprintf(ctx->gaps, sizeof(ctx->gaps), "%s", "pipeline=TOKEN_VAZIO");
    raf_log(ctx, RAF_TOKEN_VAZIO, "pipeline", "rotina registrada; integracao futura pendente");
    return RAF_TOKEN_VAZIO;
}

typedef RafStatus (*RafPipelineRun)(RafContext *ctx);

typedef struct {
    const RafPipelineSpec *spec;
    RafPipelineRun run;
} RafPipelineBinding;

static const RafPipelineSpec raf_pipeline_specs[] = {
    {"encoders", "Testar encoders ARM64", "Executa golden cases Python e teste C em host ARM.", 0u, 0u, 0u},
    {"roundtrip", "Assembler roundtrip", "Executa tests/test_asm_roundtrip.sh.", 0u, 0u, 1u},
    {"apkc_validate", "Validar APKC", "Executa scripts/apkc_validate.sh.", 0u, 0u, 1u},
    {"proof_chain", "Cadeia de prova", "Executa capture_android_proof_chain.sh quando ambiente permitir.", 0u, 1u, 1u},
    {"lang_matrix", "Matriz de linguagens", "Reservado para cobertura de linguagens.", 0u, 0u, 0u},
    {"verbovivo", "Verbovivo", "Reservado para Verbovivo.", 0u, 0u, 0u},
    {"export_manifest", "Exportar manifesto", "Exporta manifesto da ultima execucao.", 0u, 0u, 1u}
};

static RafPipelineRun raf_pipeline_runs[] = {
    raf_pipe_encoders,
    raf_pipe_roundtrip,
    raf_pipe_apkc_validate,
    raf_pipe_proof_chain,
    raf_pipe_placeholder,
    raf_pipe_placeholder,
    raf_pipe_placeholder
};

#define RAFBBS_PIPELINE_COUNT ((RafU32)(sizeof(raf_pipeline_specs) / sizeof(raf_pipeline_specs[0])))
#define RAFBBS_PIPELINE_RUN_COUNT ((RafU32)(sizeof(raf_pipeline_runs) / sizeof(raf_pipeline_runs[0])))
_Static_assert(RAFBBS_PIPELINE_COUNT == RAFBBS_PIPELINE_RUN_COUNT, "pipeline spec/run count mismatch");

static const RafU32 raf_pipeline_count = RAFBBS_PIPELINE_COUNT;

static RafPipelineBinding raf_find_pipeline(const char *id)
{
    RafPipelineBinding binding;
    RafU32 index = raf_pipeline_find_index(
        raf_pipeline_specs, raf_pipeline_count, id
    );

    binding.spec = (const RafPipelineSpec *)0;
    binding.run = (RafPipelineRun)0;

    if (index != RAFBBS_PIPELINE_NOT_FOUND) {
        binding.spec = &raf_pipeline_specs[index];
        binding.run = raf_pipeline_runs[index];
    }

    return binding;
}
#endif
