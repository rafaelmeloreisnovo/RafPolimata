#ifndef RAF_NETGUARD_H
#define RAF_NETGUARD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAF_NG_ADDR_BYTES 16u
#define RAF_NG_HASH_BYTES 32u

typedef enum {
    RAF_NG_DIR_ANY = 0,
    RAF_NG_DIR_INGRESS = 1,
    RAF_NG_DIR_EGRESS = 2
} raf_ng_direction;

typedef enum {
    RAF_NG_ACTION_UNSET = 0,
    RAF_NG_ACTION_ALLOW = 1,
    RAF_NG_ACTION_DENY = 2
} raf_ng_action;

typedef enum {
    RAF_NG_PHASE_PRE = 1,
    RAF_NG_PHASE_POST = 2,
    RAF_NG_PHASE_BLOCK = 3,
    RAF_NG_PHASE_ERROR = 4
} raf_ng_phase;

typedef enum {
    RAF_NG_HASH_NONE = 0,
    RAF_NG_HASH_SHA256 = 1,
    RAF_NG_HASH_BLAKE3_256 = 2
} raf_ng_hash_kind;

enum {
    RAF_NG_XFORM_NONE = 0u,
    RAF_NG_XFORM_NAT_SRC = 1u << 0,
    RAF_NG_XFORM_NAT_DST = 1u << 1,
    RAF_NG_XFORM_PAT_SRC = 1u << 2,
    RAF_NG_XFORM_PAT_DST = 1u << 3
};

/*
 * Address bytes are in network byte order.
 * IPv4 uses bytes [0..3] and zeros [4..15].
 * Ports are normalized to host byte order before entering the engine.
 */
typedef struct {
    uint8_t ip_version; /* 4 or 6 */
    uint8_t protocol;   /* IPPROTO_* numeric value; 0 is invalid for an observed tuple */
    uint8_t src_addr[RAF_NG_ADDR_BYTES];
    uint8_t dst_addr[RAF_NG_ADDR_BYTES];
    uint16_t src_port;
    uint16_t dst_port;
} raf_ng_tuple;

typedef struct {
    uint32_t rule_id;
    uint8_t direction;      /* raf_ng_direction; 0 = wildcard */
    uint8_t protocol;       /* 0 = wildcard */
    uint8_t src_prefix_bits;
    uint8_t dst_prefix_bits;
    uint8_t src_addr[RAF_NG_ADDR_BYTES];
    uint8_t dst_addr[RAF_NG_ADDR_BYTES];
    uint16_t src_port_min;  /* 0..65535 inclusive */
    uint16_t src_port_max;
    uint16_t dst_port_min;
    uint16_t dst_port_max;
    uint8_t action;         /* raf_ng_action */
} raf_ng_rule;

typedef struct {
    const raf_ng_rule *rules;
    size_t rule_count;
    uint8_t default_action; /* fail-closed profile uses RAF_NG_ACTION_DENY */
} raf_ng_policy;

typedef struct {
    uint8_t allowed;
    uint8_t matched;
    uint8_t action;
    uint32_t rule_id;
} raf_ng_decision;

typedef struct {
    uint64_t sequence;
    uint8_t phase;
    uint8_t direction;
    uint32_t adapter_id;
    raf_ng_tuple pre;
    raf_ng_tuple post;
    raf_ng_decision decision;
    uint32_t transform_flags;
    uint64_t byte_count;
    uint8_t payload_hash_kind;
    uint8_t payload_hash[RAF_NG_HASH_BYTES];
    int32_t error_code;
} raf_ng_event;

int raf_ng_tuple_valid(const raf_ng_tuple *tuple);
int raf_ng_tuple_equal(const raf_ng_tuple *a, const raf_ng_tuple *b);
int raf_ng_prefix_match(const uint8_t *address, const uint8_t *prefix, uint8_t prefix_bits);
raf_ng_decision raf_ng_decide(
    const raf_ng_policy *policy,
    uint8_t direction,
    const raf_ng_tuple *tuple
);
uint32_t raf_ng_transform_flags(const raf_ng_tuple *pre, const raf_ng_tuple *post);

#ifdef __cplusplus
}
#endif

#endif
