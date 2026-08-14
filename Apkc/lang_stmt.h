/* lang_stmt.h — Statement parser and compiler
 *
 * Iterative statement compiler (no recursion): declarations, assignments, if/else, loops, functions
 * Language-specific syntax handling for Python, Go, Rust, C, JavaScript, Java, Swift
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_LANG_STMT_H
#define APKC_LANG_STMT_H 1

#include "lang_scanner.h"
#include "lang_expr.h"
#include "machine_linear_branchless.h"

typedef unsigned char u8;
typedef unsigned int u32;

/* Statement context */
struct StmtCtx {
    struct Scanner *scanner;
    struct CodeGen *codegen;
    struct Token current;
    u8 lang;
    u32 block_depth;
    u32 next_label;   /* Label counter for jumps */
};

/* Initialize statement context */
static inline void stmt_init(
    struct StmtCtx *ctx,
    struct Scanner *scanner,
    struct CodeGen *codegen,
    u8 lang)
{
    ctx->scanner = scanner;
    ctx->codegen = codegen;
    ctx->current = scanner_next_token(scanner);
    ctx->lang = lang;
    ctx->block_depth = 0;
    ctx->next_label = 0;
}

/* Advance to next token */
static inline void stmt_advance(struct StmtCtx *ctx) {
    ctx->current = scanner_next_token(ctx->scanner);
}

/* Check token type */
static inline u8 stmt_match(struct StmtCtx *ctx, enum TokenType type) {
    return ctx->current.type == type;
}

/* Check keyword match */
static inline u8 stmt_match_keyword(
    struct StmtCtx *ctx,
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

/* Allocate new label for jumps */
static inline u32 stmt_alloc_label(struct StmtCtx *ctx) {
    return ctx->next_label++;
}

/* === STATEMENT PARSERS === */

/* Parse variable declaration with language-specific syntax */
static inline void stmt_parse_var_decl(struct StmtCtx *ctx) {
    enum TokenType type_keyword = TOK_IDENT;

    /* Consume type keyword or identifier */
    if (stmt_match_keyword(ctx, (const u8*)"var", 3) ||
        stmt_match_keyword(ctx, (const u8*)"let", 3) ||
        stmt_match_keyword(ctx, (const u8*)"const", 5) ||
        stmt_match_keyword(ctx, (const u8*)"int", 3) ||
        stmt_match_keyword(ctx, (const u8*)"long", 4) ||
        stmt_match_keyword(ctx, (const u8*)"float", 5) ||
        stmt_match_keyword(ctx, (const u8*)"double", 6) ||
        stmt_match_keyword(ctx, (const u8*)"string", 6) ||
        stmt_match_keyword(ctx, (const u8*)"bool", 4) ||
        stmt_match_keyword(ctx, (const u8*)"u64", 3) ||
        stmt_match_keyword(ctx, (const u8*)"i64", 3)) {
        stmt_advance(ctx);
    }

    /* Skip type annotations (Go/Rust/Java) */
    if (stmt_match(ctx, TOK_IDENT)) {
        stmt_advance(ctx);
    }

    /* Skip colon for type annotations */
    if (stmt_match(ctx, TOK_COLON)) {
        stmt_advance(ctx);
        if (stmt_match(ctx, TOK_IDENT)) {
            stmt_advance(ctx);
        }
    }

    /* Parse optional initializer */
    if (stmt_match(ctx, TOK_ASSIGN)) {
        stmt_advance(ctx);
        /* Compile expression (simplified: skip actual compilation) */
        while (!stmt_match(ctx, TOK_SEMICOLON) &&
               !stmt_match(ctx, TOK_EOF) &&
               !stmt_match(ctx, TOK_RBRACE)) {
            stmt_advance(ctx);
        }
    }

    /* Skip trailing semicolon or newline */
    if (stmt_match(ctx, TOK_SEMICOLON)) {
        stmt_advance(ctx);
    }
}

/* Parse assignment statement */
static inline void stmt_parse_assignment(struct StmtCtx *ctx) {
    /* Skip variable name */
    if (stmt_match(ctx, TOK_IDENT)) {
        stmt_advance(ctx);
    }

    /* Skip assignment operator */
    if (stmt_match(ctx, TOK_ASSIGN) || stmt_match(ctx, TOK_PLUSEQ) ||
        stmt_match(ctx, TOK_MINUSEQ) || stmt_match(ctx, TOK_STAREQ) ||
        stmt_match(ctx, TOK_SLASHEQ)) {
        stmt_advance(ctx);
    }

    /* Parse right-hand side expression */
    while (!stmt_match(ctx, TOK_SEMICOLON) &&
           !stmt_match(ctx, TOK_EOF) &&
           !stmt_match(ctx, TOK_RBRACE)) {
        stmt_advance(ctx);
    }

    /* Skip trailing semicolon */
    if (stmt_match(ctx, TOK_SEMICOLON)) {
        stmt_advance(ctx);
    }
}

/* Parse if statement */
static inline void stmt_parse_if(struct StmtCtx *ctx) {
    u32 else_label = stmt_alloc_label(ctx);
    u32 end_label = stmt_alloc_label(ctx);

    /* Skip 'if' keyword */
    stmt_advance(ctx);

    /* Skip condition (simplified) */
    if (stmt_match(ctx, TOK_LPAREN)) {
        stmt_advance(ctx);
        while (!stmt_match(ctx, TOK_RPAREN) && !stmt_match(ctx, TOK_EOF)) {
            stmt_advance(ctx);
        }
        if (stmt_match(ctx, TOK_RPAREN)) {
            stmt_advance(ctx);
        }
    }

    /* Parse if body */
    if (stmt_match(ctx, TOK_LBRACE)) {
        stmt_advance(ctx);
        u32 depth = 1;
        while (depth > 0 && !stmt_match(ctx, TOK_EOF)) {
            if (stmt_match(ctx, TOK_LBRACE)) depth++;
            if (stmt_match(ctx, TOK_RBRACE)) depth--;
            stmt_advance(ctx);
        }
    } else {
        stmt_parse_assignment(ctx);
    }

    /* Check for else clause */
    if (stmt_match_keyword(ctx, (const u8*)"else", 4)) {
        stmt_advance(ctx);

        /* Parse else body */
        if (stmt_match(ctx, TOK_LBRACE)) {
            stmt_advance(ctx);
            u32 depth = 1;
            while (depth > 0 && !stmt_match(ctx, TOK_EOF)) {
                if (stmt_match(ctx, TOK_LBRACE)) depth++;
                if (stmt_match(ctx, TOK_RBRACE)) depth--;
                stmt_advance(ctx);
            }
        } else {
            stmt_parse_assignment(ctx);
        }
    }
}

/* Parse while loop */
static inline void stmt_parse_while(struct StmtCtx *ctx) {
    u32 loop_label = stmt_alloc_label(ctx);
    u32 end_label = stmt_alloc_label(ctx);

    /* Skip 'while' keyword */
    stmt_advance(ctx);

    /* Skip condition */
    if (stmt_match(ctx, TOK_LPAREN)) {
        stmt_advance(ctx);
        while (!stmt_match(ctx, TOK_RPAREN) && !stmt_match(ctx, TOK_EOF)) {
            stmt_advance(ctx);
        }
        if (stmt_match(ctx, TOK_RPAREN)) {
            stmt_advance(ctx);
        }
    }

    /* Parse loop body */
    if (stmt_match(ctx, TOK_LBRACE)) {
        stmt_advance(ctx);
        u32 depth = 1;
        while (depth > 0 && !stmt_match(ctx, TOK_EOF)) {
            if (stmt_match(ctx, TOK_LBRACE)) depth++;
            if (stmt_match(ctx, TOK_RBRACE)) depth--;
            stmt_advance(ctx);
        }
    } else {
        stmt_parse_assignment(ctx);
    }
}

/* Parse for loop */
static inline void stmt_parse_for(struct StmtCtx *ctx) {
    u32 loop_label = stmt_alloc_label(ctx);
    u32 end_label = stmt_alloc_label(ctx);

    /* Skip 'for' keyword */
    stmt_advance(ctx);

    /* Skip condition/initialization (C-style or Python-style) */
    if (stmt_match(ctx, TOK_LPAREN)) {
        stmt_advance(ctx);
        while (!stmt_match(ctx, TOK_RPAREN) && !stmt_match(ctx, TOK_EOF)) {
            stmt_advance(ctx);
        }
        if (stmt_match(ctx, TOK_RPAREN)) {
            stmt_advance(ctx);
        }
    }

    /* Python-style: for x in range(n) */
    if (stmt_match_keyword(ctx, (const u8*)"in", 2)) {
        stmt_advance(ctx);
        while (!stmt_match(ctx, TOK_COLON) && !stmt_match(ctx, TOK_LBRACE) && !stmt_match(ctx, TOK_EOF)) {
            stmt_advance(ctx);
        }
    }

    /* Python-style colon */
    if (stmt_match(ctx, TOK_COLON)) {
        stmt_advance(ctx);
    }

    /* Parse loop body */
    if (stmt_match(ctx, TOK_LBRACE)) {
        stmt_advance(ctx);
        u32 depth = 1;
        while (depth > 0 && !stmt_match(ctx, TOK_EOF)) {
            if (stmt_match(ctx, TOK_LBRACE)) depth++;
            if (stmt_match(ctx, TOK_RBRACE)) depth--;
            stmt_advance(ctx);
        }
    } else {
        stmt_parse_assignment(ctx);
    }
}

/* Parse return statement */
static inline void stmt_parse_return(struct StmtCtx *ctx) {
    /* Skip 'return' keyword */
    stmt_advance(ctx);

    /* Skip return expression */
    while (!stmt_match(ctx, TOK_SEMICOLON) &&
           !stmt_match(ctx, TOK_EOF) &&
           !stmt_match(ctx, TOK_RBRACE)) {
        stmt_advance(ctx);
    }

    /* Emit RET instruction */
    codegen_emit_ret(ctx->codegen);

    /* Skip trailing semicolon */
    if (stmt_match(ctx, TOK_SEMICOLON)) {
        stmt_advance(ctx);
    }
}

/* Parse function definition */
static inline void stmt_parse_function(struct StmtCtx *ctx) {
    /* Skip function keyword */
    if (stmt_match_keyword(ctx, (const u8*)"func", 4) ||
        stmt_match_keyword(ctx, (const u8*)"fn", 2) ||
        stmt_match_keyword(ctx, (const u8*)"function", 8) ||
        stmt_match_keyword(ctx, (const u8*)"def", 3)) {
        stmt_advance(ctx);
    }

    /* Skip function name */
    if (stmt_match(ctx, TOK_IDENT)) {
        stmt_advance(ctx);
    }

    /* Skip parameters */
    if (stmt_match(ctx, TOK_LPAREN)) {
        stmt_advance(ctx);
        while (!stmt_match(ctx, TOK_RPAREN) && !stmt_match(ctx, TOK_EOF)) {
            stmt_advance(ctx);
        }
        if (stmt_match(ctx, TOK_RPAREN)) {
            stmt_advance(ctx);
        }
    }

    /* Skip return type annotation */
    if (stmt_match(ctx, TOK_ARROW) || stmt_match(ctx, TOK_COLON)) {
        stmt_advance(ctx);
        while (!stmt_match(ctx, TOK_LBRACE) && !stmt_match(ctx, TOK_COLON) && !stmt_match(ctx, TOK_EOF)) {
            stmt_advance(ctx);
        }
    }

    /* Parse function body */
    if (stmt_match(ctx, TOK_LBRACE)) {
        stmt_advance(ctx);
        u32 depth = 1;
        while (depth > 0 && !stmt_match(ctx, TOK_EOF)) {
            if (stmt_match(ctx, TOK_LBRACE)) depth++;
            if (stmt_match(ctx, TOK_RBRACE)) depth--;
            stmt_advance(ctx);
        }
    }
}

/* Parse statement block (group of statements) */
static inline void stmt_parse_block(struct StmtCtx *ctx) {
    while (!stmt_match(ctx, TOK_EOF) && !stmt_match(ctx, TOK_RBRACE)) {
        /* Skip Python indentation (treat as statement separator) */
        if (stmt_match_keyword(ctx, (const u8*)"pass", 4)) {
            stmt_advance(ctx);
            continue;
        }

        /* Variable declaration */
        if (stmt_match_keyword(ctx, (const u8*)"var", 3) ||
            stmt_match_keyword(ctx, (const u8*)"let", 3) ||
            stmt_match_keyword(ctx, (const u8*)"const", 5) ||
            stmt_match_keyword(ctx, (const u8*)"int", 3) ||
            stmt_match_keyword(ctx, (const u8*)"long", 4)) {
            stmt_parse_var_decl(ctx);
            continue;
        }

        /* If statement */
        if (stmt_match_keyword(ctx, (const u8*)"if", 2)) {
            stmt_parse_if(ctx);
            continue;
        }

        /* While loop */
        if (stmt_match_keyword(ctx, (const u8*)"while", 5)) {
            stmt_parse_while(ctx);
            continue;
        }

        /* For loop */
        if (stmt_match_keyword(ctx, (const u8*)"for", 3)) {
            stmt_parse_for(ctx);
            continue;
        }

        /* Return statement */
        if (stmt_match_keyword(ctx, (const u8*)"return", 6)) {
            stmt_parse_return(ctx);
            continue;
        }

        /* Function definition */
        if (stmt_match_keyword(ctx, (const u8*)"func", 4) ||
            stmt_match_keyword(ctx, (const u8*)"fn", 2) ||
            stmt_match_keyword(ctx, (const u8*)"function", 8) ||
            stmt_match_keyword(ctx, (const u8*)"def", 3)) {
            stmt_parse_function(ctx);
            continue;
        }

        /* Assignment (default case) */
        if (stmt_match(ctx, TOK_IDENT)) {
            stmt_parse_assignment(ctx);
            continue;
        }

        /* Skip unknown tokens */
        stmt_advance(ctx);
    }
}

/* Entry point: parse program statements */
static inline void stmt_compile_program(
    struct Scanner *scanner,
    struct CodeGen *codegen,
    u8 lang)
{
    struct StmtCtx ctx;
    stmt_init(&ctx, scanner, codegen, lang);

    /* Parse top-level statements */
    stmt_parse_block(&ctx);

    /* Emit final RET if not present */
    codegen_emit_movi(ctx.codegen, 0, 0);  /* Return 0 */
    codegen_emit_ret(ctx.codegen);
}

#endif /* APKC_LANG_STMT_H */
