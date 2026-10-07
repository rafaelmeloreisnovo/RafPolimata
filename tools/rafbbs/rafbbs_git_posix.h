#ifndef RAFBBS_GIT_POSIX_H
#define RAFBBS_GIT_POSIX_H

#include <stdio.h>
#include "rafbbs_git_core.h"
#include "rafbbs_log_core.h"

#define RAFBBS_GIT_OBS_BRANCH 1u
#define RAFBBS_GIT_OBS_COMMIT 2u
#define RAFBBS_GIT_OBS_LOOSE_REF 4u
#define RAFBBS_GIT_OBS_PACKED_REF 8u
#define RAFBBS_GIT_OBS_DETACHED 16u

static int raf_git_posix_read_line(
    const char *path,
    char *buf,
    RafU32 cap
) {
    FILE *f;
    if (cap == 0u) return -1;
    buf[0] = 0;
    f = fopen(path, "r");
    if (f == (FILE *)0) return -1;
    if (fgets(buf, (int)cap, f) == (char *)0) {
        (void)fclose(f);
        buf[0] = 0;
        return -1;
    }
    if (fclose(f) != 0) return -1;
    return 0;
}

static int raf_git_posix_ref_path(
    char *path,
    RafU32 cap,
    const char *ref
) {
    RafLogText out;
    raf_log_text_init(&out, path, cap);
    raf_log_text_puts(&out, ".git/");
    raf_log_text_puts(&out, ref);
    return out.dropped == 0u ? 0 : -1;
}

static RafU32 raf_git_posix_observe(
    char *branch_out,
    RafU32 branch_cap,
    char *commit_out,
    RafU32 commit_cap
) {
    char head_line[256];
    char ref[192];
    char oid[65];
    char path[256];
    char line[384];
    RafGitHeadState head;
    RafU32 state = 0u;
    FILE *packed;

    if (branch_cap != 0u) branch_out[0] = 0;
    if (commit_cap != 0u) commit_out[0] = 0;

    if (raf_git_posix_read_line(
            ".git/HEAD", head_line, (RafU32)sizeof(head_line)
        ) != 0)
        return 0u;

    head = raf_git_parse_head(
        head_line,
        ref, (RafU32)sizeof(ref),
        branch_out, branch_cap,
        oid, (RafU32)sizeof(oid)
    );
    if (head.branch_valid != 0u) state |= RAFBBS_GIT_OBS_BRANCH;

    if (head.kind == RAF_GIT_HEAD_DETACHED && head.oid_valid != 0u) {
        if (raf_git_copy_span(
                commit_out, commit_cap, oid, 0u, raf_git_line_len(oid)
            ) == 0u) {
            state |= RAFBBS_GIT_OBS_COMMIT;
            state |= RAFBBS_GIT_OBS_DETACHED;
        }
        return state;
    }

    if (head.kind != RAF_GIT_HEAD_SYMBOLIC || head.ref_valid == 0u)
        return state;

    if (raf_git_posix_ref_path(path, (RafU32)sizeof(path), ref) == 0 &&
        raf_git_posix_read_line(path, line, (RafU32)sizeof(line)) == 0 &&
        raf_git_parse_oid_line(line, commit_out, commit_cap) != 0u) {
        state |= RAFBBS_GIT_OBS_COMMIT;
        state |= RAFBBS_GIT_OBS_LOOSE_REF;
        return state;
    }

    packed = fopen(".git/packed-refs", "r");
    if (packed == (FILE *)0) return state;
    while (fgets(line, (int)sizeof(line), packed) != (char *)0) {
        if (raf_git_match_packed_ref(line, ref, commit_out, commit_cap) != 0u) {
            state |= RAFBBS_GIT_OBS_COMMIT;
            state |= RAFBBS_GIT_OBS_PACKED_REF;
            break;
        }
    }
    (void)fclose(packed);
    return state;
}

#endif
