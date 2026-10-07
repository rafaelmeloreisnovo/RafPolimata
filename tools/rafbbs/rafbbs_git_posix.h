#ifndef RAFBBS_GIT_POSIX_H
#define RAFBBS_GIT_POSIX_H

#include <stdio.h>
#include "rafbbs_git_core.h"

#define RAF_GIT_OBS_BRANCH 1u
#define RAF_GIT_OBS_COMMIT 2u

static RafU32 raf_git_posix_read(
    const char *path, char *dst, RafU32 cap, RafU32 *len_out
)
{
    FILE *f;
    size_t n;
    if (len_out != (RafU32 *)0)
        *len_out = 0u;
    if (cap < 2u)
        return 0u;
    f = fopen(path, "rb");
    if (f == (FILE *)0)
        return 0u;
    n = fread(dst, 1u, (size_t)(cap - 1u), f);
    if (ferror(f)) {
        fclose(f);
        dst[0] = 0;
        return 0u;
    }
    if (!feof(f)) {
        fclose(f);
        dst[0] = 0;
        return 0u;
    }
    fclose(f);
    dst[n] = 0;
    if (len_out != (RafU32 *)0)
        *len_out = (RafU32)n;
    return 1u;
}

static RafU32 raf_git_posix_observe(
    char *branch, RafU32 branch_cap,
    char *commit, RafU32 commit_cap
)
{
    char head[256];
    char ref_path[192];
    char loose_path[224];
    char oid_text[128];
    char packed[8192];
    RafU32 head_len = 0u;
    RafU32 oid_len = 0u;
    RafU32 packed_len = 0u;
    RafU32 state = 0u;
    RafGitHeadParse parsed;

    if (branch_cap != 0u)
        branch[0] = 0;
    if (commit_cap != 0u)
        commit[0] = 0;
    if (!raf_git_posix_read(
            ".git/HEAD", head, (RafU32)sizeof(head), &head_len
        ))
        return 0u;

    parsed = raf_git_parse_head(
        head, head_len,
        branch, branch_cap,
        ref_path, (RafU32)sizeof(ref_path),
        commit, commit_cap
    );
    if (parsed.kind == RAF_GIT_HEAD_INVALID || parsed.dropped != 0u)
        return 0u;

    state |= RAF_GIT_OBS_BRANCH;
    if (parsed.kind == RAF_GIT_HEAD_DETACHED)
        return state | RAF_GIT_OBS_COMMIT;

    if (raf_git_build_loose_ref_path(
            ref_path, loose_path, (RafU32)sizeof(loose_path)
        ) == 0u &&
        raf_git_posix_read(
            loose_path, oid_text, (RafU32)sizeof(oid_text), &oid_len
        ) &&
        raf_git_parse_oid_line(oid_text, oid_len, commit, commit_cap))
        return state | RAF_GIT_OBS_COMMIT;

    if (raf_git_posix_read(
            ".git/packed-refs", packed, (RafU32)sizeof(packed), &packed_len
        ) &&
        raf_git_parse_packed_refs(
            packed, packed_len, ref_path, commit, commit_cap
        ))
        return state | RAF_GIT_OBS_COMMIT;

    return state;
}

#endif
