#include "raf_convscan.h"

#define CHECK(x, code) do { if (!(x)) return (code); } while (0)

static int feed_chunks(const char *json, rcs_u32 len, rcs_u32 chunk, rcs_stats *out) {
    rcs_ctx ctx;
    rcs_u32 off = 0u;
    int rc;
    rcs_init(&ctx);
    while (off < len) {
        rcs_u32 n = len - off;
        if (n > chunk) n = chunk;
        rc = rcs_feed(&ctx, json + off, n);
        if (rc) return rc;
        off += n;
    }
    return rcs_finish(&ctx, out);
}

static int test_chatgpt_shape(void) {
    static const char json[] =
        "[{\"id\":\"c1\",\"title\":\"A\",\"create_time\":1.5,\"update_time\":2,"
        "\"mapping\":{\"n1\":{\"message\":{\"id\":\"m1\",\"content\":{\"parts\":[\"ola\"]}}}}}]";
    rcs_stats a, b;
    CHECK(feed_chunks(json, (rcs_u32)(sizeof(json) - 1u), 1u, &a) == RCS_OK, 10);
    CHECK(feed_chunks(json, (rcs_u32)(sizeof(json) - 1u), 17u, &b) == RCS_OK, 11);
    CHECK(a.bytes == b.bytes, 12);
    CHECK(a.crc32c == b.crc32c, 13);
    CHECK(a.title_keys == 1u, 14);
    CHECK(a.create_time_keys == 1u, 15);
    CHECK(a.update_time_keys == 1u, 16);
    CHECK(a.mapping_keys == 1u, 17);
    CHECK(a.message_keys == 1u, 18);
    CHECK(a.conversation_id_keys == 2u, 19);
    CHECK(a.objects == 5u, 20);
    CHECK(a.arrays == 2u, 21);
    CHECK(a.max_depth >= 5u, 22);
    return 0;
}

static int test_escapes(void) {
    static const char json[] = "{\"title\":\"linha\\n\\u00e1\\\"\",\"id\":null}";
    rcs_stats s;
    CHECK(feed_chunks(json, (rcs_u32)(sizeof(json) - 1u), 2u, &s) == RCS_OK, 30);
    CHECK(s.title_keys == 1u, 31);
    CHECK(s.conversation_id_keys == 1u, 32);
    CHECK(s.literals == 1u, 33);
    return 0;
}

static int test_rejection(void) {
    static const char truncated[] = "[{\"id\":\"x\"}";
    static const char bad_escape[] = "{\"x\":\"\\q\"}";
    static const char bad_close[] = "{]";
    rcs_stats s;
    CHECK(feed_chunks(truncated, (rcs_u32)(sizeof(truncated) - 1u), 3u, &s) == RCS_E_TRUNCATED, 40);
    CHECK(feed_chunks(bad_escape, (rcs_u32)(sizeof(bad_escape) - 1u), 4u, &s) == RCS_E_SYNTAX, 41);
    CHECK(feed_chunks(bad_close, (rcs_u32)(sizeof(bad_close) - 1u), 2u, &s) == RCS_E_SYNTAX, 42);
    return 0;
}

static int test_depth_limit(void) {
    char json[RCS_MAX_DEPTH + 2u];
    rcs_ctx ctx;
    rcs_u32 i;
    for (i = 0u; i < RCS_MAX_DEPTH + 1u; ++i) json[i] = '[';
    rcs_init(&ctx);
    CHECK(rcs_feed(&ctx, json, RCS_MAX_DEPTH + 1u) == RCS_E_DEPTH, 50);
    return 0;
}

int main(void) {
    int rc;
    rc = test_chatgpt_shape(); if (rc) return rc;
    rc = test_escapes(); if (rc) return rc;
    rc = test_rejection(); if (rc) return rc;
    rc = test_depth_limit(); if (rc) return rc;
    return 0;
}
