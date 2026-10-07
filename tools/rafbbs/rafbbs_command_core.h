#ifndef RAFBBS_COMMAND_CORE_H
#define RAFBBS_COMMAND_CORE_H

#include "rafbbs_status.h"
#include "rafbbs_types.h"

typedef struct {
    RafStatus status;
    RafU32 limited;
    RafU32 failed;
} RafCommandDecision;

static inline RafCommandDecision raf_command_decide(
    RafU32 executed, int rc, RafU32 optional
)
{
    RafCommandDecision out;
    out.status = RAF_PASS;
    out.limited = 0u;
    out.failed = 0u;

    if (executed == 0u) {
        out.status = RAF_TOKEN_VAZIO;
        out.limited = 1u;
        return out;
    }
    if (rc == 0)
        return out;
    if (optional != 0u) {
        out.status = RAF_SKIP;
        out.limited = 1u;
        return out;
    }
    out.status = RAF_FAIL;
    out.failed = 1u;
    return out;
}

#endif
