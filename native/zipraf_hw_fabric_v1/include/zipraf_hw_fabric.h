/* Governance binding: CLOSURE_L11 — unknown-state markers remain gaps. */
/*
 * RAFCODE-IP-NOTICE
 * Copyright (c) 2026 Rafael Melo Reis.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Berne Convention orientation: this notice records provenance; it does not
 * create, enlarge, or replace rights supplied by applicable law.
 * Module scope: LICENSE_SCOPE_V1.json. Third-party rights remain separate.
 */

#ifndef ZIPRAF_HW_FABRIC_V1_H
#define ZIPRAF_HW_FABRIC_V1_H

/*
 * ZIPRAF-HW-FABRIC-V1
 * PURPOSE: deterministic capability/permission/dispatch math for ZIPRAF-adjacent
 *          cryptographic and integrity adapters.
 * SCOPE:   routing/control only; this file does NOT implement cryptographic primitives.
 * EVIDENCE: source contract until exact-commit tests/benchmarks are executed.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char      zhf_u8;
typedef unsigned short     zhf_u16;
typedef unsigned int       zhf_u32;
typedef unsigned long long zhf_u64;
typedef signed int         zhf_i32;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(zhf_u8) == 1, "zhf_u8 must be 8-bit storage");
_Static_assert(sizeof(zhf_u16) == 2, "zhf_u16 must be 16-bit storage");
_Static_assert(sizeof(zhf_u32) == 4, "zhf_u32 must be 32-bit storage");
_Static_assert(sizeof(zhf_u64) == 8, "zhf_u64 must be 64-bit storage");
#endif

#define ZHF_ABI_VERSION 0x00010000u

enum zhf_status {
    ZHF_OK = 0,
    ZHF_DENIED = -1,
    ZHF_UNSUPPORTED = -2,
    ZHF_TOKEN_VAZIO = -3,
    ZHF_INVALID = -4
};

enum zhf_arch {
    ZHF_ARCH_UNKNOWN = 0,
    ZHF_ARCH_SCALAR = 1,
    ZHF_ARCH_ARMV7 = 2,
    ZHF_ARCH_AARCH64 = 3,
    ZHF_ARCH_X86_64 = 4,
    ZHF_ARCH_RISCV64 = 5,
    ZHF_ARCH_WASM32 = 6
};

enum zhf_capability {
    ZHF_CAP_SCALAR    = 1u << 0,
    ZHF_CAP_ASM       = 1u << 1,
    ZHF_CAP_SIMD128   = 1u << 2,
    ZHF_CAP_SIMD256   = 1u << 3,
    ZHF_CAP_SIMD512   = 1u << 4,
    ZHF_CAP_CRYPTOEXT = 1u << 5,
    ZHF_CAP_CT_IMPL   = 1u << 6
};

enum zhf_permission {
    ZHF_PERM_READ      = 1ull << 0,
    ZHF_PERM_WRITE     = 1ull << 1,
    ZHF_PERM_EXEC      = 1ull << 2,
    ZHF_PERM_VERIFY    = 1ull << 3,
    ZHF_PERM_SIGN      = 1ull << 4,
    ZHF_PERM_DERIVE    = 1ull << 5,
    ZHF_PERM_EXPORT    = 1ull << 6,
    ZHF_PERM_ADMIN     = 1ull << 7,
    ZHF_PERM_PRIVATE   = 1ull << 8,
    ZHF_PERM_NETWORK   = 1ull << 9,
    ZHF_PERM_METADATA  = 1ull << 10,
    ZHF_PERM_BENCH     = 1ull << 11
};

enum zhf_primitive {
    ZHF_PRIM_SHA256 = 1,
    ZHF_PRIM_SHA512 = 2,
    ZHF_PRIM_SHA3_256 = 3,
    ZHF_PRIM_BLAKE2S = 4,
    ZHF_PRIM_BLAKE2B = 5,
    ZHF_PRIM_BLAKE3 = 6,
    ZHF_PRIM_CHACHA20 = 7,
    ZHF_PRIM_POLY1305 = 8,
    ZHF_PRIM_XCHACHA20_POLY1305 = 9,
    ZHF_PRIM_ED25519 = 10,
    ZHF_PRIM_X25519 = 11,
    ZHF_PRIM_CRC32C = 12,
    ZHF_PRIM_MD5_LEGACY = 13
};

enum zhf_operation {
    ZHF_OP_HASH = 1,
    ZHF_OP_STREAM_XOR = 2,
    ZHF_OP_MAC = 3,
    ZHF_OP_SEAL = 4,
    ZHF_OP_OPEN = 5,
    ZHF_OP_SIGN = 6,
    ZHF_OP_VERIFY = 7,
    ZHF_OP_KEX = 8,
    ZHF_OP_CHECKSUM = 9
};

enum zhf_security_class {
    ZHF_CLASS_CRYPTO_HASH = 1,
    ZHF_CLASS_STREAM = 2,
    ZHF_CLASS_MAC = 3,
    ZHF_CLASS_AEAD = 4,
    ZHF_CLASS_SIGNATURE = 5,
    ZHF_CLASS_KEX = 6,
    ZHF_CLASS_CHECKSUM = 7,
    ZHF_CLASS_LEGACY_INSECURE = 8
};

struct zhf_profile {
    zhf_u32 abi_version;
    zhf_u16 arch;
    zhf_u16 vector_bits;
    zhf_u16 native_word_bits;
    zhf_u16 max_parallel;
    zhf_u32 capability_bits;
    zhf_u64 allow_bits;
    zhf_u64 deny_bits;
};

struct zhf_request {
    zhf_u16 primitive;
    zhf_u16 operation;
    zhf_u32 bytes;
    zhf_u64 required_permissions;
};

struct zhf_plan {
    zhf_i32 status;
    zhf_u16 primitive;
    zhf_u16 operation;
    zhf_u16 lanes;
    zhf_u16 reserved;
    zhf_u32 normalized_cost_q16;
    zhf_u32 selected_capability_bits;
    zhf_u64 effective_permissions;
};

struct zhf_descriptor {
    zhf_u16 primitive;
    zhf_u16 security_class;
    zhf_u16 word_bits;
    zhf_u16 block_or_chunk_bytes;
    zhf_u16 parallel_safe;
    zhf_u16 reserved;
    zhf_u32 setup_q16;
    zhf_u32 unit_q16;
};

zhf_u64 zhf_effective_permissions(zhf_u64 allow_bits, zhf_u64 deny_bits);
zhf_u16 zhf_parallel_lanes(const struct zhf_profile *profile, const struct zhf_descriptor *desc);
const struct zhf_descriptor *zhf_descriptor_for(zhf_u16 primitive);
zhf_i32 zhf_plan_request(const struct zhf_profile *profile,
                         const struct zhf_request *request,
                         struct zhf_plan *out_plan);

#ifdef __cplusplus
}
#endif

#endif
