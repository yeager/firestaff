#include "theron_v1_track02_live_loader_route_admission.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "failed: %s\n", #c); ++failures; } } while (0)

int main(void)
{
    Theron_V1Track02DynamicCdReadOwnershipReceipt ownership = {0};
    Theron_V1Track02Huc6280CaptureEventLog log = {0};
    Theron_V1Track02ProvenanceRuntimeConsumerReceipt provenance = {0};
    Theron_V1Track02LevelObjectTracePreparationReceipt preparation = {0};
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt runtime;
    Theron_V1Track02LiveLoaderRouteAdmissionReceipt receipt;
    Theron_V1Track02MednafenTraceConvertReceipt trace_identity = {0};
    const char *md5 = THERON_TRACK02_MD5_US_BIN;

    ownership.valid = ownership.raw_cue_bin_identity_consumed = 1;
    ownership.loader_trace_consumed = ownership.register_record_normalized = 1;
    ownership.raw_sector_window_owned = 1;
    ownership.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(ownership.track02_md5, sizeof(ownership.track02_md5), "%s", md5);
    ownership.track02_record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    ownership.destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    ownership.user_data_bytes = 2048u;
    ownership.destination_span_checksum = 1u;
    ownership.full_payload_checksum = 0x7b0f13c9u;
    trace_identity.status = THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED;
    trace_identity.source_trace_md5_verified = trace_identity.source_rows_observed = 1;
    trace_identity.huc6280_event_log_md5_verified = 1;
    snprintf(trace_identity.source_trace_md5, sizeof(trace_identity.source_trace_md5),
             "11111111111111111111111111111111");
    snprintf(trace_identity.event_log_md5, sizeof(trace_identity.event_log_md5),
             "22222222222222222222222222222222");
    log.status = THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_READY;
    log.opaque_cd_read_window_retained = log.opaque_dungeon_window_retained = 1;
    log.opaque_object_window_retained = 1;
    log.loader_pc = THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS;
    log.loader_record = ownership.track02_record;
    log.loader_destination = ownership.destination;
    log.loader_payload_bytes = 2048u;
    log.loader_payload_checksum = ownership.full_payload_checksum;
    log.consumer_trace_checksum = 0x2468ace0u;
    provenance.valid = 1; provenance.track02_variant = ownership.track02_variant;
    snprintf(provenance.track02_md5, sizeof(provenance.track02_md5), "%s", md5);
    provenance.loader_record = ownership.track02_record;
    provenance.loader_destination = ownership.destination;
    provenance.loader_payload_bytes = 2048u; provenance.loader_payload_checksum = ownership.full_payload_checksum;
    preparation.valid = 1; preparation.track02_variant = ownership.track02_variant;
    snprintf(preparation.track02_md5, sizeof(preparation.track02_md5), "%s", md5);
    preparation.loader_record = ownership.track02_record; preparation.consumer_trace_checksum = log.consumer_trace_checksum;
    preparation.dungeon_record_consumer_pc = log.dungeon_record_consumer_pc = 0x4120u;
    preparation.dungeon_record_payload_offset = log.dungeon_record_payload_offset = 0x114u;
    preparation.dungeon_record_byte_count = log.dungeon_record_byte_count = 876u;
    preparation.dungeon_record_window_checksum = log.dungeon_record_window_checksum = 0x3a5d7811u;
    preparation.object_table_consumer_pc = log.object_table_consumer_pc = 0x4180u;
    preparation.object_table_payload_offset = log.object_table_payload_offset = 0x480u;
    preparation.object_table_byte_count = log.object_table_byte_count = 896u;
    preparation.object_table_window_checksum = log.object_table_window_checksum = 0x55aa7744u;
    CHECK(theron_v1_track02_live_loader_route_admit(&ownership, &log, &trace_identity, &provenance, &preparation, &runtime, &receipt));
    CHECK(receipt.valid && receipt.opaque_runtime_route_ready && runtime.opaque_route_ready);
    CHECK(!strcmp(receipt.huc6280_event_log_md5, trace_identity.event_log_md5));
    CHECK(!runtime.level_field_decoder_allowed && !runtime.dungeon_draw_allowed);
    log.loader_record++;
    CHECK(!theron_v1_track02_live_loader_route_admit(&ownership, &log, &trace_identity, &provenance, &preparation, &runtime, &receipt));
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
