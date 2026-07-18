#include "raf_segment_v1.h"

static const rcs_u8 RAF_SEG_MAGIC[8] = {
    'R', 'A', 'F', 'S', 'E', 'G', '1', 0
};

static void put_u32le(rcs_u8 *out, rcs_u32 value) {
    out[0] = (rcs_u8)(value);
    out[1] = (rcs_u8)(value >> 8u);
    out[2] = (rcs_u8)(value >> 16u);
    out[3] = (rcs_u8)(value >> 24u);
}

static void put_u64le(rcs_u8 *out, rcs_u64 value) {
    for (rcs_u32 i = 0; i < 8u; ++i) out[i] = (rcs_u8)(value >> (i * 8u));
}

static rcs_u32 get_u32le(const rcs_u8 *in) {
    return (rcs_u32)in[0]
        | ((rcs_u32)in[1] << 8u)
        | ((rcs_u32)in[2] << 16u)
        | ((rcs_u32)in[3] << 24u);
}

static rcs_u64 get_u64le(const rcs_u8 *in) {
    rcs_u64 value = 0;
    for (rcs_u32 i = 0; i < 8u; ++i) value |= ((rcs_u64)in[i]) << (i * 8u);
    return value;
}

rcs_u32 raf_segment_crc32c(const rcs_u8 *data, rcs_u32 size) {
    rcs_u32 crc = 0xffffffffu;
    if (!data && size != 0u) return 0u;
    for (rcs_u32 i = 0; i < size; ++i) {
        crc ^= (rcs_u32)data[i];
        for (rcs_u32 bit = 0; bit < 8u; ++bit) {
            rcs_u32 mask = (rcs_u32)(0u - (crc & 1u));
            crc = (crc >> 1u) ^ (0x82f63b78u & mask);
        }
    }
    return ~crc;
}

static int validate_offsets(const raf_segment_header_v1 *header) {
    if (header->index_offset < RAF_SEGMENT_V1_HEADER_SIZE) return RAF_SEG_E_OFFSET;
    if (header->payload_offset < RAF_SEGMENT_V1_HEADER_SIZE) return RAF_SEG_E_OFFSET;
    return RAF_SEG_OK;
}

int raf_segment_header_v1_encode(const raf_segment_header_v1 *header,
                                 rcs_u8 out[RAF_SEGMENT_V1_HEADER_SIZE]) {
    if (!header || !out) return RAF_SEG_E_NULL;
    int offset_status = validate_offsets(header);
    if (offset_status != RAF_SEG_OK) return offset_status;

    for (rcs_u32 i = 0; i < RAF_SEGMENT_V1_HEADER_SIZE; ++i) out[i] = 0u;
    for (rcs_u32 i = 0; i < 8u; ++i) out[i] = RAF_SEG_MAGIC[i];
    put_u32le(out + 8u, RAF_SEGMENT_V1_VERSION);
    put_u32le(out + 12u, header->flags);
    put_u64le(out + 16u, header->record_count);
    put_u64le(out + 24u, header->index_offset);
    put_u64le(out + 32u, header->payload_offset);
    put_u64le(out + 40u, header->source_size);
    put_u32le(out + 48u, header->source_crc32c);
    put_u32le(out + 52u, 0u);

    rcs_u32 crc = raf_segment_crc32c(out, RAF_SEGMENT_V1_HEADER_SIZE);
    put_u32le(out + 52u, crc);
    return RAF_SEG_OK;
}

int raf_segment_header_v1_decode(const rcs_u8 in[RAF_SEGMENT_V1_HEADER_SIZE],
                                 raf_segment_header_v1 *header) {
    if (!in || !header) return RAF_SEG_E_NULL;
    for (rcs_u32 i = 0; i < 8u; ++i) {
        if (in[i] != RAF_SEG_MAGIC[i]) return RAF_SEG_E_MAGIC;
    }

    rcs_u32 version = get_u32le(in + 8u);
    if ((version >> 16u) != (RAF_SEGMENT_V1_VERSION >> 16u)) return RAF_SEG_E_VERSION;
    for (rcs_u32 i = 56u; i < 64u; ++i) {
        if (in[i] != 0u) return RAF_SEG_E_RESERVED;
    }

    rcs_u8 checked[RAF_SEGMENT_V1_HEADER_SIZE];
    for (rcs_u32 i = 0; i < RAF_SEGMENT_V1_HEADER_SIZE; ++i) checked[i] = in[i];
    rcs_u32 stored_crc = get_u32le(checked + 52u);
    put_u32le(checked + 52u, 0u);
    if (raf_segment_crc32c(checked, RAF_SEGMENT_V1_HEADER_SIZE) != stored_crc)
        return RAF_SEG_E_CRC;

    header->flags = get_u32le(in + 12u);
    header->record_count = get_u64le(in + 16u);
    header->index_offset = get_u64le(in + 24u);
    header->payload_offset = get_u64le(in + 32u);
    header->source_size = get_u64le(in + 40u);
    header->source_crc32c = get_u32le(in + 48u);
    header->header_crc32c = stored_crc;
    return validate_offsets(header);
}
