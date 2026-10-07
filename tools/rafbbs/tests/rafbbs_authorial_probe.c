/* Governance: CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY.
 * RAF_TOKEN_VAZIO below is status vocabulary under test, not a promoted claim.
 */
#include "rafbbs_core.h"
#include "rafbbs_baremetal.h"
#include "rafbbs_crc32_core.h"
#include "rafbbs_sha256_core.h"
#include "rafbbs_manifest_core.h"
#include "rafbbs_manifest_bin_core.h"
#include "rafbbs_filepicker_core.h"
#include "rafbbs_theme.h"

static RafContext rafbbs_authorial_manifest_ctx;
static char rafbbs_authorial_manifest_buf[128];

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

    return caller_word ^ crc ^ rollback_step ^ watchdog_state ^
           flags_state ^ arch_state ^ time_state ^ status_state ^ arm_state ^
           manifest.magic ^ manifest.hash_state ^
           manifest_decoded.magic ^ manifest_decoded.hash_state ^
           (RafU32)manifest_wire[0] ^ (RafU32)manifest_wire[95] ^
           output.pos ^ output.dropped ^ sink_state ^
           manifest_text.pos ^ manifest_text.dropped ^ picker_state ^ theme_state ^
           (RafU32)(unsigned char)digest[0] ^
           (RafU32)(unsigned char)hex[0];
}
