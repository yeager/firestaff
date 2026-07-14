#include "theron_v1_raw_loader_trace.h"

#include <stdlib.h>
#include <string.h>

int main(void)
{
    Theron_V1RawLoaderTraceReceipt trace;
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1RawLoaderTraceStage3SectorReceipt receipt;
    Theron_V1RawLoaderTraceLaterSectorReceipt later_receipt;
    uint8_t *track;
    size_t track_size = 0x0511u * THERON_TRACK02_RAW_SECTOR_BYTES;
    static const char later_capture[] =
        "source=mednafen-pce-instrumented\n"
        "later_system_card_e009_dispatch caller_pc=ea00 return_pc=ea03 sector_count=1 record_cl=10 record_dl=5 record_ch=0 record=510\n"
        "later_system_card_e009_return caller_pc=ea00 return_pc=ea03 record=510\n";

    memset(&trace, 0, sizeof(trace));
    memset(&payload, 0, sizeof(payload));
    trace.valid = 1;
    trace.variant = THERON_TRACK02_VARIANT_US_BIN;
    strcpy(trace.track02_md5, THERON_TRACK02_MD5_US_BIN);
    trace.dynamic_cd_read_record = 0x0004e0u;
    trace.dynamic_cd_read_destination = 0x3800u;
    trace.dynamic_cd_read_destination_span_bytes = 32u;
    trace.dynamic_cd_read_destination_span_checksum = 1u;
    trace.dynamic_cd_read_raw_sector = 0x0004e0u;
    trace.dynamic_cd_read_raw_offset = 0x002cca00u;
    trace.dynamic_cd_read_user_data_offset = 0x002cca10u;
    trace.dynamic_cd_read_verified = 1;
    trace.dynamic_cd_read_registers_verified = 1;
    trace.dynamic_cd_read_destination_span_verified = 1;
    trace.dynamic_cd_read_media_span_verified = 1;
    trace.stage2_dynamic_payload_verified = 1;
    payload.valid = 1;
    payload.variant = THERON_TRACK02_VARIANT_US_BIN;
    payload.track02_record = 0x0004e0u;
    payload.raw_sector = 0x0004e0u;
    payload.raw_offset = 0x002cca00u;
    payload.user_data_offset = 0x002cca10u;
    payload.user_data_bytes = THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES;
    payload.user_data_hash = 1u;

    if (!theron_v1_raw_loader_trace_stage3_sector_receipt_from_bound_span(
            &trace, &payload, &receipt) || !receipt.valid ||
        !receipt.observed_cd_read_to_media_span_verified ||
        !receipt.stage3_handoff_record_proven ||
        receipt.stage3_track02_record != payload.track02_record ||
        receipt.stage3_user_data_hash != payload.user_data_hash ||
        receipt.observed_destination_span_checksum !=
            trace.dynamic_cd_read_destination_span_checksum) {
        return 1;
    }

    track = (uint8_t *)calloc(1u, track_size);
    if (!track) return 1;
    track[0x0510u * THERON_TRACK02_RAW_SECTOR_BYTES +
          THERON_TRACK02_RAW_USER_DATA_OFFSET] = 0x5au;
    if (!theron_v1_raw_loader_trace_bind_later_e009_sector(
            &trace, later_capture, track, track_size,
            THERON_TRACK02_MD5_US_BIN, &later_receipt) ||
        !later_receipt.valid || !later_receipt.later_e009_return_verified ||
        !later_receipt.later_cd_read_to_media_verified ||
        later_receipt.stage3_track02_record != 0x0004e0u ||
        later_receipt.later_track02_record != 0x000510u ||
        later_receipt.caller_pc != 0xea00u || later_receipt.return_pc != 0xea03u ||
        later_receipt.sector_count != 1u ||
        later_receipt.first_raw_offset !=
            0x0510u * THERON_TRACK02_RAW_SECTOR_BYTES ||
        later_receipt.user_data_bytes != THERON_TRACK02_RAW_USER_DATA_BYTES ||
        !later_receipt.user_data_hash) {
        free(track);
        return 1;
    }
    free(track);

    if (theron_v1_raw_loader_trace_bind_later_e009_sector(
            &trace,
            "source=mednafen-pce-instrumented\n"
            "later_system_card_e009_dispatch caller_pc=ea00 return_pc=ea03 sector_count=1 record_cl=10 record_dl=5 record_ch=0 record=510\n"
            "later_system_card_e009_return caller_pc=ea00 return_pc=ea03 record=510\n"
            "later_system_card_e009_return caller_pc=ea00 return_pc=ea03 record=510\n",
            (const uint8_t *)"x", THERON_TRACK02_RAW_SECTOR_BYTES,
            THERON_TRACK02_MD5_US_BIN, &later_receipt)) return 1;

    payload.raw_sector++;
    return !theron_v1_raw_loader_trace_stage3_sector_receipt_from_bound_span(
        &trace, &payload, &receipt) ? 0 : 1;
}
