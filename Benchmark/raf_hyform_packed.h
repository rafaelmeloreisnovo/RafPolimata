#ifndef RAF_HYFORM_PACKED_H
#define RAF_HYFORM_PACKED_H

/*
 * RAFAELIA packed hyperform contract.
 *
 * Production properties:
 * - freestanding C11;
 * - no libc, heap, syscall, I/O or runtime dependency;
 * - fail-closed constructors;
 * - exact 6 sectors x 7 levels = 42 primary states;
 * - assembly is not required for this bitfield operator.
 */

#include "raf_types.h"

typedef u32 raf_hyform32_t;
typedef u16 raf_hyedge16_t;

_Static_assert(sizeof(raf_hyform32_t) == 4u, "HYFORM32 must be 32 bits");
_Static_assert(sizeof(raf_hyedge16_t) == 2u, "HYEDGE16 must be 16 bits");

#define RAF_HYFORM_SECTOR_COUNT 6u
#define RAF_HYFORM_LEVEL_COUNT 7u
#define RAF_HYFORM_PRIMARY_STATE_COUNT 42u
#define RAF_HYFORM_PHASE_LAYER_DEFINED 0u
#define RAF_HYFORM_ASM_REQUIRED 0u

#define RAF_HYFORM_LEVEL_SHIFT 0u
#define RAF_HYFORM_SECTOR_SHIFT 3u
#define RAF_HYFORM_OPERATOR_SHIFT 6u
#define RAF_HYFORM_GEOMETRY_SHIFT 10u
#define RAF_HYFORM_RECURRENCE_SHIFT 14u
#define RAF_HYFORM_LAYER_SHIFT 17u
#define RAF_HYFORM_EPISTEMIC_SHIFT 20u
#define RAF_HYFORM_POLARITY_SHIFT 23u

#define RAF_HYFORM_LEVEL_MASK ((raf_hyform32_t)0x7u << RAF_HYFORM_LEVEL_SHIFT)
#define RAF_HYFORM_SECTOR_MASK ((raf_hyform32_t)0x7u << RAF_HYFORM_SECTOR_SHIFT)
#define RAF_HYFORM_OPERATOR_MASK ((raf_hyform32_t)0xFu << RAF_HYFORM_OPERATOR_SHIFT)
#define RAF_HYFORM_GEOMETRY_MASK ((raf_hyform32_t)0xFu << RAF_HYFORM_GEOMETRY_SHIFT)
#define RAF_HYFORM_RECURRENCE_MASK ((raf_hyform32_t)0x7u << RAF_HYFORM_RECURRENCE_SHIFT)
#define RAF_HYFORM_LAYER_MASK ((raf_hyform32_t)0x7u << RAF_HYFORM_LAYER_SHIFT)
#define RAF_HYFORM_EPISTEMIC_MASK ((raf_hyform32_t)0x7u << RAF_HYFORM_EPISTEMIC_SHIFT)
#define RAF_HYFORM_POLARITY_MASK ((raf_hyform32_t)0x3u << RAF_HYFORM_POLARITY_SHIFT)

#define RAF_HYFORM_SOURCE_PRESENT ((raf_hyform32_t)1u << 25u)
#define RAF_HYFORM_UNIT_DEFINED ((raf_hyform32_t)1u << 26u)
#define RAF_HYFORM_TEST_DEFINED ((raf_hyform32_t)1u << 27u)
#define RAF_HYFORM_RECEIPT_PRESENT ((raf_hyform32_t)1u << 28u)
#define RAF_HYFORM_CLAIM_ALLOWED ((raf_hyform32_t)1u << 29u)
#define RAF_HYFORM_TOKEN_VAZIO ((raf_hyform32_t)1u << 30u)
#define RAF_HYFORM_VALID ((raf_hyform32_t)1u << 31u)

#define RAF_HYEDGE_SOURCE_SHIFT 0u
#define RAF_HYEDGE_TARGET_SHIFT 6u
#define RAF_HYEDGE_RELATION_SHIFT 12u
#define RAF_HYEDGE_NODE_MASK ((raf_hyedge16_t)0x3Fu)
#define RAF_HYEDGE_RELATION_MASK ((raf_hyedge16_t)0xFu)
#define RAF_HYEDGE_INVALID ((raf_hyedge16_t)0xFFFFu)

enum raf_hyform_epistemic_state {
    RAF_HYFORM_EP_OBSERVED = 0,
    RAF_HYFORM_EP_DECLARED = 1,
    RAF_HYFORM_EP_TESTED_LOCAL = 2,
    RAF_HYFORM_EP_VERIFIED = 3,
    RAF_HYFORM_EP_CONTRADICTED = 4,
    RAF_HYFORM_EP_REFUTED = 5,
    RAF_HYFORM_EP_TOKEN_VAZIO = 6,
    RAF_HYFORM_EP_UNKNOWN_UNKNOWN = 7
};

enum raf_hyedge_relation {
    RAF_HYEDGE_CIRCULAR_NEIGHBOR = 0,
    RAF_HYEDGE_PHASE_CHANGE = 1,
    RAF_HYEDGE_SECTOR_CHANGE = 2,
    RAF_HYEDGE_COMPLEMENT = 3,
    RAF_HYEDGE_MODULAR_DOUBLING = 4,
    RAF_HYEDGE_FIBONACCI_JUMP = 5,
    RAF_HYEDGE_PROJECTION = 6,
    RAF_HYEDGE_EVIDENCE = 7,
    RAF_HYEDGE_CONTRADICTION = 8,
    RAF_HYEDGE_CAUSAL_CANDIDATE = 9,
    RAF_HYEDGE_PROVENANCE = 10,
    RAF_HYEDGE_FEEDBACK = 11
};

static inline u8 raf_hyform_primary_state(u8 sector, u8 level) {
    if (sector >= RAF_HYFORM_SECTOR_COUNT || level >= RAF_HYFORM_LEVEL_COUNT) {
        return (u8)0xFFu;
    }
    return (u8)((u8)(sector * RAF_HYFORM_LEVEL_COUNT) + level);
}

static inline raf_hyform32_t raf_hyform32_pack_checked(
    u8 level, u8 sector, u8 operator_id, u8 geometry, u8 recurrence,
    u8 layer, u8 epistemic, u8 polarity, u8 source_present,
    u8 unit_defined, u8 test_defined, u8 receipt_present,
    u8 claim_allowed, u8 token_vazio
) {
    raf_hyform32_t value;

    if (level >= RAF_HYFORM_LEVEL_COUNT || sector >= RAF_HYFORM_SECTOR_COUNT) return 0u;
    if (operator_id > 15u || geometry > 15u || recurrence > 7u || layer > 7u ||
        epistemic > 7u || polarity > 3u) return 0u;
    if (claim_allowed != 0u && token_vazio != 0u) return 0u;

    value = RAF_HYFORM_VALID;
    value |= ((raf_hyform32_t)level << RAF_HYFORM_LEVEL_SHIFT);
    value |= ((raf_hyform32_t)sector << RAF_HYFORM_SECTOR_SHIFT);
    value |= ((raf_hyform32_t)operator_id << RAF_HYFORM_OPERATOR_SHIFT);
    value |= ((raf_hyform32_t)geometry << RAF_HYFORM_GEOMETRY_SHIFT);
    value |= ((raf_hyform32_t)recurrence << RAF_HYFORM_RECURRENCE_SHIFT);
    value |= ((raf_hyform32_t)layer << RAF_HYFORM_LAYER_SHIFT);
    value |= ((raf_hyform32_t)epistemic << RAF_HYFORM_EPISTEMIC_SHIFT);
    value |= ((raf_hyform32_t)polarity << RAF_HYFORM_POLARITY_SHIFT);
    if (source_present != 0u) value |= RAF_HYFORM_SOURCE_PRESENT;
    if (unit_defined != 0u) value |= RAF_HYFORM_UNIT_DEFINED;
    if (test_defined != 0u) value |= RAF_HYFORM_TEST_DEFINED;
    if (receipt_present != 0u) value |= RAF_HYFORM_RECEIPT_PRESENT;
    if (claim_allowed != 0u) value |= RAF_HYFORM_CLAIM_ALLOWED;
    if (token_vazio != 0u) value |= RAF_HYFORM_TOKEN_VAZIO;
    return value;
}

static inline u8 raf_hyform32_level(raf_hyform32_t v) { return (u8)((v & RAF_HYFORM_LEVEL_MASK) >> RAF_HYFORM_LEVEL_SHIFT); }
static inline u8 raf_hyform32_sector(raf_hyform32_t v) { return (u8)((v & RAF_HYFORM_SECTOR_MASK) >> RAF_HYFORM_SECTOR_SHIFT); }
static inline u8 raf_hyform32_operator(raf_hyform32_t v) { return (u8)((v & RAF_HYFORM_OPERATOR_MASK) >> RAF_HYFORM_OPERATOR_SHIFT); }
static inline u8 raf_hyform32_geometry(raf_hyform32_t v) { return (u8)((v & RAF_HYFORM_GEOMETRY_MASK) >> RAF_HYFORM_GEOMETRY_SHIFT); }
static inline u8 raf_hyform32_recurrence(raf_hyform32_t v) { return (u8)((v & RAF_HYFORM_RECURRENCE_MASK) >> RAF_HYFORM_RECURRENCE_SHIFT); }
static inline u8 raf_hyform32_layer(raf_hyform32_t v) { return (u8)((v & RAF_HYFORM_LAYER_MASK) >> RAF_HYFORM_LAYER_SHIFT); }
static inline u8 raf_hyform32_epistemic(raf_hyform32_t v) { return (u8)((v & RAF_HYFORM_EPISTEMIC_MASK) >> RAF_HYFORM_EPISTEMIC_SHIFT); }
static inline u8 raf_hyform32_polarity(raf_hyform32_t v) { return (u8)((v & RAF_HYFORM_POLARITY_MASK) >> RAF_HYFORM_POLARITY_SHIFT); }

static inline u8 raf_hyform32_is_valid(raf_hyform32_t value) {
    if ((value & RAF_HYFORM_VALID) == 0u) return 0u;
    if (raf_hyform32_level(value) >= RAF_HYFORM_LEVEL_COUNT) return 0u;
    if (raf_hyform32_sector(value) >= RAF_HYFORM_SECTOR_COUNT) return 0u;
    if ((value & RAF_HYFORM_CLAIM_ALLOWED) != 0u &&
        (value & RAF_HYFORM_TOKEN_VAZIO) != 0u) return 0u;
    return 1u;
}

static inline raf_hyedge16_t raf_hyedge16_pack_checked(u8 source, u8 target, u8 relation) {
    if (source >= RAF_HYFORM_PRIMARY_STATE_COUNT ||
        target >= RAF_HYFORM_PRIMARY_STATE_COUNT || relation > 15u) {
        return RAF_HYEDGE_INVALID;
    }
    return (raf_hyedge16_t)(((raf_hyedge16_t)source << RAF_HYEDGE_SOURCE_SHIFT) |
                            ((raf_hyedge16_t)target << RAF_HYEDGE_TARGET_SHIFT) |
                            ((raf_hyedge16_t)relation << RAF_HYEDGE_RELATION_SHIFT));
}

static inline u8 raf_hyedge16_source(raf_hyedge16_t v) { return (u8)((v >> RAF_HYEDGE_SOURCE_SHIFT) & RAF_HYEDGE_NODE_MASK); }
static inline u8 raf_hyedge16_target(raf_hyedge16_t v) { return (u8)((v >> RAF_HYEDGE_TARGET_SHIFT) & RAF_HYEDGE_NODE_MASK); }
static inline u8 raf_hyedge16_relation(raf_hyedge16_t v) { return (u8)((v >> RAF_HYEDGE_RELATION_SHIFT) & RAF_HYEDGE_RELATION_MASK); }

static inline u8 raf_hyedge16_is_valid(raf_hyedge16_t value) {
    if (value == RAF_HYEDGE_INVALID) return 0u;
    return (u8)(raf_hyedge16_source(value) < RAF_HYFORM_PRIMARY_STATE_COUNT &&
                raf_hyedge16_target(value) < RAF_HYFORM_PRIMARY_STATE_COUNT);
}

#endif
