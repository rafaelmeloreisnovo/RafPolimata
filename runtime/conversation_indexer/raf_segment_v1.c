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
    rcs_u32 i;
    for (i = 0u; i < 8u; ++i) out[i] = (rcs_u8)(value >> (i * 8u));
}

static rcs_u32 get_u32le(const rcs_u8 *in) {
    return (rcs_u32)in[0]
        | ((rcs_u32)in[1] << 8u)
        | ((rcs_u32)in[2] << 16u)
        | ((rcs_u32)in[3] << 24u);
}

static rcs_u64 get_u64le(const rcs_u8 *in) {
    rcs_u64 value = 0u;
    rcs_u32 i;
    for (i = 0u; i < 8u; ++i) value |= ((rcs_u64)in[i]) << (i * 8u);
    return value;
}

static rcs_u32 crc32c_u64(const rcs_u8 *data, rcs_u64 size) {
    rcs_u32 crc = 0xffffffffu;
    rcs_u64 i;
    if (!data && size != 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        rcs_u32 bit;
        crc ^= (rcs_u32)data[i];
        for (bit = 0u; bit < 8u; ++bit) {
            rcs_u32 mask = (rcs_u32)(0u - (crc & 1u));
            crc = (crc >> 1u) ^ (0x82f63b78u & mask);
        }
    }
    return ~crc;
}

rcs_u32 raf_segment_crc32c(const rcs_u8 *data, rcs_u32 size) {
    return crc32c_u64(data, (rcs_u64)size);
}

static int range_within(rcs_u64 offset, rcs_u64 length, rcs_u64 limit) {
    return offset <= limit && length <= (limit - offset);
}

static int validate_header_offsets(const raf_segment_header_v1 *header) {
    if (header->index_offset < RAF_SEGMENT_V1_HEADER_SIZE) return RAF_SEG_E_OFFSET;
    if (header->payload_offset < header->index_offset) return RAF_SEG_E_OFFSET;
    if (header->record_count == 0u && header->index_offset != header->payload_offset)
        return RAF_SEG_E_LAYOUT;
    if (header->record_count != 0u && header->index_offset == header->payload_offset)
        return RAF_SEG_E_LAYOUT;
    return RAF_SEG_OK;
}

static void clear_bytes(rcs_u8 *out, rcs_u32 size) {
    rcs_u32 i;
    for (i = 0u; i < size; ++i) out[i] = 0u;
}

int raf_segment_header_v1_encode(const raf_segment_header_v1 *header,
                                 rcs_u8 out[RAF_SEGMENT_V1_HEADER_SIZE]) {
    rcs_u32 i;
    rcs_u32 crc;
    int offset_status;
    if (!header || !out) return RAF_SEG_E_NULL;
    offset_status = validate_header_offsets(header);
    if (offset_status != RAF_SEG_OK) return offset_status;

    clear_bytes(out, RAF_SEGMENT_V1_HEADER_SIZE);
    for (i = 0u; i < 8u; ++i) out[i] = RAF_SEG_MAGIC[i];
    put_u32le(out + 8u, RAF_SEGMENT_V1_VERSION);
    put_u32le(out + 12u, header->flags);
    put_u64le(out + 16u, header->record_count);
    put_u64le(out + 24u, header->index_offset);
    put_u64le(out + 32u, header->payload_offset);
    put_u64le(out + 40u, header->source_size);
    put_u32le(out + 48u, header->source_crc32c);
    put_u32le(out + 52u, 0u);

    crc = raf_segment_crc32c(out, RAF_SEGMENT_V1_HEADER_SIZE);
    put_u32le(out + 52u, crc);
    return RAF_SEG_OK;
}

int raf_segment_header_v1_decode(const rcs_u8 in[RAF_SEGMENT_V1_HEADER_SIZE],
                                 raf_segment_header_v1 *header) {
    rcs_u8 checked[RAF_SEGMENT_V1_HEADER_SIZE];
    rcs_u32 stored_crc;
    rcs_u32 version;
    rcs_u32 i;
    if (!in || !header) return RAF_SEG_E_NULL;
    for (i = 0u; i < 8u; ++i) {
        if (in[i] != RAF_SEG_MAGIC[i]) return RAF_SEG_E_MAGIC;
    }

    version = get_u32le(in + 8u);
    if ((version >> 16u) != (RAF_SEGMENT_V1_VERSION >> 16u)) return RAF_SEG_E_VERSION;
    for (i = 56u; i < 64u; ++i) {
        if (in[i] != 0u) return RAF_SEG_E_RESERVED;
    }

    for (i = 0u; i < RAF_SEGMENT_V1_HEADER_SIZE; ++i) checked[i] = in[i];
    stored_crc = get_u32le(checked + 52u);
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
    return validate_header_offsets(header);
}

int raf_segment_conversation_v1_encode(
    const raf_segment_conversation_v1 *record,
    rcs_u8 out[RAF_SEGMENT_V1_CONVERSATION_SIZE]) {
    rcs_u32 crc;
    if (!record || !out) return RAF_SEG_E_NULL;
    if (record->message_count == 0u && record->first_message_index != RAF_SEGMENT_INDEX_NONE)
        return RAF_SEG_E_LAYOUT;
    if (record->message_count != 0u && record->first_message_index == RAF_SEGMENT_INDEX_NONE)
        return RAF_SEG_E_LAYOUT;

    clear_bytes(out, RAF_SEGMENT_V1_CONVERSATION_SIZE);
    put_u32le(out + 0u, RAF_SEGMENT_RECORD_CONVERSATION);
    put_u32le(out + 4u, RAF_SEGMENT_V1_CONVERSATION_SIZE);
    put_u32le(out + 8u, record->flags);
    put_u32le(out + 12u, record->message_count);
    put_u64le(out + 16u, record->id_hi);
    put_u64le(out + 24u, record->id_lo);
    put_u64le(out + 32u, record->source_offset);
    put_u64le(out + 40u, record->source_length);
    put_u64le(out + 48u, record->title_offset);
    put_u64le(out + 56u, record->title_length);
    put_u64le(out + 64u, record->first_message_index);
    put_u64le(out + 72u, record->create_time_us);
    put_u64le(out + 80u, record->update_time_us);
    put_u32le(out + 88u, record->title_crc32c);
    put_u32le(out + 92u, 0u);
    crc = raf_segment_crc32c(out, RAF_SEGMENT_V1_CONVERSATION_SIZE);
    put_u32le(out + 92u, crc);
    return RAF_SEG_OK;
}

int raf_segment_conversation_v1_decode(
    const rcs_u8 in[RAF_SEGMENT_V1_CONVERSATION_SIZE],
    raf_segment_conversation_v1 *record) {
    rcs_u8 checked[RAF_SEGMENT_V1_CONVERSATION_SIZE];
    rcs_u32 stored_crc;
    rcs_u32 i;
    if (!in || !record) return RAF_SEG_E_NULL;
    if (get_u32le(in + 0u) != RAF_SEGMENT_RECORD_CONVERSATION) return RAF_SEG_E_TYPE;
    if (get_u32le(in + 4u) != RAF_SEGMENT_V1_CONVERSATION_SIZE) return RAF_SEG_E_SIZE;
    for (i = 0u; i < RAF_SEGMENT_V1_CONVERSATION_SIZE; ++i) checked[i] = in[i];
    stored_crc = get_u32le(checked + 92u);
    put_u32le(checked + 92u, 0u);
    if (raf_segment_crc32c(checked, RAF_SEGMENT_V1_CONVERSATION_SIZE) != stored_crc)
        return RAF_SEG_E_CRC;

    record->flags = get_u32le(in + 8u);
    record->message_count = get_u32le(in + 12u);
    record->id_hi = get_u64le(in + 16u);
    record->id_lo = get_u64le(in + 24u);
    record->source_offset = get_u64le(in + 32u);
    record->source_length = get_u64le(in + 40u);
    record->title_offset = get_u64le(in + 48u);
    record->title_length = get_u64le(in + 56u);
    record->first_message_index = get_u64le(in + 64u);
    record->create_time_us = get_u64le(in + 72u);
    record->update_time_us = get_u64le(in + 80u);
    record->title_crc32c = get_u32le(in + 88u);
    record->record_crc32c = stored_crc;

    if (record->message_count == 0u && record->first_message_index != RAF_SEGMENT_INDEX_NONE)
        return RAF_SEG_E_LAYOUT;
    if (record->message_count != 0u && record->first_message_index == RAF_SEGMENT_INDEX_NONE)
        return RAF_SEG_E_LAYOUT;
    return RAF_SEG_OK;
}

int raf_segment_message_v1_encode(
    const raf_segment_message_v1 *record,
    rcs_u8 out[RAF_SEGMENT_V1_MESSAGE_SIZE]) {
    rcs_u32 crc;
    if (!record || !out) return RAF_SEG_E_NULL;
    if (record->role > RAF_SEGMENT_ROLE_TOOL) return RAF_SEG_E_ROLE;

    clear_bytes(out, RAF_SEGMENT_V1_MESSAGE_SIZE);
    put_u32le(out + 0u, RAF_SEGMENT_RECORD_MESSAGE);
    put_u32le(out + 4u, RAF_SEGMENT_V1_MESSAGE_SIZE);
    put_u32le(out + 8u, record->flags);
    put_u32le(out + 12u, record->role);
    put_u64le(out + 16u, record->conversation_index);
    put_u64le(out + 24u, record->message_index);
    put_u64le(out + 32u, record->parent_index);
    put_u64le(out + 40u, record->id_hi);
    put_u64le(out + 48u, record->id_lo);
    put_u64le(out + 56u, record->source_offset);
    put_u64le(out + 64u, record->source_length);
    put_u64le(out + 72u, record->author_offset);
    put_u64le(out + 80u, record->author_length);
    put_u64le(out + 88u, record->content_offset);
    put_u64le(out + 96u, record->content_length);
    put_u64le(out + 104u, record->create_time_us);
    put_u32le(out + 112u, record->content_crc32c);
    put_u32le(out + 116u, record->author_crc32c);
    put_u32le(out + 120u, 0u);
    put_u32le(out + 124u, 0u);
    crc = raf_segment_crc32c(out, RAF_SEGMENT_V1_MESSAGE_SIZE);
    put_u32le(out + 120u, crc);
    return RAF_SEG_OK;
}

int raf_segment_message_v1_decode(
    const rcs_u8 in[RAF_SEGMENT_V1_MESSAGE_SIZE],
    raf_segment_message_v1 *record) {
    rcs_u8 checked[RAF_SEGMENT_V1_MESSAGE_SIZE];
    rcs_u32 stored_crc;
    rcs_u32 i;
    if (!in || !record) return RAF_SEG_E_NULL;
    if (get_u32le(in + 0u) != RAF_SEGMENT_RECORD_MESSAGE) return RAF_SEG_E_TYPE;
    if (get_u32le(in + 4u) != RAF_SEGMENT_V1_MESSAGE_SIZE) return RAF_SEG_E_SIZE;
    if (get_u32le(in + 124u) != 0u) return RAF_SEG_E_RESERVED;
    for (i = 0u; i < RAF_SEGMENT_V1_MESSAGE_SIZE; ++i) checked[i] = in[i];
    stored_crc = get_u32le(checked + 120u);
    put_u32le(checked + 120u, 0u);
    if (raf_segment_crc32c(checked, RAF_SEGMENT_V1_MESSAGE_SIZE) != stored_crc)
        return RAF_SEG_E_CRC;

    record->flags = get_u32le(in + 8u);
    record->role = get_u32le(in + 12u);
    if (record->role > RAF_SEGMENT_ROLE_TOOL) return RAF_SEG_E_ROLE;
    record->conversation_index = get_u64le(in + 16u);
    record->message_index = get_u64le(in + 24u);
    record->parent_index = get_u64le(in + 32u);
    record->id_hi = get_u64le(in + 40u);
    record->id_lo = get_u64le(in + 48u);
    record->source_offset = get_u64le(in + 56u);
    record->source_length = get_u64le(in + 64u);
    record->author_offset = get_u64le(in + 72u);
    record->author_length = get_u64le(in + 80u);
    record->content_offset = get_u64le(in + 88u);
    record->content_length = get_u64le(in + 96u);
    record->create_time_us = get_u64le(in + 104u);
    record->content_crc32c = get_u32le(in + 112u);
    record->author_crc32c = get_u32le(in + 116u);
    record->record_crc32c = stored_crc;
    return RAF_SEG_OK;
}

static int record_bounds(const raf_segment_reader_v1 *reader,
                         rcs_u64 record_offset,
                         rcs_u32 expected_kind,
                         rcs_u32 expected_size) {
    if (!reader || !reader->bytes) return RAF_SEG_E_NULL;
    if (record_offset < reader->header.index_offset) return RAF_SEG_E_BOUNDS;
    if (!range_within(record_offset, 8u, reader->header.payload_offset))
        return RAF_SEG_E_BOUNDS;
    if (get_u32le(reader->bytes + record_offset) != expected_kind) return RAF_SEG_E_TYPE;
    if (get_u32le(reader->bytes + record_offset + 4u) != expected_size) return RAF_SEG_E_SIZE;
    if (!range_within(record_offset, expected_size, reader->header.payload_offset))
        return RAF_SEG_E_BOUNDS;
    return RAF_SEG_OK;
}

static int payload_range(const raf_segment_reader_v1 *reader,
                         rcs_u64 offset,
                         rcs_u64 length) {
    if (offset < reader->header.payload_offset) return RAF_SEG_E_BOUNDS;
    return range_within(offset, length, reader->size) ? RAF_SEG_OK : RAF_SEG_E_BOUNDS;
}

int raf_segment_reader_v1_init(raf_segment_reader_v1 *reader,
                               const rcs_u8 *bytes,
                               rcs_u64 size) {
    int status;
    if (!reader || !bytes) return RAF_SEG_E_NULL;
    if (size < RAF_SEGMENT_V1_HEADER_SIZE) return RAF_SEG_E_BOUNDS;
    status = raf_segment_header_v1_decode(bytes, &reader->header);
    if (status != RAF_SEG_OK) return status;
    if (reader->header.payload_offset > size) return RAF_SEG_E_BOUNDS;
    reader->bytes = bytes;
    reader->size = size;
    reader->cursor = reader->header.index_offset;
    reader->records_seen = 0u;
    return RAF_SEG_OK;
}

int raf_segment_reader_v1_next(raf_segment_reader_v1 *reader,
                               rcs_u64 *record_offset,
                               rcs_u32 *record_kind) {
    rcs_u32 kind;
    rcs_u32 size;
    if (!reader || !record_offset || !record_kind || !reader->bytes)
        return RAF_SEG_E_NULL;
    if (reader->records_seen == reader->header.record_count) {
        return reader->cursor == reader->header.payload_offset
            ? RAF_SEG_END : RAF_SEG_E_LAYOUT;
    }
    if (!range_within(reader->cursor, 8u, reader->header.payload_offset))
        return RAF_SEG_E_BOUNDS;

    kind = get_u32le(reader->bytes + reader->cursor);
    size = get_u32le(reader->bytes + reader->cursor + 4u);
    if (kind == RAF_SEGMENT_RECORD_CONVERSATION) {
        if (size != RAF_SEGMENT_V1_CONVERSATION_SIZE) return RAF_SEG_E_SIZE;
    } else if (kind == RAF_SEGMENT_RECORD_MESSAGE) {
        if (size != RAF_SEGMENT_V1_MESSAGE_SIZE) return RAF_SEG_E_SIZE;
    } else {
        return RAF_SEG_E_TYPE;
    }
    if (!range_within(reader->cursor, size, reader->header.payload_offset))
        return RAF_SEG_E_BOUNDS;

    *record_offset = reader->cursor;
    *record_kind = kind;
    reader->cursor += size;
    reader->records_seen += 1u;
    return RAF_SEG_OK;
}

int raf_segment_reader_v1_read_conversation(
    const raf_segment_reader_v1 *reader,
    rcs_u64 record_offset,
    raf_segment_conversation_v1 *record) {
    int status;
    if (!record) return RAF_SEG_E_NULL;
    status = record_bounds(reader, record_offset,
                           RAF_SEGMENT_RECORD_CONVERSATION,
                           RAF_SEGMENT_V1_CONVERSATION_SIZE);
    if (status != RAF_SEG_OK) return status;
    status = raf_segment_conversation_v1_decode(reader->bytes + record_offset, record);
    if (status != RAF_SEG_OK) return status;
    if (!range_within(record->source_offset, record->source_length,
                      reader->header.source_size)) return RAF_SEG_E_BOUNDS;
    status = payload_range(reader, record->title_offset, record->title_length);
    if (status != RAF_SEG_OK) return status;
    if (crc32c_u64(reader->bytes + record->title_offset, record->title_length)
        != record->title_crc32c) return RAF_SEG_E_CRC;
    if (record->message_count != 0u) {
        if (record->first_message_index >= reader->header.record_count)
            return RAF_SEG_E_LAYOUT;
        if ((rcs_u64)record->message_count
            > reader->header.record_count - record->first_message_index)
            return RAF_SEG_E_LAYOUT;
    }
    return RAF_SEG_OK;
}

int raf_segment_reader_v1_read_message(
    const raf_segment_reader_v1 *reader,
    rcs_u64 record_offset,
    raf_segment_message_v1 *record) {
    int status;
    if (!record) return RAF_SEG_E_NULL;
    status = record_bounds(reader, record_offset,
                           RAF_SEGMENT_RECORD_MESSAGE,
                           RAF_SEGMENT_V1_MESSAGE_SIZE);
    if (status != RAF_SEG_OK) return status;
    status = raf_segment_message_v1_decode(reader->bytes + record_offset, record);
    if (status != RAF_SEG_OK) return status;
    if (record->conversation_index >= reader->header.record_count)
        return RAF_SEG_E_LAYOUT;
    if (record->message_index >= reader->header.record_count)
        return RAF_SEG_E_LAYOUT;
    if (record->parent_index != RAF_SEGMENT_INDEX_NONE
        && record->parent_index >= reader->header.record_count)
        return RAF_SEG_E_LAYOUT;
    if (!range_within(record->source_offset, record->source_length,
                      reader->header.source_size)) return RAF_SEG_E_BOUNDS;
    status = payload_range(reader, record->author_offset, record->author_length);
    if (status != RAF_SEG_OK) return status;
    status = payload_range(reader, record->content_offset, record->content_length);
    if (status != RAF_SEG_OK) return status;
    if (crc32c_u64(reader->bytes + record->author_offset, record->author_length)
        != record->author_crc32c) return RAF_SEG_E_CRC;
    if (crc32c_u64(reader->bytes + record->content_offset, record->content_length)
        != record->content_crc32c) return RAF_SEG_E_CRC;
    return RAF_SEG_OK;
}
