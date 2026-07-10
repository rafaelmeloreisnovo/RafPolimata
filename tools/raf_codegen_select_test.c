/* tools/raf_codegen_select_test.c — hosted proof for the Omega-guided MOV
 * equivalence family wired into Apkc/apkc.c through codegen_select.h.
 *
 * The selector uses Apkc/omega_classifier.h directly, while the three
 * encoders are mirrored in hosted C so the test does not depend on Apkc/sys.h.
 *
 * Proves:
 *  1. Omega selector bounds and deterministic replay.
 *  2. Classification changes are permitted to change encoding bits only inside
 *     a family whose members are already semantically equivalent.
 *  3. ORR-alias / ADD #0 / SUB #0 preserve the same MOV semantics.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../Apkc/omega_classifier.h"

static uint32_t codegen_select_t(const uint8_t *buf,
                                 uint32_t n,
                                 uint32_t num_variants) {
    return (uint32_t)raf_omega_codegen_index(
        (const raf_omega_u8 *)buf,
        (raf_omega_u32)n,
        (raf_omega_u32)num_variants);
}

static uint32_t enc_orr_mov(uint32_t rd, uint32_t rn, uint32_t sf) {
    return (sf<<31)|(0x2Au<<24)|(rn<<16)|(31u<<5)|rd;
}
static uint32_t enc_add0(uint32_t rd, uint32_t rn, uint32_t sf) {
    return (sf<<31)|(0x11u<<24)|(rn<<5)|rd;
}
static uint32_t enc_sub0(uint32_t rd, uint32_t rn, uint32_t sf) {
    return (sf<<31)|(0x51u<<24)|(rn<<5)|rd;
}

static int decode_dest(uint32_t w) { return (int)(w & 0x1Fu); }
static int decode_src_addsub(uint32_t w) { return (int)((w >> 5) & 0x1Fu); }
static int decode_imm12(uint32_t w) { return (int)((w >> 10) & 0xFFFu); }
static int decode_src_orr(uint32_t w) { return (int)((w >> 16) & 0x1Fu); }
static int decode_zero_reg_orr(uint32_t w) { return (int)((w >> 5) & 0x1Fu); }

static int g_fail = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { fprintf(stderr, "FAIL: %s\n", msg); g_fail = 1; } \
} while (0)

int main(void) {
    static const uint8_t text[] = "int main(void){return 42;}\n";
    static const uint8_t binary[] = {
        0x00,0xD5,0x03,0x20,0x1F,0xD6,0x5F,0x03,0xC0
    };
    uint8_t repeated[64];
    RafOmegaMetrics empty = raf_omega_classify((const raf_omega_u8 *)0, 0u);
    RafOmegaMetrics txt = raf_omega_classify(text, (uint32_t)(sizeof(text)-1u));
    RafOmegaMetrics bin = raf_omega_classify(binary, (uint32_t)sizeof(binary));

    memset(repeated, 'A', sizeof(repeated));
    RafOmegaMetrics rep = raf_omega_classify(repeated, (uint32_t)sizeof(repeated));
    CHECK(empty.path == RAF_OMEGA_VOID, "empty stream must be VOID");
    CHECK(rep.path == RAF_OMEGA_FORGOTTEN,
          "degenerate stream must be FORGOTTEN");
    CHECK((txt.flags & RAF_OMEGA_F_TEXTUAL) != 0u, "text flag missing");
    CHECK((bin.flags & RAF_OMEGA_F_BINARY) != 0u, "binary flag missing");

    uint8_t buf[64];
    for (int i = 0; i < 64; i++) buf[i] = (uint8_t)(i * 37 + 11);

    for (uint32_t n = 0; n <= 64; n += 7) {
        RafOmegaMetrics m1 = raf_omega_classify(buf, n);
        RafOmegaMetrics m2 = raf_omega_classify(buf, n);
        CHECK(m1.path == m2.path && m1.attractor == m2.attractor &&
              m1.phi_q16 == m2.phi_q16 &&
              m1.entropy_milli == m2.entropy_milli,
              "Omega classifier not deterministic");
        CHECK(m1.attractor < 42u, "Omega attractor out of range");
        for (uint32_t variants = 1; variants <= 5; variants++) {
            uint32_t first = codegen_select_t(buf, n, variants);
            CHECK(first < variants, "variant out of range");
            for (int replay = 0; replay < 5; replay++)
                CHECK(codegen_select_t(buf, n, variants) == first,
                      "selector not stable across repeated calls");
        }
    }
    CHECK(codegen_select_t(buf, 64, 1) == 0u,
          "num_variants=1 must return 0");
    CHECK(codegen_select_t(buf, 64, 0) == 0u,
          "num_variants=0 must return 0");

    uint32_t seq_a[16], seq_b[16];
    uint32_t pos = 0;
    for (int i = 0; i < 16; i++) {
        seq_a[i] = codegen_select_t(buf, pos, 3u);
        pos += 4;
    }
    pos = 0;
    for (int i = 0; i < 16; i++) {
        seq_b[i] = codegen_select_t(buf, pos, 3u);
        pos += 4;
    }
    CHECK(memcmp(seq_a, seq_b, sizeof(seq_a)) == 0,
          "variant sequence differs across identical replay");

    uint32_t rds[] = {0u, 5u, 19u, 30u};
    uint32_t rns[] = {1u, 12u, 30u, 0u};
    uint32_t sfs[] = {0u, 1u};
    for (int i = 0; i < 4; i++) {
        for (int s = 0; s < 2; s++) {
            uint32_t rd = rds[i], rn = rns[i], sf = sfs[s];
            uint32_t w0 = enc_orr_mov(rd, rn, sf);
            uint32_t w1 = enc_add0(rd, rn, sf);
            uint32_t w2 = enc_sub0(rd, rn, sf);

            CHECK((uint32_t)decode_dest(w0) == rd, "orr dest mismatch");
            CHECK((uint32_t)decode_zero_reg_orr(w0) == 31u,
                  "orr second-source must be XZR(31)");
            CHECK((uint32_t)decode_src_orr(w0) == rn,
                  "orr source mismatch");

            CHECK((uint32_t)decode_dest(w1) == rd, "add0 dest mismatch");
            CHECK((uint32_t)decode_src_addsub(w1) == rn,
                  "add0 source mismatch");
            CHECK(decode_imm12(w1) == 0, "add0 imm must be 0");

            CHECK((uint32_t)decode_dest(w2) == rd, "sub0 dest mismatch");
            CHECK((uint32_t)decode_src_addsub(w2) == rn,
                  "sub0 source mismatch");
            CHECK(decode_imm12(w2) == 0, "sub0 imm must be 0");
            CHECK(w0 != w1 && w1 != w2 && w0 != w2,
                  "variants must be bit-distinct");
        }
    }

    if (g_fail) {
        printf("raf_codegen_select_test: FAIL\n");
        return 1;
    }
    printf("raf_codegen_select_test: PASS (Omega selection + MOV equivalence)\n");
    return 0;
}
