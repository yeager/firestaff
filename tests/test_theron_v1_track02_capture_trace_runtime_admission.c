#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_track02_capture_trace_runtime_admission.h"

static int failures;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); ++failures; \
} } while (0)

int main(void) {
    Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt evidence = {0};
    Theron_V1Track02ProvenanceRuntimeConsumerReceipt provenance = {0};
    Theron_V1Track02LevelObjectTracePreparationReceipt preparation = {0};
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt admission;

    provenance.valid = 1;
    provenance.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(provenance.track02_md5, sizeof(provenance.track02_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    provenance.loader_record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    preparation.valid = 1;
    preparation.track02_variant = provenance.track02_variant;
    snprintf(preparation.track02_md5, sizeof(preparation.track02_md5), "%s",
             provenance.track02_md5);
    preparation.loader_record = provenance.loader_record;
    preparation.consumer_trace_checksum = 0x2468ace0u;
    preparation.dungeon_record_consumer_pc = 0x4120u;
    preparation.dungeon_record_payload_offset = THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    preparation.dungeon_record_byte_count = THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES;
    preparation.dungeon_record_window_checksum = 0x3a5d7811u;
    preparation.object_table_consumer_pc = 0x4180u;
    preparation.object_table_payload_offset = THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    preparation.object_table_byte_count = THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES;
    preparation.object_table_window_checksum = 0x55aa7744u;

    evidence.valid = 1;
    evidence.raw_media_intake_consumed = 1;
    evidence.raw_trace_input_consumed = 1;
    evidence.provenance_runtime_consumed = 1;
    evidence.trace_preparation_consumed = 1;
    evidence.external_capture_manifest_consumed = 1;
    evidence.track02_variant = provenance.track02_variant;
    snprintf(evidence.track02_md5, sizeof(evidence.track02_md5), "%s",
             provenance.track02_md5);
    evidence.loader_record = provenance.loader_record;
    evidence.consumer_trace_checksum = preparation.consumer_trace_checksum;
    evidence.dungeon_record_consumer_pc = preparation.dungeon_record_consumer_pc;
    evidence.dungeon_record_payload_offset = preparation.dungeon_record_payload_offset;
    evidence.dungeon_record_byte_count = preparation.dungeon_record_byte_count;
    evidence.dungeon_record_window_checksum = preparation.dungeon_record_window_checksum;
    evidence.object_table_consumer_pc = preparation.object_table_consumer_pc;
    evidence.object_table_payload_offset = preparation.object_table_payload_offset;
    evidence.object_table_byte_count = preparation.object_table_byte_count;
    evidence.object_table_window_checksum = preparation.object_table_window_checksum;
    evidence.level_fields_blocked = 1;
    evidence.object_fields_blocked = 1;

    CHECK(theron_v1_track02_capture_trace_runtime_admit(
        &evidence, &provenance, &preparation, &admission));
    CHECK(admission.valid && admission.opaque_route_ready);
    CHECK(admission.external_capture_manifest_consumed);
    CHECK(!admission.level_field_decoder_allowed);
    CHECK(!admission.object_field_decoder_allowed);
    CHECK(!admission.bitmap_palette_admission_allowed);
    CHECK(!admission.pixel_decode_allowed && !admission.dungeon_draw_allowed);
    CHECK(!admission.fallback_visuals_allowed);

    evidence.dungeon_record_consumer_pc++;
    CHECK(!theron_v1_track02_capture_trace_runtime_admit(
        &evidence, &provenance, &preparation, &admission));
    evidence.dungeon_record_consumer_pc = preparation.dungeon_record_consumer_pc;
    evidence.external_capture_manifest_consumed = 0;
    CHECK(!theron_v1_track02_capture_trace_runtime_admit(
        &evidence, &provenance, &preparation, &admission));
    evidence.external_capture_manifest_consumed = 1;
    evidence.dungeon_draw_allowed = 1;
    CHECK(!theron_v1_track02_capture_trace_runtime_admit(
        &evidence, &provenance, &preparation, &admission));
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
