#include "../include/raf_fs_z0_token.h"

#define Z0_ASSERT(expr, code) do { if (!(expr)) return (code); } while (0)

int main(void)
{
    static const raf_z0_u8 sentinel[1] = { 0xabu };
    static const raf_z0_u8 nul_byte[1] = { 0x00u };
    static const raf_z0_u8 space_byte[1] = { 0x20u };
    static const raf_z0_u8 bytes[3] = { 0x41u, 0x42u, 0x43u };
    raf_z0_token token;
    raf_z0_view absent = { (const raf_z0_u8 *)0, 0u };
    raf_z0_view empty = { sentinel, 0u };
    raf_z0_view nul = { nul_byte, 1u };
    raf_z0_view space = { space_byte, 1u };
    raf_z0_view present = { bytes, 3u };
    raf_z0_view invalid = { (const raf_z0_u8 *)0, 1u };

    Z0_ASSERT(RAF_Z0_CONTEXT_LEFT == 0u, 1);
    Z0_ASSERT(RAF_Z0_CONTEXT_RIGHT == 0u, 2);
    Z0_ASSERT(RAF_Z0_ATTENTION_EDGE_COUNT == 0u, 3);
    Z0_ASSERT(RAF_Z0_LEARNED_WEIGHT_COUNT == 0u, 4);
    Z0_ASSERT(RAF_Z0_VOCAB_ENTRY_COUNT == 0u, 5);
    Z0_ASSERT(RAF_Z0_NORMALIZATION_PASSES == 0u, 6);
    Z0_ASSERT(RAF_Z0_SYNTHETIC_TOKEN_COUNT == 0u, 7);
    Z0_ASSERT(RAF_Z0_HISTORY_BYTES == 0u, 8);

    Z0_ASSERT(raf_z0_classify(absent) == RAF_Z0_STATE_ABSENT, 10);
    Z0_ASSERT(raf_z0_token_count(absent) == 0u, 11);
    Z0_ASSERT(raf_z0_token_at(absent, 0u, &token) == RAF_Z0_INPUT_ABSENT, 12);

    Z0_ASSERT(raf_z0_classify(empty) == RAF_Z0_STATE_EMPTY, 20);
    Z0_ASSERT(raf_z0_token_count(empty) == 0u, 21);
    Z0_ASSERT(raf_z0_token_at(empty, 0u, &token) == RAF_Z0_NO_TOKEN, 22);

    Z0_ASSERT(raf_z0_classify(nul) == RAF_Z0_STATE_PRESENT, 30);
    Z0_ASSERT(raf_z0_token_count(nul) == 1u, 31);
    Z0_ASSERT(raf_z0_token_at(nul, 0u, &token) == RAF_Z0_EMIT_TOKEN, 32);
    Z0_ASSERT(token.byte == 0x00u, 33);
    Z0_ASSERT(token.offset == 0u, 34);

    Z0_ASSERT(raf_z0_classify(space) == RAF_Z0_STATE_PRESENT, 40);
    Z0_ASSERT(raf_z0_token_count(space) == 1u, 41);
    Z0_ASSERT(raf_z0_token_at(space, 0u, &token) == RAF_Z0_EMIT_TOKEN, 42);
    Z0_ASSERT(token.byte == 0x20u, 43);

    Z0_ASSERT(raf_z0_classify(present) == RAF_Z0_STATE_PRESENT, 50);
    Z0_ASSERT(raf_z0_token_count(present) == 3u, 51);
    Z0_ASSERT(raf_z0_token_at(present, 2u, &token) == RAF_Z0_EMIT_TOKEN, 52);
    Z0_ASSERT(token.byte == 0x43u && token.offset == 2u, 53);
    Z0_ASSERT(raf_z0_token_at(present, 3u, &token) == RAF_Z0_NO_TOKEN, 54);

    Z0_ASSERT(raf_z0_classify(invalid) == RAF_Z0_STATE_INVALID, 60);
    Z0_ASSERT(raf_z0_token_at(invalid, 0u, &token) == RAF_Z0_INVALID_VIEW, 61);
    Z0_ASSERT(raf_z0_token_at(present, 0u, (raf_z0_token *)0) == RAF_Z0_INVALID_VIEW, 62);

    return 0;
}
