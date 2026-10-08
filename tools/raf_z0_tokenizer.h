#ifndef RAF_Z0_TOKENIZER_H
#define RAF_Z0_TOKENIZER_H

/*
 * RAFAELIA z0 byte observer.
 *
 * Scope: deterministic byte classification only. No model context, no
 * attention, no learned weights, no heap, no libc, no external tables.
 * VISIBLE_FIELD means nonblank/non-underscore bytes, not printable text.
 */

typedef enum raf_z0_token_kind {
    RAF_Z0_ABSOLUTE_EMPTY = 0,
    RAF_Z0_BLANK_FIELD = 1,
    RAF_Z0_UNDERSCORE_FIELD = 2,
    RAF_Z0_VISIBLE_FIELD = 3,
    RAF_Z0_MIXED_FIELD = 4,
    RAF_Z0_MISSING_SOURCE = 5
} raf_z0_token_kind;

typedef struct raf_z0_observation {
    unsigned long byte_count;
    unsigned long blank_count;
    unsigned long underscore_count;
    unsigned long visible_count;
    raf_z0_token_kind kind;
} raf_z0_observation;

static inline int raf_z0_is_blank_byte(unsigned char byte)
{
    return byte == (unsigned char)' ' ||
           byte == (unsigned char)'\t' ||
           byte == (unsigned char)'\n' ||
           byte == (unsigned char)'\r';
}

static inline raf_z0_observation raf_z0_observe_bytes(const unsigned char *bytes,
                                               unsigned long length)
{
    raf_z0_observation observation;
    unsigned long index;

    observation.byte_count = length;
    observation.blank_count = 0UL;
    observation.underscore_count = 0UL;
    observation.visible_count = 0UL;
    observation.kind = RAF_Z0_ABSOLUTE_EMPTY;

    if (length == 0UL) {
        return observation;
    }

    if (bytes == 0) {
        observation.kind = RAF_Z0_MISSING_SOURCE;
        return observation;
    }

    for (index = 0UL; index < length; index++) {
        unsigned char byte = bytes[index];

        if (raf_z0_is_blank_byte(byte)) {
            observation.blank_count++;
        } else if (byte == (unsigned char)'_') {
            observation.underscore_count++;
        } else {
            observation.visible_count++;
        }
    }

    if (observation.blank_count == length) {
        observation.kind = RAF_Z0_BLANK_FIELD;
    } else if (observation.underscore_count == length) {
        observation.kind = RAF_Z0_UNDERSCORE_FIELD;
    } else if (observation.visible_count == length) {
        observation.kind = RAF_Z0_VISIBLE_FIELD;
    } else {
        observation.kind = RAF_Z0_MIXED_FIELD;
    }

    return observation;
}

#endif
