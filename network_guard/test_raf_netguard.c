#include "raf_netguard.h"

#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) {     fprintf(stderr, "FAIL line=%d expr=%s\n", __LINE__, #x); return 1; } } while (0)

static raf_ng_tuple tcp4(
    uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3, uint16_t sport,
    uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint16_t dport
) {
    raf_ng_tuple t;
    memset(&t, 0, sizeof(t));
    t.ip_version = 4u;
    t.protocol = 6u;
    t.src_addr[0] = s0; t.src_addr[1] = s1; t.src_addr[2] = s2; t.src_addr[3] = s3;
    t.dst_addr[0] = d0; t.dst_addr[1] = d1; t.dst_addr[2] = d2; t.dst_addr[3] = d3;
    t.src_port = sport;
    t.dst_port = dport;
    return t;
}

int main(void) {
    raf_ng_rule allow_https_private;
    raf_ng_policy policy;
    raf_ng_tuple allowed;
    raf_ng_tuple denied;
    raf_ng_tuple translated;
    raf_ng_decision d;
    uint32_t flags;

    memset(&allow_https_private, 0, sizeof(allow_https_private));
    allow_https_private.rule_id = 1001u;
    allow_https_private.direction = RAF_NG_DIR_EGRESS;
    allow_https_private.protocol = 6u;
    allow_https_private.dst_addr[0] = 10u;
    allow_https_private.dst_prefix_bits = 8u;
    allow_https_private.src_port_min = 0u;
    allow_https_private.src_port_max = 65535u;
    allow_https_private.dst_port_min = 443u;
    allow_https_private.dst_port_max = 443u;
    allow_https_private.action = RAF_NG_ACTION_ALLOW;

    policy.rules = &allow_https_private;
    policy.rule_count = 1u;
    policy.default_action = RAF_NG_ACTION_DENY;

    allowed = tcp4(192,168,1,20,51000,10,2,3,4,443);
    d = raf_ng_decide(&policy, RAF_NG_DIR_EGRESS, &allowed);
    CHECK(d.allowed == 1u);
    CHECK(d.matched == 1u);
    CHECK(d.rule_id == 1001u);

    denied = tcp4(192,168,1,20,51000,8,8,8,8,443);
    d = raf_ng_decide(&policy, RAF_NG_DIR_EGRESS, &denied);
    CHECK(d.allowed == 0u);
    CHECK(d.matched == 0u);

    translated = allowed;
    translated.src_addr[0] = 203u;
    translated.src_addr[1] = 0u;
    translated.src_addr[2] = 113u;
    translated.src_addr[3] = 55u;
    translated.src_port = 62001u;

    flags = raf_ng_transform_flags(&allowed, &translated);
    CHECK((flags & RAF_NG_XFORM_NAT_SRC) != 0u);
    CHECK((flags & RAF_NG_XFORM_PAT_SRC) != 0u);
    CHECK((flags & RAF_NG_XFORM_NAT_DST) == 0u);
    CHECK((flags & RAF_NG_XFORM_PAT_DST) == 0u);

    CHECK(raf_ng_prefix_match(allowed.dst_addr, allow_https_private.dst_addr, 8u) == 1);
    CHECK(raf_ng_tuple_equal(&allowed, &allowed) == 1);
    CHECK(raf_ng_tuple_equal(&allowed, &translated) == 0);

    puts("RAF_NETGUARD_SELFTEST PASS");
    return 0;
}
