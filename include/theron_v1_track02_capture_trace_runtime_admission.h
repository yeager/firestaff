#ifndef THERON_V1_TRACK02_CAPTURE_TRACE_RUNTIME_ADMISSION_H
#define THERON_V1_TRACK02_CAPTURE_TRACE_RUNTIME_ADMISSION_H

#include "theron_v1_track02_capture_trace_manifest.h"

/* Runtime-visible readiness for an explicitly bound external capture trace.
 * `opaque_route_ready` means only that later reviewed decoder work may inspect
 * the retained source coordinates; it never authorizes record interpretation
 * or any visual output. */
typedef struct {
    int valid;
    int external_capture_manifest_consumed;
    int provenance_runtime_consumed;
    int trace_preparation_consumed;
    int opaque_route_ready;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
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
    int level_field_decoder_allowed;
    int object_field_decoder_allowed;
    int bitmap_palette_admission_allowed;
    int pixel_decode_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt;

/* Admits only manifest-bound opaque coordinates that exactly match the
 * existing provenance and preparation receipts. It cannot replace a manifest
 * with runtime fixture state and leaves every decoder/draw path closed. */
int theron_v1_track02_capture_trace_runtime_admit(
    const Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt *evidence,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *out);

#endif
