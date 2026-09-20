#include "raf_netguard.h"

static size_t raf_ng_addr_len(uint8_t ip_version) {
    if (ip_version == 4u) return 4u;
    if (ip_version == 6u) return 16u;
    return 0u;
}

static int raf_ng_bytes_equal(const uint8_t *a, const uint8_t *b, size_t n) {
    size_t i;
    uint8_t diff = 0u;
    for (i = 0u; i < n; ++i) diff = (uint8_t)(diff | (uint8_t)(a[i] ^ b[i]));
    return diff == 0u;
}

int raf_ng_tuple_valid(const raf_ng_tuple *tuple) {
    if (tuple == NULL) return 0;
    if (raf_ng_addr_len(tuple->ip_version) == 0u) return 0;
    if (tuple->protocol == 0u) return 0;
    return 1;
}

int raf_ng_tuple_equal(const raf_ng_tuple *a, const raf_ng_tuple *b) {
    size_t n;
    if (!raf_ng_tuple_valid(a) || !raf_ng_tuple_valid(b)) return 0;
    if (a->ip_version != b->ip_version) return 0;
    if (a->protocol != b->protocol) return 0;
    if (a->src_port != b->src_port || a->dst_port != b->dst_port) return 0;
    n = raf_ng_addr_len(a->ip_version);
    return raf_ng_bytes_equal(a->src_addr, b->src_addr, n) &&
           raf_ng_bytes_equal(a->dst_addr, b->dst_addr, n);
}

int raf_ng_prefix_match(const uint8_t *address, const uint8_t *prefix, uint8_t prefix_bits) {
    uint8_t full_bytes;
    uint8_t rem_bits;
    uint8_t mask;
    uint8_t i;

    if (address == NULL || prefix == NULL) return 0;
    if (prefix_bits > 128u) return 0;
    if (prefix_bits == 0u) return 1;

    full_bytes = (uint8_t)(prefix_bits / 8u);
    rem_bits = (uint8_t)(prefix_bits % 8u);

    for (i = 0u; i < full_bytes; ++i) {
        if (address[i] != prefix[i]) return 0;
    }
    if (rem_bits == 0u) return 1;

    mask = (uint8_t)(0xffu << (8u - rem_bits));
    return (uint8_t)(address[full_bytes] & mask) ==
           (uint8_t)(prefix[full_bytes] & mask);
}

static int raf_ng_rule_valid_for_tuple(const raf_ng_rule *rule, const raf_ng_tuple *tuple) {
    uint8_t max_bits;
    if (rule == NULL || tuple == NULL) return 0;
    max_bits = tuple->ip_version == 4u ? 32u : 128u;
    if (rule->src_prefix_bits > max_bits || rule->dst_prefix_bits > max_bits) return 0;
    if (rule->src_port_min > rule->src_port_max) return 0;
    if (rule->dst_port_min > rule->dst_port_max) return 0;
    if (rule->action != RAF_NG_ACTION_ALLOW && rule->action != RAF_NG_ACTION_DENY) return 0;
    return 1;
}

static int raf_ng_rule_matches(
    const raf_ng_rule *rule,
    uint8_t direction,
    const raf_ng_tuple *tuple
) {
    if (!raf_ng_rule_valid_for_tuple(rule, tuple)) return 0;
    if (rule->direction != RAF_NG_DIR_ANY && rule->direction != direction) return 0;
    if (rule->protocol != 0u && rule->protocol != tuple->protocol) return 0;

    if (!raf_ng_prefix_match(tuple->src_addr, rule->src_addr, rule->src_prefix_bits)) return 0;
    if (!raf_ng_prefix_match(tuple->dst_addr, rule->dst_addr, rule->dst_prefix_bits)) return 0;

    if (tuple->src_port < rule->src_port_min || tuple->src_port > rule->src_port_max) return 0;
    if (tuple->dst_port < rule->dst_port_min || tuple->dst_port > rule->dst_port_max) return 0;
    return 1;
}

raf_ng_decision raf_ng_decide(
    const raf_ng_policy *policy,
    uint8_t direction,
    const raf_ng_tuple *tuple
) {
    raf_ng_decision out = {0u, 0u, RAF_NG_ACTION_DENY, 0u};
    size_t i;

    if (policy == NULL || !raf_ng_tuple_valid(tuple)) return out;
    if (direction != RAF_NG_DIR_INGRESS && direction != RAF_NG_DIR_EGRESS) return out;

    for (i = 0u; i < policy->rule_count; ++i) {
        const raf_ng_rule *rule = &policy->rules[i];
        if (raf_ng_rule_matches(rule, direction, tuple)) {
            out.matched = 1u;
            out.action = rule->action;
            out.rule_id = rule->rule_id;
            out.allowed = (uint8_t)(rule->action == RAF_NG_ACTION_ALLOW);
            return out;
        }
    }

    out.action = policy->default_action == RAF_NG_ACTION_ALLOW
        ? RAF_NG_ACTION_ALLOW
        : RAF_NG_ACTION_DENY;
    out.allowed = (uint8_t)(out.action == RAF_NG_ACTION_ALLOW);
    return out;
}

uint32_t raf_ng_transform_flags(const raf_ng_tuple *pre, const raf_ng_tuple *post) {
    uint32_t flags = RAF_NG_XFORM_NONE;
    size_t n;

    if (!raf_ng_tuple_valid(pre) || !raf_ng_tuple_valid(post)) return flags;
    if (pre->ip_version != post->ip_version || pre->protocol != post->protocol) return flags;

    n = raf_ng_addr_len(pre->ip_version);
    if (!raf_ng_bytes_equal(pre->src_addr, post->src_addr, n)) flags |= RAF_NG_XFORM_NAT_SRC;
    if (!raf_ng_bytes_equal(pre->dst_addr, post->dst_addr, n)) flags |= RAF_NG_XFORM_NAT_DST;
    if (pre->src_port != post->src_port) flags |= RAF_NG_XFORM_PAT_SRC;
    if (pre->dst_port != post->dst_port) flags |= RAF_NG_XFORM_PAT_DST;
    return flags;
}
