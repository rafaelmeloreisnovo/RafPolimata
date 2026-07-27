/*
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Coupling-ID: APKC-RMR-RESEARCH-CORE-V1-20260726
 * Contract-Role: POLICY_STUB_INTERFACE
 * License-Role: RESEARCH_NONCOMMERCIAL_ONLY
 *
 * Normative comment: every unimplemented or unauthorized capability remains
 * compiled as an explicit state. A stub may not silently become success.
 */
#ifndef APKC_RMR_RESEARCH_STUBS_H
#define APKC_RMR_RESEARCH_STUBS_H

#include <stdint.h>

enum apkc_rmr_stub_state {
    APKC_RMR_STUB_FORBIDDEN = -2,
    APKC_RMR_STUB_TOKEN_VAZIO = -1,
    APKC_RMR_STUB_NOT_AUTHORIZED = 0
};

enum apkc_rmr_stub_id {
    APKC_RMR_STUB_COMMERCIAL_LICENSE = 0,
    APKC_RMR_STUB_HIGH_RISK_PRODUCTION = 1,
    APKC_RMR_STUB_APK_SIGNING = 2,
    APKC_RMR_STUB_DEVICE_INSTALL = 3,
    APKC_RMR_STUB_PACKAGE_LAUNCH = 4,
    APKC_RMR_STUB_REGULATORY_APPROVAL = 5,
    APKC_RMR_RESEARCH_STUB_COUNT = 6
};

struct apkc_rmr_stub_record {
    uint32_t id;
    int32_t state;
    const char *name;
    const char *reason;
};

const struct apkc_rmr_stub_record *
apkc_rmr_research_stub_get(uint32_t id);

#endif
