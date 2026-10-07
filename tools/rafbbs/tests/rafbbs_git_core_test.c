#include "rafbbs_git_core.h"

static int raf_streq(const char *a, const char *b)
{
    RafU32 i = 0u;
    while (a[i] != 0 && b[i] != 0) {
        if (a[i] != b[i])
            return 0;
        ++i;
    }
    return a[i] == b[i];
}

static int rafbbs_git_core_test(void)
{
    char branch[64];
    char ref_path[128];
    char oid[16];
    char loose[160];
    RafGitHeadParse parsed;
    static const char symbolic[] = "ref: refs/heads/feature/x\n";
    static const char detached[] =
        "0123456789abcdef0123456789abcdef01234567\n";
    static const char loose_oid[] =
        "abcdef0123456789abcdef0123456789abcdef01\n";
    static const char packed[] =
        "# pack-refs with: peeled fully-peeled\n"
        "1111111111111111111111111111111111111111 refs/heads/other\n"
        "abcdef0123456789abcdef0123456789abcdef01 refs/heads/feature/x\n";

    parsed = raf_git_parse_head(
        symbolic, (RafU32)(sizeof(symbolic) - 1u),
        branch, (RafU32)sizeof(branch),
        ref_path, (RafU32)sizeof(ref_path),
        oid, (RafU32)sizeof(oid)
    );
    if (parsed.kind != RAF_GIT_HEAD_SYMBOLIC || parsed.dropped != 0u)
        return 1;
    if (!raf_streq(branch, "feature/x") ||
        !raf_streq(ref_path, "refs/heads/feature/x"))
        return 2;
    if (raf_git_build_loose_ref_path(
            ref_path, loose, (RafU32)sizeof(loose)
        ) != 0u ||
        !raf_streq(loose, ".git/refs/heads/feature/x"))
        return 3;
    if (!raf_git_parse_oid_line(
            loose_oid, (RafU32)(sizeof(loose_oid) - 1u),
            oid, (RafU32)sizeof(oid)
        ) || !raf_streq(oid, "abcdef0"))
        return 4;
    if (!raf_git_parse_packed_refs(
            packed, (RafU32)(sizeof(packed) - 1u), ref_path,
            oid, (RafU32)sizeof(oid)
        ) || !raf_streq(oid, "abcdef0"))
        return 5;

    parsed = raf_git_parse_head(
        detached, (RafU32)(sizeof(detached) - 1u),
        branch, (RafU32)sizeof(branch),
        ref_path, (RafU32)sizeof(ref_path),
        oid, (RafU32)sizeof(oid)
    );
    if (parsed.kind != RAF_GIT_HEAD_DETACHED ||
        !raf_streq(branch, "HEAD") || !raf_streq(oid, "0123456"))
        return 6;

    parsed = raf_git_parse_head(
        "ref: refs/heads/../escape\n", 25u,
        branch, (RafU32)sizeof(branch),
        ref_path, (RafU32)sizeof(ref_path),
        oid, (RafU32)sizeof(oid)
    );
    if (parsed.kind != RAF_GIT_HEAD_INVALID)
        return 7;

    return 0;
}

#ifdef RAFBBS_GIT_CORE_TEST_MAIN
int main(void)
{
    return rafbbs_git_core_test();
}
#endif
