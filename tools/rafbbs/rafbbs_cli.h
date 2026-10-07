#ifndef RAFBBS_CLI_H
#define RAFBBS_CLI_H
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rafbbs_pipeline.h"
#include "rafbbs_cli_core.h"
#include "rafbbs_filepicker.h"
#include "rafbbs_baremetal.h"
#include "rafbbs_time_posix.h"
#include "rafbbs_context_core.h"
#include "rafbbs_git_posix.h"
#include "rafbbs_result_core.h"

static void raf_print_help(void) {
    puts("RafBBS Operator Console\nuso:\n  rafbbs              abre menu BBS\n  rafbbs --help       mostra ajuda\n  rafbbs list         lista pipelines\n  rafbbs run <id>     executa pipeline\n  rafbbs logs         mostra logs recentes\n  rafbbs manifest     mostra manifestos recentes\n  rafbbs files        mostra entradas conhecidas");
}
static void raf_list_pipelines(void) {
    int i;
    for (i = 0; i < (int)raf_pipeline_count; i++)
        printf("%-16s %s - %s\n",
               raf_pipeline_specs[i].id,
               raf_pipeline_specs[i].title,
               raf_pipeline_specs[i].description);
}
static void raf_init_context(RafContext *ctx, const char *pipeline) {
    time_t t = time(NULL);
    struct tm tmv;
    RafMonoTime mono_start = raf_mono_posix_now();
    char run_id[32] = {0};
    char branch[128] = {0};
    char commit[128] = {0};
    const char *arch;
    RafContextSeed seed;
    RafU32 git_observation;

    localtime_r(&t, &tmv);
    (void)strftime(run_id, sizeof(run_id), "%Y%m%d-%H%M%S", &tmv);
#if defined(__x86_64__)
    arch = "x86_64";
#elif defined(__aarch64__)
    arch = "aarch64";
#else
    arch = "unknown";
#endif
    git_observation = raf_git_posix_observe(
        branch, (RafU32)sizeof(branch),
        commit, (RafU32)sizeof(commit)
    );
    if ((git_observation & RAFBBS_GIT_OBS_BRANCH) == 0u)
        (void)raf_context_text_copy(
            branch, (RafU32)sizeof(branch),
            "TOKEN_VAZIO" /* CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY */
        );
    if ((git_observation & RAFBBS_GIT_OBS_COMMIT) == 0u)
        (void)raf_context_text_copy(
            commit, (RafU32)sizeof(commit),
            "TOKEN_VAZIO" /* CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY */
        );
    (void)mkdir("tools/rafbbs/logs", 0777);

    seed.run_id = run_id;
    seed.pipeline = pipeline;
    seed.host = "posix";
    seed.arch = arch;
    seed.branch = branch;
    seed.commit = commit;
    seed.start = mono_start;
    (void)raf_context_init_seeded(ctx, &seed);
}
static int raf_execute_pipeline(const char *id) {
    RafContext ctx;
    RafPipelineBinding pipeline = raf_find_pipeline(id);
    if (pipeline.spec == (const RafPipelineSpec *)0 ||
        pipeline.run == (RafPipelineRun)0) {
        fprintf(stderr, "pipeline desconhecido: %s\n", id);
        return 2;
    }
    raf_init_context(&ctx, id);
    raf_log_s(&ctx, RAF_INFO, "rafbbs", "iniciando pipeline ", id);
    {
        RafResultDecision result;
        ctx.final_status = pipeline.run(&ctx);
        result = raf_result_decide(
            ctx.final_status,
            (RafU32)(ctx.failed != 0),
            ctx.input_sha256_valid,
            ctx.input_crc32_valid
        );
        ctx.final_status = result.final_status;
        ctx.hash_state = result.hash_state;
    }
    raf_log_s(&ctx, ctx.final_status == RAF_FAIL ? RAF_FAIL : RAF_DONE, "rafbbs", "pipeline finalizado status=", raf_status_name(ctx.final_status));
    if (raf_write_manifest(&ctx) != 0) fprintf(stderr, "manifesto nao gravado\n");
    if (raf_write_log(&ctx) != 0) fprintf(stderr, "log nao gravado\n");
    printf("manifest=%s\nlog=%s\n", ctx.manifest_path, ctx.log_path);
    return ctx.final_status == RAF_FAIL ? 1 : 0;
}
static int raf_cli(int argc, char **argv) {
    RafCliRoute route = raf_cli_route(
        argc > 0 ? (RafU32)argc : 0u,
        (const char *const *)argv
    );

    if (route.action == RAF_CLI_HELP) {
        raf_print_help();
        return 0;
    }
    if (route.action == RAF_CLI_LIST) {
        raf_list_pipelines();
        return 0;
    }
    if (route.action == RAF_CLI_RUN)
        return raf_execute_pipeline(route.argument);
    if (route.action == RAF_CLI_LOGS)
        return system("find tools/rafbbs/logs -maxdepth 1 -name 'run-*.txt' -type f | sort | tail -10");
    if (route.action == RAF_CLI_MANIFEST)
        return system("find tools/rafbbs/logs -maxdepth 1 -name 'manifest-*.txt' -type f | sort | tail -10");
    if (route.action == RAF_CLI_FILES) {
        RafFilePicker fp;
        raf_filepicker_init(&fp);
        raf_filepicker_print(&fp);
        return 0;
    }

    raf_print_help();
    return 2;
}
#endif
