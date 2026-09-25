/*
 * Pkc/pkc_wordcode.h — bounded VERB + NOUN frontend V1.
 *
 * Copyright (c) 2024-2026 Rafael Melo Reis.
 * License status: TOKEN_VAZIO_FINAL_LEGAL_TEXT\n * Governance: CLOSURE_L11 (lexicon gaps) + CLOSURE_L12 (license gap)
 *
 * Properties:
 *   - no includes
 *   - no libc / libm / heap / syscall
 *   - caller-owned output
 *   - deterministic
 *   - human lexicon does not need to be retained as binary strings
 *
 * V1 vocabulary:
 *   PT: MOVER, SOMAR, SUBTRAIR, MULTIPLICAR, RETORNAR
 *   EN: MOVE, ADD, SUBTRACT, MULTIPLY, RETURN
 */
#ifndef RAF_PKC_WORDCODE_H
#define RAF_PKC_WORDCODE_H 1

#include "pkc_contract.h"

#define PKC_OP_MOVE_IMM  1u
#define PKC_OP_MOVE_REG  2u
#define PKC_OP_ADD_REG   3u
#define PKC_OP_SUB_REG   4u
#define PKC_OP_MUL_REG   5u
#define PKC_OP_RETURN    6u

typedef struct {
    pkc_u8 op;
    pkc_u8 rd;
    pkc_u8 rn;
    pkc_u8 rm;
    pkc_u32 imm;
} PkcInsnV1;

static inline pkc_u8 pkc_ascii_upper(pkc_u8 c) {
    return (c >= (pkc_u8)'a' && c <= (pkc_u8)'z')
        ? (pkc_u8)(c - ((pkc_u8)'a' - (pkc_u8)'A'))
        : c;
}

/* FNV-1a 32-bit over uppercase ASCII token bytes. */
static inline pkc_u32 pkc_word_hash(const char *src, pkc_u32 a, pkc_u32 b) {
    pkc_u32 h = 2166136261u;
    pkc_u32 i;
    for (i = a; i < b; ++i) {
        h ^= (pkc_u32)pkc_ascii_upper((pkc_u8)src[i]);
        h *= 16777619u;
    }
    return h;
}

static inline pkc_i32 pkc_is_space(char c) {
    return c == ' ' || c == '\t' || c == '\r';
}

static inline void pkc_skip_space(const char *src, pkc_u32 len, pkc_u32 *p) {
    while (*p < len && pkc_is_space(src[*p])) ++(*p);
}

static inline pkc_i32 pkc_next_word(
    const char *src, pkc_u32 len, pkc_u32 *p, pkc_u32 *a, pkc_u32 *b)
{
    pkc_skip_space(src, len, p);
    if (*p >= len || src[*p] == '\n' || src[*p] == '#') return -1;
    *a = *p;
    while (*p < len && !pkc_is_space(src[*p]) &&
           src[*p] != '\n' && src[*p] != '#') {
        ++(*p);
    }
    *b = *p;
    return *b > *a ? 0 : -1;
}

static inline pkc_i32 pkc_parse_reg(
    const char *src, pkc_u32 a, pkc_u32 b, pkc_u8 *out)
{
    pkc_u32 v = 0u;
    pkc_u32 i;
    if (!out || a >= b || pkc_ascii_upper((pkc_u8)src[a]) != (pkc_u8)'R')
        return PKC_E_OPERAND;
    if (a + 1u >= b) return PKC_E_OPERAND;
    for (i = a + 1u; i < b; ++i) {
        pkc_u8 c = (pkc_u8)src[i];
        if (c < (pkc_u8)'0' || c > (pkc_u8)'9') return PKC_E_OPERAND;
        v = v * 10u + (pkc_u32)(c - (pkc_u8)'0');
        if (v > 15u) return PKC_E_OPERAND;
    }
    *out = (pkc_u8)v;
    return PKC_OK;
}

static inline pkc_i32 pkc_parse_u8_literal(
    const char *src, pkc_u32 a, pkc_u32 b, pkc_u32 *out)
{
    pkc_u32 base = 10u;
    pkc_u32 i = a;
    pkc_u32 v = 0u;
    if (!out || a >= b) return PKC_E_OPERAND;
    if (b - a > 2u && src[a] == '0' &&
        (src[a + 1u] == 'x' || src[a + 1u] == 'X')) {
        base = 16u;
        i += 2u;
    }
    if (i >= b) return PKC_E_OPERAND;
    for (; i < b; ++i) {
        pkc_u32 d;
        char c = src[i];
        if (c >= '0' && c <= '9') d = (pkc_u32)(c - '0');
        else if (base == 16u && c >= 'a' && c <= 'f') d = (pkc_u32)(c - 'a' + 10);
        else if (base == 16u && c >= 'A' && c <= 'F') d = (pkc_u32)(c - 'A' + 10);
        else return PKC_E_OPERAND;
        if (d >= base || v > (255u - d) / base) return PKC_E_OPERAND;
        v = v * base + d;
    }
    *out = v;
    return PKC_OK;
}

/* Hash constants keep readable words in source comments while avoiding
 * mandatory lexicon strings in the compiled core.
 *
 * PT:
 *   MOVER       0x75d85c72
 *   SOMAR       0xe5536d3d
 *   SUBTRAIR    0xe978c9ed
 *   MULTIPLICAR 0x324057b1
 *   RETORNAR    0xe107a062
 *
 * EN:
 *   MOVE        0xc2064154
 *   ADD         0x7d7558d4
 *   SUBTRACT    0xe75f2ee1
 *   MULTIPLY    0xac422b45
 *   RETURN      0x28af377f
 */
static inline pkc_i32 pkc_decode_verb(
    pkc_u8 lang, pkc_u32 h, pkc_u8 *op, pkc_u8 *operand_count)
{
    if (!op || !operand_count) return PKC_E_ARGUMENT;

    if (lang == PKC_LANG_PT) {
        if (h == 0x75d85c72u) { *op = PKC_OP_MOVE_IMM; *operand_count = 2u; return PKC_OK; }
        if (h == 0xe5536d3du) { *op = PKC_OP_ADD_REG;  *operand_count = 3u; return PKC_OK; }
        if (h == 0xe978c9edu) { *op = PKC_OP_SUB_REG;  *operand_count = 3u; return PKC_OK; }
        if (h == 0x324057b1u) { *op = PKC_OP_MUL_REG;  *operand_count = 3u; return PKC_OK; }
        if (h == 0xe107a062u) { *op = PKC_OP_RETURN;   *operand_count = 0u; return PKC_OK; }
        return PKC_E_VERB;
    }

    if (lang == PKC_LANG_EN) {
        if (h == 0xc2064154u) { *op = PKC_OP_MOVE_IMM; *operand_count = 2u; return PKC_OK; }
        if (h == 0x7d7558d4u) { *op = PKC_OP_ADD_REG;  *operand_count = 3u; return PKC_OK; }
        if (h == 0xe75f2ee1u) { *op = PKC_OP_SUB_REG;  *operand_count = 3u; return PKC_OK; }
        if (h == 0xac422b45u) { *op = PKC_OP_MUL_REG;  *operand_count = 3u; return PKC_OK; }
        if (h == 0x28af377fu) { *op = PKC_OP_RETURN;   *operand_count = 0u; return PKC_OK; }
        return PKC_E_VERB;
    }

    if (lang == PKC_LANG_GRC || lang == PKC_LANG_HE || lang == PKC_LANG_SYR)
        return PKC_E_TOKEN_VAZIO_LEXICON;

    return PKC_E_ARGUMENT;
}

static inline pkc_i32 pkc_parse_wordcode_v1(
    const char *src, pkc_u32 len, pkc_u8 lang,
    PkcInsnV1 *out, pkc_u32 cap, pkc_u32 *count_out,
    PkcStatusV1 *status)
{
    pkc_u32 p = 0u;
    pkc_u32 count = 0u;

    if (status) pkc_status_reset(status);
    if (!src || !out || !count_out || cap == 0u) {
        if (status) status->code = PKC_E_ARGUMENT;
        return PKC_E_ARGUMENT;
    }

    if (lang == PKC_LANG_GRC || lang == PKC_LANG_HE || lang == PKC_LANG_SYR) {
        if (status) {
            status->code = PKC_E_TOKEN_VAZIO_LEXICON;
            status->warnings |= PKC_WARN_TOKEN_VAZIO_LEXICON;
        }
        return PKC_E_TOKEN_VAZIO_LEXICON;
    }

    while (p < len) {
        pkc_u32 va, vb;
        pkc_u8 op;
        pkc_u8 operands;
        pkc_u32 a[3];
        pkc_u32 b[3];
        pkc_u32 k;
        PkcInsnV1 insn = {0u, 0u, 0u, 0u, 0u};

        pkc_skip_space(src, len, &p);
        if (p >= len) break;
        if (src[p] == '\n') { ++p; continue; }
        if (src[p] == '#') {
            while (p < len && src[p] != '\n') ++p;
            continue;
        }
        if (count >= cap) {
            if (status) { status->code = PKC_E_CAPACITY; status->source_offset = p; }
            return PKC_E_CAPACITY;
        }

        if (pkc_next_word(src, len, &p, &va, &vb) != 0) {
            if (status) { status->code = PKC_E_SYNTAX; status->source_offset = p; }
            return PKC_E_SYNTAX;
        }

        {
            pkc_i32 vr = pkc_decode_verb(lang, pkc_word_hash(src, va, vb), &op, &operands);
            if (vr != PKC_OK) {
                if (status) { status->code = vr; status->source_offset = va; }
                return vr;
            }
        }

        for (k = 0u; k < (pkc_u32)operands; ++k) {
            if (pkc_next_word(src, len, &p, &a[k], &b[k]) != 0) {
                if (status) { status->code = PKC_E_OPERAND; status->source_offset = p; }
                return PKC_E_OPERAND;
            }
        }

        if (op == PKC_OP_MOVE_IMM) {
            if (pkc_parse_reg(src, a[0], b[0], &insn.rd) != PKC_OK) {
                if (status) { status->code = PKC_E_OPERAND; status->source_offset = a[0]; }
                return PKC_E_OPERAND;
            }
            if (pkc_parse_reg(src, a[1], b[1], &insn.rm) == PKC_OK) {
                insn.op = PKC_OP_MOVE_REG;
            } else {
                if (pkc_parse_u8_literal(src, a[1], b[1], &insn.imm) != PKC_OK) {
                    if (status) { status->code = PKC_E_OPERAND; status->source_offset = a[1]; }
                    return PKC_E_OPERAND;
                }
                insn.op = PKC_OP_MOVE_IMM;
            }
        } else if (op == PKC_OP_ADD_REG || op == PKC_OP_SUB_REG || op == PKC_OP_MUL_REG) {
            insn.op = op;
            if (pkc_parse_reg(src, a[0], b[0], &insn.rd) != PKC_OK ||
                pkc_parse_reg(src, a[1], b[1], &insn.rn) != PKC_OK ||
                pkc_parse_reg(src, a[2], b[2], &insn.rm) != PKC_OK) {
                if (status) { status->code = PKC_E_OPERAND; status->source_offset = a[0]; }
                return PKC_E_OPERAND;
            }
        } else if (op == PKC_OP_RETURN) {
            insn.op = PKC_OP_RETURN;
        } else {
            if (status) { status->code = PKC_E_VERB; status->source_offset = va; }
            return PKC_E_VERB;
        }

        pkc_skip_space(src, len, &p);
        if (p < len && src[p] == '#') {
            while (p < len && src[p] != '\n') ++p;
        }
        if (p < len && src[p] != '\n') {
            if (status) { status->code = PKC_E_SYNTAX; status->source_offset = p; }
            return PKC_E_SYNTAX;
        }
        if (p < len && src[p] == '\n') ++p;

        out[count++] = insn;
    }

    if (count == 0u) {
        if (status) status->code = PKC_E_SYNTAX;
        return PKC_E_SYNTAX;
    }

    *count_out = count;
    if (status) status->code = PKC_OK;
    return PKC_OK;
}

#endif /* RAF_PKC_WORDCODE_H */
