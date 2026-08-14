/* lang_scanner.h — Freestanding language tokenizer
 *
 * Real tokenization for 7 languages (Python, Go, Rust, C, JavaScript, Java, Swift)
 * Character-by-character linear scan, no malloc, no libc.
 *
 * FREESTANDING: No malloc, no libc, all stack allocation.
 */

#ifndef APKC_LANG_SCANNER_H
#define APKC_LANG_SCANNER_H 1

typedef unsigned char u8;
typedef unsigned int u32;

/* Token types */
enum TokenType {
    TOK_EOF = 0,
    TOK_DIGIT,       /* 0-9 or 0x... */
    TOK_IDENT,       /* identifier or keyword */
    TOK_STRING,      /* "..." or '...' */
    TOK_LPAREN,      /* ( */
    TOK_RPAREN,      /* ) */
    TOK_LBRACE,      /* { */
    TOK_RBRACE,      /* } */
    TOK_LBRACKET,    /* [ */
    TOK_RBRACKET,    /* ] */
    TOK_SEMICOLON,   /* ; */
    TOK_COMMA,       /* , */
    TOK_COLON,       /* : */
    TOK_DOT,         /* . */
    TOK_ARROW,       /* -> or => */
    TOK_ASSIGN,      /* = */
    TOK_EQ,          /* == */
    TOK_NE,          /* != */
    TOK_LT,          /* < */
    TOK_LE,          /* <= */
    TOK_GT,          /* > */
    TOK_GE,          /* >= */
    TOK_PLUS,        /* + */
    TOK_MINUS,       /* - */
    TOK_STAR,        /* * */
    TOK_SLASH,       /* / */
    TOK_PERCENT,     /* % */
    TOK_AMP,         /* & */
    TOK_PIPE,        /* | */
    TOK_CARET,       /* ^ */
    TOK_TILDE,       /* ~ */
    TOK_BANG,        /* ! */
    TOK_QUESTION,    /* ? */
    TOK_LSHIFT,      /* << */
    TOK_RSHIFT,      /* >> */
    TOK_AND,         /* && */
    TOK_OR,          /* || */
    TOK_PLUSEQ,      /* += */
    TOK_MINUSEQ,     /* -= */
    TOK_STAREQ,      /* *= */
    TOK_SLASHEQ,     /* /= */
    TOK_INCREQ,      /* := */
    TOK_WALRUS,      /* := (Go short declaration) */
};

/* Token struct */
struct Token {
    enum TokenType type;
    u32 pos;              /* Position in source */
    u32 len;              /* Token length */
    const u8 *text;       /* Pointer to token text in source */
};

/* Scanner state */
struct Scanner {
    const u8 *src;        /* Source buffer */
    u32 src_len;          /* Source length */
    u32 pos;              /* Current position */
    u8 lang;              /* Language type (LP_PY, LP_GO, etc.) */
};

/* === SCANNER API === */

/* Initialize scanner from source buffer */
static inline void scanner_init(
    struct Scanner *s,
    const u8 *src, u32 src_len,
    u8 lang)
{
    s->src = src;
    s->src_len = src_len;
    s->pos = 0;
    s->lang = lang;
}

/* Check if character is whitespace */
static inline u8 is_whitespace(u8 c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/* Check if character is digit */
static inline u8 is_digit(u8 c) {
    return c >= '0' && c <= '9';
}

/* Check if character is hex digit */
static inline u8 is_hex_digit(u8 c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

/* Check if character is alpha */
static inline u8 is_alpha(u8 c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

/* Check if character starts identifier */
static inline u8 is_ident_start(u8 c) {
    return is_alpha(c);
}

/* Check if character continues identifier */
static inline u8 is_ident_cont(u8 c) {
    return is_alpha(c) || is_digit(c);
}

/* Peek current character */
static inline u8 scanner_peek(struct Scanner *s) {
    if (s->pos >= s->src_len) return 0;
    return s->src[s->pos];
}

/* Peek next character */
static inline u8 scanner_peek_next(struct Scanner *s) {
    if (s->pos + 1 >= s->src_len) return 0;
    return s->src[s->pos + 1];
}

/* Advance past current character */
static inline void scanner_advance(struct Scanner *s) {
    if (s->pos < s->src_len) s->pos++;
}

/* Skip whitespace and comments */
static inline void scanner_skip_whitespace_and_comments(struct Scanner *s) {
    while (s->pos < s->src_len) {
        u8 c = scanner_peek(s);

        if (is_whitespace(c)) {
            scanner_advance(s);
            continue;
        }

        /* C-style comments */
        if (c == '/' && scanner_peek_next(s) == '/') {
            scanner_advance(s);  /* skip / */
            scanner_advance(s);  /* skip / */
            while (s->pos < s->src_len && scanner_peek(s) != '\n') {
                scanner_advance(s);
            }
            if (s->pos < s->src_len) scanner_advance(s);  /* skip \n */
            continue;
        }

        if (c == '/' && scanner_peek_next(s) == '*') {
            scanner_advance(s);  /* skip / */
            scanner_advance(s);  /* skip * */
            while (s->pos + 1 < s->src_len) {
                if (scanner_peek(s) == '*' && scanner_peek_next(s) == '/') {
                    scanner_advance(s);  /* skip * */
                    scanner_advance(s);  /* skip / */
                    break;
                }
                scanner_advance(s);
            }
            continue;
        }

        /* Python-style comments */
        if (c == '#') {
            while (s->pos < s->src_len && scanner_peek(s) != '\n') {
                scanner_advance(s);
            }
            if (s->pos < s->src_len) scanner_advance(s);  /* skip \n */
            continue;
        }

        break;
    }
}

/* Scan next token */
static inline struct Token scanner_next_token(struct Scanner *s) {
    struct Token tok = {TOK_EOF, 0, 0, NULL};

    scanner_skip_whitespace_and_comments(s);

    if (s->pos >= s->src_len) {
        tok.type = TOK_EOF;
        return tok;
    }

    tok.pos = s->pos;
    tok.text = &s->src[s->pos];
    u8 c = scanner_peek(s);

    /* Single-character tokens */
    if (c == '(') {
        tok.type = TOK_LPAREN;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == ')') {
        tok.type = TOK_RPAREN;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '{') {
        tok.type = TOK_LBRACE;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '}') {
        tok.type = TOK_RBRACE;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '[') {
        tok.type = TOK_LBRACKET;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == ']') {
        tok.type = TOK_RBRACKET;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == ';') {
        tok.type = TOK_SEMICOLON;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == ',') {
        tok.type = TOK_COMMA;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '.') {
        tok.type = TOK_DOT;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '?') {
        tok.type = TOK_QUESTION;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '~') {
        tok.type = TOK_TILDE;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    /* Multi-character operators */
    if (c == '-') {
        if (scanner_peek_next(s) == '>') {
            tok.type = TOK_ARROW;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        if (scanner_peek_next(s) == '=') {
            tok.type = TOK_MINUSEQ;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_MINUS;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '+') {
        if (scanner_peek_next(s) == '=') {
            tok.type = TOK_PLUSEQ;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_PLUS;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '*') {
        if (scanner_peek_next(s) == '=') {
            tok.type = TOK_STAREQ;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_STAR;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '/') {
        if (scanner_peek_next(s) == '=') {
            tok.type = TOK_SLASHEQ;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_SLASH;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '%') {
        tok.type = TOK_PERCENT;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '=') {
        if (scanner_peek_next(s) == '=') {
            tok.type = TOK_EQ;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        if (scanner_peek_next(s) == '>') {
            tok.type = TOK_ARROW;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_ASSIGN;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '!') {
        if (scanner_peek_next(s) == '=') {
            tok.type = TOK_NE;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_BANG;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '<') {
        if (scanner_peek_next(s) == '<') {
            tok.type = TOK_LSHIFT;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        if (scanner_peek_next(s) == '=') {
            tok.type = TOK_LE;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_LT;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '>') {
        if (scanner_peek_next(s) == '>') {
            tok.type = TOK_RSHIFT;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        if (scanner_peek_next(s) == '=') {
            tok.type = TOK_GE;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_GT;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '&') {
        if (scanner_peek_next(s) == '&') {
            tok.type = TOK_AND;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_AMP;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '|') {
        if (scanner_peek_next(s) == '|') {
            tok.type = TOK_OR;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_PIPE;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == '^') {
        tok.type = TOK_CARET;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    if (c == ':') {
        if (scanner_peek_next(s) == '=') {
            tok.type = TOK_INCREQ;
            tok.len = 2;
            scanner_advance(s);
            scanner_advance(s);
            return tok;
        }
        tok.type = TOK_COLON;
        tok.len = 1;
        scanner_advance(s);
        return tok;
    }

    /* String literals */
    if (c == '"' || c == '\'') {
        u8 quote = c;
        scanner_advance(s);
        u32 start = s->pos;

        while (s->pos < s->src_len && scanner_peek(s) != quote) {
            if (scanner_peek(s) == '\\') {
                scanner_advance(s);  /* skip backslash */
                if (s->pos < s->src_len) scanner_advance(s);  /* skip escaped char */
            } else {
                scanner_advance(s);
            }
        }

        if (s->pos < s->src_len) scanner_advance(s);  /* skip closing quote */

        tok.type = TOK_STRING;
        tok.len = s->pos - tok.pos;
        return tok;
    }

    /* Numeric literals */
    if (is_digit(c)) {
        u32 start = s->pos;

        if (c == '0' && (scanner_peek_next(s) == 'x' || scanner_peek_next(s) == 'X')) {
            scanner_advance(s);  /* skip 0 */
            scanner_advance(s);  /* skip x */
            while (s->pos < s->src_len && is_hex_digit(scanner_peek(s))) {
                scanner_advance(s);
            }
        } else {
            while (s->pos < s->src_len && is_digit(scanner_peek(s))) {
                scanner_advance(s);
            }
        }

        tok.type = TOK_DIGIT;
        tok.len = s->pos - start;
        return tok;
    }

    /* Identifiers and keywords */
    if (is_ident_start(c)) {
        u32 start = s->pos;
        scanner_advance(s);

        while (s->pos < s->src_len && is_ident_cont(scanner_peek(s))) {
            scanner_advance(s);
        }

        tok.type = TOK_IDENT;
        tok.len = s->pos - start;
        return tok;
    }

    /* Unknown character — skip it */
    tok.type = TOK_IDENT;  /* treat as token to avoid infinite loop */
    tok.len = 1;
    scanner_advance(s);
    return tok;
}

#endif /* APKC_LANG_SCANNER_H */
