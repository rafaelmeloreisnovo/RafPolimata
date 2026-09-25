/*
 * PKC FREESTANDING MODULE TEMPLATE V1
 *
 * Copyright (c) 2024-2026 Rafael Melo Reis.
 * Provenance: REPOSITORY@COMMIT:PATH
 * Version: 0.1.0
 * License status: TOKEN_VAZIO_FINAL_LEGAL_TEXT
 * Policy: docs/legal/PKC_USE_POLICY_DRAFT_V1.md
 * Upstream: NONE | LIST_EXPLICITLY
 *
 * Contract: no libc, no libm, no heap, no syscall, no external runtime.
 */
#ifndef PKC_MODULE_TEMPLATE_H
#define PKC_MODULE_TEMPLATE_H 1

typedef unsigned char pkcm_u8;
typedef unsigned int pkcm_u32;

typedef struct {
    pkcm_u32 warnings;
    pkcm_u32 produced;
} PkcModuleStatusV1;

/* One canonical operation. Language aliases must lower here rather than
 * duplicating implementations per vocabulary. */
static inline int pkc_module_step_v1(
    const pkcm_u8 *input, pkcm_u32 input_len,
    pkcm_u8 *output, pkcm_u32 output_cap,
    PkcModuleStatusV1 *status)
{
    pkcm_u32 i;
    if (!input || !output || !status) return -1;
    status->warnings = 0u;
    status->produced = 0u;
    if (input_len > output_cap) return -2;

    for (i = 0u; i < input_len; ++i) {
        output[i] = input[i];
    }
    status->produced = input_len;
    return 0;
}

#endif
