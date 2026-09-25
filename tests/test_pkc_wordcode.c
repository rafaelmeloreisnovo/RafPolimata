#include "../Pkc/pkc_wordcode.h"\n\n/* CLOSURE_L11: explicit lexicon gap is expected and fail-closed. */

static int test_pt(void) {
    static const char src[] =
        "MOVER R0 7\n"
        "MOVER R1 R0\n"
        "SOMAR R2 R0 R1\n"
        "RETORNAR\n";
    PkcInsnV1 out[8];
    PkcStatusV1 st;
    pkc_u32 n = 0u;
    if (pkc_parse_wordcode_v1(src, (pkc_u32)(sizeof(src) - 1u), PKC_LANG_PT,
                              out, 8u, &n, &st) != PKC_OK) return 1;
    if (n != 4u) return 2;
    if (out[0].op != PKC_OP_MOVE_IMM || out[0].rd != 0u || out[0].imm != 7u) return 3;
    if (out[1].op != PKC_OP_MOVE_REG || out[1].rd != 1u || out[1].rm != 0u) return 4;
    if (out[2].op != PKC_OP_ADD_REG || out[2].rd != 2u || out[2].rn != 0u || out[2].rm != 1u) return 5;
    if (out[3].op != PKC_OP_RETURN) return 6;
    return 0;
}

static int test_en(void) {
    static const char src[] =
        "MOVE R0 7\n"
        "MOVE R1 R0\n"
        "ADD R2 R0 R1\n"
        "RETURN\n";
    PkcInsnV1 out[8];
    PkcStatusV1 st;
    pkc_u32 n = 0u;
    if (pkc_parse_wordcode_v1(src, (pkc_u32)(sizeof(src) - 1u), PKC_LANG_EN,
                              out, 8u, &n, &st) != PKC_OK) return 10;
    if (n != 4u) return 11;
    if (out[0].op != PKC_OP_MOVE_IMM || out[0].imm != 7u) return 12;
    if (out[1].op != PKC_OP_MOVE_REG) return 13;
    if (out[2].op != PKC_OP_ADD_REG) return 14;
    if (out[3].op != PKC_OP_RETURN) return 15;
    return 0;
}

static int test_token_vazio(void) {
    static const char src[] = "x\n";
    PkcInsnV1 out[2];
    PkcStatusV1 st;
    pkc_u32 n = 0u;
    pkc_i32 rc = pkc_parse_wordcode_v1(src, 2u, PKC_LANG_GRC, out, 2u, &n, &st);
    if (rc != PKC_E_TOKEN_VAZIO_LEXICON) return 20;
    if ((st.warnings & PKC_WARN_TOKEN_VAZIO_LEXICON) == 0u) return 21;
    return 0;
}

static int test_contract(void) {
    PkcStatusV1 st;
    if (pkc_contract_evaluate(PKC_STRICT_REQUIREMENTS, &st) != PKC_OK) return 30;
    if (st.missing_requirements != 0ULL) return 31;
    if (pkc_contract_evaluate(PKC_STRICT_REQUIREMENTS & ~PKC_REQ_NO_TAILCALL, &st) == PKC_OK) return 32;
    if ((st.warnings & PKC_WARN_CONTRACT_INCOMPLETE) == 0u) return 33;
    if ((st.missing_requirements & PKC_REQ_NO_TAILCALL) == 0ULL) return 34;
    return 0;
}

int main(void) {
    int rc;
    rc = test_pt(); if (rc) return rc;
    rc = test_en(); if (rc) return rc;
    rc = test_token_vazio(); if (rc) return rc;
    rc = test_contract(); if (rc) return rc;
    return 0;
}
