#ifndef THERON_V1_TRACK02_CAPTURE_ARTIFACT_IMPORTER_H
#define THERON_V1_TRACK02_CAPTURE_ARTIFACT_IMPORTER_H

#include "theron_v1_track02_capture_target_plan.h"
#include "theron_v1_track02_mednafen_trace_converter.h"

typedef enum {
    THERON_V1_TRACK02_CAPTURE_ARTIFACT_UNAVAILABLE = 0,
    THERON_V1_TRACK02_CAPTURE_ARTIFACT_REJECTED,
    THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY
} Theron_V1Track02CaptureArtifactStatus;

typedef struct {
    const char *bundle_path;
    const char *expected_bundle_md5;
    const char *mednafen_trace_path;
    const char *expected_mednafen_trace_md5;
} Theron_V1Track02CaptureArtifactImportRequest;

/* Runtime may retain this receipt only as an external capture identity and
 * opaque route coordinates. It is never a pixel, palette, level, or object
 * admission. */
typedef struct {
    Theron_V1Track02CaptureArtifactStatus status;
    int bundle_md5_verified;
    int mednafen_trace_md5_verified;
    int complete_route_set_consumed;
    /* Every descriptor/loader, palette, bitmap, and destination identity was
     * present, nonzero, and exact for all three envelope rows. */
    int opaque_envelope_verified;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    char bundle_md5[33];
    char mednafen_trace_md5[33];
    Theron_V1Track02CaptureTargetRoute campaign_route;
    uint32_t cd_read_record[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT];
    uint32_t loader_output_identity[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT];
    uint32_t palette_output_identity[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT];
    uint32_t bitmap_transfer_identity[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT];
    uint32_t destination_record[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT];
    uint32_t destination_identity[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT];
    int opaque_runtime_ready;
    int pixel_decode_allowed;
    int level_object_semantics_allowed;
    int render_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt;

/* Imports an external artifact manifest with exactly three ordered plan rows.
 * Both files must be direct regular files and retain their supplied MD5 values
 * before and after consumption. */
int theron_v1_track02_capture_artifact_import(
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02CaptureArtifactImportRequest *request,
    Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *out);

#endif
