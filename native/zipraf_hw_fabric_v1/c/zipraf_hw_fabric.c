/*
 * RAFCODE-IP-NOTICE
 * Copyright (c) 2026 Rafael Melo Reis.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Berne Convention orientation: this notice records provenance; it does not
 * create, enlarge, or replace rights supplied by applicable law.
 * Module scope: LICENSE_SCOPE_V1.json. Third-party rights remain separate.
 */

#include "../include/zipraf_hw_fabric.h"

#define Q16_ONE 65536u
#define ARRAY_LEN(x) ((zhf_u32)(sizeof(x) / sizeof((x)[0])))

static const struct zhf_descriptor g_desc[] = {
    {ZHF_PRIM_SHA256, ZHF_CLASS_CRYPTO_HASH, 32, 64, 1, 0, 0, Q16_ONE},
    {ZHF_PRIM_SHA512, ZHF_CLASS_CRYPTO_HASH, 64, 128, 1, 0, 0, Q16_ONE},
    {ZHF_PRIM_SHA3_256, ZHF_CLASS_CRYPTO_HASH, 64, 136, 1, 0, 0, Q16_ONE},
    {ZHF_PRIM_BLAKE2S, ZHF_CLASS_CRYPTO_HASH, 32, 64, 1, 0, 0, Q16_ONE},
    {ZHF_PRIM_BLAKE2B, ZHF_CLASS_CRYPTO_HASH, 64, 128, 1, 0, 0, Q16_ONE},
    {ZHF_PRIM_BLAKE3, ZHF_CLASS_CRYPTO_HASH, 32, 1024, 1, 0, 0, Q16_ONE},
    {ZHF_PRIM_CHACHA20, ZHF_CLASS_STREAM, 32, 64, 1, 0, 0, Q16_ONE},
    {ZHF_PRIM_POLY1305, ZHF_CLASS_MAC, 32, 16, 1, 0, 0, Q16_ONE},
    {ZHF_PRIM_XCHACHA20_POLY1305, ZHF_CLASS_AEAD, 32, 64, 1, 0, 0, Q16_ONE},
    {ZHF_PRIM_ED25519, ZHF_CLASS_SIGNATURE, 32, 64, 0, 0, 0, Q16_ONE},
    {ZHF_PRIM_X25519, ZHF_CLASS_KEX, 32, 32, 0, 0, 0, Q16_ONE},
    {ZHF_PRIM_CRC32C, ZHF_CLASS_CHECKSUM, 32, 64, 1, 0, 0, Q16_ONE},
    {ZHF_PRIM_MD5_LEGACY, ZHF_CLASS_LEGACY_INSECURE, 32, 64, 1, 0, 0, Q16_ONE}
};

static zhf_u64 required_for_operation(zhf_u16 op) {
    switch (op) {
        case ZHF_OP_HASH: return ZHF_PERM_READ | ZHF_PERM_EXEC;
        case ZHF_OP_STREAM_XOR: return ZHF_PERM_READ | ZHF_PERM_WRITE | ZHF_PERM_EXEC | ZHF_PERM_PRIVATE;
        case ZHF_OP_MAC: return ZHF_PERM_READ | ZHF_PERM_EXEC | ZHF_PERM_PRIVATE;
        case ZHF_OP_SEAL: return ZHF_PERM_READ | ZHF_PERM_WRITE | ZHF_PERM_EXEC | ZHF_PERM_PRIVATE;
        case ZHF_OP_OPEN: return ZHF_PERM_READ | ZHF_PERM_WRITE | ZHF_PERM_EXEC | ZHF_PERM_PRIVATE;
        case ZHF_OP_SIGN: return ZHF_PERM_READ | ZHF_PERM_EXEC | ZHF_PERM_SIGN | ZHF_PERM_PRIVATE;
        case ZHF_OP_VERIFY: return ZHF_PERM_READ | ZHF_PERM_EXEC | ZHF_PERM_VERIFY;
        case ZHF_OP_KEX: return ZHF_PERM_READ | ZHF_PERM_EXEC | ZHF_PERM_DERIVE | ZHF_PERM_PRIVATE;
        case ZHF_OP_CHECKSUM: return ZHF_PERM_READ | ZHF_PERM_EXEC;
        default: return 0;
    }
}

static zhf_u32 selected_caps(const struct zhf_profile *p) {
    zhf_u32 c = p->capability_bits;
    if ((c & ZHF_CAP_SIMD512) != 0u) return ZHF_CAP_SIMD512 | ZHF_CAP_SCALAR;
    if ((c & ZHF_CAP_SIMD256) != 0u) return ZHF_CAP_SIMD256 | ZHF_CAP_SCALAR;
    if ((c & ZHF_CAP_SIMD128) != 0u) return ZHF_CAP_SIMD128 | ZHF_CAP_SCALAR;
    if ((c & ZHF_CAP_ASM) != 0u) return ZHF_CAP_ASM | ZHF_CAP_SCALAR;
    return c & ZHF_CAP_SCALAR;
}

zhf_u64 zhf_effective_permissions(zhf_u64 allow_bits, zhf_u64 deny_bits) {
    return allow_bits & ~deny_bits;
}

const struct zhf_descriptor *zhf_descriptor_for(zhf_u16 primitive) {
    zhf_u32 i;
    for (i = 0; i < ARRAY_LEN(g_desc); ++i) {
        if (g_desc[i].primitive == primitive) return &g_desc[i];
    }
    return (const struct zhf_descriptor *)0;
}

zhf_u16 zhf_parallel_lanes(const struct zhf_profile *p, const struct zhf_descriptor *d) {
    zhf_u16 lanes = 1;
    zhf_u16 vector_lanes;
    if (!p || !d || !d->parallel_safe) return 1;
    if (p->vector_bits == 0u || d->word_bits == 0u) return 1;
    vector_lanes = (zhf_u16)(p->vector_bits / d->word_bits);
    if (vector_lanes > lanes) lanes = vector_lanes;
    if (p->max_parallel != 0u && lanes > p->max_parallel) lanes = p->max_parallel;
    if (lanes == 0u) lanes = 1;
    return lanes;
}

zhf_i32 zhf_plan_request(const struct zhf_profile *p,
                         const struct zhf_request *r,
                         struct zhf_plan *out) {
    const struct zhf_descriptor *d;
    zhf_u64 effective;
    zhf_u64 required;
    zhf_u32 units;
    zhf_u16 lanes;
    zhf_u64 raw;

    if (!p || !r || !out || p->abi_version != ZHF_ABI_VERSION) return ZHF_INVALID;
    d = zhf_descriptor_for(r->primitive);
    if (!d) return ZHF_UNSUPPORTED;
    if ((p->capability_bits & ZHF_CAP_SCALAR) == 0u) return ZHF_TOKEN_VAZIO;

    effective = zhf_effective_permissions(p->allow_bits, p->deny_bits);
    required = required_for_operation(r->operation) | r->required_permissions;
    if (required == 0u) return ZHF_INVALID;
    if ((effective & required) != required) return ZHF_DENIED;

    lanes = zhf_parallel_lanes(p, d);
    units = r->bytes == 0u ? 1u :
        (zhf_u32)(((zhf_u64)r->bytes + (zhf_u64)d->block_or_chunk_bytes - 1ull) /
                  (zhf_u64)d->block_or_chunk_bytes);
    raw = (zhf_u64)d->setup_q16 +
          (((zhf_u64)units * (zhf_u64)d->unit_q16 + (zhf_u64)lanes - 1ull) /
           (zhf_u64)lanes);
    if (raw > 0xffffffffull) raw = 0xffffffffull;

    out->status = ZHF_OK;
    out->primitive = r->primitive;
    out->operation = r->operation;
    out->lanes = lanes;
    out->reserved = 0;
    out->normalized_cost_q16 = (zhf_u32)raw;
    out->selected_capability_bits = selected_caps(p);
    out->effective_permissions = effective;
    return ZHF_OK;
}
