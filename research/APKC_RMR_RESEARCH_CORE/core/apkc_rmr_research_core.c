/*
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Required Notice: Copyright 2026 Rafael Melo Reis.
 * Required Notice: Commercial use requires a separate written license.
 * Coupling-ID: APKC-RMR-RESEARCH-CORE-V1-20260726
 * Contract-Role: COMPILER_MANIFEST_IMPLEMENTATION
 * License-Role: RESEARCH_NONCOMMERCIAL_ONLY
 *
 * Normative comment: code, comments, stubs, documentation, license policy,
 * generated coupling header and integrity lock are one versioned build unit.
 * The build gate must run before compilation and fail on any changed bit.
 */
#include "apkc_rmr_research_core.h"
#include "../generated/apkc_rmr_coupling_generated.h"
#include "../stubs/apkc_rmr_research_stubs.h"

#define APKC_RMR_POLICY_NONCOMMERCIAL       (1u << 0)
#define APKC_RMR_POLICY_COMMERCIAL_SEPARATE (1u << 1)
#define APKC_RMR_POLICY_HIGH_RISK_BLOCKED   (1u << 2)
#define APKC_RMR_POLICY_CLAIM_GATED         (1u << 3)

static int apkc_rmr_text_equal(const char *left, const char *right)
{
    unsigned int diff = 0u;
    unsigned int i = 0u;

    if (left == (const char *)0 || right == (const char *)0)
        return 0;

    for (;;) {
        const unsigned char a = (unsigned char)left[i];
        const unsigned char b = (unsigned char)right[i];
        diff |= (unsigned int)(a ^ b);
        if (a == 0u || b == 0u)
            break;
        ++i;
    }
    return diff == 0u && left[i] == '\0' && right[i] == '\0';
}

static const struct apkc_rmr_research_manifest g_apkc_rmr_research_manifest = {
    APKC_RMR_RESEARCH_CORE_ABI,
    APKC_RMR_RESEARCH_CORE_CLAIM_ALLOWED,
    APKC_RMR_POLICY_NONCOMMERCIAL |
        APKC_RMR_POLICY_COMMERCIAL_SEPARATE |
        APKC_RMR_POLICY_HIGH_RISK_BLOCKED |
        APKC_RMR_POLICY_CLAIM_GATED,
    APKC_RMR_RESEARCH_STUB_COUNT,
    APKC_RMR_COUPLING_ID,
    APKC_RMR_ARTIFACT_ROOT_SHA256,
    "RESEARCH_SOURCE_AVAILABLE_NONCOMMERCIAL",
    "SEPARATE_WRITTEN_LICENSE_REQUIRED"
};

const struct apkc_rmr_research_manifest *
apkc_rmr_research_manifest_get(void)
{
    return &g_apkc_rmr_research_manifest;
}

int apkc_rmr_research_manifest_is_closed(
    const struct apkc_rmr_research_manifest *manifest)
{
    if (manifest == (const struct apkc_rmr_research_manifest *)0)
        return 0;

    return manifest->abi_version == APKC_RMR_RESEARCH_CORE_ABI &&
           manifest->claim_allowed == 0u &&
           manifest->stub_count == APKC_RMR_RESEARCH_STUB_COUNT &&
           apkc_rmr_text_equal(manifest->coupling_id, APKC_RMR_COUPLING_ID) &&
           apkc_rmr_text_equal(
               manifest->artifact_root_sha256,
               APKC_RMR_ARTIFACT_ROOT_SHA256);
}
