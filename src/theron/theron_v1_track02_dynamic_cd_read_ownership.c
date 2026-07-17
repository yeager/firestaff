#include "theron_v1_track02_dynamic_cd_read_ownership.h"

#include <stdio.h>
#include <string.h>

int theron_v1_track02_dynamic_cd_read_ownership_normalize(
    const Theron_V1Track02RawMediaIntakeReceipt *intake,
    const Theron_V1RawLoaderTraceReceipt *trace,
    Theron_V1Track02DynamicCdReadOwnershipReceipt *out)
{
    Theron_V1Track02DynamicCdReadOwnershipReceipt receipt = {0};
    uint32_t register_record;

    if (!out) return 0;
    *out = receipt;
    if (!intake || !trace ||
        intake->status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        !intake->cue_consumed || !intake->mode1_2352 ||
        !intake->raw_trace_preparation_allowed ||
        (intake->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         intake->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        intake->payload_bytes == 0u || intake->payload_bytes % 2352u != 0u ||
        intake->sector_count != intake->payload_bytes / 2352u ||
        !trace->valid || !trace->dynamic_cd_read_verified ||
        !trace->dynamic_cd_read_registers_verified ||
        !trace->dynamic_cd_read_destination_span_verified ||
        !trace->dynamic_cd_read_media_span_verified ||
        !trace->stage2_dynamic_payload_verified ||
        trace->variant != intake->variant ||
        strcmp(trace->track02_md5, intake->track02_md5) != 0 ||
        trace->dynamic_cd_read_destination != 0x3800u ||
        trace->dynamic_cd_read_destination_span_bytes == 0u ||
        trace->dynamic_cd_read_destination_span_checksum == 0u ||
        trace->stage2_dynamic_payload_bytes != 2048u ||
        trace->stage2_dynamic_payload_checksum == 0u) return 0;

    register_record = (uint32_t)trace->dynamic_cd_read_record_cl |
        ((uint32_t)trace->dynamic_cd_read_record_dl << 8u) |
        ((uint32_t)trace->dynamic_cd_read_record_ch << 16u);
    if (register_record == 0u || register_record != trace->dynamic_cd_read_record ||
        trace->dynamic_cd_read_raw_sector != register_record ||
        trace->dynamic_cd_read_raw_sector >= intake->sector_count ||
        trace->dynamic_cd_read_raw_sector > SIZE_MAX / 2352u ||
        trace->dynamic_cd_read_raw_offset != trace->dynamic_cd_read_raw_sector * 2352u ||
        trace->dynamic_cd_read_raw_offset > intake->payload_bytes ||
        intake->payload_bytes - trace->dynamic_cd_read_raw_offset < 2352u ||
        trace->dynamic_cd_read_user_data_offset !=
            trace->dynamic_cd_read_raw_offset + 16u ||
        trace->dynamic_cd_read_user_data_offset < intake->first_user_data_offset ||
        trace->dynamic_cd_read_user_data_offset > intake->payload_bytes ||
        intake->payload_bytes - trace->dynamic_cd_read_user_data_offset < 2048u ||
        trace->dynamic_cd_read_destination_span_bytes > 2048u) return 0;

    receipt.valid = 1;
    receipt.raw_cue_bin_identity_consumed = 1;
    receipt.loader_trace_consumed = 1;
    receipt.register_record_normalized = 1;
    receipt.raw_sector_window_owned = 1;
    receipt.track02_variant = intake->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", intake->track02_md5);
    receipt.record_cl = trace->dynamic_cd_read_record_cl;
    receipt.record_dl = trace->dynamic_cd_read_record_dl;
    receipt.record_ch = trace->dynamic_cd_read_record_ch;
    receipt.track02_record = register_record;
    receipt.destination = trace->dynamic_cd_read_destination;
    receipt.raw_sector = trace->dynamic_cd_read_raw_sector;
    receipt.raw_offset = trace->dynamic_cd_read_raw_offset;
    receipt.user_data_offset = trace->dynamic_cd_read_user_data_offset;
    receipt.user_data_bytes = 2048u;
    receipt.destination_span_checksum = trace->dynamic_cd_read_destination_span_checksum;
    receipt.full_payload_checksum = trace->stage2_dynamic_payload_checksum;
    *out = receipt;
    return 1;
}
