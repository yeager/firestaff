#include "theron_v1_raw_loader_trace.h"

#include <string.h>

int main(void)
{
    Theron_V1RawLoaderTraceReceipt trace;
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1RawLoaderTraceStage3SectorReceipt receipt;

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

    payload.raw_sector++;
    return !theron_v1_raw_loader_trace_stage3_sector_receipt_from_bound_span(
        &trace, &payload, &receipt) ? 0 : 1;
}
