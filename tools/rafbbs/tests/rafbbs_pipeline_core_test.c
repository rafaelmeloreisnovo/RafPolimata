#include "rafbbs_pipeline_core.h"

static const RafPipelineSpec rafbbs_pipeline_test_items[] = {
    {"alpha", "Alpha", "first", 0u, 0u, 0u},
    {"arm", "ARM", "second", 1u, 0u, 1u}
};

int rafbbs_pipeline_core_test(void)
{
    RafU32 index;

    if (!raf_pipeline_text_equal("alpha", "alpha"))
        return 1;
    if (raf_pipeline_text_equal("alpha", "alph"))
        return 2;
    if (raf_pipeline_text_equal("alpha", "Alpha"))
        return 3;
    if (raf_pipeline_text_equal((const char *)0, "alpha"))
        return 4;

    index = raf_pipeline_find_index(
        rafbbs_pipeline_test_items, 2u, "arm"
    );
    if (index != 1u)
        return 5;
    if (rafbbs_pipeline_test_items[index].requires_arm != 1u ||
        rafbbs_pipeline_test_items[index].writes_artifacts != 1u)
        return 6;

    if (raf_pipeline_find_index(
            rafbbs_pipeline_test_items, 2u, "missing"
        ) != RAFBBS_PIPELINE_NOT_FOUND)
        return 7;
    if (raf_pipeline_find_index(
            (const RafPipelineSpec *)0, 2u, "alpha"
        ) != RAFBBS_PIPELINE_NOT_FOUND)
        return 8;
    if (raf_pipeline_find_index(
            rafbbs_pipeline_test_items, 2u, (const char *)0
        ) != RAFBBS_PIPELINE_NOT_FOUND)
        return 9;

    return 0;
}

#if defined(RAFBBS_PIPELINE_TEST_MAIN)
int main(void)
{
    return rafbbs_pipeline_core_test();
}
#endif
