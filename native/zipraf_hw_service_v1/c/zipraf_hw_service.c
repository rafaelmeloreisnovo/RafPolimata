#include "../include/zipraf_hw_service.h"

static const zhw_algorithm_info ZHW_REGISTRY[] = {
    { ZHW_ALG_SHA256, ZHW_CLASS_HASH, ZHW_REFERENCE, 256, 0, ZHW_CAP_ARM_SHA2|ZHW_CAP_AVX2|ZHW_CAP_NEON, "SHA-256", "custody/hash; standard-equivalent implementation required" },
    { ZHW_ALG_SHA512, ZHW_CLASS_HASH, ZHW_REFERENCE, 512, 0, ZHW_CAP_ARM_SHA2|ZHW_CAP_AVX2, "SHA-512", "custody/hash; standard-equivalent implementation required" },
    { ZHW_ALG_SHA3_256, ZHW_CLASS_HASH, ZHW_REFERENCE, 256, 0, ZHW_CAP_ARM_SHA3|ZHW_CAP_AVX2, "SHA3-256", "hash/XOF family boundary; standard-equivalent implementation required" },
    { ZHW_ALG_BLAKE2S_256, ZHW_CLASS_HASH, ZHW_REFERENCE, 256, 0, ZHW_CAP_NEON|ZHW_CAP_AVX2, "BLAKE2s-256", "hash; upstream/spec provenance required" },
    { ZHW_ALG_BLAKE2B_512, ZHW_CLASS_HASH, ZHW_REFERENCE, 512, 0, ZHW_CAP_AVX2|ZHW_CAP_AVX512, "BLAKE2b-512", "hash; upstream/spec provenance required" },
    { ZHW_ALG_BLAKE3_256, ZHW_CLASS_HASH, ZHW_CANDIDATE, 256, 0, ZHW_CAP_NEON|ZHW_CAP_AVX2|ZHW_CAP_AVX512, "BLAKE3-256", "official KAT equivalence required before VALIDATED" },
    { ZHW_ALG_CHACHA20, ZHW_CLASS_STREAM_CIPHER, ZHW_REFERENCE, 0, 256, ZHW_CAP_NEON|ZHW_CAP_AVX2, "ChaCha20", "cipher only; nonce/key contract required" },
    { ZHW_ALG_CHACHA20_POLY1305, ZHW_CLASS_AEAD, ZHW_REFERENCE, 128, 256, ZHW_CAP_NEON|ZHW_CAP_AVX2, "ChaCha20-Poly1305", "AEAD; RFC-compatible nonce/tag behavior required" },
    { ZHW_ALG_ED25519, ZHW_CLASS_SIGNATURE, ZHW_REFERENCE, 512, 256, ZHW_CAP_SCALAR|ZHW_CAP_NEON, "Ed25519", "signature; vetted implementation and test vectors required" },
    { ZHW_ALG_HMAC_SHA256, ZHW_CLASS_MAC, ZHW_REFERENCE, 256, 0, ZHW_CAP_ARM_SHA2|ZHW_CAP_AVX2, "HMAC-SHA-256", "MAC; key handling remains external to this registry" },
    { ZHW_ALG_AES128_GCM, ZHW_CLASS_AEAD, ZHW_REFERENCE, 128, 128, ZHW_CAP_AES|ZHW_CAP_PMULL, "AES-128-GCM", "AEAD; accelerated path only when AES+GHASH support is validated" },
    { ZHW_ALG_AES256_GCM, ZHW_CLASS_AEAD, ZHW_REFERENCE, 128, 256, ZHW_CAP_AES|ZHW_CAP_PMULL, "AES-256-GCM", "AEAD; accelerated path only when AES+GHASH support is validated" },
    { ZHW_ALG_MD5, ZHW_CLASS_HASH, ZHW_LEGACY_ONLY, 128, 0, ZHW_CAP_SCALAR, "MD5", "legacy identity/import only; forbidden where collision resistance or signatures are required" }
};

const zhw_algorithm_info *zhw_algorithm(zhw_algorithm_id id) {
    zhw_u32 i;
    for (i = 0; i < (zhw_u32)(sizeof(ZHW_REGISTRY)/sizeof(ZHW_REGISTRY[0])); ++i)
        if (ZHW_REGISTRY[i].id == (zhw_u16)id) return &ZHW_REGISTRY[i];
    return (const zhw_algorithm_info *)0;
}

zhw_u32 zhw_registry_count(void) {
    return (zhw_u32)(sizeof(ZHW_REGISTRY)/sizeof(ZHW_REGISTRY[0]));
}

int zhw_weights_valid(const zhw_weights_q16 *w) {
    zhw_u64 sum;
    if (!w) return 0;
    sum = (zhw_u64)w->latency + w->cycles + w->throughput_inverse + w->code_size + w->stack + w->energy;
    return sum == (zhw_u64)ZHW_Q16_ONE;
}

zhw_u32 zhw_score_q16(const zhw_norm_cost_q16 *c, const zhw_weights_q16 *w, zhw_u8 required_mask) {
    zhw_u64 acc = 0;
    if (!c || !w || !zhw_weights_valid(w)) return ZHW_TOKEN_VAZIO_U32;
    if ((c->complete_mask & required_mask) != required_mask) return ZHW_TOKEN_VAZIO_U32;
    if (c->latency > ZHW_Q16_ONE || c->cycles > ZHW_Q16_ONE || c->throughput_inverse > ZHW_Q16_ONE ||
        c->code_size > ZHW_Q16_ONE || c->stack > ZHW_Q16_ONE || c->energy > ZHW_Q16_ONE) return ZHW_TOKEN_VAZIO_U32;
    acc += (zhw_u64)c->latency * w->latency;
    acc += (zhw_u64)c->cycles * w->cycles;
    acc += (zhw_u64)c->throughput_inverse * w->throughput_inverse;
    acc += (zhw_u64)c->code_size * w->code_size;
    acc += (zhw_u64)c->stack * w->stack;
    acc += (zhw_u64)c->energy * w->energy;
    return (zhw_u32)(acc >> 16);
}

int zhw_backend_eligible(const zhw_measurement *m, zhw_u32 required_caps, int require_validated, int require_constant_time_evidence) {
    if (!m) return 0;
    if ((m->backend_caps & required_caps) != required_caps) return 0;
    if (require_validated && !m->validated) return 0;
    if (require_constant_time_evidence && !m->constant_time_evidence) return 0;
    return 1;
}
