#ifndef RAFBBS_GIT_CORE_H
#define RAFBBS_GIT_CORE_H

#include "rafbbs_types.h"

#define RAF_GIT_HEAD_INVALID 0u
#define RAF_GIT_HEAD_SYMBOLIC 1u
#define RAF_GIT_HEAD_DETACHED 2u
#define RAF_GIT_SHORT_OID_LEN 7u

typedef struct {
    RafU32 kind;
    RafU32 dropped;
} RafGitHeadParse;

static inline RafU32 raf_git_line_len(const char *text, RafU32 len)
{
    RafU32 i = 0u;
    if (text == (const char *)0)
        return 0u;
    while (i < len && text[i] != 0 && text[i] != '\n' && text[i] != '\r')
        ++i;
    return i;
}

static inline RafU32 raf_git_text_len(const char *text)
{
    RafU32 n = 0u;
    if (text == (const char *)0)
        return 0u;
    while (text[n] != 0)
        ++n;
    return n;
}

static inline RafU32 raf_git_copy_range(
    char *dst, RafU32 cap, const char *src, RafU32 len
)
{
    RafU32 i = 0u;
    RafU32 dropped = 0u;
    if (cap == 0u)
        return (RafU32)(len != 0u);
    while (i < len) {
        if (i + 1u < cap)
            dst[i] = src[i];
        else
            dropped = 1u;
        ++i;
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

static inline RafU32 raf_git_ref_is_safe(const char *ref, RafU32 len)
{
    RafU32 i;
    if (ref == (const char *)0 || len <= 11u)
        return 0u;
    if (!(ref[0] == 'r' && ref[1] == 'e' && ref[2] == 'f' &&
          ref[3] == 's' && ref[4] == '/' && ref[5] == 'h' &&
          ref[6] == 'e' && ref[7] == 'a' && ref[8] == 'd' &&
          ref[9] == 's' && ref[10] == '/'))
        return 0u;
    if (ref[11] == '/' || ref[11] == '.')
        return 0u;
    if (ref[len - 1u] == '/' || ref[len - 1u] == '.')
        return 0u;

    for (i = 0u; i < len; ++i) {
        char c = ref[i];
        RafU32 ok = (RafU32)(
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '/' || c == '_' || c == '-' || c == '.'
        );
        if (!ok)
            return 0u;
        if (i + 1u < len) {
            if ((c == '.' && ref[i + 1u] == '.') ||
                (c == '/' && ref[i + 1u] == '/'))
                return 0u;
        }
    }
    return 1u;
}

static inline RafU32 raf_git_parse_oid_line(
    const char *text, RafU32 len, char *short_oid, RafU32 short_cap
)
{
    RafU32 line = raf_git_line_len(text, len);
    RafU32 i;
    if (line != 40u && line != 64u) {
        if (short_cap != 0u)
            short_oid[0] = 0;
        return 0u;
    }
    for (i = 0u; i < line; ++i) {
        if (!raf_git_is_hex(text[i])) {
            if (short_cap != 0u)
                short_oid[0] = 0;
            return 0u;
        }
    }
    if (raf_git_copy_range(
            short_oid, short_cap, text, RAF_GIT_SHORT_OID_LEN
        ) != 0u)
        return 0u;
    return 1u;
}

static inline RafU32 raf_git_build_loose_ref_path(
    const char *ref_path, char *dst, RafU32 cap
)
{
    static const char prefix[] = ".git/";
    RafU32 ref_len = raf_git_text_len(ref_path);
    RafU32 i = 0u;
    RafU32 pos = 0u;
    RafU32 dropped = 0u;
    if (!raf_git_ref_is_safe(ref_path, ref_len)) {
        if (cap != 0u)
            dst[0] = 0;
        return 1u;
    }
    if (cap != 0u)
        dst[0] = 0;
    while (prefix[i] != 0) {
        if (cap != 0u && pos + 1u < cap)
            dst[pos] = prefix[i];
        else
            dropped = 1u;
        ++pos;
        ++i;
    }
    for (i = 0u; i < ref_len; ++i) {
        if (cap != 0u && pos + 1u < cap)
            dst[pos] = ref_path[i];
        else
            dropped = 1u;
        ++pos;
    }
    if (cap != 0u)
        dst[(pos < cap) ? pos : (cap - 1u)] = 0;
    return dropped;
}

static inline RafGitHeadParse raf_git_parse_head(
    const char *text, RafU32 len,
    char *branch, RafU32 branch_cap,
    char *ref_path, RafU32 ref_cap,
    char *short_oid, RafU32 oid_cap
)
{
    static const char sym_prefix[] = "ref: ";
    static const char head_name[] = "HEAD";
    RafGitHeadParse out = {RAF_GIT_HEAD_INVALID, 0u};
    RafU32 line = raf_git_line_len(text, len);
    RafU32 i;

    if (branch_cap != 0u)
        branch[0] = 0;
    if (ref_cap != 0u)
        ref_path[0] = 0;
    if (oid_cap != 0u)
        short_oid[0] = 0;
    if (line == 0u)
        return out;

    if (line >= 5u) {
        RafU32 prefix_ok = 1u;
        for (i = 0u; i < 5u; ++i)
            prefix_ok &= (RafU32)(text[i] == sym_prefix[i]);
        if (prefix_ok != 0u) {
            const char *ref = text + 5u;
            RafU32 ref_len = line - 5u;
            if (!raf_git_ref_is_safe(ref, ref_len))
                return out;
            out.kind = RAF_GIT_HEAD_SYMBOLIC;
            out.dropped |= raf_git_copy_range(
                ref_path, ref_cap, ref, ref_len
            );
            out.dropped |= raf_git_copy_range(
                branch, branch_cap, ref + 11u, ref_len - 11u
            );
            return out;
        }
    }

    if (raf_git_parse_oid_line(text, line, short_oid, oid_cap) != 0u) {
        out.kind = RAF_GIT_HEAD_DETACHED;
        out.dropped |= raf_git_copy_range(
            branch, branch_cap, head_name, 4u
        );
    }
    return out;
}

static inline RafU32 raf_git_parse_packed_refs(
    const char *text, RafU32 len, const char *wanted_ref,
    char *short_oid, RafU32 oid_cap
)
{
    RafU32 wanted_len = raf_git_text_len(wanted_ref);
    RafU32 pos = 0u;
    if (!raf_git_ref_is_safe(wanted_ref, wanted_len)) {
        if (oid_cap != 0u)
            short_oid[0] = 0;
        return 0u;
    }

    while (pos < len) {
        RafU32 start = pos;
        RafU32 line_len;
        RafU32 oid_len;
        RafU32 i;
        while (pos < len && text[pos] != '\n' && text[pos] != '\r')
            ++pos;
        line_len = pos - start;
        while (pos < len && (text[pos] == '\n' || text[pos] == '\r'))
            ++pos;
        if (line_len == 0u || text[start] == '#' || text[start] == '^')
            continue;

        oid_len = 0u;
        while (oid_len < line_len && text[start + oid_len] != ' ')
            ++oid_len;
        if ((oid_len != 40u && oid_len != 64u) ||
            oid_len + 1u + wanted_len != line_len)
            continue;
        for (i = 0u; i < oid_len; ++i)
            if (!raf_git_is_hex(text[start + i]))
                break;
        if (i != oid_len)
            continue;
        for (i = 0u; i < wanted_len; ++i)
            if (text[start + oid_len + 1u + i] != wanted_ref[i])
                break;
        if (i != wanted_len)
            continue;
        return raf_git_parse_oid_line(
            text + start, oid_len, short_oid, oid_cap
        );
    }

    if (oid_cap != 0u)
        short_oid[0] = 0;
    return 0u;
}

#endif
