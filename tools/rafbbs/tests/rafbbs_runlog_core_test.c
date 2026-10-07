#include "rafbbs_runlog_core.h"

static int raf_runlog_test_equal(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

static void raf_runlog_test_copy(char *dst, RafU32 cap, const char *src)
{
    RafU32 i = 0u;
    if (cap == 0u) return;
    while (src[i] != 0 && i + 1u < cap) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

int rafbbs_runlog_core_test(void)
{
    RafContext ctx = {0};
    RafLogText out;
    char header[1024];
    char tail[1024];
    char small[8];

    raf_runlog_test_copy(ctx.run_id, (RafU32)sizeof(ctx.run_id), "run-1");
    raf_runlog_test_copy(ctx.pipeline, (RafU32)sizeof(ctx.pipeline), "probe");
    raf_runlog_test_copy(ctx.host, (RafU32)sizeof(ctx.host), "caller");
    raf_runlog_test_copy(ctx.arch, (RafU32)sizeof(ctx.arch), "armv7");
    raf_runlog_test_copy(ctx.commit, (RafU32)sizeof(ctx.commit), "abc123");
    raf_runlog_test_copy(ctx.branch, (RafU32)sizeof(ctx.branch), "main");
    raf_runlog_test_copy(ctx.manifest_path, (RafU32)sizeof(ctx.manifest_path), "m.txt");
    raf_runlog_test_copy(ctx.bin_manifest_path, (RafU32)sizeof(ctx.bin_manifest_path), "m.bin");
    raf_runlog_test_copy(ctx.input, (RafU32)sizeof(ctx.input), "in.bin");
    raf_runlog_test_copy(ctx.output, (RafU32)sizeof(ctx.output), "out.bin");
    raf_runlog_test_copy(ctx.input_sha256, (RafU32)sizeof(ctx.input_sha256), "aa");
    raf_runlog_test_copy(ctx.output_sha256, (RafU32)sizeof(ctx.output_sha256), "bb");
    ctx.final_status = RAF_PASS_LIMITED;
    ctx.input_crc32 = 0x0000000au;
    ctx.output_crc32 = 0x89abcdefu;
    ctx.hash_state = 0x10203040u;

    raf_log_text_init(&out, header, (RafU32)sizeof(header));
    raf_runlog_header_render(&out, &ctx);
    if (out.dropped != 0u) return 1;
    if (!raf_runlog_test_equal(
            header,
            "# RafBBS Run Log\n\n"
            "run_id=run-1\n"
            "pipeline=probe\n"
            "status=PASS_LIMITED\n"
            "host=caller\n"
            "arch=armv7\n"
            "commit=abc123\n"
            "branch=main\n"
            "manifest=m.txt\n"
            "bin_manifest=m.bin\n"
            "\n[SYSLOG]\n"
        )) return 2;

    raf_log_text_init(&out, tail, (RafU32)sizeof(tail));
    raf_runlog_tail_render(&out, &ctx);
    if (out.dropped != 0u) return 3;
    if (!raf_runlog_test_equal(
            tail,
            "\n[ARTIFACTS]\n"
            "input=in.bin\n"
            "output=out.bin\n"
            "input_crc32=0000000a\n"
            "output_crc32=89abcdef\n"
            "input_sha256=aa\n"
            "output_sha256=bb\n"
            "hash_state=10203040\n"
            "\n[GAPS]\n"
            "none=TOKEN_VAZIO\n"
        )) return 4;

    raf_runlog_test_copy(ctx.gaps, (RafU32)sizeof(ctx.gaps), "device=TOKEN_VAZIO");
    raf_log_text_init(&out, tail, (RafU32)sizeof(tail));
    raf_runlog_tail_render(&out, &ctx);
    if (out.dropped != 0u) return 5;
    if (raf_runlog_cstr_len(tail) != out.pos) return 6;
    if (tail[out.pos] != 0) return 7;

    raf_log_text_init(&out, small, (RafU32)sizeof(small));
    raf_runlog_header_render(&out, &ctx);
    if (out.dropped == 0u || small[7] != 0) return 8;

    return 0;
}

#if defined(RAFBBS_RUNLOG_CORE_TEST_MAIN)
int main(void) { return rafbbs_runlog_core_test(); }
#endif
