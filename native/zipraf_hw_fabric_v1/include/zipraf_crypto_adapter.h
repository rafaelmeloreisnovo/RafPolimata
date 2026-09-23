/*
 * RAFCODE-IP-NOTICE
 * Copyright (c) 2026 Rafael Melo Reis.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Berne Convention orientation: provenance notice only; applicable law controls.
 * Module scope: LICENSE_SCOPE_V1.json. Third-party rights remain separate.
 */
#ifndef ZIPRAF_CRYPTO_ADAPTER_V1_H
#define ZIPRAF_CRYPTO_ADAPTER_V1_H

#include "zipraf_hw_fabric.h"

#ifdef __cplusplus
extern "C" {
#endif

enum zhf_adapter_state {
    ZHF_ADAPTER_TOKEN_VAZIO = 0,
    ZHF_ADAPTER_CANDIDATE = 1,
    ZHF_ADAPTER_VALIDATED = 2,
    ZHF_ADAPTER_DISABLED = 3
};

typedef zhf_i32 (*zhf_hash_adapter_fn)(const zhf_u8 *in, zhf_u32 in_len,
                                        zhf_u8 *out, zhf_u32 out_len);
typedef zhf_i32 (*zhf_stream_adapter_fn)(const zhf_u8 *key, zhf_u32 key_len,
                                          const zhf_u8 *nonce, zhf_u32 nonce_len,
                                          zhf_u32 counter,
                                          const zhf_u8 *in, zhf_u8 *out, zhf_u32 len);
typedef zhf_i32 (*zhf_sign_adapter_fn)(const zhf_u8 *secret, zhf_u32 secret_len,
                                        const zhf_u8 *msg, zhf_u32 msg_len,
                                        zhf_u8 *sig, zhf_u32 sig_len);
typedef zhf_i32 (*zhf_verify_adapter_fn)(const zhf_u8 *pub, zhf_u32 pub_len,
                                          const zhf_u8 *msg, zhf_u32 msg_len,
                                          const zhf_u8 *sig, zhf_u32 sig_len);
typedef zhf_i32 (*zhf_kex_adapter_fn)(const zhf_u8 *secret, zhf_u32 secret_len,
                                       const zhf_u8 *peer, zhf_u32 peer_len,
                                       zhf_u8 *shared, zhf_u32 shared_len);

struct zhf_adapter_v1 {
    zhf_u32 abi_version;
    zhf_u16 primitive;
    zhf_u16 state;
    zhf_u32 capability_bits;
    const char *implementation_id;
    zhf_hash_adapter_fn hash;
    zhf_stream_adapter_fn stream;
    zhf_sign_adapter_fn sign;
    zhf_verify_adapter_fn verify;
    zhf_kex_adapter_fn kex;
};

static inline int zhf_adapter_is_callable(const struct zhf_adapter_v1 *a) {
    return a && a->abi_version == ZHF_ABI_VERSION && a->state == ZHF_ADAPTER_VALIDATED;
}

#ifdef __cplusplus
}
#endif
#endif
