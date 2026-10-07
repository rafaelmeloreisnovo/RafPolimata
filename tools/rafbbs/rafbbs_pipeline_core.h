#ifndef RAFBBS_PIPELINE_CORE_H
#define RAFBBS_PIPELINE_CORE_H

/*
 * RafBBS pure pipeline descriptor/lookup core.
 *
 * Boundary:
 * - zero hosted headers, libc, heap, syscall or provider API;
 * - descriptors use caller/static-owned storage;
 * - command execution, logging, filesystem and hashing stay in hosted adapters;
 * - lookup is deterministic exact-byte ID matching.
 */

#include "rafbbs_core.h"

typedef RafStatus (*RafPipelineRun)(RafContext *ctx);

typedef struct {
    const char *id;
    const char *title;
    const char *description;
    int requires_arm;
    int requires_android;
    int writes_artifacts;
    RafPipelineRun run;
} RafPipeline;

static inline int raf_pipeline_text_equal(const char *a, const char *b)
{
    if (a == (const char *)0 || b == (const char *)0)
        return 0;

    while (*a != 0 && *b != 0) {
        if (*a != *b)
            return 0;
        a++;
        b++;
    }

    return (int)(*a == *b);
}

static inline RafPipeline *raf_pipeline_find(
    RafPipeline *items,
    RafU32 count,
    const char *id
)
{
    RafU32 i;

    if (items == (RafPipeline *)0 || id == (const char *)0)
        return (RafPipeline *)0;

    for (i = 0u; i < count; i++) {
        if (raf_pipeline_text_equal(items[i].id, id))
            return &items[i];
    }

    return (RafPipeline *)0;
}

#endif
