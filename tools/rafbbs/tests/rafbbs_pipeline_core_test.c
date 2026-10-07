#include "rafbbs_pipeline_core.h"

static RafStatus rafbbs_pipeline_test_run(RafContext *ctx)
{
    if (ctx == (RafContext *)0)
        return RAF_TOKEN_VAZIO;
    ctx->final_status = RAF_PASS;
    return RAF_PASS;
}

static RafPipeline rafbbs_pipeline_test_items[] = {
    {"alpha", "Alpha", "first", 0, 0, 0, rafbbs_pipeline_test_run},
    {"arm", "ARM", "second", 1, 0, 1, rafbbs_pipeline_test_run}
};

int rafbbs_pipeline_core_test(void)
{
    RafPipeline *p;

    if (!raf_pipeline_text_equal("alpha", "alpha"))
        return 1;
    if (raf_pipeline_text_equal("alpha", "alph"))
        return 2;
    if (raf_pipeline_text_equal("alpha", "Alpha"))
        return 3;
    if (raf_pipeline_text_equal((const char *)0, "alpha"))
        return 4;

    p = raf_pipeline_find(rafbbs_pipeline_test_items, 2u, "arm");
    if (p != &rafbbs_pipeline_test_items[1])
        return 5;
    if (p->requires_arm != 1 || p->writes_artifacts != 1)
        return 6;
    if (p->run == (RafPipelineRun)0)
        return 7;

    if (raf_pipeline_find(rafbbs_pipeline_test_items, 2u, "missing") != (RafPipeline *)0)
        return 8;
    if (raf_pipeline_find((RafPipeline *)0, 2u, "alpha") != (RafPipeline *)0)
        return 9;
    if (raf_pipeline_find(rafbbs_pipeline_test_items, 2u, (const char *)0) != (RafPipeline *)0)
        return 10;

    return 0;
}

#ifdef RAFBBS_PIPELINE_TEST_MAIN
int main(void)
{
    return rafbbs_pipeline_core_test();
}
#endif
