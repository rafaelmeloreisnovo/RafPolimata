#ifndef RAFBBS_CLI_H
#define RAFBBS_CLI_H
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rafbbs_pipeline.h"
#include "rafbbs_filepicker.h"
#include "rafbbs_baremetal.h"

static void raf_print_help(void) {
    puts("RafBBS Operator Console\nuso:\n  rafbbs              abre menu BBS\n  rafbbs --help       mostra ajuda\n  rafbbs list         lista pipelines\n  rafbbs run <id>     executa pipeline\n  rafbbs logs         mostra logs recentes\n  rafbbs manifest     mostra manifestos recentes\n  rafbbs files        mostra entradas conhecidas");
}
static void raf_list_pipelines(void) {
    int i;
    for (i = 0; i < raf_pipeline_count; i++) printf("%-16s %s - %s\n", raf_pipelines[i].id, raf_pipelines[i].title, raf_pipelines[i].description);
}
static void raf_init_context(RafContext *ctx, const char *pipeline) {
    time_t t = time(NULL);
    struct tm tmv;
    memset(ctx, 0, sizeof(*ctx));
    clock_gettime(CLOCK_MONOTONIC, &ctx->start);
    localtime_r(&t, &tmv);
    strftime(ctx->run_id, sizeof(ctx->run_id), "%Y%m%d-%H%M%S", &tmv);
    mkdir("tools/rafbbs/logs", 0777);
    snprintf(ctx->log_path, sizeof(ctx->log_path), "tools/rafbbs/logs/run-%s.txt", ctx->run_id);
    snprintf(ctx->manifest_path, sizeof(ctx->manifest_path), "tools/rafbbs/logs/manifest-%s.txt", ctx->run_id);
    snprintf(ctx->bin_manifest_path, sizeof(ctx->bin_manifest_path), "tools/rafbbs/logs/manifest-%s.bin", ctx->run_id);
    snprintf(ctx->pipeline, sizeof(ctx->pipeline), "%s", pipeline);
    ctx->watchdog = raf_watchdog_start(RAFBBS_WATCHDOG_DEFAULT_TICKS);
    snprintf(ctx->host, sizeof(ctx->host), "posix");
#if defined(__x86_64__)
    snprintf(ctx->arch, sizeof(ctx->arch), "x86_64");
#elif defined(__aarch64__)
    snprintf(ctx->arch, sizeof(ctx->arch), "aarch64");
#else
    snprintf(ctx->arch, sizeof(ctx->arch), "unknown");
#endif
    (void)system("git rev-parse --abbrev-ref HEAD > /tmp/rafbbs_branch.txt 2>/dev/null");
    (void)system("git rev-parse --short HEAD > /tmp/rafbbs_commit.txt 2>/dev/null");
    { FILE *f = fopen("/tmp/rafbbs_branch.txt", "r"); if (f) { (void)fgets(ctx->branch, sizeof(ctx->branch), f); ctx->branch[strcspn(ctx->branch, "\n")] = 0; fclose(f); } }
    { FILE *f = fopen("/tmp/rafbbs_commit.txt", "r"); if (f) { (void)fgets(ctx->commit, sizeof(ctx->commit), f); ctx->commit[strcspn(ctx->commit, "\n")] = 0; fclose(f); } }
}
static int raf_execute_pipeline(const char *id) {
    RafContext ctx;
    RafPipeline *p = raf_find_pipeline(id);
    if (!p) { fprintf(stderr, "pipeline desconhecido: %s\n", id); return 2; }
    raf_init_context(&ctx, id);
    raf_log(&ctx, RAF_INFO, "rafbbs", "iniciando pipeline %s", id);
    ctx.final_status = p->run(&ctx);
    ctx.hash_state = raf_hash_failover_state((uint32_t)(ctx.input_sha256[0] != 0), (uint32_t)(ctx.input_crc32 != 0));
    if (ctx.failed) ctx.final_status = RAF_FAIL;
    raf_log(&ctx, ctx.final_status == RAF_FAIL ? RAF_FAIL : RAF_DONE, "rafbbs", "pipeline finalizado status=%s", raf_status_name(ctx.final_status));
    if (raf_write_manifest(&ctx) != 0) fprintf(stderr, "manifesto nao gravado\n");
    if (raf_write_log(&ctx) != 0) fprintf(stderr, "log nao gravado\n");
    printf("manifest=%s\nlog=%s\n", ctx.manifest_path, ctx.log_path);
    return ctx.final_status == RAF_FAIL ? 1 : 0;
}
static int raf_cli(int argc, char **argv) {
    if (argc <= 1 || strcmp(argv[1], "--help") == 0) { raf_print_help(); return 0; }
    if (strcmp(argv[1], "list") == 0) { raf_list_pipelines(); return 0; }
    if (strcmp(argv[1], "run") == 0 && argc > 2) return raf_execute_pipeline(argv[2]);
    if (strcmp(argv[1], "logs") == 0) return system("find tools/rafbbs/logs -maxdepth 1 -name 'run-*.txt' -type f | sort | tail -10");
    if (strcmp(argv[1], "manifest") == 0) return system("find tools/rafbbs/logs -maxdepth 1 -name 'manifest-*.txt' -type f | sort | tail -10");
    if (strcmp(argv[1], "files") == 0) { RafFilePicker fp; raf_filepicker_init(&fp); raf_filepicker_print(&fp); return 0; }
    raf_print_help();
    return 2;
}
#endif
