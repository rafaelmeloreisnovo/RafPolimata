#ifndef ZIPRAF_HW_SERVICE_V1_H
#define ZIPRAF_HW_SERVICE_V1_H

/*
 * ZIPRAF Hardware Service V1 — original integration layer.
 * Copyright (c) 2026 Rafael Melo Reis.
 *
 * Copyright protection is not created by this notice. The notice records
 * authorship/provenance. Applicable copyright and software law, licenses,
 * third-party notices, and mandatory law control. See LEGAL_BERNE_IP_PROFILE.md.
 *
 * This layer does NOT redefine cryptographic algorithms. Compatibility claims
 * require equivalence to the applicable standard/reference vectors.
 */

typedef unsigned char      zhw_u8;
typedef unsigned short     zhw_u16;
typedef unsigned int       zhw_u32;
typedef unsigned long long zhw_u64;
typedef signed int         zhw_s32;
typedef signed long long   zhw_s64;

#define ZHW_Q16_ONE 65536u
#define ZHW_TOKEN_VAZIO_U32 0xffffffffu
#define ZHW_TOKEN_VAZIO_U64 0xffffffffffffffffULL

/* Capability bits describe observed support. They are not performance claims. */
#define ZHW_CAP_SCALAR       (1u << 0)
#define ZHW_CAP_NEON         (1u << 1)
#define ZHW_CAP_SVE          (1u << 2)
#define ZHW_CAP_SME          (1u << 3)
#define ZHW_CAP_SSE2         (1u << 4)
#define ZHW_CAP_AVX2         (1u << 5)
#define ZHW_CAP_AVX512       (1u << 6)
#define ZHW_CAP_RVV          (1u << 7)
#define ZHW_CAP_ARM_SHA2     (1u << 8)
#define ZHW_CAP_ARM_SHA3     (1u << 9)
#define ZHW_CAP_AES          (1u << 10)
#define ZHW_CAP_PMULL        (1u << 11)
#define ZHW_CAP_CRC32        (1u << 12)
#define ZHW_CAP_ASM          (1u << 13)
#define ZHW_CAP_RUST_NOSTD   (1u << 14)
#define ZHW_CAP_C11          (1u << 15)

typedef enum zhw_status {
    ZHW_VOID = 0,
    ZHW_REFERENCE,
    ZHW_IMPLEMENTED,
    ZHW_CANDIDATE,
    ZHW_VALIDATED,
    ZHW_LEGACY_ONLY,
    ZHW_BLOCKED,
    ZHW_TOKEN_VAZIO
} zhw_status;

typedef enum zhw_primitive_class {
    ZHW_CLASS_HASH = 1,
    ZHW_CLASS_MAC,
    ZHW_CLASS_STREAM_CIPHER,
    ZHW_CLASS_AEAD,
    ZHW_CLASS_SIGNATURE
} zhw_primitive_class;

typedef enum zhw_algorithm_id {
    ZHW_ALG_SHA256 = 1,
    ZHW_ALG_SHA512,
    ZHW_ALG_SHA3_256,
    ZHW_ALG_BLAKE2S_256,
    ZHW_ALG_BLAKE2B_512,
    ZHW_ALG_BLAKE3_256,
    ZHW_ALG_CHACHA20,
    ZHW_ALG_CHACHA20_POLY1305,
    ZHW_ALG_ED25519,
    ZHW_ALG_HMAC_SHA256,
    ZHW_ALG_AES128_GCM,
    ZHW_ALG_AES256_GCM,
    ZHW_ALG_MD5,
    ZHW_ALG_COUNT
} zhw_algorithm_id;

typedef struct zhw_algorithm_info {
    zhw_u16 id;
    zhw_u8 primitive_class;
    zhw_u8 default_status;
    zhw_u16 digest_or_tag_bits;
    zhw_u16 key_bits;
    zhw_u32 useful_caps;
    const char *name;
    const char *security_boundary;
} zhw_algorithm_info;

/* Measurements are supplied by evidence-producing runners. Unknown values use
 * ZHW_TOKEN_VAZIO_U64 and never become zero by implication.
 */
typedef struct zhw_measurement {
    zhw_u64 ns_per_op;
    zhw_u64 cycles_per_kib;
    zhw_u64 bytes_per_second;
    zhw_u64 code_bytes;
    zhw_u64 stack_bytes;
    zhw_u64 energy_nj_per_op;
    zhw_u32 backend_caps;
    zhw_u8 validated;
    zhw_u8 constant_time_evidence;
    zhw_u8 reserved0;
    zhw_u8 reserved1;
} zhw_measurement;

typedef struct zhw_weights_q16 {
    zhw_u32 latency;
    zhw_u32 cycles;
    zhw_u32 throughput_inverse;
    zhw_u32 code_size;
    zhw_u32 stack;
    zhw_u32 energy;
} zhw_weights_q16;

/* Normalized cost inputs are Q16.16 in the inclusive range [0, 65536].
 * The caller owns normalization against a declared benchmark baseline.
 */
typedef struct zhw_norm_cost_q16 {
    zhw_u32 latency;
    zhw_u32 cycles;
    zhw_u32 throughput_inverse;
    zhw_u32 code_size;
    zhw_u32 stack;
    zhw_u32 energy;
    zhw_u8 complete_mask; /* bit i indicates the corresponding metric is known */
} zhw_norm_cost_q16;

#ifdef __cplusplus
extern "C" {
#endif

const zhw_algorithm_info *zhw_algorithm(zhw_algorithm_id id);
zhw_u32 zhw_registry_count(void);
int zhw_weights_valid(const zhw_weights_q16 *w);
zhw_u32 zhw_score_q16(const zhw_norm_cost_q16 *cost, const zhw_weights_q16 *w, zhw_u8 required_mask);
int zhw_backend_eligible(const zhw_measurement *m, zhw_u32 required_caps, int require_validated, int require_constant_time_evidence);

#ifdef __cplusplus
}
#endif

#endif
