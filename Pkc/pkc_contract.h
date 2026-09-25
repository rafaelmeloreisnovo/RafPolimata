/*
 * Pkc/pkc_contract.h — strict freestanding contract V1.
 *
 * Copyright (c) 2024-2026 Rafael Melo Reis.
 * Provenance: rafaelmeloreisnovo/RafPolimata
 * License status: TOKEN_VAZIO_FINAL_LEGAL_TEXT
 * Policy reference: docs/legal/PKC_USE_POLICY_DRAFT_V1.md\n * Governance: CLOSURE_L11 + CLOSURE_L12
 *
 * No includes. No libc. No libm. No heap. No syscall.
 */
#ifndef RAF_PK_C_CONTRACT_H
#define RAF_PK_C_CONTRACT_H 1

typedef unsigned char      pkc_u8;
typedef unsigned int       pkc_u32;
typedef signed int         pkc_i32;
typedef unsigned long long pkc_u64;

/* Fail compilation on targets where the V1 integer contract is not true. */
typedef char pkc_assert_u32_is_4[(sizeof(pkc_u32) == 4u) ? 1 : -1];
typedef char pkc_assert_u64_is_8[(sizeof(pkc_u64) == 8u) ? 1 : -1];

#define PKC_REQ_NO_HEAP             (1ULL << 0)
#define PKC_REQ_NO_EXTERNAL_RUNTIME (1ULL << 1)
#define PKC_REQ_NO_LIBC             (1ULL << 2)
#define PKC_REQ_NO_LIBM             (1ULL << 3)
#define PKC_REQ_NO_SYSCALL          (1ULL << 4)
#define PKC_REQ_NO_TAILCALL         (1ULL << 5)
#define PKC_REQ_NO_SHADOW           (1ULL << 6)
#define PKC_REQ_NO_UNWIND           (1ULL << 7)
#define PKC_REQ_ZERO_UNDEFINED      (1ULL << 8)
#define PKC_REQ_CALLER_BUFFER       (1ULL << 9)
#define PKC_REQ_NO_DUP_KERNEL       (1ULL << 10)
#define PKC_REQ_DETERMINISTIC       (1ULL << 11)

#define PKC_STRICT_REQUIREMENTS (     PKC_REQ_NO_HEAP             |     PKC_REQ_NO_EXTERNAL_RUNTIME |     PKC_REQ_NO_LIBC             |     PKC_REQ_NO_LIBM             |     PKC_REQ_NO_SYSCALL          |     PKC_REQ_NO_TAILCALL         |     PKC_REQ_NO_SHADOW           |     PKC_REQ_NO_UNWIND           |     PKC_REQ_ZERO_UNDEFINED      |     PKC_REQ_CALLER_BUFFER       |     PKC_REQ_NO_DUP_KERNEL       |     PKC_REQ_DETERMINISTIC )

#define PKC_WARN_NONE                 0u
#define PKC_WARN_CONTRACT_INCOMPLETE  (1u << 0)
#define PKC_WARN_CLASSIC_ROUTE        (1u << 1)
#define PKC_WARN_TOKEN_VAZIO_LEXICON  (1u << 2)

#define PKC_OK                         0
#define PKC_E_ARGUMENT                -1
#define PKC_E_CAPACITY                -2
#define PKC_E_SYNTAX                  -3
#define PKC_E_VERB                    -4
#define PKC_E_OPERAND                 -5
#define PKC_E_TOKEN_VAZIO_LEXICON    -6

#define PKC_LANG_PT  1u
#define PKC_LANG_EN  2u
#define PKC_LANG_GRC 3u
#define PKC_LANG_HE  4u
#define PKC_LANG_SYR 5u

typedef struct {
    pkc_i32 code;
    pkc_u32 warnings;
    pkc_u32 source_offset;
    pkc_u64 missing_requirements;
} PkcStatusV1;

static inline void pkc_status_reset(PkcStatusV1 *s) {
    if (!s) return;
    s->code = PKC_OK;
    s->warnings = PKC_WARN_NONE;
    s->source_offset = 0u;
    s->missing_requirements = 0ULL;
}

static inline pkc_u64 pkc_contract_missing(pkc_u64 observed) {
    return PKC_STRICT_REQUIREMENTS & ~observed;
}

static inline pkc_i32 pkc_contract_evaluate(pkc_u64 observed, PkcStatusV1 *s) {
    pkc_u64 missing = pkc_contract_missing(observed);
    if (s) {
        pkc_status_reset(s);
        s->missing_requirements = missing;
        if (missing != 0ULL) {
            s->warnings |= PKC_WARN_CONTRACT_INCOMPLETE | PKC_WARN_CLASSIC_ROUTE;
        }
    }
    return missing == 0ULL ? PKC_OK : PKC_E_ARGUMENT;
}

#endif /* RAF_PK_C_CONTRACT_H */
