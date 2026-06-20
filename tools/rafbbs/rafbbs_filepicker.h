#ifndef RAFBBS_FILEPICKER_H
#define RAFBBS_FILEPICKER_H
#include <stdio.h>
#include <string.h>

#define RAFBBS_PICKER_COUNT 5
#define RAFBBS_PICKER_PATH 128

typedef struct {
    char selected[RAFBBS_PICKER_PATH];
    const char *items[RAFBBS_PICKER_COUNT];
} RafFilePicker;

static inline void raf_filepicker_init(RafFilePicker *p) {
    p->items[0] = "Apkc/hello.s.txt";
    p->items[1] = "tests/test_arm64_encoders.py";
    p->items[2] = "tests/test_asm_roundtrip.sh";
    p->items[3] = "scripts/apkc_validate.sh";
    p->items[4] = "proofs/run-arm64-full-chain/manifest.template.json";
    snprintf(p->selected, sizeof(p->selected), "%s", p->items[0]);
}

static inline void raf_filepicker_print(const RafFilePicker *p) {
    int i;
    for (i = 0; i < RAFBBS_PICKER_COUNT; i++) printf("%d %s%s\n", i + 1, p->items[i], strcmp(p->selected, p->items[i]) == 0 ? " <" : "");
}

static inline const char *raf_filepicker_select(RafFilePicker *p, int choice) {
    int idx = choice - 1;
    if (idx < 0 || idx >= RAFBBS_PICKER_COUNT) return p->selected;
    snprintf(p->selected, sizeof(p->selected), "%s", p->items[idx]);
    return p->selected;
}
#endif
