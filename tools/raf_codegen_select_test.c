/* tools/raf_codegen_select_test.c — standalone host test for the MOV
 * equivalence family wired into Apkc/apkc.c's asm_insn64() via
 * Apkc/codegen_select.h.
 *
 * Reimplements phi_fst/phi_attractor/codegen_select and the three MOV
 * encoders in plain hosted C11 (same rationale as tools/raf_watt_proxy.c:
 * avoid mixing this host tool with the freestanding Apkc/sys.h syscall
 * layer). Kept in sync deliberately with Apkc/coherence.h,
 * Apkc/codegen_select.h and the case 0/1/2 block in Apkc/apkc.c's mov
 * handler.
 *
 * Proves:
 *  1. codegen_select() output is always in [0, num_variants) and stable
 *     across repeated calls with the same input (determinism).
 *  2. The three MOV-family encodings (ORR-alias / ADD #0 / SUB #0) all
 *     decode, per the ARM64 ISA bitfields, to the same destination
 *     register and source register with no other side effects.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t phi_fst_t(const uint8_t *buf, uint32_t n) {
    if (!n) return 0u;
    uint32_t freq[256] = {0};
    for (uint32_t i = 0u; i < n; i++) freq[buf[i]]++;
    uint32_t unique = 0u;
    for (int i = 0; i < 256; i++) if (freq[i]) unique++;
    uint32_t H = (unique * 0x10000u) / 256u;
    static const uint32_t KAM7[7] = {40503u,40503u,40503u,40503u,40503u,40503u,40503u};
    uint64_t dot = 0u, ns = 0u;
    for (int i = 0; i < 7; i++) { dot += (uint64_t)freq[i]*KAM7[i]; ns += (uint64_t)freq[i]*freq[i]; }
    uint32_t C = 0u;
    if (ns) { C = (uint32_t)((dot*0x10000u)/(ns|1u)); if (C > 0x10000u) C = 0x10000u; }
    uint32_t oneMinH = (H < 0x10000u) ? (0x10000u - H) : 0u;
    return (uint32_t)(((uint64_t)oneMinH * C) >> 16);
}
static uint32_t phi_attractor_t(uint32_t phi) { return (phi ^ (phi >> 7)) % 42u; }
static uint32_t codegen_select_t(const uint8_t *buf, uint32_t n, uint32_t num_variants) {
    if (num_variants <= 1u) return 0u;
    return phi_attractor_t(phi_fst_t(buf, n)) % num_variants;
}

/* Mirror of the three encoders dispatched in Apkc/apkc.c's mov handler. */
static uint32_t enc_orr_mov(uint32_t rd, uint32_t rn, uint32_t sf) {
    return (sf<<31)|(0x2Au<<24)|(rn<<16)|(31u<<5)|rd; /* ORR rd,xzr,rn */
}
static uint32_t enc_add0(uint32_t rd, uint32_t rn, uint32_t sf) {
    return (sf<<31)|(0x11u<<24)|(0u<<22)|(0u<<10)|(rn<<5)|rd; /* ADD rd,rn,#0 */
}
static uint32_t enc_sub0(uint32_t rd, uint32_t rn, uint32_t sf) {
    return (sf<<31)|(0x51u<<24)|(0u<<22)|(0u<<10)|(rn<<5)|rd; /* SUB rd,rn,#0 */
}

/* Decode the fields that matter for "did this leave rd==rn with no other
 * effect" and check them against what each encoding claims to do. */
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
    /* 1. codegen_select(): bounds + determinism across repeated calls. */
    uint8_t buf[64];
    for (int i = 0; i < 64; i++) buf[i] = (uint8_t)(i * 37 + 11);
    for (uint32_t n = 0; n <= 64; n += 7) {
        for (uint32_t variants = 1; variants <= 5; variants++) {
            uint32_t first = codegen_select_t(buf, n, variants);
            CHECK(first < variants || variants == 1, "variant out of range");
            for (int rep = 0; rep < 5; rep++) {
                uint32_t again = codegen_select_t(buf, n, variants);
                CHECK(again == first, "codegen_select not stable across repeated calls");
            }
        }
    }
    /* num_variants<=1 always returns 0 regardless of buffer content. */
    CHECK(codegen_select_t(buf, 64, 1) == 0u, "num_variants=1 must return 0");
    CHECK(codegen_select_t(buf, 64, 0) == 0u, "num_variants=0 must return 0");

    /* Different prefixes (as built up incrementally, mirroring em->pos
     * growing during real assembly) may pick different variants, but a
     * second pass over the identical byte sequence must reproduce the
     * exact same sequence of choices — this is the reproducible-build
     * property the equivalence family relies on. */
    uint32_t seq_a[16], seq_b[16];
    uint32_t pos = 0;
    for (int i = 0; i < 16; i++) { seq_a[i] = codegen_select_t(buf, pos, 3u); pos += 4; }
    pos = 0;
    for (int i = 0; i < 16; i++) { seq_b[i] = codegen_select_t(buf, pos, 3u); pos += 4; }
    CHECK(memcmp(seq_a, seq_b, sizeof(seq_a)) == 0, "variant sequence differs across identical re-run");

    /* 2. Semantic equivalence of the 3 MOV-family encodings for a sample
     * of (rd,rn,sf) triples. */
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
            CHECK((uint32_t)decode_zero_reg_orr(w0) == 31u, "orr second-source must be XZR(31)");
            CHECK((uint32_t)decode_src_orr(w0) == rn, "orr source mismatch");

            CHECK((uint32_t)decode_dest(w1) == rd, "add0 dest mismatch");
            CHECK((uint32_t)decode_src_addsub(w1) == rn, "add0 source mismatch");
            CHECK(decode_imm12(w1) == 0, "add0 imm must be 0");

            CHECK((uint32_t)decode_dest(w2) == rd, "sub0 dest mismatch");
            CHECK((uint32_t)decode_src_addsub(w2) == rn, "sub0 source mismatch");
            CHECK(decode_imm12(w2) == 0, "sub0 imm must be 0");

            /* opcodes themselves must differ — these are genuinely distinct
             * encodings, not the same bits relabeled. */
            CHECK(w0 != w1 && w1 != w2 && w0 != w2, "variants must be bit-distinct encodings");
        }
    }

    if (g_fail) { printf("raf_codegen_select_test: FAIL\n"); return 1; }
    printf("raf_codegen_select_test: PASS (codegen_select determinism + MOV-family semantic equivalence)\n");
    return 0;
}
