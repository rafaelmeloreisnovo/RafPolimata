/* apkc_semantic_return_subset.h — bounded semantic subset for Cycle 2
 *
 * Grammar (statement fragment only):
 *
 *   return-stmt := "return" expr [";"] EOF
 *   expr        := term { ("+" | "-") term }
 *   term        := factor { ("*" | "/" | "%") factor }
 *   factor      := uint32 | "(" expr ")" | "-" factor
 *
 * Purpose:
 *   provide the smallest falsifiable semantic/action gate requested by the
 *   capability audit: `return 42` and precedence `return 2 + 3 * 4`.
 *
 * Scope boundary:
 *   this is a common arithmetic RETURN STATEMENT FRAGMENT accepted only after
 *   canonical LP_* routing is established.  It is NOT a claim of full C,
 *   Python, Rust, Go, Java, JavaScript or Swift semantics.
 *
 * FREESTANDING: no malloc, no libc. Parenthesis nesting is bounded to 8.
 */
#ifndef APKC_SEMANTIC_RETURN_SUBSET_H
#define APKC_SEMANTIC_RETURN_SUBSET_H 1

#include "compiler_language_direct.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

#define APKC_RETURN_SUBSET_OK        0u
#define APKC_RETURN_SUBSET_INVALID   1u
#define APKC_RETURN_SUBSET_NOT_MATCH 2u

#define APKC_SEMANTIC_SCOPE_NONE                       0u
#define APKC_SEMANTIC_SCOPE_RETURN_ARITHMETIC_FRAGMENT 1u

struct ApkCReturnParser {
    const u8 *src;
    u32 len;
    u32 pos;
    struct CodeGen *cg;
    u8 error;
    u8 depth;
};

static inline void apkc_ret_skip_ws(struct ApkCReturnParser *p) {
    while (p->pos < p->len) {
        u8 c = p->src[p->pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') p->pos++;
        else break;
    }
}

static inline u8 apkc_ret_is_digit(u8 c) {
    return (c >= '0' && c <= '9') ? 1u : 0u;
}

static inline u8 apkc_ret_keyword(struct ApkCReturnParser *p) {
    static const u8 kw[6] = {'r','e','t','u','r','n'};
    u32 i;
    apkc_ret_skip_ws(p);
    if (p->pos + 6u > p->len) return 0;
    for (i = 0; i < 6u; i++) if (p->src[p->pos + i] != kw[i]) return 0;
    if (p->pos + 6u < p->len) {
        u8 next = p->src[p->pos + 6u];
        if ((next >= 'a' && next <= 'z') ||
            (next >= 'A' && next <= 'Z') ||
            (next >= '0' && next <= '9') || next == '_') return 0;
    }
    p->pos += 6u;
    return 1;
}

static inline u8 apkc_ret_alloc(struct ApkCReturnParser *p) {
    u8 r = codegen_alloc_reg(p->cg);
    if (r == 255u) p->error = 1;
    return r;
}

static inline void apkc_ret_free_last(struct ApkCReturnParser *p, u8 r) {
    if ((u32)r + 1u == (u32)p->cg->r_free) p->cg->r_free--;
}

static inline u8 apkc_ret_parse_expr(struct ApkCReturnParser *p);

static inline u8 apkc_ret_parse_factor(struct ApkCReturnParser *p) {
    u8 r;
    apkc_ret_skip_ws(p);
    if (p->pos >= p->len) {
        p->error = 1;
        return 255u;
    }

    if (p->src[p->pos] == '(') {
        if (p->depth >= 8u) {
            p->error = 1;
            return 255u;
        }
        p->depth++;
        p->pos++;
        r = apkc_ret_parse_expr(p);
        apkc_ret_skip_ws(p);
        if (p->pos >= p->len || p->src[p->pos] != ')') {
            p->error = 1;
            return 255u;
        }
        p->pos++;
        p->depth--;
        return r;
    }

    if (p->src[p->pos] == '-') {
        u8 zero, val;
        p->pos++;
        zero = apkc_ret_alloc(p);
        if (p->error) return 255u;
        codegen_emit_movi(p->cg, zero, 0u);
        val = apkc_ret_parse_factor(p);
        if (p->error) return 255u;
        codegen_emit_sub(p->cg, zero, zero, val);
        apkc_ret_free_last(p, val);
        return zero;
    }

    if (!apkc_ret_is_digit(p->src[p->pos])) {
        p->error = 1;
        return 255u;
    }

    {
        u64 value = 0;
        while (p->pos < p->len && apkc_ret_is_digit(p->src[p->pos])) {
            value = value * 10u + (u64)(p->src[p->pos] - '0');
            if (value > 0xffffffffULL) {
                p->error = 1;
                return 255u;
            }
            p->pos++;
        }
        r = apkc_ret_alloc(p);
        if (p->error) return 255u;
        codegen_emit_movi(p->cg, r, (u32)value);
    }
    return r;
}

static inline u8 apkc_ret_parse_term(struct ApkCReturnParser *p) {
    u8 left = apkc_ret_parse_factor(p);
    if (p->error) return 255u;

    for (;;) {
        u8 op, right;
        apkc_ret_skip_ws(p);
        if (p->pos >= p->len) break;
        op = p->src[p->pos];
        if (op != '*' && op != '/' && op != '%') break;
        p->pos++;
        right = apkc_ret_parse_factor(p);
        if (p->error) return 255u;
        if (op == '*') codegen_emit_mul(p->cg, left, left, right);
        else if (op == '/') codegen_emit_div(p->cg, left, left, right);
        else codegen_emit_mod(p->cg, left, left, right);
        apkc_ret_free_last(p, right);
    }
    return left;
}

static inline u8 apkc_ret_parse_expr(struct ApkCReturnParser *p) {
    u8 left = apkc_ret_parse_term(p);
    if (p->error) return 255u;

    for (;;) {
        u8 op, right;
        apkc_ret_skip_ws(p);
        if (p->pos >= p->len) break;
        op = p->src[p->pos];
        if (op != '+' && op != '-') break;
        p->pos++;
        right = apkc_ret_parse_term(p);
        if (p->error) return 255u;
        if (op == '+') codegen_emit_add(p->cg, left, left, right);
        else codegen_emit_sub(p->cg, left, left, right);
        apkc_ret_free_last(p, right);
    }
    return left;
}

/* Strict tri-state compiler:
 *   OK        => complete fragment parsed and emitted
 *   INVALID   => input began with `return` but did not satisfy the grammar
 *   NOT_MATCH => input is not this semantic subset; caller may use normal frontend
 */
static inline u8 apkc_compile_return_arithmetic_subset(
    struct CodeGen *cg,
    const u8 *src, u32 src_len)
{
    struct ApkCReturnParser p;
    u8 result;
    u32 start_pos;

    if (!cg || !src || src_len == 0u) return APKC_RETURN_SUBSET_NOT_MATCH;

    p.src = src;
    p.len = src_len;
    p.pos = 0u;
    p.cg = cg;
    p.error = 0u;
    p.depth = 0u;
    start_pos = cg->pos;

    if (!apkc_ret_keyword(&p)) return APKC_RETURN_SUBSET_NOT_MATCH;
    apkc_ret_skip_ws(&p);
    if (p.pos >= p.len) return APKC_RETURN_SUBSET_INVALID;

    result = apkc_ret_parse_expr(&p);
    if (p.error || result == 255u) {
        cg->pos = start_pos;
        cg->r_free = 0u;
        return APKC_RETURN_SUBSET_INVALID;
    }

    apkc_ret_skip_ws(&p);
    if (p.pos < p.len && p.src[p.pos] == ';') {
        p.pos++;
        apkc_ret_skip_ws(&p);
    }
    if (p.pos != p.len) {
        cg->pos = start_pos;
        cg->r_free = 0u;
        return APKC_RETURN_SUBSET_INVALID;
    }

    if (result != 0u) codegen_emit_mov(cg, 0u, result);
    codegen_emit(cg, OP_HALT, 0u, 0u, 0u, 0u);
    return APKC_RETURN_SUBSET_OK;
}

#endif /* APKC_SEMANTIC_RETURN_SUBSET_H */
