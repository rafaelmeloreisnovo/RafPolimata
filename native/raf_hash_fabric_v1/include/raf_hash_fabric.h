#ifndef RAF_HASH_FABRIC_V1_H
#define RAF_HASH_FABRIC_V1_H

/* Freestanding ABI: no stdint.h, no libc, no heap. */
typedef unsigned char      rhf_u8;
typedef unsigned short     rhf_u16;
typedef unsigned int       rhf_u32;
typedef unsigned long long rhf_u64;
typedef signed int         rhf_s32;
typedef signed long long   rhf_s64;

#define RHF_TOKEN_VAZIO 0u
#define RHF_DIGEST32_BYTES 32u
#define RHF_MERKLE16_LEAVES 16u
#define RHF_MERKLE16_SCRATCH 14u

#define RHF_CAP_SCALAR      (1u << 0)
#define RHF_CAP_NEON128     (1u << 1)
#define RHF_CAP_AVX2_256    (1u << 2)
#define RHF_CAP_AVX512_512  (1u << 3)
#define RHF_CAP_ASM         (1u << 4)
#define RHF_CAP_PAR16       (1u << 5)

/* Control-plane Q formats; never crypto word-size substitutions. */
typedef rhf_s32 rhf_q16_16;
typedef rhf_s64 rhf_q32_32;
#define RHF_Q16_ONE ((rhf_q16_16)65536)
#define RHF_Q32_ONE ((rhf_q32_32)4294967296LL)

typedef enum rhf_state {
    RHF_VOID = 0,
    RHF_PROBE,
    RHF_BASELINE,
    RHF_CANDIDATE,
    RHF_VALIDATED,
    RHF_DEGRADED,
    RHF_FAILOVER,
    RHF_ROLLBACK,
    RHF_FAILBACK,
    RHF_SAFE,
    RHF_STATE_COUNT
} rhf_state;

typedef enum rhf_event {
    RHF_BOOT = 0,
    RHF_PROBE_OK,
    RHF_PROBE_FAIL,
    RHF_BEGIN_CANDIDATE,
    RHF_CANDIDATE_OK,
    RHF_CANDIDATE_FAIL,
    RHF_FAULT,
    RHF_WATCHDOG_OK,
    RHF_WATCHDOG_FAIL,
    RHF_ROLLBACK_OK,
    RHF_FAILBACK_OK,
    RHF_EVENT_COUNT
} rhf_event;

typedef struct rhf_profile {
    rhf_u32 caps;
    rhf_u32 degraded;
    rhf_u16 vector_bits;
    rhf_u16 word_bits;
    rhf_u8 lanes;
    rhf_u8 fixed_q_fraction_bits;
    rhf_u8 state;
    rhf_u8 reserved;
} rhf_profile;

typedef struct rhf_watchdog {
    rhf_u32 epoch;
    rhf_u32 heartbeat;
    rhf_u32 deadline_ticks;
    rhf_u32 elapsed_ticks;
} rhf_watchdog;

/* Hash exactly the selected domain-separated pair under the caller's standard
 * primitive adapter. Return 0 on success; nonzero on failure.
 */
typedef rhf_u32 (*rhf_hash_pair_fn)(
    const rhf_u8 left[RHF_DIGEST32_BYTES],
    const rhf_u8 right[RHF_DIGEST32_BYTES],
    rhf_u8 out[RHF_DIGEST32_BYTES],
    void *ctx);

#ifdef __cplusplus
extern "C" {
#endif

rhf_u8 rhf_lanes32(rhf_u16 vector_bits);
rhf_u8 rhf_control_q_fraction_bits(void);
rhf_state rhf_step(rhf_state state, rhf_event event);
rhf_state rhf_watchdog_tick(rhf_state state, rhf_watchdog *wd, rhf_u32 ticks);

/* Fixed 16-leaf binary Merkle reduction. Leaves are already 32-byte digests.
 * Scratch is 14 internal digests; final level writes directly to root.
 * No heap and no data-dependent loop.
 */
rhf_u32 rhf_merkle16_reduce(
    const rhf_u8 leaves[RHF_MERKLE16_LEAVES][RHF_DIGEST32_BYTES],
    rhf_u8 root[RHF_DIGEST32_BYTES],
    rhf_u8 scratch[RHF_MERKLE16_SCRATCH][RHF_DIGEST32_BYTES],
    rhf_hash_pair_fn hash_pair,
    void *ctx);

/* ARX scheduling/self-test fingerprint only. NOT a cryptographic primitive. */
rhf_u32 rhf_mix32(rhf_u32 a, rhf_u32 b, rhf_u32 c);

#ifdef __cplusplus
}
#endif

#endif
