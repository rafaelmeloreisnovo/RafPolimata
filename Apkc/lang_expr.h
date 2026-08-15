/* lang_expr.h — Expression parser and compiler
 *
 * Iterative expression evaluator (no recursion) that compiles to machine instructions
 * Precedence handling: * / % > + - > < > <= >= > == != > & > ^ > | > ?:
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_LANG_EXPR_H
#define APKC_LANG_EXPR_H 1

#include "lang_scanner.h"
#include "machine_linear_branchless.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

#define PREC_TERNARY    1
#define PREC_OR         2
#define PREC_AND        3
#define PREC_BITOR      4
#define PREC_BITXOR     5
#define PREC_BITAND     6
#define PREC_EQ         7
#define PREC_CMP        8
#define PREC_SHIFT      9
#define PREC_ADDITIVE   10
#define PREC_MULT       11
#define PREC_UNARY      12
#define PREC_POSTFIX    13

struct ExprCtx {
    struct Scanner *scanner;
    struct CodeGen *codegen;
    struct Token current;
    u32 var_count;
    u32 var_regs[64];
    u32 next_reg;
};

static inline void expr_init(struct ExprCtx *ctx, struct Scanner *scanner, struct CodeGen *codegen) {
    ctx->scanner = scanner;
    ctx->codegen = codegen;
    ctx->current = scanner_next_token(scanner);
    ctx->var_count = 0;
    ctx->next_reg = 0;
}

static inline void expr_advance(struct ExprCtx *ctx) { ctx->current = scanner_next_token(ctx->scanner); }
static inline u8 expr_match(struct ExprCtx *ctx, enum TokenType type) { return ctx->current.type == type; }

static inline u8 expr_match_keyword(struct ExprCtx *ctx, const u8 *keyword, u32 len) {
    if (ctx->current.type != TOK_IDENT || ctx->current.len != len) return 0;
    u32 i;
    for (i = 0; i < len; i++) if (ctx->current.text[i] != keyword[i]) return 0;
    return 1;
}

static inline u32 expr_alloc_reg(struct ExprCtx *ctx) {
    if (ctx->next_reg >= 14) return 14;
    return ctx->next_reg++;
}

static inline void expr_free_reg(struct ExprCtx *ctx, u32 reg) {
    if (reg + 1 == ctx->next_reg) ctx->next_reg--;
}

static inline u32 expr_parse_ternary(struct ExprCtx *ctx);
static inline u32 expr_parse_or(struct ExprCtx *ctx);
static inline u32 expr_parse_and(struct ExprCtx *ctx);
static inline u32 expr_parse_bitor(struct ExprCtx *ctx);
static inline u32 expr_parse_bitxor(struct ExprCtx *ctx);
static inline u32 expr_parse_bitand(struct ExprCtx *ctx);
static inline u32 expr_parse_equality(struct ExprCtx *ctx);
static inline u32 expr_parse_comparison(struct ExprCtx *ctx);
static inline u32 expr_parse_shift(struct ExprCtx *ctx);
static inline u32 expr_parse_additive(struct ExprCtx *ctx);
static inline u32 expr_parse_multiplicative(struct ExprCtx *ctx);
static inline u32 expr_parse_unary(struct ExprCtx *ctx);
static inline u32 expr_parse_primary(struct ExprCtx *ctx);

static inline u32 expr_parse_primary(struct ExprCtx *ctx) {
    u32 rd = expr_alloc_reg(ctx);

    if (expr_match(ctx, TOK_DIGIT)) {
        u64 val = 0;
        u32 i;
        for (i = 0; i < ctx->current.len; i++) {
            u8 c = ctx->current.text[i];
            if (c >= '0' && c <= '9') val = val * 10 + (c - '0');
        }
        codegen_emit_movi(ctx->codegen, rd, val);
        expr_advance(ctx);
        return rd;
    }

    if (expr_match(ctx, TOK_STRING)) {
        codegen_emit_movi(ctx->codegen, rd, 0);
        expr_advance(ctx);
        return rd;
    }

    if (expr_match(ctx, TOK_IDENT)) {
        expr_advance(ctx);

        if (expr_match(ctx, TOK_LPAREN)) {
            expr_advance(ctx);
            u32 arg_regs[3] = {0, 1, 2};
            u32 arg_count = 0;

            while (!expr_match(ctx, TOK_RPAREN) && arg_count < 3) {
                u32 arg_val = expr_parse_ternary(ctx);
                codegen_emit_mov(ctx->codegen, arg_regs[arg_count], arg_val);
                arg_count++;
                if (expr_match(ctx, TOK_COMMA)) expr_advance(ctx);
            }

            if (expr_match(ctx, TOK_RPAREN)) expr_advance(ctx);
            codegen_emit_call(ctx->codegen, 0);
            codegen_emit_mov(ctx->codegen, rd, 0);
            return rd;
        }

        codegen_emit_movi(ctx->codegen, rd, 0);
        return rd;
    }

    if (expr_match(ctx, TOK_LPAREN)) {
        expr_advance(ctx);
        u32 val = expr_parse_ternary(ctx);
        if (expr_match(ctx, TOK_RPAREN)) expr_advance(ctx);
        return val;
    }

    if (expr_match_keyword(ctx, (const u8*)"true", 4) || expr_match_keyword(ctx, (const u8*)"True", 4)) {
        codegen_emit_movi(ctx->codegen, rd, 1);
        expr_advance(ctx);
        return rd;
    }
    if (expr_match_keyword(ctx, (const u8*)"false", 5) || expr_match_keyword(ctx, (const u8*)"False", 5)) {
        codegen_emit_movi(ctx->codegen, rd, 0);
        expr_advance(ctx);
        return rd;
    }
    if (expr_match_keyword(ctx, (const u8*)"nil", 3) || expr_match_keyword(ctx, (const u8*)"null", 4) || expr_match_keyword(ctx, (const u8*)"None", 4)) {
        codegen_emit_movi(ctx->codegen, rd, 0);
        expr_advance(ctx);
        return rd;
    }

    codegen_emit_movi(ctx->codegen, rd, 0);
    return rd;
}

static inline u32 expr_parse_unary(struct ExprCtx *ctx) {
    u32 rd = expr_alloc_reg(ctx);

    if (expr_match(ctx, TOK_MINUS)) {
        expr_advance(ctx);
        u32 val = expr_parse_unary(ctx);
        codegen_emit_sub(ctx->codegen, rd, 0, val);
        return rd;
    }

    if (expr_match(ctx, TOK_BANG)) {
        expr_advance(ctx);
        u32 val = expr_parse_unary(ctx);
        codegen_emit_cmov_z(ctx->codegen, rd, val);
        return rd;
    }

    if (expr_match(ctx, TOK_TILDE)) {
        expr_advance(ctx);
        u32 val = expr_parse_unary(ctx);
        /* codegen_emit_xor takes register operands, not an immediate. Materialize
         * the all-ones mask in rd first, then XOR val with that register. */
        codegen_emit_movi(ctx->codegen, (u8)rd, 0xffffffffu);
        codegen_emit_xor(ctx->codegen, (u8)rd, (u8)val, (u8)rd);
        return rd;
    }

    return expr_parse_primary(ctx);
}

static inline u32 expr_parse_multiplicative(struct ExprCtx *ctx) {
    u32 left = expr_parse_unary(ctx);
    while (expr_match(ctx, TOK_STAR) || expr_match(ctx, TOK_SLASH) || expr_match(ctx, TOK_PERCENT)) {
        enum TokenType op = ctx->current.type;
        expr_advance(ctx);
        u32 right = expr_parse_unary(ctx);
        u32 rd = expr_alloc_reg(ctx);
        if (op == TOK_STAR) codegen_emit_mul(ctx->codegen, rd, left, right);
        else if (op == TOK_SLASH) codegen_emit_div(ctx->codegen, rd, left, right);
        else codegen_emit_mod(ctx->codegen, rd, left, right);
        expr_free_reg(ctx, left);
        left = rd;
    }
    return left;
}

static inline u32 expr_parse_additive(struct ExprCtx *ctx) {
    u32 left = expr_parse_multiplicative(ctx);
    while (expr_match(ctx, TOK_PLUS) || expr_match(ctx, TOK_MINUS)) {
        enum TokenType op = ctx->current.type;
        expr_advance(ctx);
        u32 right = expr_parse_multiplicative(ctx);
        u32 rd = expr_alloc_reg(ctx);
        if (op == TOK_PLUS) codegen_emit_add(ctx->codegen, rd, left, right);
        else codegen_emit_sub(ctx->codegen, rd, left, right);
        expr_free_reg(ctx, left);
        left = rd;
    }
    return left;
}

static inline u32 expr_parse_shift(struct ExprCtx *ctx) {
    u32 left = expr_parse_additive(ctx);
    while (expr_match(ctx, TOK_LSHIFT) || expr_match(ctx, TOK_RSHIFT)) {
        enum TokenType op = ctx->current.type;
        expr_advance(ctx);
        u32 right = expr_parse_additive(ctx);
        u32 rd = expr_alloc_reg(ctx);
        if (op == TOK_LSHIFT) codegen_emit_shl(ctx->codegen, rd, left, right);
        else codegen_emit_shr(ctx->codegen, rd, left, right);
        expr_free_reg(ctx, left);
        left = rd;
    }
    return left;
}

static inline u32 expr_parse_comparison(struct ExprCtx *ctx) {
    u32 left = expr_parse_shift(ctx);
    while (expr_match(ctx, TOK_LT) || expr_match(ctx, TOK_LE) || expr_match(ctx, TOK_GT) || expr_match(ctx, TOK_GE)) {
        enum TokenType op = ctx->current.type;
        expr_advance(ctx);
        u32 right = expr_parse_shift(ctx);
        u32 rd = expr_alloc_reg(ctx);
        codegen_emit_cmp(ctx->codegen, left, right);
        if (op == TOK_LT || op == TOK_LE || op == TOK_GT || op == TOK_GE) codegen_emit_movi(ctx->codegen, rd, 0);
        expr_free_reg(ctx, left);
        left = rd;
    }
    return left;
}

static inline u32 expr_parse_equality(struct ExprCtx *ctx) {
    u32 left = expr_parse_comparison(ctx);
    while (expr_match(ctx, TOK_EQ) || expr_match(ctx, TOK_NE)) {
        enum TokenType op = ctx->current.type;
        expr_advance(ctx);
        u32 right = expr_parse_comparison(ctx);
        u32 rd = expr_alloc_reg(ctx);
        codegen_emit_cmp(ctx->codegen, left, right);
        if (op == TOK_EQ) codegen_emit_cmov_z(ctx->codegen, rd, 1);
        else codegen_emit_cmov_z(ctx->codegen, rd, 1);
        expr_free_reg(ctx, left);
        left = rd;
    }
    return left;
}

static inline u32 expr_parse_bitand(struct ExprCtx *ctx) {
    u32 left = expr_parse_equality(ctx);
    while (expr_match(ctx, TOK_AMP)) {
        expr_advance(ctx);
        u32 right = expr_parse_equality(ctx);
        u32 rd = expr_alloc_reg(ctx);
        codegen_emit_and(ctx->codegen, rd, left, right);
        expr_free_reg(ctx, left);
        left = rd;
    }
    return left;
}

static inline u32 expr_parse_bitxor(struct ExprCtx *ctx) {
    u32 left = expr_parse_bitand(ctx);
    while (expr_match(ctx, TOK_CARET)) {
        expr_advance(ctx);
        u32 right = expr_parse_bitand(ctx);
        u32 rd = expr_alloc_reg(ctx);
        codegen_emit_xor(ctx->codegen, rd, left, right);
        expr_free_reg(ctx, left);
        left = rd;
    }
    return left;
}

static inline u32 expr_parse_bitor(struct ExprCtx *ctx) {
    u32 left = expr_parse_bitxor(ctx);
    while (expr_match(ctx, TOK_PIPE)) {
        expr_advance(ctx);
        u32 right = expr_parse_bitxor(ctx);
        u32 rd = expr_alloc_reg(ctx);
        codegen_emit_or(ctx->codegen, rd, left, right);
        expr_free_reg(ctx, left);
        left = rd;
    }
    return left;
}

static inline u32 expr_parse_and(struct ExprCtx *ctx) {
    u32 left = expr_parse_bitor(ctx);
    while (expr_match(ctx, TOK_AND)) {
        expr_advance(ctx);
        u32 right = expr_parse_bitor(ctx);
        u32 rd = expr_alloc_reg(ctx);
        codegen_emit_and(ctx->codegen, rd, left, right);
        expr_free_reg(ctx, left);
        left = rd;
    }
    return left;
}

static inline u32 expr_parse_or(struct ExprCtx *ctx) {
    u32 left = expr_parse_and(ctx);
    while (expr_match(ctx, TOK_OR)) {
        expr_advance(ctx);
        u32 right = expr_parse_and(ctx);
        u32 rd = expr_alloc_reg(ctx);
        codegen_emit_or(ctx->codegen, rd, left, right);
        expr_free_reg(ctx, left);
        left = rd;
    }
    return left;
}

static inline u32 expr_parse_ternary(struct ExprCtx *ctx) {
    u32 cond = expr_parse_or(ctx);
    if (expr_match(ctx, TOK_QUESTION)) {
        expr_advance(ctx);
        u32 true_val = expr_parse_ternary(ctx);
        if (expr_match(ctx, TOK_COLON)) expr_advance(ctx);
        u32 false_val = expr_parse_ternary(ctx);
        u32 rd = expr_alloc_reg(ctx);
        codegen_emit_cmov_nz(ctx->codegen, rd, true_val);
        codegen_emit_cmov_z(ctx->codegen, rd, false_val);
        expr_free_reg(ctx, cond);
        expr_free_reg(ctx, true_val);
        expr_free_reg(ctx, false_val);
        return rd;
    }
    return cond;
}

static inline u32 expr_compile(struct Scanner *scanner, struct CodeGen *codegen) {
    struct ExprCtx ctx;
    expr_init(&ctx, scanner, codegen);
    return expr_parse_ternary(&ctx);
}

#endif /* APKC_LANG_EXPR_H */
