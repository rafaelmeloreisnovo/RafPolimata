#include "raf_segment_v1.h"

#include <stdio.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int test_crc_vector(void) {
    static const rcs_u8 value[] = "123456789";
    CHECK(raf_segment_crc32c(value, 9u) == 0xe3069283u);
    return 0;
}

static int test_roundtrip(void) {
    raf_segment_header_v1 input;
    input.flags = 3u;
    input.record_count = 42u;
    input.index_offset = 64u;
    input.payload_offset = 4096u;
    input.source_size = 123456789u;
    input.source_crc32c = 0x11223344u;
    input.header_crc32c = 0u;

    rcs_u8 bytes[RAF_SEGMENT_V1_HEADER_SIZE];
    raf_segment_header_v1 output;
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

static int test_corruption_rejected(void) {
    raf_segment_header_v1 input;
    input.flags = 0u;
    input.record_count = 1u;
    input.index_offset = 64u;
    input.payload_offset = 128u;
    input.source_size = 8u;
    input.source_crc32c = 0u;
    input.header_crc32c = 0u;

    rcs_u8 bytes[RAF_SEGMENT_V1_HEADER_SIZE];
    raf_segment_header_v1 output;
    CHECK(raf_segment_header_v1_encode(&input, bytes) == RAF_SEG_OK);
    bytes[40] ^= 1u;
    CHECK(raf_segment_header_v1_decode(bytes, &output) == RAF_SEG_E_CRC);
    return 0;
}

static int test_reserved_and_offset_rejected(void) {
    raf_segment_header_v1 input;
    input.flags = 0u;
    input.record_count = 0u;
    input.index_offset = 63u;
    input.payload_offset = 64u;
    input.source_size = 0u;
    input.source_crc32c = 0u;
    input.header_crc32c = 0u;

    rcs_u8 bytes[RAF_SEGMENT_V1_HEADER_SIZE];
    CHECK(raf_segment_header_v1_encode(&input, bytes) == RAF_SEG_E_OFFSET);
    return 0;
}

int main(void) {
    if (test_crc_vector()) return 1;
    if (test_roundtrip()) return 1;
    if (test_corruption_rejected()) return 1;
    if (test_reserved_and_offset_rejected()) return 1;
    puts("PASS segment-v1");
    return 0;
}
