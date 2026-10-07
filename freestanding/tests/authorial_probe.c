#include "../include/raf_fs_authorial.h"

typedef struct raf_fs_authorial_probe_state {
    raf_u32 input[4];
    raf_fs_authorial_item item;
    raf_u32 freestanding_ok;
} raf_fs_authorial_probe_state;

void raf_fs_authorial_probe(void *state) {
    raf_fs_authorial_probe_state *s = (raf_fs_authorial_probe_state *)state;
    raf_fs_authorial_words4 words;
    words.w0 = s->input[0];
    words.w1 = s->input[1];
    words.w2 = s->input[2];
    words.w3 = s->input[3];
    raf_fs_authorial_item4(&s->item, RAF_FS_AUTHORIAL_PROVIDER_RAFA, RAF_FS_AUTHORIAL_SOURCE_ELIA, words);
    s->freestanding_ok = raf_fs_authorial_item_is_freestanding(&s->item);
}
