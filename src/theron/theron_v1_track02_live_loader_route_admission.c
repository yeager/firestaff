#include "theron_v1_track02_live_loader_route_admission.h"

#include <stdio.h>
#include <string.h>

int theron_v1_track02_live_loader_route_admit(
    const Theron_V1Track02DynamicCdReadOwnershipReceipt *ownership,
    const Theron_V1Track02Huc6280CaptureEventLog *event_log,
    const Theron_V1Track02MednafenTraceConvertReceipt *trace_identity,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out_runtime,
    Theron_V1Track02LiveLoaderRouteAdmissionReceipt *out)
{
    Theron_V1Track02LiveLoaderRouteAdmissionReceipt receipt = {0};
    Theron_V1Track02CaptureTraceManifest manifest;
    Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt evidence = {0};
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt runtime;

    if (!out || !out_runtime) return 0;
    *out = receipt;
    memset(out_runtime, 0, sizeof(*out_runtime));
    if (!ownership || !event_log || !trace_identity || !provenance || !preparation ||
        !ownership->valid || !ownership->raw_cue_bin_identity_consumed ||
        !ownership->loader_trace_consumed || !ownership->register_record_normalized ||
        !ownership->raw_sector_window_owned ||
        ownership->level_object_semantics_allowed ||
        ownership->bitmap_palette_admission_allowed || ownership->pixel_decode_allowed ||
        ownership->dungeon_draw_allowed || ownership->fallback_visuals_allowed ||
        ownership->track02_variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        !provenance->valid || !preparation->valid ||
        ownership->track02_variant != provenance->track02_variant ||
        ownership->track02_variant != preparation->track02_variant ||
        strcmp(ownership->track02_md5, provenance->track02_md5) ||
        strcmp(ownership->track02_md5, preparation->track02_md5) ||
        ownership->track02_record != provenance->loader_record ||
        ownership->track02_record != preparation->loader_record ||
        ownership->destination != provenance->loader_destination ||
        ownership->user_data_bytes != 2048u ||
        ownership->destination_span_checksum == 0u ||
        ownership->full_payload_checksum == 0u ||
        trace_identity->status != THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED ||
        !trace_identity->source_trace_md5_verified ||
        !trace_identity->source_rows_observed ||
        !trace_identity->huc6280_event_log_md5_verified ||
        !trace_identity->source_trace_md5[0] || !trace_identity->event_log_md5[0] ||
        trace_identity->emulator_launched || trace_identity->media_copied ||
        trace_identity->synthetic_event_created ||
        event_log->status != THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_READY ||
        event_log->loader_pc != THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS ||
        event_log->loader_record != ownership->track02_record ||
        event_log->loader_destination != ownership->destination ||
        event_log->loader_payload_checksum != ownership->full_payload_checksum ||
        !theron_v1_track02_huc6280_capture_event_log_bind_manifest(
            event_log, provenance, preparation, &manifest)) return 0;

    evidence.valid = 1;
    evidence.raw_media_intake_consumed = 1;
    evidence.raw_trace_input_consumed = 1;
    evidence.provenance_runtime_consumed = 1;
    evidence.trace_preparation_consumed = 1;
    evidence.external_capture_manifest_consumed = 1;
    evidence.track02_variant = manifest.track02_variant;
    snprintf(evidence.track02_md5, sizeof(evidence.track02_md5), "%s", manifest.track02_md5);
    evidence.loader_record = manifest.loader_record;
    evidence.consumer_trace_checksum = manifest.consumer_trace_checksum;
    evidence.dungeon_record_consumer_pc = manifest.dungeon_record_consumer_pc;
    evidence.dungeon_record_payload_offset = manifest.dungeon_record_payload_offset;
    evidence.dungeon_record_byte_count = manifest.dungeon_record_byte_count;
    evidence.dungeon_record_window_checksum = manifest.dungeon_record_window_checksum;
    evidence.object_table_consumer_pc = manifest.object_table_consumer_pc;
    evidence.object_table_payload_offset = manifest.object_table_payload_offset;
    evidence.object_table_byte_count = manifest.object_table_byte_count;
    evidence.object_table_window_checksum = manifest.object_table_window_checksum;
    evidence.level_fields_blocked = 1;
    evidence.object_fields_blocked = 1;
    if (!theron_v1_track02_capture_trace_runtime_admit(
            &evidence, provenance, preparation, &runtime)) return 0;

    receipt.valid = 1;
    receipt.dynamic_cd_read_ownership_consumed = 1;
    receipt.huc6280_event_log_consumed = 1;
    receipt.manifest_bound = 1;
    receipt.opaque_runtime_route_ready = 1;
    receipt.track02_variant = ownership->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", ownership->track02_md5);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s",
             trace_identity->source_trace_md5);
    snprintf(receipt.huc6280_event_log_md5,
             sizeof(receipt.huc6280_event_log_md5), "%s",
             trace_identity->event_log_md5);
    receipt.loader_record = ownership->track02_record;
    receipt.loader_destination = ownership->destination;
    receipt.consumer_trace_checksum = manifest.consumer_trace_checksum;
    *out_runtime = runtime;
    *out = receipt;
    return 1;
}
