#include "theron_v1_stage3_mode1_header.h"

#include <string.h>

#define THERON_V1_RAW_SECTOR_BYTES 2352u
#define THERON_V1_MODE1_USER_DATA_OFFSET 16u
#define THERON_V1_MODE1_USER_DATA_BYTES 2048u

int theron_v1_stage3_mode1_header_from_original_media(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1Stage3Mode1HeaderReceipt *out_receipt) {
    const uint8_t *sector;
    size_t raw_offset;
    size_t index;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!track02_data || !payload || !payload->valid ||
        (payload->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        payload->user_data_bytes != THERON_V1_MODE1_USER_DATA_BYTES ||
        payload->raw_sector > SIZE_MAX / THERON_V1_RAW_SECTOR_BYTES) {
        return 0;
    }

    raw_offset = payload->raw_sector * THERON_V1_RAW_SECTOR_BYTES;
    if (raw_offset > track02_size || THERON_V1_RAW_SECTOR_BYTES >
            track02_size - raw_offset) {
        return 0;
    }
    sector = track02_data + raw_offset;
    if (sector[0] != 0x00u || sector[11] != 0x00u || sector[15] != 0x01u) {
        return 0;
    }
    for (index = 1u; index < 11u; ++index) {
        if (sector[index] != 0xffu) return 0;
    }
    if (payload->user_data_offset != raw_offset + THERON_V1_MODE1_USER_DATA_OFFSET) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->variant = payload->variant;
    out_receipt->track02_record = payload->track02_record;
    out_receipt->raw_sector = payload->raw_sector;
    out_receipt->minute_bcd = sector[12];
    out_receipt->second_bcd = sector[13];
    out_receipt->frame_bcd = sector[14];
    out_receipt->mode = sector[15];
    out_receipt->user_data_offset = payload->user_data_offset;
    out_receipt->user_data_bytes = THERON_V1_MODE1_USER_DATA_BYTES;
    return 1;
}
