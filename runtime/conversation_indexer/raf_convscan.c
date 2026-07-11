#include "raf_convscan.h"

static void rcs_zero(void *p, rcs_u32 n) {
    rcs_u8 *b = (rcs_u8 *)p;
    rcs_u32 i;
    for (i = 0; i < n; ++i) b[i] = 0;
}

static rcs_u32 rcs_is_ws(rcs_u8 c) {
    return (rcs_u32)((c == ' ') | (c == '\n') | (c == '\r') | (c == '\t'));
}

static rcs_u32 rcs_is_digit(rcs_u8 c) {
    return (rcs_u32)((c >= '0') & (c <= '9'));
}

static rcs_u32 rcs_is_hex(rcs_u8 c) {
    return (rcs_u32)(rcs_is_digit(c) | ((c >= 'a') & (c <= 'f')) | ((c >= 'A') & (c <= 'F')));
}

static rcs_u32 rcs_eq(const rcs_u8 *a, rcs_u32 n, const char *b) {
    rcs_u32 i = 0;
    while (b[i]) {
        if (i >= n || a[i] != (rcs_u8)b[i]) return 0u;
        ++i;
    }
    return (rcs_u32)(i == n);
}

static void rcs_count_key(rcs_ctx *ctx) {
    if (!ctx->expecting_key) return;
    if (rcs_eq(ctx->token, ctx->token_len, "id")) ctx->stats.conversation_id_keys++;
    else if (rcs_eq(ctx->token, ctx->token_len, "title")) ctx->stats.title_keys++;
    else if (rcs_eq(ctx->token, ctx->token_len, "create_time")) ctx->stats.create_time_keys++;
    else if (rcs_eq(ctx->token, ctx->token_len, "update_time")) ctx->stats.update_time_keys++;
    else if (rcs_eq(ctx->token, ctx->token_len, "mapping")) ctx->stats.mapping_keys++;
    else if (rcs_eq(ctx->token, ctx->token_len, "message")) ctx->stats.message_keys++;
}

static void rcs_crc_byte(rcs_ctx *ctx, rcs_u8 c) {
    rcs_u32 b;
    ctx->crc ^= c;
    for (b = 0; b < 8u; ++b)
        ctx->crc = (ctx->crc >> 1) ^ (0x82F63B78u & (0u - (ctx->crc & 1u)));
}

void rcs_init(rcs_ctx *ctx) {
    if (!ctx) return;
    rcs_zero(ctx, (rcs_u32)sizeof(*ctx));
    ctx->crc = 0xFFFFFFFFu;
}

static int rcs_push(rcs_ctx *ctx, rcs_u8 kind) {
    if (ctx->depth >= RCS_MAX_DEPTH) return RCS_E_DEPTH;
    ctx->stack[ctx->depth++] = kind;
    if (ctx->depth > ctx->stats.max_depth) ctx->stats.max_depth = ctx->depth;
    ctx->expecting_key = (rcs_u32)(kind == '{');
    return RCS_OK;
}

static int rcs_pop(rcs_ctx *ctx, rcs_u8 kind) {
    if (ctx->depth == 0u) return RCS_E_SYNTAX;
    if (ctx->stack[ctx->depth - 1u] != kind) return RCS_E_SYNTAX;
    ctx->depth--;
    ctx->expecting_key = (rcs_u32)((ctx->depth > 0u) && (ctx->stack[ctx->depth - 1u] == '{'));
    if (ctx->depth == 0u) ctx->root_closed = 1u;
    return RCS_OK;
}

int rcs_feed(rcs_ctx *ctx, const void *data, rcs_u32 size) {
    const rcs_u8 *p = (const rcs_u8 *)data;
    rcs_u32 i;
    if (!ctx || (!data && size)) return RCS_E_NULL;
    if (ctx->root_closed && size) return RCS_E_STATE;

    for (i = 0; i < size; ++i) {
        rcs_u8 c = p[i];
        int rc;
        ctx->stats.bytes++;
        rcs_crc_byte(ctx, c);

        if (ctx->in_string) {
            if (ctx->unicode_left) {
                if (!rcs_is_hex(c)) return RCS_E_SYNTAX;
                ctx->unicode_left--;
                continue;
            }
            if (ctx->escape) {
                ctx->escape = 0u;
                if (c == 'u') ctx->unicode_left = 4u;
                else if (!(c == '"' || c == '\\' || c == '/' || c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't')) return RCS_E_SYNTAX;
                continue;
            }
            if (c == '\\') {
                ctx->escape = 1u;
                continue;
            }
            if (c == '"') {
                ctx->in_string = 0u;
                ctx->stats.strings++;
                rcs_count_key(ctx);
                continue;
            }
            if (c < 0x20u) return RCS_E_SYNTAX;
            if (ctx->token_len < (rcs_u32)sizeof(ctx->token)) ctx->token[ctx->token_len++] = c;
            continue;
        }

        if (ctx->token_kind) {
            rcs_u32 cont = 0u;
            if (ctx->token_kind == 1u)
                cont = (rcs_u32)(rcs_is_digit(c) || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-');
            else
                cont = (rcs_u32)((c >= 'a') & (c <= 'z'));
            if (cont) continue;
            if (ctx->token_kind == 1u) ctx->stats.numbers++;
            else ctx->stats.literals++;
            ctx->token_kind = 0u;
        }

        if (rcs_is_ws(c)) continue;
        if (ctx->root_closed) return RCS_E_SYNTAX;

        if (c == '{') {
            if (!ctx->root_seen) ctx->root_seen = 1u;
            ctx->stats.objects++;
            rc = rcs_push(ctx, '{');
            if (rc) return rc;
        } else if (c == '[') {
            if (!ctx->root_seen) ctx->root_seen = 1u;
            ctx->stats.arrays++;
            rc = rcs_push(ctx, '[');
            if (rc) return rc;
        } else if (c == '}') {
            rc = rcs_pop(ctx, '{');
            if (rc) return rc;
        } else if (c == ']') {
            rc = rcs_pop(ctx, '[');
            if (rc) return rc;
        } else if (c == '"') {
            ctx->in_string = 1u;
            ctx->token_len = 0u;
        } else if (c == ':') {
            ctx->stats.colons++;
            ctx->expecting_key = 0u;
        } else if (c == ',') {
            ctx->stats.commas++;
            ctx->expecting_key = (rcs_u32)((ctx->depth > 0u) && (ctx->stack[ctx->depth - 1u] == '{'));
        } else if (c == '-' || rcs_is_digit(c)) {
            ctx->token_kind = 1u;
        } else if (c == 't' || c == 'f' || c == 'n') {
            ctx->token_kind = 2u;
        } else {
            return RCS_E_SYNTAX;
        }
    }
    return RCS_OK;
}

int rcs_finish(rcs_ctx *ctx, rcs_stats *out) {
    if (!ctx || !out) return RCS_E_NULL;
    if (ctx->in_string || ctx->escape || ctx->unicode_left || ctx->depth != 0u || !ctx->root_seen || !ctx->root_closed)
        return RCS_E_TRUNCATED;
    if (ctx->token_kind == 1u) ctx->stats.numbers++;
    else if (ctx->token_kind == 2u) ctx->stats.literals++;
    ctx->token_kind = 0u;
    ctx->stats.crc32c = ~ctx->crc;
    *out = ctx->stats;
    return RCS_OK;
}
