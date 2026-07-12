#ifndef THERON_V1_STAGE3_MODE1_HEADER_H
#define THERON_V1_STAGE3_MODE1_HEADER_H

#include "theron_v1_track02.h"

#include <stddef.h>
#include <stdint.h>

/* Physical MODE1/2352 envelope for the already-proven one-sector stage-three
 * load. This is transport provenance only, not a payload decoder. */

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t track02_record;
    size_t raw_sector;
    uint8_t minute_bcd;
    uint8_t second_bcd;
    uint8_t frame_bcd;
    uint8_t mode;
    size_t user_data_offset;
    size_t user_data_bytes;
} Theron_V1Stage3Mode1HeaderReceipt;

int theron_v1_stage3_mode1_header_from_original_media(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1Stage3Mode1HeaderReceipt *out_receipt);

#endif /* THERON_V1_STAGE3_MODE1_HEADER_H */
