/*
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Required Notice: Copyright 2026 Rafael Melo Reis.
 * Required Notice: Commercial use requires a separate written license.
 * Coupling-ID: APKC-RMR-RESEARCH-CORE-V1-20260726
 *
 * This file, its comments, the coupled specification, the license notices,
 * and the integrity lock form one sealed research unit.
 */
#include <stddef.h>
#include <stdint.h>

#define APKC_RMR_RESEARCH_CORE_ABI 1u
#define APKC_RMR_RESEARCH_CORE_CLAIM_ALLOWED 0u

struct apkc_rmr_research_manifest {
    uint32_t abi_version;
    uint32_t claim_allowed;
    const char *coupling_id;
    const char *license_class;
    const char *commercial_status;
};

static const struct apkc_rmr_research_manifest g_apkc_rmr_research_manifest = {
    APKC_RMR_RESEARCH_CORE_ABI,
    APKC_RMR_RESEARCH_CORE_CLAIM_ALLOWED,
    "APKC-RMR-RESEARCH-CORE-V1-20260726",
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
    if (manifest == NULL)
        return 0;
    return manifest->abi_version == APKC_RMR_RESEARCH_CORE_ABI &&
           manifest->claim_allowed == 0u &&
           manifest->coupling_id == g_apkc_rmr_research_manifest.coupling_id;
}
