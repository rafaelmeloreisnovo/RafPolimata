/* raf_stage0_text.h — bounded textual RAFIR_TINY_V1 parser.
 *
 * No libc, no heap, no Python. Caller owns source, IR and artifact buffers.
 * Grammar (ASCII, one instruction per line):
 *   MOVI rD IMM
 *   MOV  rD rM
 *   ADD  rD rN rM
 *   SUB  rD rN rM
 *   MUL  rD rN rM
 *   BX   rM|lr
 *   RET
 *   SWI  IMM
 * Blank lines and lines beginning with '#' are ignored.
 */
#pragma once
#include "raf_stage0_arm32.h"

static inline int raf_txt_space(char c) { return c == ' ' || c == '\t' || c == '\r'; }
static inline int raf_txt_digit(char c) { return c >= '0' && c <= '9'; }
static inline int raf_txt_alpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
static inline char raf_txt_upper(char c) { return (c >= 'a' && c <= 'z') ? (char)(c - 32) : c; }

static inline void raf_txt_skip_space(const char *s, sz n, sz *p) {
    while (*p < n && raf_txt_space(s[*p])) (*p)++;
}

static inline int raf_txt_word_eq(const char *s, sz a, sz b, const char *lit) {
    sz i = 0;
    while (lit[i]) {
        if (a + i >= b || raf_txt_upper(s[a + i]) != lit[i]) return 0;
        i++;
    }
    return a + i == b;
}

static inline i32 raf_txt_word(const char *s, sz n, sz *p, sz *a, sz *b) {
    raf_txt_skip_space(s, n, p);
    if (*p >= n || s[*p] == '\n' || s[*p] == '#') return -1;
    *a = *p;
    while (*p < n && !raf_txt_space(s[*p]) && s[*p] != '\n' && s[*p] != '#') (*p)++;
    *b = *p;
    return *b > *a ? 0 : -1;
}

static inline i32 raf_txt_reg(const char *s, sz a, sz b, u8 *r) {
    if (!r || a >= b) return -1;
    if (b - a == 2u && raf_txt_upper(s[a]) == 'L' && raf_txt_upper(s[a + 1]) == 'R') {
        *r = A32_LR;
        return 0;
    }
    if (raf_txt_upper(s[a]) != 'R' || a + 1u >= b) return -1;
    u32 v = 0;
    for (sz i = a + 1u; i < b; i++) {
        if (!raf_txt_digit(s[i])) return -1;
        v = v * 10u + (u32)(s[i] - '0');
        if (v > 15u) return -1;
    }
    *r = (u8)v;
    return 0;
}

static inline i32 raf_txt_u32(const char *s, sz a, sz b, u32 max_v, u32 *out) {
    if (!out || a >= b) return -1;
    u32 base = 10u;
    sz i = a;
    if (b - a > 2u && s[a] == '0' && (s[a + 1] == 'x' || s[a + 1] == 'X')) {
        base = 16u;
        i += 2u;
    }
    if (i >= b) return -1;
    u32 v = 0;
    for (; i < b; i++) {
        u32 d;
        char c = s[i];
        if (c >= '0' && c <= '9') d = (u32)(c - '0');
        else if (base == 16u && c >= 'a' && c <= 'f') d = (u32)(c - 'a' + 10);
        else if (base == 16u && c >= 'A' && c <= 'F') d = (u32)(c - 'A' + 10);
        else return -1;
        if (d >= base || v > (max_v - d) / base) return -1;
        v = v * base + d;
    }
    *out = v;
    return 0;
}

static inline i32 raf_stage0_parse_text_v1(
    const char *src, sz len, RafIr32InsnV1 *out, u32 cap, u32 *count_out)
{
    if (!src || !out || !count_out || !cap) return -1;
    sz p = 0;
    u32 count = 0;

    while (p < len) {
        raf_txt_skip_space(src, len, &p);
        if (p >= len) break;
        if (src[p] == '\n') { p++; continue; }
        if (src[p] == '#') {
            while (p < len && src[p] != '\n') p++;
            continue;
        }
        if (count >= cap) return -2;

        sz oa, ob;
        if (raf_txt_word(src, len, &p, &oa, &ob) != 0) return -3;
        RafIr32InsnV1 in = {0u, 0u, 0u, 0u, 0u};
        int operands = 0;
        if (raf_txt_word_eq(src, oa, ob, "MOVI")) { in.op = RAF_IR32_MOV_IMM; operands = 2; }
        else if (raf_txt_word_eq(src, oa, ob, "MOV")) { in.op = RAF_IR32_MOV_REG; operands = 2; }
        else if (raf_txt_word_eq(src, oa, ob, "ADD")) { in.op = RAF_IR32_ADD_REG; operands = 3; }
        else if (raf_txt_word_eq(src, oa, ob, "SUB")) { in.op = RAF_IR32_SUB_REG; operands = 3; }
        else if (raf_txt_word_eq(src, oa, ob, "MUL")) { in.op = RAF_IR32_MUL_REG; operands = 3; }
        else if (raf_txt_word_eq(src, oa, ob, "BX")) { in.op = RAF_IR32_BX_REG; operands = 1; }
        else if (raf_txt_word_eq(src, oa, ob, "RET")) { in.op = RAF_IR32_RET; operands = 0; }
        else if (raf_txt_word_eq(src, oa, ob, "SWI")) { in.op = RAF_IR32_SWI; operands = 1; }
        else return -4;

        sz a[3], b[3];
        for (int k = 0; k < operands; k++) {
            if (raf_txt_word(src, len, &p, &a[k], &b[k]) != 0) return -5;
        }

        if (in.op == RAF_IR32_MOV_IMM) {
            if (raf_txt_reg(src, a[0], b[0], &in.rd) != 0 ||
                raf_txt_u32(src, a[1], b[1], 0xFFu, &in.imm) != 0) return -6;
        } else if (in.op == RAF_IR32_MOV_REG) {
            if (raf_txt_reg(src, a[0], b[0], &in.rd) != 0 ||
                raf_txt_reg(src, a[1], b[1], &in.rm) != 0) return -7;
        } else if (in.op == RAF_IR32_ADD_REG || in.op == RAF_IR32_SUB_REG || in.op == RAF_IR32_MUL_REG) {
            if (raf_txt_reg(src, a[0], b[0], &in.rd) != 0 ||
                raf_txt_reg(src, a[1], b[1], &in.rn) != 0 ||
                raf_txt_reg(src, a[2], b[2], &in.rm) != 0) return -8;
        } else if (in.op == RAF_IR32_BX_REG) {
            if (raf_txt_reg(src, a[0], b[0], &in.rm) != 0) return -9;
        } else if (in.op == RAF_IR32_SWI) {
            if (raf_txt_u32(src, a[0], b[0], 0x00FFFFFFu, &in.imm) != 0) return -10;
        }

        raf_txt_skip_space(src, len, &p);
        if (p < len && src[p] == '#') while (p < len && src[p] != '\n') p++;
        if (p < len && src[p] != '\n') return -11;
        if (p < len && src[p] == '\n') p++;

        out[count++] = in;
    }

    if (!count) return -12;
    *count_out = count;
    return 0;
}

static inline i32 raf_stage0_text_to_elf32(
    const char *src, sz len,
    RafIr32InsnV1 *ir_tmp, u32 ir_cap,
    u8 *text_tmp, u32 text_cap,
    u8 *elf_out, sz elf_cap,
    const char *export_name, sz *elf_size_out)
{
    u32 count = 0;
    i32 rc = raf_stage0_parse_text_v1(src, len, ir_tmp, ir_cap, &count);
    if (rc != 0) return rc;
    return raf_stage0_ir32_to_elf32(ir_tmp, count, text_tmp, text_cap,
                                     elf_out, elf_cap, export_name, elf_size_out);
}
