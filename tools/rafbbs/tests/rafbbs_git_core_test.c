#include "rafbbs_git_core.h"

static int raf_git_test_equal(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

int rafbbs_git_core_test(void)
{
    char ref[128], branch[128], oid[65];
    RafGitHeadState state;

    state = raf_git_parse_head(
        "ref: refs/heads/main\n",
        ref, (RafU32)sizeof(ref),
        branch, (RafU32)sizeof(branch),
        oid, (RafU32)sizeof(oid)
    );
    if (state.kind != RAF_GIT_HEAD_SYMBOLIC ||
        state.ref_valid == 0u || state.branch_valid == 0u ||
        state.oid_valid != 0u || state.truncated != 0u)
        return 1;
    if (!raf_git_test_equal(ref, "refs/heads/main") ||
        !raf_git_test_equal(branch, "main") || oid[0] != 0)
        return 2;

    state = raf_git_parse_head(
        "0123456789abcdef0123456789abcdef01234567\r\n",
        ref, (RafU32)sizeof(ref),
        branch, (RafU32)sizeof(branch),
        oid, (RafU32)sizeof(oid)
    );
    if (state.kind != RAF_GIT_HEAD_DETACHED ||
        state.branch_valid == 0u || state.oid_valid == 0u)
        return 3;
    if (!raf_git_test_equal(branch, "HEAD") ||
        !raf_git_test_equal(oid, "0123456789abcdef0123456789abcdef01234567"))
        return 4;

    if (raf_git_parse_oid_line(
            "0123456789abcdef0123456789abcdef01234567\n",
            oid, (RafU32)sizeof(oid)
        ) == 0u)
        return 5;
    if (!raf_git_test_equal(oid, "0123456789abcdef0123456789abcdef01234567"))
        return 6;

    if (raf_git_match_packed_ref(
            "0123456789abcdef0123456789abcdef01234567 refs/heads/main\n",
            "refs/heads/main", oid, (RafU32)sizeof(oid)
        ) == 0u)
        return 7;
    if (raf_git_match_packed_ref(
            "0123456789abcdef0123456789abcdef01234567 refs/heads/other\n",
            "refs/heads/main", oid, (RafU32)sizeof(oid)
        ) != 0u)
        return 8;

    state = raf_git_parse_head(
        "not-a-git-oid\n",
        ref, (RafU32)sizeof(ref),
        branch, (RafU32)sizeof(branch),
        oid, (RafU32)sizeof(oid)
    );
    if (state.kind != RAF_GIT_HEAD_INVALID ||
        state.branch_valid != 0u || state.oid_valid != 0u)
        return 9;

    state = raf_git_parse_head(
        "ref: refs/heads/branch-name-that-is-too-long\n",
        ref, 8u, branch, 8u, oid, (RafU32)sizeof(oid)
    );
    if (state.truncated == 0u || state.ref_valid != 0u ||
        state.branch_valid != 0u)
        return 10;

    if (raf_git_parse_oid_line(
            "zz23456789abcdef0123456789abcdef01234567\n",
            oid, (RafU32)sizeof(oid)
        ) != 0u)
        return 11;

    return 0;
}

#if defined(RAFBBS_GIT_CORE_TEST_MAIN)
int main(void) { return rafbbs_git_core_test(); }
#endif
