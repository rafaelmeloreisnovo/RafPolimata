#ifndef RAF_SEGMENT_V1_H
#define RAF_SEGMENT_V1_H

#include "raf_convscan.h"

#define RAF_SEGMENT_V1_HEADER_SIZE 64u
#define RAF_SEGMENT_V1_CONVERSATION_SIZE 96u
#define RAF_SEGMENT_V1_MESSAGE_SIZE 128u
#define RAF_SEGMENT_V1_VERSION 0x00010000u

#define RAF_SEGMENT_RECORD_CONVERSATION 1u
#define RAF_SEGMENT_RECORD_MESSAGE 2u

#define RAF_SEGMENT_ROLE_UNKNOWN 0u
#define RAF_SEGMENT_ROLE_USER 1u
#define RAF_SEGMENT_ROLE_ASSISTANT 2u
#define RAF_SEGMENT_ROLE_SYSTEM 3u
#define RAF_SEGMENT_ROLE_TOOL 4u

#define RAF_SEGMENT_INDEX_NONE ((rcs_u64)~(rcs_u64)0u)

#define RAF_SEG_OK 0
#define RAF_SEG_END 1
#define RAF_SEG_E_NULL -1
#define RAF_SEG_E_MAGIC -2
#define RAF_SEG_E_VERSION -3
#define RAF_SEG_E_RESERVED -4
#define RAF_SEG_E_CRC -5
#define RAF_SEG_E_OFFSET -6
#define RAF_SEG_E_SIZE -7
#define RAF_SEG_E_TYPE -8
#define RAF_SEG_E_BOUNDS -9
#define RAF_SEG_E_LAYOUT -10
#define RAF_SEG_E_ROLE -11

typedef struct raf_segment_header_v1 {
    rcs_u32 flags;
    rcs_u64 record_count;
    rcs_u64 index_offset;
    rcs_u64 payload_offset;
    rcs_u64 source_size;
    rcs_u32 source_crc32c;
    rcs_u32 header_crc32c;
} raf_segment_header_v1;

typedef struct raf_segment_conversation_v1 {
    rcs_u32 flags;
    rcs_u32 message_count;
    rcs_u64 id_hi;
    rcs_u64 id_lo;
    rcs_u64 source_offset;
    rcs_u64 source_length;
    rcs_u64 title_offset;
    rcs_u64 title_length;
    rcs_u64 first_message_index;
    rcs_u64 create_time_us;
    rcs_u64 update_time_us;
    rcs_u32 title_crc32c;
    rcs_u32 record_crc32c;
} raf_segment_conversation_v1;

typedef struct raf_segment_message_v1 {
    rcs_u32 flags;
    rcs_u32 role;
    rcs_u64 conversation_index;
    rcs_u64 message_index;
    rcs_u64 parent_index;
    rcs_u64 id_hi;
    rcs_u64 id_lo;
    rcs_u64 source_offset;
    rcs_u64 source_length;
    rcs_u64 author_offset;
    rcs_u64 author_length;
    rcs_u64 content_offset;
    rcs_u64 content_length;
    rcs_u64 create_time_us;
    rcs_u32 content_crc32c;
    rcs_u32 author_crc32c;
    rcs_u32 record_crc32c;
} raf_segment_message_v1;

typedef struct raf_segment_reader_v1 {
    const rcs_u8 *bytes;
    rcs_u64 size;
    raf_segment_header_v1 header;
    rcs_u64 cursor;
    rcs_u64 records_seen;
} raf_segment_reader_v1;

rcs_u32 raf_segment_crc32c(const rcs_u8 *data, rcs_u32 size);
int raf_segment_header_v1_encode(const raf_segment_header_v1 *header,
                                 rcs_u8 out[RAF_SEGMENT_V1_HEADER_SIZE]);
int raf_segment_header_v1_decode(const rcs_u8 in[RAF_SEGMENT_V1_HEADER_SIZE],
                                 raf_segment_header_v1 *header);

int raf_segment_conversation_v1_encode(
    const raf_segment_conversation_v1 *record,
    rcs_u8 out[RAF_SEGMENT_V1_CONVERSATION_SIZE]);
int raf_segment_conversation_v1_decode(
    const rcs_u8 in[RAF_SEGMENT_V1_CONVERSATION_SIZE],
    raf_segment_conversation_v1 *record);

int raf_segment_message_v1_encode(
    const raf_segment_message_v1 *record,
    rcs_u8 out[RAF_SEGMENT_V1_MESSAGE_SIZE]);
int raf_segment_message_v1_decode(
    const rcs_u8 in[RAF_SEGMENT_V1_MESSAGE_SIZE],
    raf_segment_message_v1 *record);

int raf_segment_reader_v1_init(raf_segment_reader_v1 *reader,
                               const rcs_u8 *bytes,
                               rcs_u64 size);
int raf_segment_reader_v1_next(raf_segment_reader_v1 *reader,
                               rcs_u64 *record_offset,
                               rcs_u32 *record_kind);
int raf_segment_reader_v1_read_conversation(
    const raf_segment_reader_v1 *reader,
    rcs_u64 record_offset,
    raf_segment_conversation_v1 *record);
int raf_segment_reader_v1_read_message(
    const raf_segment_reader_v1 *reader,
    rcs_u64 record_offset,
    raf_segment_message_v1 *record);

#endif
