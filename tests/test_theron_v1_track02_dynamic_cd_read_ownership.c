#include "theron_v1_track02_dynamic_cd_read_ownership.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "failed: %s\n", #c); ++failures; } } while (0)

int main(void)
{
    Theron_V1Track02RawMediaIntakeReceipt intake = {0};
    Theron_V1RawLoaderTraceReceipt trace = {0};
    Theron_V1Track02DynamicCdReadOwnershipReceipt receipt;

    intake.status = THERON_V1_TRACK02_MEDIA_INTAKE_READY;
    intake.cue_consumed = intake.mode1_2352 = intake.raw_trace_preparation_allowed = 1;
    intake.variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(intake.track02_md5, sizeof(intake.track02_md5), "%s", THERON_TRACK02_MD5_US_BIN);
    intake.payload_bytes = 2352u * 0x600u;
    intake.sector_count = 0x600u;
    intake.first_user_data_offset = 225u * 2352u + 16u;
    trace.valid = trace.dynamic_cd_read_verified = 1;
    trace.dynamic_cd_read_registers_verified = 1;
    trace.dynamic_cd_read_destination_span_verified = 1;
    trace.dynamic_cd_read_media_span_verified = 1;
    trace.stage2_dynamic_payload_verified = 1;
    trace.variant = intake.variant;
    snprintf(trace.track02_md5, sizeof(trace.track02_md5), "%s", intake.track02_md5);
    trace.dynamic_cd_read_record_cl = 0xe0u;
    trace.dynamic_cd_read_record_dl = 0x04u;
    trace.dynamic_cd_read_record = 0x4e0u;
    trace.dynamic_cd_read_destination = 0x3800u;
    trace.dynamic_cd_read_destination_span_bytes = 32u;
    trace.dynamic_cd_read_destination_span_checksum = 0x12345678u;
    trace.dynamic_cd_read_raw_sector = 0x4e0u;
    trace.dynamic_cd_read_raw_offset = 0x4e0u * 2352u;
    trace.dynamic_cd_read_user_data_offset = trace.dynamic_cd_read_raw_offset + 16u;
    trace.stage2_dynamic_payload_bytes = 2048u;
    trace.stage2_dynamic_payload_checksum = 0x87654321u;
    CHECK(theron_v1_track02_dynamic_cd_read_ownership_normalize(&intake, &trace, &receipt));
    CHECK(receipt.valid && receipt.register_record_normalized && receipt.raw_sector_window_owned);
    CHECK(receipt.track02_record == 0x4e0u && receipt.user_data_bytes == 2048u);
    CHECK(!receipt.level_object_semantics_allowed && !receipt.pixel_decode_allowed && !receipt.dungeon_draw_allowed);
    trace.dynamic_cd_read_record_cl ^= 1u;
    CHECK(!theron_v1_track02_dynamic_cd_read_ownership_normalize(&intake, &trace, &receipt));
    trace.dynamic_cd_read_record_cl ^= 1u;
    trace.dynamic_cd_read_raw_offset += 1u;
    CHECK(!theron_v1_track02_dynamic_cd_read_ownership_normalize(&intake, &trace, &receipt));
    CHECK(!theron_v1_track02_dynamic_cd_read_ownership_normalize(NULL, &trace, &receipt));
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
