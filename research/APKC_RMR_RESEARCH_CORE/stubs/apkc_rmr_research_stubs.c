/*
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Coupling-ID: APKC-RMR-RESEARCH-CORE-V1-20260726
 * Contract-Role: POLICY_STUB_IMPLEMENTATION
 * License-Role: RESEARCH_NONCOMMERCIAL_ONLY
 *
 * Normative comment: these records are executable claim boundaries. They are
 * compiled into the module so documentation cannot promise what code denies.
 */
#include "apkc_rmr_research_stubs.h"

static const struct apkc_rmr_stub_record g_apkc_rmr_stubs[
    APKC_RMR_RESEARCH_STUB_COUNT] = {
    {APKC_RMR_STUB_COMMERCIAL_LICENSE, APKC_RMR_STUB_NOT_AUTHORIZED,
     "commercial_license", "separate_executed_agreement_required"},
    {APKC_RMR_STUB_HIGH_RISK_PRODUCTION, APKC_RMR_STUB_FORBIDDEN,
     "high_risk_production", "research_grant_has_no_production_authority"},
    {APKC_RMR_STUB_APK_SIGNING, APKC_RMR_STUB_TOKEN_VAZIO,
     "apk_signing", "signature_not_observed"},
    {APKC_RMR_STUB_DEVICE_INSTALL, APKC_RMR_STUB_TOKEN_VAZIO,
     "device_install", "installation_not_observed"},
    {APKC_RMR_STUB_PACKAGE_LAUNCH, APKC_RMR_STUB_TOKEN_VAZIO,
     "package_launch", "runtime_not_observed"},
    {APKC_RMR_STUB_REGULATORY_APPROVAL, APKC_RMR_STUB_FORBIDDEN,
     "regulatory_approval", "no_certification_or_approval_granted"}
};

const struct apkc_rmr_stub_record *
apkc_rmr_research_stub_get(uint32_t id)
{
    if (id >= APKC_RMR_RESEARCH_STUB_COUNT)
        return (const struct apkc_rmr_stub_record *)0;
    return &g_apkc_rmr_stubs[id];
}
