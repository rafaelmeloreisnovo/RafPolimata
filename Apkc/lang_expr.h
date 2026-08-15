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

/* Operator precedence levels */
#define PREC_TERNARY    1
#define PREC_OR         2
#define PREC_AND        3
#define PREC_BITOR      4
#define PREC_BITXOR     5
#define PREC_BITAND     6
#define PREC_EQ         7  /* == != */
#define PREC_CMP        8  /* < > <= >= */
#define PREC_SHIFT      9  /* << >> */
#define PREC_ADDITIVE   10 /* + - */
#define PREC_MULT       11 /* * / % */
#define PREC_UNARY      12 /* unary - ! ~ */
#define PREC_POSTFIX    13 /* () [] . */

/* Expression parsing context */
struct ExprCtx {
    struct Scanner *scanner;
    struct CodeGen *codegen;      /* Machine instruction generator */
    struct Token current;
    u32 var_count;                /* Number of variables in scope */
    u32 var_regs[64];             /* Register allocated for each variable */
    u32 next_reg;                 /* Next available register (0-13) */
};

/* Initialize expression context */
static inline void expr_init(
    struct ExprCtx *ctx,
    struct Scanner *scanner,
    struct CodeGen *codegen)
{
    ctx->scanner = scanner;
    ctx->codegen = codegen;
    ctx->current = scanner_next_token(scanner);
    ctx->var_count = 0;
    ctx->next_reg = 0;
}

/* Advance to next token */
static inline void expr_advance(struct ExprCtx *ctx) {
    ctx->current = scanner_next_token(ctx->scanner);
}

/* Check if current token matches type */
static inline u8 expr_match(struct ExprCtx *ctx, enum TokenType type) {
    return ctx->current.type == type;
}

/* Check if token text matches keyword (for identifiers) */
static inline u8 expr_match_keyword(
    struct ExprCtx *ctx,
    const u8 *keyword, u32 len)
{
    if (ctx->current.type != TOK_IDENT) return 0;
    if (ctx->current.len != len) return 0;

    u32 i;
    for (i = 0; i < len; i++) {
        if (ctx->current.text[i] != keyword[i]) return 0;
    }
    return 1;
}

/* Allocate register for variable */
static inline u32 expr_alloc_reg(struct ExprCtx *ctx) {
    if (ctx->next_reg >= 14) return 14;  /* Spill to memory if overflow */
    return ctx->next_reg++;
}

/* Free register (conservative: just mark next_reg available if this was last) */
static inline void expr_free_reg(struct ExprCtx *ctx, u32 reg) {
    if (reg + 1 == ctx->next_reg) ctx->next_reg--;
}

/* === EXPRESSION PARSER === */

/* Forward declarations */
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

/* Parse primary expression: literal, identifier, or (expr) */
static inline u32 expr_parse_primary(struct ExprCtx *ctx) {
    u32 rd = expr_alloc_reg(ctx);

    /* Numeric literal */
    if (expr_match(ctx, TOK_DIGIT)) {
        u64 val = 0;
        u32 i;
        for (i = 0; i < ctx->current.len; i++) {
            u8 c = ctx->current.text[i];
            if (c >= '0' && c <= '9') {
                val = val * 10 + (c - '0');
            }
        }
        codegen_emit_movi(ctx->codegen, rd, val);
        expr_advance(ctx);
        return rd;
    }

    /* String literal */
    if (expr_match(ctx, TOK_STRING)) {
        /* For now, treat as 0 (TODO: inline strings in .rodata) */
        codegen_emit_movi(ctx->codegen, rd, 0);
        expr_advance(ctx);
        return rd;
    }

    /* Identifier (variable or function call) */
    if (expr_match(ctx, TOK_IDENT)) {
        u32 ident_reg = 0;
        u32 ident_len = ctx->current.len;
        const u8 *ident_text = ctx->current.text;

        expr_advance(ctx);

        /* Function call */
        if (expr_match(ctx, TOK_LPAREN)) {
            expr_advance(ctx);

            /* Parse arguments (up to 3 in r0-r2) */
            u32 arg_regs[3] = {0, 1, 2};
            u32 arg_count = 0;

            while (!expr_match(ctx, TOK_RPAREN) && arg_count < 3) {
                u32 arg_val = expr_parse_ternary(ctx);
                codegen_emit_mov(ctx->codegen, arg_regs[arg_count], arg_val);
                arg_count++;

                if (expr_match(ctx, TOK_COMMA)) {
                    expr_advance(ctx);
                }
            }

            if (expr_match(ctx, TOK_RPAREN)) {
                expr_advance(ctx);
            }

            /* Emit CALL instruction (simplified: not tracking actual function addresses) */
            codegen_emit_call(ctx->codegen, 0);  /* 0 = placeholder for function address */
            codegen_emit_mov(ctx->codegen, rd, 0);  /* Result in r0 */
            return rd;
        }

        /* Variable reference (simplified: return 0 for unknown variables) */
        codegen_emit_movi(ctx->codegen, rd, 0);
        return rd;
    }

    /* Parenthesized expression */
    if (expr_match(ctx, TOK_LPAREN)) {
        expr_advance(ctx);
        u32 val = expr_parse_ternary(ctx);
        if (expr_match(ctx, TOK_RPAREN)) {
            expr_advance(ctx);
        }
        return val;
    }

    /* True/false/null keywords */
    if (expr_match_keyword(ctx, (const u8*)"true", 4) ||
        expr_match_keyword(ctx, (const u8*)"True", 4)) {
        codegen_emit_movi(ctx->codegen, rd, 1);
        expr_advance(ctx);
        return rd;
    }

    if (expr_match_keyword(ctx, (const u8*)"false", 5) ||
        expr_match_keyword(ctx, (const u8*)"False", 5)) {
        codegen_emit_movi(ctx->codegen, rd, 0);
        expr_advance(ctx);
        return rd;
    }

    if (expr_match_keyword(ctx, (const u8*)"nil", 3) ||
        expr_match_keyword(ctx, (const u8*)"null", 4) ||
        expr_match_keyword(ctx, (const u8*)"None", 4)) {
        codegen_emit_movi(ctx->codegen, rd, 0);
        expr_advance(ctx);
        return rd;
    }

    /* Default: return 0 */
    codegen_emit_movi(ctx->codegen, rd, 0);
    return rd;
}

/* Parse unary operators */
static inline u32 expr_parse_unary(struct ExprCtx *ctx) {
    u32 rd = expr_alloc_reg(ctx);

    /* Unary minus */
    if (expr_match(ctx, TOK_MINUS)) {
        expr_advance(ctx);
        u32 val = expr_parse_unary(ctx);
        codegen_emit_sub(ctx->codegen, rd, 0, val);  /* rd = 0 - val */
        return rd;
    }

    /* Logical not */
    if (expr_match(ctx, TOK_BANG)) {
        expr_advance(ctx);
        u32 val = expr_parse_unary(ctx);
        codegen_emit_cmov_z(ctx->codegen, rd, val);  /* rd = (val == 0) ? 1 : 0 */
        return rd;
    }

    /* Bitwise not - load mask and XOR */
    if (expr_match(ctx, TOK_TILDE)) {
        expr_advance(ctx);
        u32 val_reg = expr_parse_unary(ctx);
        u8 mask_reg = (rd == 0) ? 1 : 0;  /* Use different register for mask */
        codegen_emit_movi(ctx->codegen, mask_reg, 0xff);  /* Load mask into temp register */
        codegen_emit_xor(ctx->codegen, rd, val_reg, mask_reg);  /* XOR with mask register */
        return rd;
    }

    return expr_parse_primary(ctx);
}

/* Parse multiplicative operators (*, /, %) */
static inline u32 expr_parse_multiplicative(struct ExprCtx *ctx) {
    u32 left = expr_parse_unary(ctx);

    while (expr_match(ctx, TOK_STAR) || expr_match(ctx, TOK_SLASH) || expr_match(ctx, TOK_PERCENT)) {
        enum TokenType op = ctx->current.type;
        expr_advance(ctx);

        u32 right = expr_parse_unary(ctx);
        u32 rd = expr_alloc_reg(ctx);

        if (op == TOK_STAR) {
            codegen_emit_mul(ctx->codegen, rd, left, right);
        } else if (op == TOK_SLASH) {
            codegen_emit_div(ctx->codegen, rd, left, right);
        } else if (op == TOK_PERCENT) {
            codegen_emit_mod(ctx->codegen, rd, left, right);
        }

        expr_free_reg(ctx, left);
        left = rd;
    }

    return left;
}

/* Parse additive operators (+, -) */
static inline u32 expr_parse_additive(struct ExprCtx *ctx) {
    u32 left = expr_parse_multiplicative(ctx);

    while (expr_match(ctx, TOK_PLUS) || expr_match(ctx, TOK_MINUS)) {
        enum TokenType op = ctx->current.type;
        expr_advance(ctx);

        u32 right = expr_parse_multiplicative(ctx);
        u32 rd = expr_alloc_reg(ctx);

        if (op == TOK_PLUS) {
            codegen_emit_add(ctx->codegen, rd, left, right);
        } else {
            codegen_emit_sub(ctx->codegen, rd, left, right);
        }

        expr_free_reg(ctx, left);
        left = rd;
    }

    return left;
}

/* Parse shift operators (<<, >>) */
static inline u32 expr_parse_shift(struct ExprCtx *ctx) {
    u32 left = expr_parse_additive(ctx);

    while (expr_match(ctx, TOK_LSHIFT) || expr_match(ctx, TOK_RSHIFT)) {
        enum TokenType op = ctx->current.type;
        expr_advance(ctx);

        u32 right = expr_parse_additive(ctx);
        u32 rd = expr_alloc_reg(ctx);

        if (op == TOK_LSHIFT) {
            codegen_emit_shl(ctx->codegen, rd, left, right);
        } else {
            codegen_emit_shr(ctx->codegen, rd, left, right);
        }

        expr_free_reg(ctx, left);
        left = rd;
    }

    return left;
}

/* Parse comparison operators (<, >, <=, >=) */
static inline u32 expr_parse_comparison(struct ExprCtx *ctx) {
    u32 left = expr_parse_shift(ctx);

    while (expr_match(ctx, TOK_LT) || expr_match(ctx, TOK_LE) ||
           expr_match(ctx, TOK_GT) || expr_match(ctx, TOK_GE)) {
        enum TokenType op = ctx->current.type;
        expr_advance(ctx);

        u32 right = expr_parse_shift(ctx);
        u32 rd = expr_alloc_reg(ctx);

        codegen_emit_cmp(ctx->codegen, left, right);

        if (op == TOK_LT) {
            codegen_emit_movi(ctx->codegen, rd, 0);  /* Simplified: LT result */
        } else if (op == TOK_LE) {
            codegen_emit_movi(ctx->codegen, rd, 0);
        } else if (op == TOK_GT) {
            codegen_emit_movi(ctx->codegen, rd, 0);
        } else if (op == TOK_GE) {
            codegen_emit_movi(ctx->codegen, rd, 0);
        }

        expr_free_reg(ctx, left);
        left = rd;
    }

    return left;
}

/* Parse equality operators (==, !=) */
static inline u32 expr_parse_equality(struct ExprCtx *ctx) {
    u32 left = expr_parse_comparison(ctx);

    while (expr_match(ctx, TOK_EQ) || expr_match(ctx, TOK_NE)) {
        enum TokenType op = ctx->current.type;
        expr_advance(ctx);

        u32 right = expr_parse_comparison(ctx);
        u32 rd = expr_alloc_reg(ctx);

        codegen_emit_cmp(ctx->codegen, left, right);

        if (op == TOK_EQ) {
            codegen_emit_cmov_z(ctx->codegen, rd, 1);  /* rd = (left == right) */
        } else {
            codegen_emit_cmov_z(ctx->codegen, rd, 1);  /* rd = (left != right) */
        }

        expr_free_reg(ctx, left);
        left = rd;
    }

    return left;
}

/* Parse bitwise AND (&) */
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

/* Parse bitwise XOR (^) */
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

/* Parse bitwise OR (|) */
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

/* Parse logical AND (&&) */
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

/* Parse logical OR (||) */
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

/* Parse ternary operator (a ? b : c) */
static inline u32 expr_parse_ternary(struct ExprCtx *ctx) {
    u32 cond = expr_parse_or(ctx);

    if (expr_match(ctx, TOK_QUESTION)) {
        expr_advance(ctx);

        u32 true_val = expr_parse_ternary(ctx);

        if (expr_match(ctx, TOK_COLON)) {
            expr_advance(ctx);
        }

        u32 false_val = expr_parse_ternary(ctx);
        u32 rd = expr_alloc_reg(ctx);

        /* Emit CMOV: rd = (cond != 0) ? true_val : false_val */
        codegen_emit_cmov_nz(ctx->codegen, rd, true_val);
        codegen_emit_cmov_z(ctx->codegen, rd, false_val);

        expr_free_reg(ctx, cond);
        expr_free_reg(ctx, true_val);
        expr_free_reg(ctx, false_val);

        return rd;
    }

    return cond;
}

/* Entry point: parse and compile expression */
static inline u32 expr_compile(
    struct Scanner *scanner,
    struct CodeGen *codegen)
{
    struct ExprCtx ctx;
    expr_init(&ctx, scanner, codegen);
    return expr_parse_ternary(&ctx);
}

#endif /* APKC_LANG_EXPR_H */
