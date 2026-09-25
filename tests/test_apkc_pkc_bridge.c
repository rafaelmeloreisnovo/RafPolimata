#include "../Apkc/apkc_pkc_bridge.h"

/* CLOSURE_L6: deterministic PT/EN lowering parity gate. */

static int same_pkc(const PkcInsnV1 *a, const PkcInsnV1 *b, u32 n) {
    u32 i;
    for (i = 0u; i < n; ++i) {
        if (a[i].op != b[i].op ||
            a[i].rd != b[i].rd ||
            a[i].rn != b[i].rn ||
            a[i].rm != b[i].rm ||
            a[i].imm != b[i].imm) return 0;
    }
    return 1;
}

static int same_rafir(const RafIr32InsnV1 *a, const RafIr32InsnV1 *b, u32 n) {
    u32 i;
    for (i = 0u; i < n; ++i) {
        if (a[i].op != b[i].op ||
            a[i].rd != b[i].rd ||
            a[i].rn != b[i].rn ||
            a[i].rm != b[i].rm ||
            a[i].imm != b[i].imm) return 0;
    }
    return 1;
}

static int same_bytes(const u8 *a, const u8 *b, u32 n) {
    u32 i;
    for (i = 0u; i < n; ++i) if (a[i] != b[i]) return 0;
    return 1;
}

int main(void) {
    static const char pt[] =
        "MOVER R0 7\n"
        "MOVER R1 3\n"
        "SOMAR R2 R0 R1\n"
        "MULTIPLICAR R3 R2 R1\n"
        "RETORNAR\n";
    static const char en[] =
        "MOVE R0 7\n"
        "MOVE R1 3\n"
        "ADD R2 R0 R1\n"
        "MULTIPLY R3 R2 R1\n"
        "RETURN\n";

    PkcInsnV1 p_pt[8], p_en[8];
    PkcStatusV1 s_pt, s_en;
    u32 n_pt = 0u, n_en = 0u;
    RafIr32InsnV1 r_pt[8], r_en[8];
    u8 a32_pt[64], a32_en[64];
    u32 a32_pt_n = 0u, a32_en_n = 0u;

    /* fmt_elf.h owns both target writers; keep the sibling reference visible
     * under strict warning builds without calling it. */
    (void)elf64_build_so;

    if (pkc_parse_wordcode_v1(pt, (u32)(sizeof(pt) - 1u), PKC_LANG_PT,
                              p_pt, 8u, &n_pt, &s_pt) != PKC_OK) return 1;
    if (pkc_parse_wordcode_v1(en, (u32)(sizeof(en) - 1u), PKC_LANG_EN,
                              p_en, 8u, &n_en, &s_en) != PKC_OK) return 2;
    if (n_pt != n_en || n_pt != 5u) return 3;
    if (!same_pkc(p_pt, p_en, n_pt)) return 4;

    if (apkc_pkc_to_rafir_v1(p_pt, n_pt, r_pt, 8u) != 0) return 5;
    if (apkc_pkc_to_rafir_v1(p_en, n_en, r_en, 8u) != 0) return 6;
    if (!same_rafir(r_pt, r_en, n_pt)) return 7;

    if (raf_stage0_ir32_encode(r_pt, n_pt, a32_pt, (u32)sizeof(a32_pt), &a32_pt_n) != 0) return 8;
    if (raf_stage0_ir32_encode(r_en, n_en, a32_en, (u32)sizeof(a32_en), &a32_en_n) != 0) return 9;
    if (a32_pt_n != a32_en_n || a32_pt_n != 20u) return 10;
    if (!same_bytes(a32_pt, a32_en, a32_pt_n)) return 11;

    /* Exact first and last A32 words: MOV r0,#7 ; ... ; BX lr. */
    if (a32_pt[0] != 0x07u || a32_pt[1] != 0x00u ||
        a32_pt[2] != 0xA0u || a32_pt[3] != 0xE3u) return 12;
    if (a32_pt[16] != 0x1Eu || a32_pt[17] != 0xFFu ||
        a32_pt[18] != 0x2Fu || a32_pt[19] != 0xE1u) return 13;

    return 0;
}
