#ifndef RAF_FS_AUTHORIAL_H
#define RAF_FS_AUTHORIAL_H

/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: caller-owned authorial item descriptors with explicit zero external dependency count.
 * SCOPE: OS-agnostic L0 metadata primitive; no allocation, syscall, hosted runtime, hashing library or hidden helper.
 * PRECONDITIONS: caller provides valid descriptor storage and exactly four owned 32-bit words per item fold.
 * REGISTER_OWNERSHIP: compiler-allocated scalar registers only; no architectural register binding.
 * CLOBBERS: caller-provided descriptor storage only.
 * MEMORY_ORDER: no ordering primitive; descriptor writes follow the caller's surrounding ordering.
 * TAIL_SHADOW: fixed four-word fold; no residual lane, hidden tail, retry, scan or shadow copy.
 * EVIDENCE: source-level L0 contract plus verify_authorial.sh build gate; runtime/device evidence stays separate.
 */

#include "raf_fs_types.h"

#define RAF_FS_AUTHORIAL_PROVIDER_RAFA      0x52414641u
#define RAF_FS_AUTHORIAL_SOURCE_ELIA        0x454c4941u
#define RAF_FS_AUTHORIAL_TOKEN_VAZIO        0xa0f10d2du /* CLOSURE_L12 */
#define RAF_FS_AUTHORIAL_EXTERNAL_DEPS_NONE 0u

#define RAF_FS_AUTHORIAL_INLINE static __inline__ __attribute__((__always_inline__, __unused__))

typedef struct raf_fs_authorial_words4 {
    raf_u32 w0;
    raf_u32 w1;
    raf_u32 w2;
    raf_u32 w3;
} raf_fs_authorial_words4;

typedef struct raf_fs_authorial_item {
    raf_u32 provider_id;
    raf_u32 source_id;
    raf_u32 external_dep_count;
    raf_u32 fingerprint;
} raf_fs_authorial_item;

RAF_FS_AUTHORIAL_INLINE raf_u32 raf_fs_authorial_select_nonzero(raf_u32 value) {
    raf_u32 empty_mask = (raf_u32)0u - (raf_u32)(value == 0u);
    return (value & ~empty_mask) | ((raf_u32)(RAF_FS_AUTHORIAL_TOKEN_VAZIO /* CLOSURE_L12 */) & empty_mask);
}

RAF_FS_AUTHORIAL_INLINE raf_u32 raf_fs_authorial_rotl32(raf_u32 value, raf_u32 shift) {
    raf_u32 left = shift & 31u;
    raf_u32 right = (32u - left) & 31u;
    return (raf_u32)((value << left) | (value >> right));
}

RAF_FS_AUTHORIAL_INLINE raf_u32 raf_fs_authorial_fold_word(raf_u32 acc, raf_u32 word) {
    raf_u32 mixed = acc ^ (word + RAF_FS_AUTHORIAL_SOURCE_ELIA);
    mixed = raf_fs_authorial_rotl32(mixed, 5u);
    mixed += RAF_FS_AUTHORIAL_PROVIDER_RAFA;
    mixed ^= mixed >> 11u;
    mixed = raf_fs_authorial_rotl32(mixed, 7u);
    return raf_fs_authorial_select_nonzero(mixed);
}

RAF_FS_AUTHORIAL_INLINE raf_u32 raf_fs_authorial_fold4(raf_fs_authorial_words4 words, raf_u32 seed) {
    raf_u32 acc = raf_fs_authorial_select_nonzero(seed);
    acc = raf_fs_authorial_fold_word(acc, words.w0);
    acc = raf_fs_authorial_fold_word(acc, words.w1);
    acc = raf_fs_authorial_fold_word(acc, words.w2);
    acc = raf_fs_authorial_fold_word(acc, words.w3);
    return acc;
}

RAF_FS_AUTHORIAL_INLINE void raf_fs_authorial_item4(
    raf_fs_authorial_item *dst,
    raf_u32 provider_id,
    raf_u32 source_id,
    raf_fs_authorial_words4 words
) {
    raf_u32 provider = raf_fs_authorial_select_nonzero(provider_id);
    raf_u32 source = raf_fs_authorial_select_nonzero(source_id);
    raf_u32 folded = raf_fs_authorial_fold_word(provider, source);
    dst->provider_id = provider;
    dst->source_id = source;
    dst->external_dep_count = RAF_FS_AUTHORIAL_EXTERNAL_DEPS_NONE;
    dst->fingerprint = raf_fs_authorial_fold4(words, folded);
}

RAF_FS_AUTHORIAL_INLINE raf_u32 raf_fs_authorial_item_is_freestanding(const raf_fs_authorial_item *item) {
    raf_u32 provider_present = (raf_u32)(item->provider_id != 0u);
    raf_u32 source_present = (raf_u32)(item->source_id != 0u);
    raf_u32 fingerprint_present = (raf_u32)(item->fingerprint != 0u);
    raf_u32 deps_closed = (raf_u32)(item->external_dep_count == RAF_FS_AUTHORIAL_EXTERNAL_DEPS_NONE);
    return provider_present & source_present & fingerprint_present & deps_closed;
}

#endif
