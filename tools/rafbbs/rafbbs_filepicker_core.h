#ifndef RAFBBS_FILEPICKER_CORE_H
#define RAFBBS_FILEPICKER_CORE_H

#include "rafbbs_types.h"

#define RAFBBS_PICKER_COUNT 5u

typedef struct {
    RafU32 selected_index;
} RafFilePickerCore;

static const char *const raf_filepicker_catalog[RAFBBS_PICKER_COUNT] = {
    "Apkc/hello.s.txt",
    "tests/test_arm64_encoders.py",
    "tests/test_asm_roundtrip.sh",
    "scripts/apkc_validate.sh",
    "proofs/run-arm64-full-chain/manifest.template.json"
};

static inline RafU32 raf_filepicker_core_count(void)
{
    return RAFBBS_PICKER_COUNT;
}

static inline RafFilePickerCore raf_filepicker_core_init(void)
{
    RafFilePickerCore p;
    p.selected_index = 0u;
    return p;
}

static inline const char *raf_filepicker_core_item(RafU32 index)
{
    return index < RAFBBS_PICKER_COUNT ?
        raf_filepicker_catalog[index] : (const char *)0;
}

static inline RafU32
raf_filepicker_core_select(RafFilePickerCore *p, RafU32 choice_one_based)
{
    RafU32 candidate = choice_one_based - 1u;
    RafU32 valid = (RafU32)(choice_one_based != 0u) &
                   (RafU32)(candidate < RAFBBS_PICKER_COUNT);

    p->selected_index = valid ? candidate : p->selected_index;
    return p->selected_index;
}

static inline const char *
raf_filepicker_core_selected(const RafFilePickerCore *p)
{
    return raf_filepicker_catalog[p->selected_index];
}

static inline RafU32
raf_filepicker_core_is_selected(const RafFilePickerCore *p, RafU32 index)
{
    return (RafU32)(index < RAFBBS_PICKER_COUNT) &
           (RafU32)(p->selected_index == index);
}

#endif
