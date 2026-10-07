#ifndef RAFBBS_RECENT_CORE_H
#define RAFBBS_RECENT_CORE_H

#include "rafbbs_log_core.h"

#define RAFBBS_RECENT_LIMIT 10u
#define RAFBBS_RECENT_NAME_CAP 128u

typedef struct {
    char names[RAFBBS_RECENT_LIMIT][RAFBBS_RECENT_NAME_CAP];
    RafU32 count;
    RafU32 dropped;
} RafRecentCatalog;

static inline RafU32 raf_recent_len(const char *text)
{
    RafU32 n = 0u;
    if (text == (const char *)0) return 0u;
    while (text[n] != 0) n++;
    return n;
}

static inline int raf_recent_cmp(const char *a, const char *b)
{
    RafU32 i = 0u;
    while (a[i] != 0 && b[i] != 0 && a[i] == b[i]) i++;
    return (int)(unsigned char)a[i] - (int)(unsigned char)b[i];
}

static inline RafU32 raf_recent_has_prefix(
    const char *name,
    const char *prefix
) {
    RafU32 i = 0u;
    if (name == (const char *)0 || prefix == (const char *)0) return 0u;
    while (prefix[i] != 0) {
        if (name[i] != prefix[i]) return 0u;
        i++;
    }
    return 1u;
}

static inline RafU32 raf_recent_has_suffix(
    const char *name,
    const char *suffix
) {
    RafU32 name_len = raf_recent_len(name);
    RafU32 suffix_len = raf_recent_len(suffix);
    RafU32 i;
    if (suffix_len > name_len) return 0u;
    for (i = 0u; i < suffix_len; i++)
        if (name[name_len - suffix_len + i] != suffix[i]) return 0u;
    return 1u;
}

static inline RafU32 raf_recent_copy(char *dst, const char *src)
{
    RafU32 i = 0u;
    if (src == (const char *)0) {
        dst[0] = 0;
        return 0u;
    }
    while (src[i] != 0 && i + 1u < RAFBBS_RECENT_NAME_CAP) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
    return (RafU32)(src[i] != 0);
}

static inline void raf_recent_swap(char *a, char *b)
{
    char tmp[RAFBBS_RECENT_NAME_CAP];
    RafU32 i;
    for (i = 0u; i < RAFBBS_RECENT_NAME_CAP; i++) tmp[i] = a[i];
    for (i = 0u; i < RAFBBS_RECENT_NAME_CAP; i++) a[i] = b[i];
    for (i = 0u; i < RAFBBS_RECENT_NAME_CAP; i++) b[i] = tmp[i];
}

static inline void raf_recent_sort(RafRecentCatalog *catalog)
{
    RafU32 i;
    RafU32 j;
    for (i = 1u; i < catalog->count; i++) {
        j = i;
        while (j > 0u &&
               raf_recent_cmp(catalog->names[j - 1u], catalog->names[j]) > 0) {
            raf_recent_swap(catalog->names[j - 1u], catalog->names[j]);
            j--;
        }
    }
}

static inline void raf_recent_init(RafRecentCatalog *catalog)
{
    RafU32 i;
    RafU32 j;
    catalog->count = 0u;
    catalog->dropped = 0u;
    for (i = 0u; i < RAFBBS_RECENT_LIMIT; i++)
        for (j = 0u; j < RAFBBS_RECENT_NAME_CAP; j++)
            catalog->names[i][j] = 0;
}

static inline RafU32 raf_recent_offer(
    RafRecentCatalog *catalog,
    const char *name,
    const char *prefix,
    const char *suffix
) {
    if (name == (const char *)0 ||
        raf_recent_has_prefix(name, prefix) == 0u ||
        raf_recent_has_suffix(name, suffix) == 0u)
        return 0u;

    if (raf_recent_len(name) + 1u > RAFBBS_RECENT_NAME_CAP) {
        catalog->dropped += 1u;
        return 0u;
    }

    if (catalog->count < RAFBBS_RECENT_LIMIT) {
        if (raf_recent_copy(catalog->names[catalog->count], name) != 0u) {
            catalog->dropped += 1u;
            return 0u;
        }
        catalog->count += 1u;
        raf_recent_sort(catalog);
        return 1u;
    }

    if (raf_recent_cmp(name, catalog->names[0]) <= 0)
        return 0u;

    if (raf_recent_copy(catalog->names[0], name) != 0u) {
        catalog->dropped += 1u;
        return 0u;
    }
    raf_recent_sort(catalog);
    return 1u;
}

static inline void raf_recent_render(
    RafLogText *out,
    const RafRecentCatalog *catalog
) {
    RafU32 i;
    for (i = 0u; i < catalog->count; i++) {
        raf_log_text_puts(out, catalog->names[i]);
        raf_log_text_putc(out, '\n');
    }
}

#endif
