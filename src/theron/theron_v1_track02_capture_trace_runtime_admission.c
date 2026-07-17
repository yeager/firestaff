#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_capture_trace_runtime_admission.h"

int theron_v1_track02_capture_trace_runtime_admit(
    const Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt *evidence,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out) {
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt receipt = {0};

    if (!out) return 0;
    *out = receipt;
    if (!evidence || !provenance || !preparation || !evidence->valid ||
        !evidence->raw_media_intake_consumed || !evidence->raw_trace_input_consumed ||
        !evidence->provenance_runtime_consumed || !evidence->trace_preparation_consumed ||
        !evidence->external_capture_manifest_consumed || !provenance->valid ||
        !preparation->valid || evidence->track02_variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        evidence->track02_variant != provenance->track02_variant ||
        evidence->track02_variant != preparation->track02_variant ||
        strcmp(evidence->track02_md5, provenance->track02_md5) ||
        strcmp(evidence->track02_md5, preparation->track02_md5) ||
        evidence->loader_record != provenance->loader_record ||
        evidence->loader_record != preparation->loader_record ||
        evidence->consumer_trace_checksum != preparation->consumer_trace_checksum ||
        evidence->dungeon_record_consumer_pc != preparation->dungeon_record_consumer_pc ||
        evidence->dungeon_record_payload_offset != preparation->dungeon_record_payload_offset ||
        evidence->dungeon_record_byte_count != preparation->dungeon_record_byte_count ||
        evidence->dungeon_record_window_checksum != preparation->dungeon_record_window_checksum ||
        evidence->object_table_consumer_pc != preparation->object_table_consumer_pc ||
        evidence->object_table_payload_offset != preparation->object_table_payload_offset ||
        evidence->object_table_byte_count != preparation->object_table_byte_count ||
        evidence->object_table_window_checksum != preparation->object_table_window_checksum ||
        !evidence->level_fields_blocked || !evidence->object_fields_blocked ||
        evidence->bitmap_palette_admission_allowed || evidence->pixel_decode_allowed ||
        evidence->dungeon_draw_allowed || evidence->fallback_visuals_allowed ||
        preparation->level_admission_allowed || preparation->object_admission_allowed ||
        preparation->bitmap_palette_admission_allowed || preparation->dungeon_draw_allowed ||
        preparation->fallback_visuals_allowed) return 0;

    receipt.valid = 1;
    receipt.external_capture_manifest_consumed = 1;
    receipt.provenance_runtime_consumed = 1;
    receipt.trace_preparation_consumed = 1;
    receipt.opaque_route_ready = 1;
    receipt.track02_variant = evidence->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             evidence->track02_md5);
    receipt.loader_record = evidence->loader_record;
    receipt.consumer_trace_checksum = evidence->consumer_trace_checksum;
    receipt.dungeon_record_consumer_pc = evidence->dungeon_record_consumer_pc;
    receipt.dungeon_record_payload_offset = evidence->dungeon_record_payload_offset;
    receipt.dungeon_record_byte_count = evidence->dungeon_record_byte_count;
    receipt.dungeon_record_window_checksum = evidence->dungeon_record_window_checksum;
    receipt.object_table_consumer_pc = evidence->object_table_consumer_pc;
    receipt.object_table_payload_offset = evidence->object_table_payload_offset;
    receipt.object_table_byte_count = evidence->object_table_byte_count;
    receipt.object_table_window_checksum = evidence->object_table_window_checksum;
    *out = receipt;
    return 1;
}
