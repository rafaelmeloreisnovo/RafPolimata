#include "raf_segment_v1.h"

#include <stdio.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static void copy_bytes(rcs_u8 *out, const rcs_u8 *in, rcs_u32 size) {
    rcs_u32 i;
    for (i = 0u; i < size; ++i) out[i] = in[i];
}

static int test_crc_vector(void) {
    static const rcs_u8 value[] = "123456789";
    CHECK(raf_segment_crc32c(value, 9u) == 0xe3069283u);
    return 0;
}

static int test_header_roundtrip(void) {
    raf_segment_header_v1 input;
    rcs_u8 bytes[RAF_SEGMENT_V1_HEADER_SIZE];
    raf_segment_header_v1 output;
    input.flags = 3u;
    input.record_count = 2u;
    input.index_offset = 64u;
    input.payload_offset = 288u;
    input.source_size = 123456789u;
    input.source_crc32c = 0x11223344u;
    input.header_crc32c = 0u;

    CHECK(raf_segment_header_v1_encode(&input, bytes) == RAF_SEG_OK);
    CHECK(bytes[0] == 'R' && bytes[6] == '1' && bytes[7] == 0u);
    CHECK(raf_segment_header_v1_decode(bytes, &output) == RAF_SEG_OK);
    CHECK(output.flags == input.flags);
    CHECK(output.record_count == input.record_count);
    CHECK(output.index_offset == input.index_offset);
    CHECK(output.payload_offset == input.payload_offset);
    CHECK(output.source_size == input.source_size);
    CHECK(output.source_crc32c == input.source_crc32c);
    CHECK(output.header_crc32c != 0u);
    return 0;
}

static int test_record_roundtrips(void) {
    raf_segment_conversation_v1 conv;
    raf_segment_conversation_v1 conv_out;
    raf_segment_message_v1 msg;
    raf_segment_message_v1 msg_out;
    rcs_u8 conv_bytes[RAF_SEGMENT_V1_CONVERSATION_SIZE];
    rcs_u8 msg_bytes[RAF_SEGMENT_V1_MESSAGE_SIZE];

    conv.flags = 7u;
    conv.message_count = 1u;
    conv.id_hi = 0x0102030405060708ull;
    conv.id_lo = 0x1112131415161718ull;
    conv.source_offset = 10u;
    conv.source_length = 90u;
    conv.title_offset = 288u;
    conv.title_length = 5u;
    conv.first_message_index = 1u;
    conv.create_time_us = 1000u;
    conv.update_time_us = 2000u;
    conv.title_crc32c = 0x12345678u;
    conv.record_crc32c = 0u;

    CHECK(raf_segment_conversation_v1_encode(&conv, conv_bytes) == RAF_SEG_OK);
    CHECK(conv_bytes[0] == RAF_SEGMENT_RECORD_CONVERSATION);
    CHECK(conv_bytes[4] == RAF_SEGMENT_V1_CONVERSATION_SIZE);
    CHECK(raf_segment_conversation_v1_decode(conv_bytes, &conv_out) == RAF_SEG_OK);
    CHECK(conv_out.message_count == conv.message_count);
    CHECK(conv_out.id_hi == conv.id_hi);
    CHECK(conv_out.id_lo == conv.id_lo);
    CHECK(conv_out.title_offset == conv.title_offset);
    CHECK(conv_out.record_crc32c != 0u);

    msg.flags = 9u;
    msg.role = RAF_SEGMENT_ROLE_USER;
    msg.conversation_index = 0u;
    msg.message_index = 1u;
    msg.parent_index = RAF_SEGMENT_INDEX_NONE;
    msg.id_hi = 0x2122232425262728ull;
    msg.id_lo = 0x3132333435363738ull;
    msg.source_offset = 100u;
    msg.source_length = 50u;
    msg.author_offset = 293u;
    msg.author_length = 4u;
    msg.content_offset = 297u;
    msg.content_length = 5u;
    msg.create_time_us = 3000u;
    msg.content_crc32c = 0xabcdef01u;
    msg.author_crc32c = 0x10203040u;
    msg.record_crc32c = 0u;

    CHECK(raf_segment_message_v1_encode(&msg, msg_bytes) == RAF_SEG_OK);
    CHECK(msg_bytes[0] == RAF_SEGMENT_RECORD_MESSAGE);
    CHECK(msg_bytes[4] == RAF_SEGMENT_V1_MESSAGE_SIZE);
    CHECK(raf_segment_message_v1_decode(msg_bytes, &msg_out) == RAF_SEG_OK);
    CHECK(msg_out.role == msg.role);
    CHECK(msg_out.parent_index == RAF_SEGMENT_INDEX_NONE);
    CHECK(msg_out.content_offset == msg.content_offset);
    CHECK(msg_out.record_crc32c != 0u);
    return 0;
}

static int build_fixture(rcs_u8 *segment, rcs_u32 segment_size) {
    static const rcs_u8 title[] = "Title";
    static const rcs_u8 author[] = "user";
    static const rcs_u8 content[] = "hello";
    raf_segment_header_v1 header;
    raf_segment_conversation_v1 conv;
    raf_segment_message_v1 msg;
    rcs_u32 i;

    CHECK(segment_size == 302u);
    for (i = 0u; i < segment_size; ++i) segment[i] = 0u;
    copy_bytes(segment + 288u, title, 5u);
    copy_bytes(segment + 293u, author, 4u);
    copy_bytes(segment + 297u, content, 5u);

    conv.flags = 0u;
    conv.message_count = 1u;
    conv.id_hi = 0xaaaaull;
    conv.id_lo = 0xbbbbull;
    conv.source_offset = 0u;
    conv.source_length = 500u;
    conv.title_offset = 288u;
    conv.title_length = 5u;
    conv.first_message_index = 1u;
    conv.create_time_us = 100u;
    conv.update_time_us = 200u;
    conv.title_crc32c = raf_segment_crc32c(title, 5u);
    conv.record_crc32c = 0u;
    CHECK(raf_segment_conversation_v1_encode(&conv, segment + 64u) == RAF_SEG_OK);

    msg.flags = 0u;
    msg.role = RAF_SEGMENT_ROLE_USER;
    msg.conversation_index = 0u;
    msg.message_index = 1u;
    msg.parent_index = RAF_SEGMENT_INDEX_NONE;
    msg.id_hi = 0xccccull;
    msg.id_lo = 0xddddull;
    msg.source_offset = 500u;
    msg.source_length = 200u;
    msg.author_offset = 293u;
    msg.author_length = 4u;
    msg.content_offset = 297u;
    msg.content_length = 5u;
    msg.create_time_us = 300u;
    msg.content_crc32c = raf_segment_crc32c(content, 5u);
    msg.author_crc32c = raf_segment_crc32c(author, 4u);
    msg.record_crc32c = 0u;
    CHECK(raf_segment_message_v1_encode(&msg, segment + 160u) == RAF_SEG_OK);

    header.flags = 0u;
    header.record_count = 2u;
    header.index_offset = 64u;
    header.payload_offset = 288u;
    header.source_size = 1000u;
    header.source_crc32c = 0u;
    header.header_crc32c = 0u;
    CHECK(raf_segment_header_v1_encode(&header, segment) == RAF_SEG_OK);
    return 0;
}

static int test_bounded_reader(void) {
    rcs_u8 segment[302];
    raf_segment_reader_v1 reader;
    raf_segment_conversation_v1 conv;
    raf_segment_message_v1 msg;
    rcs_u64 offset;
    rcs_u32 kind;

    CHECK(build_fixture(segment, 302u) == 0);
    CHECK(raf_segment_reader_v1_init(&reader, segment, 302u) == RAF_SEG_OK);

    CHECK(raf_segment_reader_v1_next(&reader, &offset, &kind) == RAF_SEG_OK);
    CHECK(offset == 64u && kind == RAF_SEGMENT_RECORD_CONVERSATION);
    CHECK(raf_segment_reader_v1_read_conversation(&reader, offset, &conv) == RAF_SEG_OK);
    CHECK(conv.message_count == 1u);

    CHECK(raf_segment_reader_v1_next(&reader, &offset, &kind) == RAF_SEG_OK);
    CHECK(offset == 160u && kind == RAF_SEGMENT_RECORD_MESSAGE);
    CHECK(raf_segment_reader_v1_read_message(&reader, offset, &msg) == RAF_SEG_OK);
    CHECK(msg.role == RAF_SEGMENT_ROLE_USER);
    CHECK(raf_segment_reader_v1_next(&reader, &offset, &kind) == RAF_SEG_END);

    CHECK(raf_segment_reader_v1_init(&reader, segment, 301u) == RAF_SEG_OK);
    CHECK(raf_segment_reader_v1_next(&reader, &offset, &kind) == RAF_SEG_OK);
    CHECK(raf_segment_reader_v1_next(&reader, &offset, &kind) == RAF_SEG_OK);
    CHECK(raf_segment_reader_v1_read_message(&reader, offset, &msg) == RAF_SEG_E_BOUNDS);
    return 0;
}

static int test_corruption_and_bounds_rejected(void) {
    rcs_u8 segment[302];
    raf_segment_reader_v1 reader;
    raf_segment_conversation_v1 conv;
    raf_segment_message_v1 msg;
    rcs_u64 offset;
    rcs_u32 kind;

    CHECK(build_fixture(segment, 302u) == 0);
    segment[72] ^= 1u;
    CHECK(raf_segment_reader_v1_init(&reader, segment, 302u) == RAF_SEG_OK);
    CHECK(raf_segment_reader_v1_next(&reader, &offset, &kind) == RAF_SEG_OK);
    CHECK(raf_segment_reader_v1_read_conversation(&reader, offset, &conv) == RAF_SEG_E_CRC);

    CHECK(build_fixture(segment, 302u) == 0);
    CHECK(raf_segment_reader_v1_init(&reader, segment, 302u) == RAF_SEG_OK);
    CHECK(raf_segment_reader_v1_next(&reader, &offset, &kind) == RAF_SEG_OK);
    CHECK(raf_segment_reader_v1_next(&reader, &offset, &kind) == RAF_SEG_OK);
    CHECK(raf_segment_message_v1_decode(segment + offset, &msg) == RAF_SEG_OK);
    msg.content_offset = 303u;
    msg.content_length = 1u;
    CHECK(raf_segment_message_v1_encode(&msg, segment + offset) == RAF_SEG_OK);
    CHECK(raf_segment_reader_v1_read_message(&reader, offset, &msg) == RAF_SEG_E_BOUNDS);
    return 0;
}

static int test_layout_and_role_rejected(void) {
    raf_segment_header_v1 header;
    raf_segment_message_v1 msg;
    rcs_u8 header_bytes[RAF_SEGMENT_V1_HEADER_SIZE];
    rcs_u8 msg_bytes[RAF_SEGMENT_V1_MESSAGE_SIZE];

    header.flags = 0u;
    header.record_count = 0u;
    header.index_offset = 64u;
    header.payload_offset = 65u;
    header.source_size = 0u;
    header.source_crc32c = 0u;
    header.header_crc32c = 0u;
    CHECK(raf_segment_header_v1_encode(&header, header_bytes) == RAF_SEG_E_LAYOUT);

    msg.flags = 0u;
    msg.role = RAF_SEGMENT_ROLE_TOOL + 1u;
    msg.conversation_index = 0u;
    msg.message_index = 0u;
    msg.parent_index = RAF_SEGMENT_INDEX_NONE;
    msg.id_hi = 0u;
    msg.id_lo = 0u;
    msg.source_offset = 0u;
    msg.source_length = 0u;
    msg.author_offset = 0u;
    msg.author_length = 0u;
    msg.content_offset = 0u;
    msg.content_length = 0u;
    msg.create_time_us = 0u;
    msg.content_crc32c = 0u;
    msg.author_crc32c = 0u;
    msg.record_crc32c = 0u;
    CHECK(raf_segment_message_v1_encode(&msg, msg_bytes) == RAF_SEG_E_ROLE);
    return 0;
}

int main(void) {
    if (test_crc_vector()) return 1;
    if (test_header_roundtrip()) return 1;
    if (test_record_roundtrips()) return 1;
    if (test_bounded_reader()) return 1;
    if (test_corruption_and_bounds_rejected()) return 1;
    if (test_layout_and_role_rejected()) return 1;
    puts("PASS segment-v1 header+records+bounded-reader");
    return 0;
}
