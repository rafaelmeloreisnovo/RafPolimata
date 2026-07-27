#include "raf_compile.h"

/* Source-dependent bounded lowering for the canonical u32 kernel contract.
 * Accepted forms (outside comments/strings):
 *   RAF_RETURN(<constant expression>)
 *   return <constant expression>;
 *   exit <constant expression>
 * Operators: + - * / % << >> & ^ | ~ and parentheses.
 * No heap, no symbol table and no silent fixed-value fallback. */

typedef struct {
    const char *p;
    const char *end;
    int err;
} RafExpr;

static int raf_is_ident(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

static void raf_skip_ws(RafExpr *e) {
    while (e->p < e->end && (*e->p == ' ' || *e->p == '\t' || *e->p == '\r')) ++e->p;
}

static int raf_match(RafExpr *e, const char *token) {
    raf_skip_ws(e);
    const char *p = e->p;
    const char *t = token;
    while (*t && p < e->end && *p == *t) { ++p; ++t; }
    if (*t) return 0;
    e->p = p;
    return 1;
}

static uint64_t raf_parse_or(RafExpr *e);

static uint64_t raf_parse_number(RafExpr *e) {
    raf_skip_ws(e);
    unsigned base = 10u;
    if (e->p + 2 <= e->end && e->p[0] == '0' && (e->p[1] == 'x' || e->p[1] == 'X')) {
        base = 16u; e->p += 2;
    } else if (e->p + 2 <= e->end && e->p[0] == '0' && (e->p[1] == 'b' || e->p[1] == 'B')) {
        base = 2u; e->p += 2;
    }
    uint64_t value = 0u;
    unsigned digits = 0u;
    while (e->p < e->end) {
        unsigned d;
        char c = *e->p;
        if (c >= '0' && c <= '9') d = (unsigned)(c - '0');
        else if (c >= 'a' && c <= 'f') d = (unsigned)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') d = (unsigned)(c - 'A' + 10);
        else break;
        if (d >= base) break;
        value = value * base + d;
        ++digits;
        ++e->p;
    }
    if (digits == 0u) { e->err = 1; return 0u; }
    while (e->p < e->end && (*e->p == 'u' || *e->p == 'U' || *e->p == 'l' || *e->p == 'L')) ++e->p;
    return value;
}

static uint64_t raf_parse_primary(RafExpr *e) {
    raf_skip_ws(e);
    if (raf_match(e, "(")) {
        uint64_t value = raf_parse_or(e);
        if (!raf_match(e, ")")) e->err = 1;
        return value;
    }
    return raf_parse_number(e);
}

static uint64_t raf_parse_unary(RafExpr *e) {
    if (raf_match(e, "+")) return raf_parse_unary(e);
    if (raf_match(e, "-")) return (uint64_t)(0u - raf_parse_unary(e));
    if (raf_match(e, "~")) return ~raf_parse_unary(e);
    return raf_parse_primary(e);
}

static uint64_t raf_parse_mul(RafExpr *e) {
    uint64_t value = raf_parse_unary(e);
    for (;;) {
        if (raf_match(e, "*")) value *= raf_parse_unary(e);
        else if (raf_match(e, "/")) {
            uint64_t rhs = raf_parse_unary(e);
            if (rhs == 0u) { e->err = 1; return 0u; }
            value /= rhs;
        } else if (raf_match(e, "%")) {
            uint64_t rhs = raf_parse_unary(e);
            if (rhs == 0u) { e->err = 1; return 0u; }
            value %= rhs;
        } else break;
    }
    return value;
}

static uint64_t raf_parse_add(RafExpr *e) {
    uint64_t value = raf_parse_mul(e);
    for (;;) {
        if (raf_match(e, "+")) value += raf_parse_mul(e);
        else if (raf_match(e, "-")) value -= raf_parse_mul(e);
        else break;
    }
    return value;
}

static uint64_t raf_parse_shift(RafExpr *e) {
    uint64_t value = raf_parse_add(e);
    for (;;) {
        if (raf_match(e, "<<")) {
            uint64_t rhs = raf_parse_add(e);
            if (rhs >= 64u) { e->err = 1; return 0u; }
            value <<= (unsigned)rhs;
        } else if (raf_match(e, ">>")) {
            uint64_t rhs = raf_parse_add(e);
            if (rhs >= 64u) { e->err = 1; return 0u; }
            value >>= (unsigned)rhs;
        } else break;
    }
    return value;
}

static uint64_t raf_parse_and(RafExpr *e) {
    uint64_t value = raf_parse_shift(e);
    while (!e->err) {
        raf_skip_ws(e);
        if (e->p + 1 < e->end && e->p[0] == '&' && e->p[1] == '&') break;
        if (!raf_match(e, "&")) break;
        value &= raf_parse_shift(e);
    }
    return value;
}

static uint64_t raf_parse_xor(RafExpr *e) {
    uint64_t value = raf_parse_and(e);
    while (raf_match(e, "^")) value ^= raf_parse_and(e);
    return value;
}

static uint64_t raf_parse_or(RafExpr *e) {
    uint64_t value = raf_parse_xor(e);
    while (!e->err) {
        raf_skip_ws(e);
        if (e->p + 1 < e->end && e->p[0] == '|' && e->p[1] == '|') break;
        if (!raf_match(e, "|")) break;
        value |= raf_parse_xor(e);
    }
    return value;
}

static const char *raf_find_keyword(const char *src, size_t len, const char *keyword) {
    size_t kw_len = 0u;
    while (keyword[kw_len]) ++kw_len;
    int state = 0; /* 0 code, 1 line comment, 2 block comment, 3 string, 4 char */
    for (size_t i = 0u; i < len; ++i) {
        char c = src[i];
        char n = i + 1u < len ? src[i + 1u] : '\0';
        if (state == 0) {
            if (c == '/' && n == '/') { state = 1; ++i; continue; }
            if (c == '/' && n == '*') { state = 2; ++i; continue; }
            if (c == '"') { state = 3; continue; }
            if (c == '\'') { state = 4; continue; }
            if ((i == 0u || !raf_is_ident(src[i - 1u])) && i + kw_len <= len) {
                size_t j = 0u;
                while (j < kw_len && src[i + j] == keyword[j]) ++j;
                if (j == kw_len && (i + kw_len == len || !raf_is_ident(src[i + kw_len])))
                    return src + i + kw_len;
            }
        } else if (state == 1) {
            if (c == '\n') state = 0;
        } else if (state == 2) {
            if (c == '*' && n == '/') { state = 0; ++i; }
        } else if (state == 3 || state == 4) {
            if (c == '\\' && n != '\0') { ++i; continue; }
            if ((state == 3 && c == '"') || (state == 4 && c == '\'')) state = 0;
        }
    }
    return (const char *)0;
}

static int raf_expression_tail_ok(RafExpr *e, int marker_parenthesized) {
    raf_skip_ws(e);
    if (marker_parenthesized) {
        if (e->p >= e->end || *e->p != ')') return 0;
        ++e->p;
        raf_skip_ws(e);
    }
    if (e->p >= e->end) return 1;
    return *e->p == ';' || *e->p == '\n' || *e->p == '\r' || *e->p == '}' ||
           (e->p + 1 < e->end && e->p[0] == '/' && (e->p[1] == '/' || e->p[1] == '*'));
}

int raf_precompile(RafCtx *ctx) {
    if (!ctx || !ctx->src) return -1;
    const char *start = raf_find_keyword(ctx->src, ctx->src_len, "RAF_RETURN");
    int marker_parenthesized = 0;
    if (start) {
        while (start < ctx->src + ctx->src_len && (*start == ' ' || *start == '\t' || *start == '\r')) ++start;
        if (start < ctx->src + ctx->src_len && *start == '(') { ++start; marker_parenthesized = 1; }
    } else {
        start = raf_find_keyword(ctx->src, ctx->src_len, "return");
        if (!start) start = raf_find_keyword(ctx->src, ctx->src_len, "exit");
    }
    if (!start) return -2;

    RafExpr expr = { start, ctx->src + ctx->src_len, 0 };
    uint64_t value = raf_parse_or(&expr);
    if (expr.err || !raf_expression_tail_ok(&expr, marker_parenthesized)) return -3;

    ctx->ir.n = 2u;
    ctx->ir.buf[0] = ((uint64_t)IR_MOVIMM << 56) | (uint64_t)(uint32_t)value;
    ctx->ir.buf[1] = ((uint64_t)IR_RET << 56);
    return 0;
}
