#ifndef RAF_CONVSCAN_H
#define RAF_CONVSCAN_H

typedef unsigned char      rcs_u8;
typedef unsigned int       rcs_u32;
typedef unsigned long long rcs_u64;
typedef signed int         rcs_s32;

#define RCS_OK 0
#define RCS_E_NULL -1
#define RCS_E_SYNTAX -2
#define RCS_E_DEPTH -3
#define RCS_E_TRUNCATED -4
#define RCS_E_STATE -5
#define RCS_MAX_DEPTH 256u

typedef struct rcs_stats {
    rcs_u64 bytes;
    rcs_u64 objects;
    rcs_u64 arrays;
    rcs_u64 strings;
    rcs_u64 numbers;
    rcs_u64 literals;
    rcs_u64 colons;
    rcs_u64 commas;
    rcs_u64 conversation_id_keys;
    rcs_u64 title_keys;
    rcs_u64 create_time_keys;
    rcs_u64 update_time_keys;
    rcs_u64 mapping_keys;
    rcs_u64 message_keys;
    rcs_u32 max_depth;
    rcs_u32 crc32c;
} rcs_stats;

typedef struct rcs_ctx {
    rcs_stats stats;
    rcs_u32 crc;
    rcs_u32 depth;
    rcs_u32 in_string;
    rcs_u32 escape;
    rcs_u32 unicode_left;
    rcs_u32 token_kind;
    rcs_u32 token_len;
    rcs_u32 expecting_key;
    rcs_u32 root_seen;
    rcs_u32 root_closed;
    rcs_u8 token[32];
    rcs_u8 stack[RCS_MAX_DEPTH];
} rcs_ctx;

void rcs_init(rcs_ctx *ctx);
int rcs_feed(rcs_ctx *ctx, const void *data, rcs_u32 size);
int rcs_finish(rcs_ctx *ctx, rcs_stats *out);

#endif
