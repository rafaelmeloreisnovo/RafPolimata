#ifndef RAFBBS_RESULT_CORE_H
#define RAFBBS_RESULT_CORE_H

#include "rafbbs_baremetal.h"
#include "rafbbs_status.h"
#include "rafbbs_types.h"

typedef struct {
    RafStatus final_status;
    RafU32 hash_state;
} RafResultDecision;

/*
 * Evidence validity is explicit. A numeric digest value of zero is not
 * overloaded as "missing"; the caller supplies validity independently.
 */
static inline RafResultDecision raf_result_decide(
    RafStatus pipeline_status,
    RafU32 failed,
    RafU32 sha256_valid,
    RafU32 crc32_valid
)
{
    RafResultDecision out;
    out.final_status = failed != 0u ? RAF_FAIL : pipeline_status;
    out.hash_state = raf_hash_failover_state(
        sha256_valid != 0u ? 1u : 0u,
        crc32_valid != 0u ? 1u : 0u
    );
    return out;
}

#endif
