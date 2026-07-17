#ifndef THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_H
#define THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_H

#include "theron_v1_track02_level_object_trace_preparation.h"
#include "theron_v1_track02_raw_media_intake.h"

typedef enum {
    THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_UNAVAILABLE = 0,
    THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_REJECTED,
    THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_READY
} Theron_V1Track02CaptureTraceManifestStatus;

/* Parsed external capture coordinates. These rows carry no byte payload and
 * do not assign record, object, bitmap, palette, or pixel semantics. */
typedef struct {
    Theron_V1Track02CaptureTraceManifestStatus status;
    char track02_md5[33];
    Theron_Track02Variant track02_variant;
    uint32_t loader_record;
    uint32_t loader_destination;
    size_t loader_payload_bytes;
    uint32_t loader_payload_checksum;
    uint32_t consumer_trace_checksum;
    uint32_t dungeon_record_consumer_pc;
    size_t dungeon_record_payload_offset;
    size_t dungeon_record_byte_count;
    uint32_t dungeon_record_window_checksum;
    uint32_t object_table_consumer_pc;
    size_t object_table_payload_offset;
    size_t object_table_byte_count;
    uint32_t object_table_window_checksum;
} Theron_V1Track02CaptureTraceManifest;

/* Source-bound, opaque handoff for a later field decoder. The records remain
 * unavailable to gameplay and visual consumers until independently reviewed
 * decoder semantics exist. */
typedef struct {
    int valid;
    int raw_media_intake_consumed;
    int raw_trace_input_consumed;
    int provenance_runtime_consumed;
    int trace_preparation_consumed;
    int external_capture_manifest_consumed;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    size_t first_user_data_offset;
    size_t logical_user_data_window_bytes;
    uint32_t loader_record;
    uint32_t consumer_trace_checksum;
    uint32_t dungeon_record_consumer_pc;
    size_t dungeon_record_payload_offset;
    size_t dungeon_record_byte_count;
    uint32_t dungeon_record_window_checksum;
    uint32_t object_table_consumer_pc;
    size_t object_table_payload_offset;
    size_t object_table_byte_count;
    uint32_t object_table_window_checksum;
    int level_fields_blocked;
    int object_fields_blocked;
    int bitmap_palette_admission_allowed;
    int pixel_decode_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt;

/* Parses a bounded external capture manifest. It accepts only the documented
 * keys once each; absent files report UNAVAILABLE and malformed input reports
 * REJECTED. */
int theron_v1_track02_capture_trace_manifest_parse(
    const char *manifest_path,
    Theron_V1Track02CaptureTraceManifest *out);

/* Joins a raw CUE/BIN intake, its raw-trace coordinates, runtime provenance,
 * preparation receipt, and a manifest. Every manifest coordinate must match
 * the already authenticated preparation; no new consumer PC or payload window
 * can be introduced by external text. */
int theron_v1_track02_capture_trace_manifest_bind(
    const Theron_V1Track02RawMediaIntakeReceipt *intake,
    const Theron_V1Track02RawTraceMediaInput *raw_trace,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    const Theron_V1Track02CaptureTraceManifest *manifest,
    Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt *out);

/* Convenience entry point for a user-supplied CUE/BIN path and external
 * manifest. It safely reports UNAVAILABLE when either local input is absent. */
int theron_v1_track02_capture_trace_manifest_discover_and_bind(
    const char *media_path,
    const char *manifest_path,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt *out);

#endif
