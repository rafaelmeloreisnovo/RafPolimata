#ifndef RAFBBS_FILEPICKER_H
#define RAFBBS_FILEPICKER_H

/*
 * Hosted presentation adapter.
 * Selection state and the canonical static catalog live in the authorial
 * zero-dependency rafbbs_filepicker_core.h.
 */
#include <stdio.h>
#include "rafbbs_filepicker_core.h"

typedef struct {
    RafFilePickerCore core;
    const char *selected;
} RafFilePicker;

static inline void raf_filepicker_init(RafFilePicker *p)
{
    p->core = raf_filepicker_core_init();
    p->selected = raf_filepicker_core_selected(&p->core);
}

static inline void raf_filepicker_print(const RafFilePicker *p)
{
    RafU32 i;
    for (i = 0u; i < raf_filepicker_core_count(); i++) {
        const char *item = raf_filepicker_core_item(i);
        printf("%u %s%s\n",
               (unsigned int)(i + 1u),
               item,
               raf_filepicker_core_is_selected(&p->core, i) ? " <" : "");
    }
}

static inline const char *raf_filepicker_select(RafFilePicker *p, int choice)
{
    RafU32 one_based = choice > 0 ? (RafU32)choice : 0u;
    (void)raf_filepicker_core_select(&p->core, one_based);
    p->selected = raf_filepicker_core_selected(&p->core);
    return p->selected;
}

#endif
