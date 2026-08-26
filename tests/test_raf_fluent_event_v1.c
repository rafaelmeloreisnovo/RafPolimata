#include "../Apkc/raf_fluent_event.h"

int main(void) {
    u8 out[1024];
    RafFluentEventV1 e;
    e.tag = "rafaelia.build";
    e.event = "artifact_emitted";
    e.component = "RafPolimata";
    e.arch = "arm32";
    e.artifact_kind = "ELF32";
    e.state = "STATIC_TEST";
    e.source_sha256 = "TOKEN_VAZIO";
    e.artifact_sha256 = "TOKEN_VAZIO";
    e.time_unix_s = 0;
    e.seq = 1;
    e.claim_allowed = 0;

    sz n = raf_fluent_encode_event_v1(out, sizeof(out), &e);
    if (!n) return 1;
    if (out[0] != 0x93u) return 2; /* Forward Message-mode array(3) */
    if (out[1] != 0xAEu) return 3; /* fixstr(14): rafaelia.build */

    /* Capacity failure must fail closed, never truncate into a valid-looking event. */
    u8 tiny[8];
    if (raf_fluent_encode_event_v1(tiny, sizeof(tiny), &e) != 0) return 4;

    return 0;
}
