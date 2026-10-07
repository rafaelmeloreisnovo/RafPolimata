#include "rafbbs_freestanding.h"
#include "rafbbs_baremetal.h"
#include "rafbbs_crc32_core.h"
#include "rafbbs_sha256_core.h"

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
    RafArchFlags arch;

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
    manifest = raf_bin_manifest_make(
        0u, RAF_ARCH_ARM64, crc, 0u, hash_state, 0u
    );
    arch = raf_arch_flags(RAF_ARCH_ARM64);

    watchdog_state = raf_watchdog_step(&watchdog);
    flags_state = flags.no_heap ^ flags.no_gc ^ flags.syscall_free_hint;
    arch_state = arch.no_heap ^ arch.no_syscall ^ arch.simd;

    return caller_word ^ crc ^ rollback_step ^ watchdog_state ^
           flags_state ^ arch_state ^ manifest.magic ^ manifest.hash_state ^
           output.pos ^ output.dropped ^ sink_state ^
           (RafU32)(unsigned char)digest[0] ^
           (RafU32)(unsigned char)hex[0];
}
