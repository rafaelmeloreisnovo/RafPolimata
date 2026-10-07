#ifndef RAFBBS_GIT_CORE_H
#define RAFBBS_GIT_CORE_H

#include "rafbbs_types.h"

/*
 * Pure Git provenance parsing for caller-supplied bytes.
 *
 * This module does not open .git, execute git, allocate memory or know a
 * filesystem. It recognizes only the byte contracts needed to recover HEAD,
 * refs and object ids from observations supplied by an adapter.
 */

#define RAF_GIT_HEAD_INVALID 0u
#define RAF_GIT_HEAD_SYMBOLIC 1u
#define RAF_GIT_HEAD_DETACHED 2u

typedef struct {
    RafU32 kind;
    RafU32 ref_valid;
    RafU32 branch_valid;
    RafU32 oid_valid;
    RafU32 truncated;
} RafGitHeadState;

static inline RafU32 raf_git_line_len(const char *text)
{
    RafU32 n = 0u;
    if (text == (const char *)0) return 0u;
    while (text[n] != 0 && text[n] != '\n' && text[n] != '\r') n++;
    return n;
}

static inline RafU32 raf_git_span_equal(
    const char *text,
    RafU32 start,
    RafU32 len,
    const char *expected
) {
    RafU32 i = 0u;
    if (text == (const char *)0 || expected == (const char *)0) return 0u;
    while (i < len && expected[i] != 0) {
        if (text[start + i] != expected[i]) return 0u;
        i++;
    }
    return (RafU32)(i == len && expected[i] == 0);
}

static inline RafU32 raf_git_has_prefix(
    const char *text,
    RafU32 len,
    const char *prefix,
    RafU32 prefix_len
) {
    RafU32 i;
    if (text == (const char *)0 || prefix == (const char *)0 || len < prefix_len)
        return 0u;
    for (i = 0u; i < prefix_len; i++)
        if (text[i] != prefix[i]) return 0u;
    return 1u;
}

static inline RafU32 raf_git_copy_span(
    char *dst,
    RafU32 cap,
    const char *src,
    RafU32 start,
    RafU32 len
) {
    RafU32 i = 0u;
    RafU32 dropped = 0u;
    if (cap == 0u) return (RafU32)(len != 0u);
    if (src == (const char *)0) {
        dst[0] = 0;
        return 0u;
    }
    while (i < len) {
        if (i + 1u < cap) dst[i] = src[start + i];
        else dropped = 1u;
        i++;
    }
    dst[(i < cap) ? i : (cap - 1u)] = 0;
    return dropped;
}

static inline RafU32 raf_git_is_hex(char c)
{
    return (RafU32)(
        (c >= '0' && c <= '9') ||
        (c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'F')
    );
}

static inline RafU32 raf_git_oid_span_valid(
    const char *text,
    RafU32 start,
    RafU32 len
) {
    RafU32 i;
    if (text == (const char *)0 || (len != 40u && len != 64u)) return 0u;
    for (i = 0u; i < len; i++)
        if (raf_git_is_hex(text[start + i]) == 0u) return 0u;
    return 1u;
}

static inline RafGitHeadState raf_git_parse_head(
    const char *head_text,
    char *ref_out,
    RafU32 ref_cap,
    char *branch_out,
    RafU32 branch_cap,
    char *oid_out,
    RafU32 oid_cap
) {
    RafGitHeadState state = {0u, 0u, 0u, 0u, 0u};
    RafU32 len = raf_git_line_len(head_text);
    RafU32 branch_start;
    if (ref_cap != 0u) ref_out[0] = 0;
    if (branch_cap != 0u) branch_out[0] = 0;
    if (oid_cap != 0u) oid_out[0] = 0;

    if (len > 5u && raf_git_has_prefix(head_text, len, "ref: ", 5u) != 0u) {
        state.kind = RAF_GIT_HEAD_SYMBOLIC;
        state.truncated |= raf_git_copy_span(
            ref_out, ref_cap, head_text, 5u, len - 5u
        );
        state.ref_valid = (RafU32)(state.truncated == 0u);

        branch_start = 5u;
        if (len >= 16u &&
            raf_git_has_prefix(head_text + 5u, len - 5u, "refs/heads/", 11u) != 0u)
            branch_start = 16u;

        if (branch_start < len) {
            state.truncated |= raf_git_copy_span(
                branch_out, branch_cap, head_text, branch_start, len - branch_start
            );
            state.branch_valid = (RafU32)(state.truncated == 0u);
        }
        return state;
    }

    if (raf_git_oid_span_valid(head_text, 0u, len) != 0u) {
        static const char detached[] = "HEAD";
        state.kind = RAF_GIT_HEAD_DETACHED;
        state.truncated |= raf_git_copy_span(
            branch_out, branch_cap, detached, 0u, 4u
        );
        state.truncated |= raf_git_copy_span(
            oid_out, oid_cap, head_text, 0u, len
        );
        state.branch_valid = (RafU32)(state.truncated == 0u);
        state.oid_valid = (RafU32)(state.truncated == 0u);
    }
    return state;
}

static inline RafU32 raf_git_parse_oid_line(
    const char *text,
    char *oid_out,
    RafU32 oid_cap
) {
    RafU32 len = raf_git_line_len(text);
    if (oid_cap != 0u) oid_out[0] = 0;
    if (raf_git_oid_span_valid(text, 0u, len) == 0u) return 0u;
    if (raf_git_copy_span(oid_out, oid_cap, text, 0u, len) != 0u) return 0u;
    return 1u;
}

static inline RafU32 raf_git_match_packed_ref(
    const char *line,
    const char *wanted_ref,
    char *oid_out,
    RafU32 oid_cap
) {
    RafU32 len = raf_git_line_len(line);
    RafU32 oid_len;
    RafU32 ref_start;
    RafU32 ref_len;

    if (oid_cap != 0u) oid_out[0] = 0;
    if (line == (const char *)0 || wanted_ref == (const char *)0 ||
        len == 0u || line[0] == '#' || line[0] == '^')
        return 0u;

    oid_len = (len > 40u && line[40] == ' ') ? 40u :
              ((len > 64u && line[64] == ' ') ? 64u : 0u);
    if (oid_len == 0u || raf_git_oid_span_valid(line, 0u, oid_len) == 0u)
        return 0u;

    ref_start = oid_len + 1u;
    if (ref_start >= len) return 0u;
    ref_len = len - ref_start;
    if (raf_git_span_equal(line, ref_start, ref_len, wanted_ref) == 0u)
        return 0u;

    if (raf_git_copy_span(oid_out, oid_cap, line, 0u, oid_len) != 0u)
        return 0u;
    return 1u;
}

#endif
