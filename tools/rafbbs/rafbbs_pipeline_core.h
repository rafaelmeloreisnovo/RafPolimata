#ifndef RAFBBS_PIPELINE_CORE_H
#define RAFBBS_PIPELINE_CORE_H

#include "rafbbs_types.h"

#define RAFBBS_PIPELINE_NOT_FOUND 0xffffffffu

typedef struct {
    const char *id;
    const char *title;
    const char *description;
    RafU32 requires_arm;
    RafU32 requires_android;
    RafU32 writes_artifacts;
} RafPipelineSpec;

static inline RafU32
raf_pipeline_text_equal(const char *a, const char *b)
{
    if (a == (const char *)0 || b == (const char *)0)
        return 0u;

    while (*a != 0 && *b != 0) {
        if (*a != *b)
            return 0u;
        a++;
        b++;
    }

    return (RafU32)(*a == *b);
}

static inline RafU32
raf_pipeline_find_index(
    const RafPipelineSpec *items,
    RafU32 count,
    const char *id
)
{
    RafU32 i;

    if (items == (const RafPipelineSpec *)0 || id == (const char *)0)
        return RAFBBS_PIPELINE_NOT_FOUND;

    for (i = 0u; i < count; i++) {
        if (raf_pipeline_text_equal(items[i].id, id))
            return i;
    }

    return RAFBBS_PIPELINE_NOT_FOUND;
}

#endif
