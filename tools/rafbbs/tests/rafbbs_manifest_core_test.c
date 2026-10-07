#include "rafbbs_manifest_core.h"

static RafContext rafbbs_manifest_test_ctx;
static char rafbbs_manifest_test_out[RAFBBS_MANIFEST_TEXT_CAP];
static char rafbbs_manifest_test_tiny[8];

static void rafbbs_test_copy(char *dst, RafU32 cap, const char *src)
{
    RafU32 i = 0u;
    if (cap == 0u)
        return;
    while (src[i] != 0 && (i + 1u) < cap) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

static int rafbbs_test_equal(const char *a, const char *b)
{
    RafU32 i = 0u;
    while (a[i] != 0 && b[i] != 0) {
        if (a[i] != b[i])
            return 0;
        i++;
    }
    return (int)(a[i] == b[i]);
}

static int rafbbs_test_contains(const char *text, const char *needle)
{
    RafU32 i = 0u;
    while (text[i] != 0) {
        RafU32 j = 0u;
        while (needle[j] != 0 && text[i + j] == needle[j])
            j++;
        if (needle[j] == 0)
            return 1;
        i++;
    }
    return 0;
}

int rafbbs_manifest_core_test(void)
{
    static const char expected[] =
        "[RAFBBS_MANIFEST]\n"
        "pipeline=encoders\n"
        "status=PASS\n"
        "input=in\n"
        "output=out\n"
        "arch=aarch64\n"
        "host=none\n"
        "commit=abc\n"
        "branch=main\n"
        "command=noop\n"
        "elapsed_ms=42\n"
        "input_crc32=1234abcd\n"
        "output_crc32=00000000\n"
        "hash_state=00000080\n"
        "input_sha256=aa\n"
        "output_sha256=bb\n"
        "log=log\n"
        "bin_manifest=bin\n"
        "gaps=none\n";
    RafManifestText out;
    RafManifestText tiny;

    rafbbs_test_copy(rafbbs_manifest_test_ctx.pipeline, 64u, "encoders");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.input, 256u, "in");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.output, 256u, "out");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.arch, 64u, "aarch64");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.host, 128u, "none");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.commit, 128u, "abc");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.branch, 128u, "main");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.command, 512u, "noop");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.input_sha256, 65u, "aa");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.output_sha256, 65u, "bb");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.log_path, 256u, "log");
    rafbbs_test_copy(rafbbs_manifest_test_ctx.bin_manifest_path, 256u, "bin");
    rafbbs_manifest_test_ctx.gaps[0] = 0;
    rafbbs_manifest_test_ctx.final_status = RAF_PASS;
    rafbbs_manifest_test_ctx.input_crc32 = 0x1234abcdu;
    rafbbs_manifest_test_ctx.output_crc32 = 0u;
    rafbbs_manifest_test_ctx.hash_state = 0x00000080u;

    raf_manifest_text_init(
        &out,
        rafbbs_manifest_test_out,
        (RafU32)sizeof(rafbbs_manifest_test_out)
    );
    raf_manifest_render(&out, &rafbbs_manifest_test_ctx, 42ull, 1u);

    if (out.dropped != 0u)
        return 1;
    if (!rafbbs_test_equal(rafbbs_manifest_test_out, expected))
        return 2;

    raf_manifest_text_init(
        &tiny,
        rafbbs_manifest_test_tiny,
        (RafU32)sizeof(rafbbs_manifest_test_tiny)
    );
    raf_manifest_render(&tiny, &rafbbs_manifest_test_ctx, 42ull, 1u);
    if (tiny.dropped == 0u)
        return 3;
    if (rafbbs_manifest_test_tiny[7] != 0)
        return 4;

    raf_manifest_text_init(
        &out,
        rafbbs_manifest_test_out,
        (RafU32)sizeof(rafbbs_manifest_test_out)
    );
    raf_manifest_render(&out, &rafbbs_manifest_test_ctx, 0ull, 0u);
    if (!rafbbs_test_contains(rafbbs_manifest_test_out, "elapsed_ms=-1\n"))
        return 5;

    return 0;
}

#ifdef RAFBBS_MANIFEST_TEST_MAIN
int main(void)
{
    return rafbbs_manifest_core_test();
}
#endif
