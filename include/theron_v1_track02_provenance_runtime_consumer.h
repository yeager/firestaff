#ifndef THERON_V1_TRACK02_PROVENANCE_RUNTIME_CONSUMER_H
#define THERON_V1_TRACK02_PROVENANCE_RUNTIME_CONSUMER_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_bitmap_capture_runtime_admission.h"

/* A runtime-owned join of two independent Track 02 source receipts. It proves
 * only that the initial opaque loader record and Soul Room bitmap capture have
 * the same authenticated source identity. */
typedef struct {
    int valid;
    int bitmap_capture_runtime_consumed;
    int loader_record_runtime_consumed;
    int same_track02_source_verified;
    int original_level_object_consumer_trace_required;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint32_t loader_record;
    uint32_t loader_destination;
    size_t loader_raw_user_data_offset;
    size_t loader_payload_bytes;
    uint32_t loader_payload_checksum;
    size_t level_envelope_offset;
    size_t level_envelope_bytes;
    uint32_t level_envelope_checksum;
    size_t post_envelope_offset;
    size_t post_envelope_bytes;
    uint32_t post_envelope_checksum;
    unsigned int bitmap_route_bit;
    uint32_t bitmap_checksum;
    int level_admission_allowed;
    int object_admission_allowed;
    int pixel_decode_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02ProvenanceRuntimeConsumerReceipt;

/* Consumes the capture-bound Soul Room receipt beside the opaque initial
 * loader record already owned by runtime. It never decodes or publishes a
 * level/object record and fails closed until an original consumer trace is
 * available. */
int theron_v1_track02_provenance_runtime_consume(
    const Theron_V1BitmapCaptureRuntimeAdmissionReceipt *bitmap_capture,
    const Theron_V1_World *world,
    Theron_V1Track02ProvenanceRuntimeConsumerReceipt *out);

#endif
