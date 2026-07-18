#ifndef RAF_SEGMENT_V1_H
#define RAF_SEGMENT_V1_H

#include "raf_convscan.h"

#define RAF_SEGMENT_V1_HEADER_SIZE 64u
#define RAF_SEGMENT_V1_VERSION 0x00010000u

#define RAF_SEG_OK 0
#define RAF_SEG_E_NULL -1
#define RAF_SEG_E_MAGIC -2
#define RAF_SEG_E_VERSION -3
#define RAF_SEG_E_RESERVED -4
#define RAF_SEG_E_CRC -5
#define RAF_SEG_E_OFFSET -6

typedef struct raf_segment_header_v1 {
    rcs_u32 flags;
    rcs_u64 record_count;
    rcs_u64 index_offset;
    rcs_u64 payload_offset;
    rcs_u64 source_size;
    rcs_u32 source_crc32c;
    rcs_u32 header_crc32c;
} raf_segment_header_v1;

rcs_u32 raf_segment_crc32c(const rcs_u8 *data, rcs_u32 size);
int raf_segment_header_v1_encode(const raf_segment_header_v1 *header,
                                 rcs_u8 out[RAF_SEGMENT_V1_HEADER_SIZE]);
int raf_segment_header_v1_decode(const rcs_u8 in[RAF_SEGMENT_V1_HEADER_SIZE],
                                 raf_segment_header_v1 *header);

#endif
