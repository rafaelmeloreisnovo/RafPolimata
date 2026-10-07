/* Governance: CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY.
 * RAF_TOKEN_VAZIO below is status vocabulary under test, not a promoted claim.
 */
#include "rafbbs_core.h"
#include "rafbbs_context_core.h"
#include "rafbbs_baremetal.h"
#include "rafbbs_crc32_core.h"
#include "rafbbs_sha256_core.h"
#include "rafbbs_manifest_core.h"
#include "rafbbs_manifest_bin_core.h"
#include "rafbbs_filepicker_core.h"
#include "rafbbs_theme.h"
#include "rafbbs_log_core.h"
#include "rafbbs_runlog_core.h"
#include "rafbbs_pipeline_core.h"
#include "rafbbs_command_core.h"
#include "rafbbs_cli_core.h"
#include "rafbbs_tui_core.h"

static RafContext rafbbs_authorial_manifest_ctx;
static char rafbbs_authorial_manifest_buf[128];
static char rafbbs_authorial_log_buf[64];
static char rafbbs_authorial_runlog_buf[1024];
static const char *const rafbbs_authorial_cli_argv[] = {
    "rafbbs", "run", "arm"
};
static const RafPipelineSpec rafbbs_authorial_pipeline_specs[] = {
    {"probe", "Probe", "authorial", 0u, 0u, 0u},
    {"arm", "ARM", "authorial", 1u, 0u, 1u}
};

static void rafbbs_authorial_probe_sink(RafU8 byte, void *user)
{
    RafU32 *state = (RafU32 *)user;
    *state ^= (RafU32)byte;
}

RafU32 rafbbs_authorial_probe(void *state)
{
    static const RafU8 input[] = {'a', 'b', 'c'};
    static const char text[] = {'R', 'A', 'F', 0};
    RafU32 caller_word = 0u;
    RafU32 sink_state = 0u;
    RafU32 crc;
    RafU32 rollback_step;
    RafU32 watchdog_state;
    RafU32 flags_state;
    RafU32 arch_state;
    RafU32 hash_state;
    RafU32 time_state;
    RafU32 status_state;
    RafU32 arm_state;
    RafSha256 sha;
    RafU8 digest[32];
    char hex[65];
    RafWatchdog watchdog = raf_watchdog_start(3u);
    RafRollbackRing rollback = {{{0u, 0u, 0u, 0u}}, 0u};
    RafRollbackFrame frame;
    RafFreestandingFlags flags = raf_freestanding_flags();
    RafBaremetalOut output;
    RafBaremetalPort port;
    RafBinManifest manifest;
    RafBinManifest manifest_decoded;
    RafU8 manifest_wire[RAFBBS_BIN_MANIFEST_V1_SIZE];
    RafArchFlags arch;
    RafMonoTime mono_start = raf_mono_from_ns(1000000000ull);
    RafMonoTime mono_end = raf_mono_from_ns(2500000000ull);
    RafMonoElapsed mono_elapsed = raf_mono_elapsed_ms(mono_start, mono_end);
    RafManifestText manifest_text;
    RafFilePickerCore picker = raf_filepicker_core_init();
    RafU32 picker_state;
    RafU32 theme_state;
    RafU32 log_state;
    RafU32 runlog_state;
    RafU32 pipeline_state;
    RafU32 cli_state;
    RafU32 tui_state;
    RafU32 context_state;
    RafU32 command_state;

    if (state != (void *)0)
        caller_word = *(const RafU32 *)state;

    crc = raf_crc32_update(0u, input, 3u);
    frame.step = 1u;
    frame.status = 2u;
    frame.input_crc32 = crc;
    frame.output_crc32 = 0u;
    raf_rollback_push(&rollback, frame);
    rollback_step = raf_rollback_last(&rollback).step;

    raf_sha256_init(&sha);
    raf_sha256_update(&sha, input, 3u);
    raf_sha256_final(&sha, digest);
    raf_sha256_hex(digest, hex);

    raf_baremetal_out_init(&output);
    raf_baremetal_write(&output, text);
    port.sink = rafbbs_authorial_probe_sink;
    port.user = &sink_state;
    raf_baremetal_flush(&output, port);

    hash_state = raf_hash_failover_state(1u, (RafU32)(crc != 0u));
    raf_bin_manifest_init(
        &manifest,
        0u, RAF_ARCH_ARM64, crc, 0u, hash_state, 0u
    );
    if (raf_bin_manifest_encode_v1(
            manifest_wire,
            RAFBBS_BIN_MANIFEST_V1_SIZE,
            &manifest
        ) != 0)
        return caller_word ^ 0x42494e31u;
    if (raf_bin_manifest_decode_v1(
            &manifest_decoded,
            manifest_wire,
            RAFBBS_BIN_MANIFEST_V1_SIZE
        ) != 0)
        return caller_word ^ 0x42494e32u;
    arch = raf_arch_flags(RAF_ARCH_ARM64);
    {
        RafContextSeed context_seed;
        context_seed.run_id = "probe";
        context_seed.pipeline = "arm";
        context_seed.host = "authorial";
        context_seed.arch = "arm64";
        context_seed.branch = "";
        context_seed.commit = "";
        context_seed.start = mono_start;
        context_state = raf_context_init_seeded(
            &rafbbs_authorial_manifest_ctx, &context_seed
        );
        context_state ^= (RafU32)(unsigned char)
            rafbbs_authorial_manifest_ctx.log_path[0];
    }
    raf_manifest_text_init(
        &manifest_text,
        rafbbs_authorial_manifest_buf,
        (RafU32)sizeof(rafbbs_authorial_manifest_buf)
    );
    raf_manifest_render(
        &manifest_text,
        &rafbbs_authorial_manifest_ctx,
        mono_elapsed.ms,
        mono_elapsed.valid
    );

    watchdog_state = raf_watchdog_step(&watchdog);
    flags_state = flags.no_heap ^ flags.no_gc ^ flags.syscall_free_hint;
    arch_state = arch.no_heap ^ arch.no_syscall ^ arch.simd;
    time_state = (RafU32)mono_elapsed.ms ^ mono_elapsed.valid ^
                 (RafU32)sizeof(RafContext);
    status_state = (RafU32)(unsigned char)raf_status_name(RAF_PASS)[0];
    arm_state = (RafU32)raf_is_arm_host();
    (void)raf_filepicker_core_select(&picker, 5u);
    picker_state = picker.selected_index ^
                   raf_filepicker_core_is_selected(&picker, 4u) ^
                   (RafU32)(unsigned char)raf_filepicker_core_selected(&picker)[0];
    theme_state =
        (RafU32)(unsigned char)raf_status_color(RAF_PASS)[0] ^
        (RafU32)(unsigned char)raf_status_color(RAF_TOKEN_VAZIO)[3] ^
        (RafU32)(unsigned char)raf_status_name(RAF_PASS)[0];
    {
        RafLogText log_text;
        raf_log_text_init(
            &log_text,
            rafbbs_authorial_log_buf,
            (RafU32)sizeof(rafbbs_authorial_log_buf)
        );
        raf_log_line_render(
            &log_text, RAF_INFO, "probe", "ok",
            mono_elapsed.ms, mono_elapsed.valid
        );
        log_state = log_text.pos ^ log_text.dropped ^
                    (RafU32)(unsigned char)rafbbs_authorial_log_buf[0];
    }
    {
        RafLogText runlog_text;
        raf_log_text_init(
            &runlog_text,
            rafbbs_authorial_runlog_buf,
            (RafU32)sizeof(rafbbs_authorial_runlog_buf)
        );
        raf_runlog_header_render(
            &runlog_text, &rafbbs_authorial_manifest_ctx
        );
        raf_runlog_tail_render(
            &runlog_text, &rafbbs_authorial_manifest_ctx
        );
        if (runlog_text.dropped != 0u)
            return caller_word ^ 0x52554e31u;
        runlog_state = runlog_text.pos ^
                       raf_runlog_cstr_len(rafbbs_authorial_runlog_buf) ^
                       (RafU32)(unsigned char)rafbbs_authorial_runlog_buf[0];
    }
    pipeline_state = raf_pipeline_find_index(
        rafbbs_authorial_pipeline_specs, 2u, "arm"
    );
    if (pipeline_state == RAFBBS_PIPELINE_NOT_FOUND)
        return caller_word ^ 0x50495045u;
    pipeline_state ^= rafbbs_authorial_pipeline_specs[pipeline_state].requires_arm ^
                      rafbbs_authorial_pipeline_specs[pipeline_state].writes_artifacts;
    {
        RafCommandDecision command_decision = raf_command_decide(0u, 0, 0u);
        if (command_decision.status != RAF_TOKEN_VAZIO ||
            command_decision.limited == 0u || command_decision.failed != 0u)
            return caller_word ^ 0x434d4431u;
        command_state = (RafU32)command_decision.status ^
                        command_decision.limited ^ command_decision.failed;
    }
    {
        RafCliRoute cli_route = raf_cli_route(
            3u, rafbbs_authorial_cli_argv
        );
        if (cli_route.action != RAF_CLI_RUN ||
            cli_route.argument == (const char *)0)
            return caller_word ^ 0x434c4931u;
        cli_state = (RafU32)cli_route.action ^
                    (RafU32)(unsigned char)cli_route.argument[0];
    }
    {
        RafTuiRoute tui_route = raf_tui_route_char('2');
        if (tui_route.action != RAF_TUI_RUN ||
            tui_route.pipeline_id == (const char *)0)
            return caller_word ^ 0x54554931u;
        tui_state = (RafU32)tui_route.action ^
                    (RafU32)(unsigned char)tui_route.pipeline_id[0];
    }

    return caller_word ^ crc ^ rollback_step ^ watchdog_state ^
           flags_state ^ arch_state ^ time_state ^ status_state ^ arm_state ^
           manifest.magic ^ manifest.hash_state ^
           manifest_decoded.magic ^ manifest_decoded.hash_state ^
           (RafU32)manifest_wire[0] ^ (RafU32)manifest_wire[95] ^
           output.pos ^ output.dropped ^ sink_state ^
           manifest_text.pos ^ manifest_text.dropped ^ picker_state ^ theme_state ^
           log_state ^ runlog_state ^ pipeline_state ^ cli_state ^ tui_state ^ context_state ^
           command_state ^
           (RafU32)(unsigned char)digest[0] ^
           (RafU32)(unsigned char)hex[0];
}
