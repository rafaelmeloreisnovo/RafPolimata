/*
 * RAFCODE-IP-NOTICE
 * Copyright (c) 2026 Rafael Melo Reis.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Berne Convention orientation: this notice records provenance; it does not
 * create, enlarge, or replace rights supplied by applicable law.
 * Module scope: LICENSE_SCOPE_V1.json. Third-party rights remain separate.
 */

#include "../include/zipraf_hw_fabric.h"\n#include "../include/zipraf_crypto_adapter.h"

static int test_permissions(void) {
    zhf_u64 allow = ZHF_PERM_READ | ZHF_PERM_WRITE | ZHF_PERM_EXEC | ZHF_PERM_SIGN;
    zhf_u64 deny = ZHF_PERM_SIGN;
    zhf_u64 e = zhf_effective_permissions(allow, deny);
    return ((e & ZHF_PERM_SIGN) == 0u) && ((e & ZHF_PERM_EXEC) != 0u);
}

static int test_parallel_plan(void) {
    struct zhf_profile p = {
        ZHF_ABI_VERSION, ZHF_ARCH_AARCH64, 128, 64, 4,
        ZHF_CAP_SCALAR | ZHF_CAP_ASM | ZHF_CAP_SIMD128,
        ZHF_PERM_READ | ZHF_PERM_WRITE | ZHF_PERM_EXEC |
        ZHF_PERM_VERIFY | ZHF_PERM_BENCH, 0
    };
    struct zhf_request r = {ZHF_PRIM_BLAKE3, ZHF_OP_HASH, 4096, 0};
    struct zhf_plan plan;
    if (zhf_plan_request(&p, &r, &plan) != ZHF_OK) return 0;
    return plan.lanes == 4 && plan.primitive == ZHF_PRIM_BLAKE3 &&
           plan.normalized_cost_q16 > 0u;
}

static int test_deny_overrides_allow(void) {
    struct zhf_profile p = {
        ZHF_ABI_VERSION, ZHF_ARCH_SCALAR, 0, 32, 1,
        ZHF_CAP_SCALAR,
        ZHF_PERM_READ | ZHF_PERM_EXEC | ZHF_PERM_SIGN | ZHF_PERM_PRIVATE,
        ZHF_PERM_SIGN
    };
    struct zhf_request r = {ZHF_PRIM_ED25519, ZHF_OP_SIGN, 64, 0};
    struct zhf_plan plan;
    return zhf_plan_request(&p, &r, &plan) == ZHF_DENIED;
}

static int test_md5_not_promoted(void) {
    const struct zhf_descriptor *d = zhf_descriptor_for(ZHF_PRIM_MD5_LEGACY);
    return d && d->security_class == ZHF_CLASS_LEGACY_INSECURE;
}

int main(void) {
    unsigned mask = 0;
    if (test_permissions()) mask |= 1u << 0;
    if (test_parallel_plan()) mask |= 1u << 1;
    if (test_deny_overrides_allow()) mask |= 1u << 2;
    if (test_md5_not_promoted()) mask |= 1u << 3;
    return mask == 0x0fu ? 0 : (int)(0x80u | mask);
}
