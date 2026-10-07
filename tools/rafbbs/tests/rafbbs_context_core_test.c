#include "rafbbs_context_core.h"

static int rafbbs_context_text_equal(const char *a, const char *b)
{
    RafU32 i = 0u;
    while (a[i] != 0 && b[i] != 0) {
        if (a[i] != b[i])
            return 0;
        ++i;
    }
    return a[i] == b[i];
}

int rafbbs_context_core_test(void)
{
    RafContext ctx;
    RafContextSeed seed;
    RafMonoTime start = raf_mono_from_ns(123456789ull);
    RafU32 dropped;

    seed.run_id = "20261007-042500";
    seed.pipeline = "encoders";
    seed.host = "posix";
    seed.arch = "armv7";
    seed.branch = "main";
    seed.commit = "318abf0";
    seed.start = start;

    dropped = raf_context_init_seeded(&ctx, &seed);
    if (dropped != 0u)
        return 1;
    if (!rafbbs_context_text_equal(ctx.run_id, "20261007-042500"))
        return 2;
    if (!rafbbs_context_text_equal(
            ctx.log_path,
            "tools/rafbbs/logs/run-20261007-042500.txt"
        ))
        return 3;
    if (!rafbbs_context_text_equal(
            ctx.manifest_path,
            "tools/rafbbs/logs/manifest-20261007-042500.txt"
        ))
        return 4;
    if (!rafbbs_context_text_equal(
            ctx.bin_manifest_path,
            "tools/rafbbs/logs/manifest-20261007-042500.bin"
        ))
        return 5;
    if (!rafbbs_context_text_equal(ctx.pipeline, "encoders") ||
        !rafbbs_context_text_equal(ctx.host, "posix") ||
        !rafbbs_context_text_equal(ctx.arch, "armv7") ||
        !rafbbs_context_text_equal(ctx.branch, "main") ||
        !rafbbs_context_text_equal(ctx.commit, "318abf0"))
        return 6;
    if (ctx.start.valid != 1u || ctx.start.ns != 123456789ull)
        return 7;
    if (ctx.watchdog.budget != RAFBBS_WATCHDOG_DEFAULT_TICKS ||
        ctx.watchdog.tick != 0u || ctx.watchdog.tripped != 0u)
        return 8;
    if (raf_status_name(ctx.final_status)[0] != 'P')
        return 9;
    if (ctx.failed != 0 || ctx.limited != 0 || ctx.syslog_count != 0)
        return 10;

    seed.run_id = "123456789012345678901234567890123456";
    dropped = raf_context_init_seeded(&ctx, &seed);
    if (dropped == 0u || ctx.run_id[31] != 0)
        return 11;

    if (raf_context_init_seeded(&ctx, (const RafContextSeed *)0) != 0u)
        return 12;
    if (ctx.start.valid != 0u || ctx.watchdog.tick != 0u)
        return 13;

    return 0;
}

#ifdef RAFBBS_CONTEXT_CORE_TEST_MAIN
int main(void)
{
    return rafbbs_context_core_test();
}
#endif
