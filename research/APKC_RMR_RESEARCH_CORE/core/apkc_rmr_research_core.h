/*
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Coupling-ID: APKC-RMR-RESEARCH-CORE-V1-20260726
 * Contract-Role: COMPILER_PUBLIC_INTERFACE
 * License-Role: RESEARCH_NONCOMMERCIAL_ONLY
 *
 * Normative comment: this header is part of the sealed compilation unit.
 * Removing or changing this notice requires a new coupling generation.
 */
#ifndef APKC_RMR_RESEARCH_CORE_H
#define APKC_RMR_RESEARCH_CORE_H

#include <stdint.h>

#define APKC_RMR_RESEARCH_CORE_ABI 2u
#define APKC_RMR_RESEARCH_CORE_CLAIM_ALLOWED 0u

struct apkc_rmr_research_manifest {
    uint32_t abi_version;
    uint32_t claim_allowed;
    uint32_t policy_bits;
    uint32_t stub_count;
    const char *coupling_id;
    const char *artifact_root_sha256;
    const char *license_class;
    const char *commercial_status;
};

const struct apkc_rmr_research_manifest *
apkc_rmr_research_manifest_get(void);

int apkc_rmr_research_manifest_is_closed(
    const struct apkc_rmr_research_manifest *manifest);

#endif
