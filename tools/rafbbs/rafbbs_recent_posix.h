#ifndef RAFBBS_RECENT_POSIX_H
#define RAFBBS_RECENT_POSIX_H

#include <dirent.h>
#include <stdio.h>
#include "rafbbs_recent_core.h"

static int raf_recent_posix_print(
    const char *directory,
    const char *prefix,
    const char *suffix
) {
    DIR *dir;
    struct dirent *entry;
    RafRecentCatalog catalog;
    RafLogText out;
    char rendered[
        RAFBBS_RECENT_LIMIT * (RAFBBS_RECENT_NAME_CAP + 1u) + 1u
    ];

    raf_recent_init(&catalog);
    dir = opendir(directory);
    if (dir == (DIR *)0) return -1;

    while ((entry = readdir(dir)) != (struct dirent *)0)
        (void)raf_recent_offer(&catalog, entry->d_name, prefix, suffix);

    if (closedir(dir) != 0) return -1;
    if (catalog.dropped != 0u) return -1;

    raf_log_text_init(&out, rendered, (RafU32)sizeof(rendered));
    raf_recent_render(&out, &catalog);
    if (out.dropped != 0u) return -1;
    if (out.pos == 0u) return 0;

    return fwrite(rendered, 1u, (size_t)out.pos, stdout) == (size_t)out.pos
        ? 0 : -1;
}

#endif
