#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../Apkc/omega_classifier.h"

static int failures = 0;
#define CHECK(c, m) do { \
    if (!(c)) { fprintf(stderr, "FAIL: %s\n", (m)); failures++; } \
} while (0)

static int metrics_equal(const RafOmegaMetrics *a, const RafOmegaMetrics *b) {
    return a->bytes == b->bytes && a->unique == b->unique &&
           a->transitions == b->transitions && a->printable == b->printable &&
           a->controls == b->controls && a->zeros == b->zeros &&
           a->fold_ones == b->fold_ones &&
           a->entropy_milli == b->entropy_milli &&
           a->coherence_q16 == b->coherence_q16 &&
           a->phi_q16 == b->phi_q16 && a->attractor == b->attractor &&
           a->flags == b->flags && a->path == b->path;
}

int main(void) {
    static const uint8_t source[] =
        "int main(void) { unsigned x = 42u; return (int)(x ^ (x >> 1)); }\n";
    static const uint8_t binary[] = {
        0x00,0xD5,0x03,0x20,0x1F,0xD6,0x5F,0x03,
        0xC0,0x14,0x00,0x00,0x00,0x94,0xFF,0xFF
    };
    uint8_t repeated[64];
    RafOmegaMetrics e = raf_omega_classify((const raf_omega_u8 *)0, 0u);
    RafOmegaMetrics s1 = raf_omega_classify(source,
                                             (uint32_t)(sizeof(source)-1u));
    RafOmegaMetrics s2 = raf_omega_classify(source,
                                             (uint32_t)(sizeof(source)-1u));
    RafOmegaMetrics b = raf_omega_classify(binary, (uint32_t)sizeof(binary));

    memset(repeated, 'A', sizeof(repeated));
    RafOmegaMetrics r = raf_omega_classify(repeated,
                                            (uint32_t)sizeof(repeated));

    CHECK(e.path == RAF_OMEGA_VOID, "empty stream must be VOID");
    CHECK(e.bytes == 0u && e.attractor == 0u,
          "empty metrics must be zeroed");
    CHECK(metrics_equal(&s1, &s2), "same source must classify identically");
    CHECK(s1.attractor < 42u, "source attractor out of range");
    CHECK(s1.entropy_milli <= 8000u, "entropy proxy out of range");
    CHECK(s1.phi_q16 <= 0x10000u, "phi out of range");
    CHECK((s1.flags & RAF_OMEGA_F_TEXTUAL) != 0u,
          "source should be textual");
    CHECK((b.flags & RAF_OMEGA_F_BINARY) != 0u,
          "binary vector should be binary");
    CHECK(r.path == RAF_OMEGA_FORGOTTEN,
          "degenerate repeated stream must be FORGOTTEN");

    for (uint32_t variants = 0u; variants <= 7u; variants++) {
        uint32_t a = raf_omega_codegen_index(source,
                                              (uint32_t)(sizeof(source)-1u),
                                              variants);
        uint32_t c = raf_omega_codegen_index(source,
                                              (uint32_t)(sizeof(source)-1u),
                                              variants);
        CHECK(a == c, "selector must be deterministic");
        CHECK(variants <= 1u ? a == 0u : a < variants,
              "selector out of range");
    }

    if (failures) return 1;
    printf("raf_omega_classifier_test: PASS path=%s attr=%u phi=%u entropy=%u\n",
           raf_omega_path_name(s1.path), s1.attractor,
           s1.phi_q16, s1.entropy_milli);
    return 0;
}
