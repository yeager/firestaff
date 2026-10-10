#ifndef FIRESTAFF_CAPS_RAW_H
#define FIRESTAFF_CAPS_RAW_H

#include <stddef.h>
#include <stdint.h>

#define FIRESTAFF_CAPS_RAW_MAX_MEMBER_BYTES (16u * 1024u * 1024u)
#define FIRESTAFF_CAPS_RAW_MAX_TRACKS 256u
#define FIRESTAFF_CAPS_RAW_MAX_PACK_UNPACKED_BYTES (64u * 1024u * 1024u)
#define FIRESTAFF_CAPS_RAW_MAX_TOTAL_UNPACKED_BYTES (256u * 1024u * 1024u)
#define FIRESTAFF_CAPS_RAW_PACKS_PER_TRACK 2u

typedef enum FirestaffCapsRawStatus {
    FIRESTAFF_CAPS_RAW_OK = 0,
    FIRESTAFF_CAPS_RAW_INVALID_ARGUMENT,
    FIRESTAFF_CAPS_RAW_TOO_LARGE,
    FIRESTAFF_CAPS_RAW_TRUNCATED,
    FIRESTAFF_CAPS_RAW_BAD_SIGNATURE,
    FIRESTAFF_CAPS_RAW_BAD_LENGTH,
    FIRESTAFF_CAPS_RAW_BAD_CRC,
    FIRESTAFF_CAPS_RAW_LIMIT_EXCEEDED,
    FIRESTAFF_CAPS_RAW_CALLBACK_ABORTED
} FirestaffCapsRawStatus;

typedef struct FirestaffCapsRawPack {
    uint32_t track_index;
    uint32_t pack_index;
    const uint8_t *packed_bytes;
    size_t packed_size;
    uint32_t unpacked_size;
    uint32_t unpacked_crc32;
} FirestaffCapsRawPack;

typedef int (*FirestaffCapsRawPackVisitor)(const FirestaffCapsRawPack *pack,
                                           void *user_data);

typedef struct FirestaffCapsRawResult {
    size_t track_count;
    size_t pack_count;
    size_t error_offset;
} FirestaffCapsRawResult;

/* Reads one complete in-memory CAPS CTRaw member. The visitor sees borrowed
 * payload spans, valid for the duration of this call. This module checks
 * framing and CRCs but deliberately leaves PACK decompression to a codec. */
FirestaffCapsRawStatus firestaff_caps_raw_read(
    const uint8_t *bytes,
    size_t byte_count,
    FirestaffCapsRawPackVisitor visitor,
    void *user_data,
    FirestaffCapsRawResult *out_result);

const char *firestaff_caps_raw_status_name(FirestaffCapsRawStatus status);

#endif
