#ifndef THERON_V1_TRACK02_CAPTURE_TARGET_PLAN_H
#define THERON_V1_TRACK02_CAPTURE_TARGET_PLAN_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_bitmap_capture_runtime_admission.h"
#include "theron_v1_track02_capture_trace_runtime_admission.h"
#include "theron_v1_track02_palette_route.h"
#include "theron_v1_track02_raw_media_intake.h"

typedef enum {
    THERON_V1_TRACK02_CAPTURE_TARGET_START = 0,
    THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM,
    THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF
} Theron_V1Track02CaptureTargetRoute;

#define THERON_V1_TRACK02_CAPTURE_TARGET_COUNT 3u

/* A capture target names only source/capture coordinates.  `bitmap_transfer`
 * is an observation request, not an asserted pixel format or decoded image. */
typedef struct {
    Theron_V1Track02CaptureTargetRoute route;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint32_t cd_read_record;
    size_t loader_output_raw_offset;
    size_t loader_output_bytes;
    uint32_t loader_output_checksum;
    uint16_t palette_index_address;
    uint16_t palette_low_address;
    uint16_t palette_high_address;
    uint32_t palette_output_identity;
    int bitmap_transfer_capture_required;
    size_t bitmap_raw_offset;
    size_t bitmap_bytes;
    uint32_t bitmap_identity;
    uint32_t destination_record;
    size_t destination_offset;
    size_t destination_bytes;
    uint32_t destination_identity;
    int level_object_semantics_allowed;
    int pixel_decode_allowed;
    int render_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02CaptureTarget;

typedef struct {
    int valid;
    int cue_track_consumed;
    int cd_read_chain_consumed;
    int loader_output_consumed;
    int palette_output_consumed;
    int bitmap_transfer_consumed;
    int destination_record_consumed;
    Theron_V1Track02CaptureTarget targets[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT];
    int level_object_semantics_allowed;
    int pixel_decode_allowed;
    int render_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02CaptureTargetPlan;

/* Builds all three source-locked capture targets only when every input is an
 * already admitted receipt for the same raw CUE/Track 02 identity. */
int theron_v1_track02_capture_target_plan_build(
    const Theron_V1Track02RawMediaIntakeReceipt *media,
    const Theron_V1RawLoaderTraceReceipt *loader,
    const Theron_V1Track02PaletteRouteReceipt *palette,
    const Theron_V1BitmapCaptureRuntimeAdmissionReceipt *bitmap,
    const Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *destination,
    Theron_V1Track02CaptureTargetPlan *out);

/* Stable opaque identity for source-owned capture coordinates and output
 * witnesses. Returns zero for a malformed or decoding-enabled plan. */
uint32_t theron_v1_track02_capture_target_plan_identity(
    const Theron_V1Track02CaptureTargetPlan *plan);

#endif
