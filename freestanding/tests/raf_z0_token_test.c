#include "../include/raf_fs_z0_token.h"

#define Z0_ASSERT(expr, code) do { if (!(expr)) return (code); } while (0)

static raf_u32
z0_class_count(const raf_fs_z0_view *views, raf_u32 count, raf_u8 mask)
{
    raf_u32 classes = 0u;
    raf_u32 i;
    raf_u32 j;

    for (i = 0u; i < count; ++i) {
        raf_u8 seen = 0u;
        for (j = 0u; j < i; ++j) {
            if (raf_fs_z0_equiv(views[i], views[j], mask) != 0u) {
                seen = 1u;
                break;
            }
        }
        if (seen == 0u) {
            ++classes;
        }
    }
    return classes;
}

int main(void)
{
    static const raf_u8 nul_byte[1] = { 0x00u };
    static const raf_u8 space_byte[1] = { 0x20u };
    static const raf_u8 byte_value[1] = { 0x41u };
    static const raf_u8 bytes[3] = { 0x41u, 0x00u, 0x20u };
    raf_fs_z0_token token;
    raf_fs_z0_view absent = { (const raf_u8 *)0, 0u, 0u };
    raf_fs_z0_view empty = { (const raf_u8 *)0, 0u, 1u };
    raf_fs_z0_view nul = { nul_byte, 1u, 1u };
    raf_fs_z0_view space = { space_byte, 1u, 1u };
    raf_fs_z0_view byte = { byte_value, 1u, 1u };
    raf_fs_z0_view sequence = { bytes, 3u, 1u };
    raf_fs_z0_view invalid = { (const raf_u8 *)0, 1u, 1u };
    raf_fs_z0_view states[7];

    states[0] = absent;
    states[1] = empty;
    states[2] = nul;
    states[3] = space;
    states[4] = byte;
    states[5] = sequence;
    states[6] = invalid;

    Z0_ASSERT(RAF_Z0_CONTEXT_LEFT == 0u, 1);
    Z0_ASSERT(RAF_Z0_CONTEXT_RIGHT == 0u, 2);
    Z0_ASSERT(RAF_Z0_ATTENTION_EDGE_COUNT == 0u, 3);
    Z0_ASSERT(RAF_Z0_LEARNED_WEIGHT_COUNT == 0u, 4);
    Z0_ASSERT(RAF_Z0_VOCAB_ENTRY_COUNT == 0u, 5);
    Z0_ASSERT(RAF_Z0_NORMALIZATION_PASSES == 0u, 6);
    Z0_ASSERT(RAF_Z0_SYNTHETIC_TOKEN_COUNT == 0u, 7);
    Z0_ASSERT(RAF_Z0_HISTORY_BYTES == 0u, 8);

    Z0_ASSERT(raf_fs_z0_classify(absent) == RAF_FS_Z0_ABSENT, 10);
    Z0_ASSERT(raf_fs_z0_token_count(absent) == 0u, 11);
    Z0_ASSERT(raf_fs_z0_token_at(absent, 0u, &token) == RAF_FS_Z0_INPUT_ABSENT, 12);

    Z0_ASSERT(raf_fs_z0_classify(empty) == RAF_FS_Z0_EMPTY, 20);
    Z0_ASSERT(raf_fs_z0_token_count(empty) == 0u, 21);
    Z0_ASSERT(raf_fs_z0_token_at(empty, 0u, &token) == RAF_FS_Z0_NO_TOKEN, 22);

    Z0_ASSERT(raf_fs_z0_classify(nul) == RAF_FS_Z0_NUL, 30);
    Z0_ASSERT(raf_fs_z0_token_count(nul) == 1u, 31);
    Z0_ASSERT(raf_fs_z0_token_at(nul, 0u, &token) == RAF_FS_Z0_EMIT_TOKEN, 32);
    Z0_ASSERT(token.byte == 0x00u && token.kind == RAF_FS_Z0_NUL, 33);

    Z0_ASSERT(raf_fs_z0_classify(space) == RAF_FS_Z0_SPACE, 40);
    Z0_ASSERT(raf_fs_z0_token_count(space) == 1u, 41);
    Z0_ASSERT(raf_fs_z0_token_at(space, 0u, &token) == RAF_FS_Z0_EMIT_TOKEN, 42);
    Z0_ASSERT(token.byte == 0x20u && token.kind == RAF_FS_Z0_SPACE, 43);

    Z0_ASSERT(raf_fs_z0_classify(byte) == RAF_FS_Z0_BYTE, 50);
    Z0_ASSERT(raf_fs_z0_token_at(byte, 0u, &token) == RAF_FS_Z0_EMIT_TOKEN, 51);
    Z0_ASSERT(token.byte == 0x41u && token.kind == RAF_FS_Z0_BYTE, 52);

    Z0_ASSERT(raf_fs_z0_classify(sequence) == RAF_FS_Z0_SEQUENCE, 60);
    Z0_ASSERT(raf_fs_z0_token_count(sequence) == 3u, 61);
    Z0_ASSERT(raf_fs_z0_token_at(sequence, 0u, &token) == RAF_FS_Z0_EMIT_TOKEN, 62);
    Z0_ASSERT(token.byte == 0x41u && token.offset == 0u && token.kind == RAF_FS_Z0_BYTE, 63);
    Z0_ASSERT(raf_fs_z0_token_at(sequence, 1u, &token) == RAF_FS_Z0_EMIT_TOKEN, 64);
    Z0_ASSERT(token.byte == 0x00u && token.offset == 1u && token.kind == RAF_FS_Z0_NUL, 65);
    Z0_ASSERT(raf_fs_z0_token_at(sequence, 2u, &token) == RAF_FS_Z0_EMIT_TOKEN, 66);
    Z0_ASSERT(token.byte == 0x20u && token.offset == 2u && token.kind == RAF_FS_Z0_SPACE, 67);
    Z0_ASSERT(raf_fs_z0_token_at(sequence, 3u, &token) == RAF_FS_Z0_NO_TOKEN, 68);

    Z0_ASSERT(raf_fs_z0_classify(invalid) == RAF_FS_Z0_INVALID, 70);
    Z0_ASSERT(raf_fs_z0_token_count(invalid) == 0u, 71);
    Z0_ASSERT(raf_fs_z0_token_at(invalid, 0u, &token) == RAF_FS_Z0_INVALID_VIEW, 72);
    Z0_ASSERT(raf_fs_z0_token_at(sequence, 0u, (raf_fs_z0_token *)0) == RAF_FS_Z0_INVALID_VIEW, 73);

    Z0_ASSERT(z0_class_count(states, 7u, RAF_Z0_OBS_FULL) == 7u, 80);
    Z0_ASSERT(z0_class_count(states, 7u,
        RAF_Z0_OBS_PROVIDED | RAF_Z0_OBS_SIZE_CLASS | RAF_Z0_OBS_DATA_PRESENT) == 5u, 81);
    Z0_ASSERT(z0_class_count(states, 7u,
        RAF_Z0_OBS_PROVIDED | RAF_Z0_OBS_SIZE_CLASS) == 4u, 82);
    Z0_ASSERT(z0_class_count(states, 7u, RAF_Z0_OBS_SIZE_CLASS) == 3u, 83);
    Z0_ASSERT(z0_class_count(states, 7u, 0u) == 1u, 84);
    Z0_ASSERT(z0_class_count(states, 7u, RAF_Z0_OBS_PROVIDED) == 2u, 85);

    Z0_ASSERT(raf_fs_z0_equiv(absent, empty, RAF_Z0_OBS_SIZE_CLASS) != 0u, 90);
    Z0_ASSERT(raf_fs_z0_equiv(absent, empty, RAF_Z0_OBS_PROVIDED) == 0u, 91);
    Z0_ASSERT(raf_fs_z0_equiv(nul, space,
        RAF_Z0_OBS_PROVIDED | RAF_Z0_OBS_SIZE_CLASS | RAF_Z0_OBS_DATA_PRESENT) != 0u, 92);
    Z0_ASSERT(raf_fs_z0_equiv(nul, space, RAF_Z0_OBS_FULL) == 0u, 93);
    Z0_ASSERT(raf_fs_z0_equiv(byte, invalid,
        RAF_Z0_OBS_PROVIDED | RAF_Z0_OBS_SIZE_CLASS) != 0u, 94);
    Z0_ASSERT(raf_fs_z0_equiv(byte, invalid,
        RAF_Z0_OBS_PROVIDED | RAF_Z0_OBS_SIZE_CLASS | RAF_Z0_OBS_DATA_PRESENT) == 0u, 95);

    return 0;
}
