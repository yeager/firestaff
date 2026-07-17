#ifndef THERON_V1_TRACK02_CAPTURE_CAMPAIGN_H
#define THERON_V1_TRACK02_CAPTURE_CAMPAIGN_H

#include "theron_v1_track02_capture_artifact_importer.h"

typedef struct {
    Theron_V1Track02CaptureTargetRoute route;
    const Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *bundle;
} Theron_V1Track02CaptureCampaignRouteInput;

typedef struct {
    int valid;
    int independent_bundles_verified;
    int shared_track02_provenance_verified;
    int shared_loader_provenance_verified;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    char mednafen_trace_md5[33];
    char bundle_md5[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT][33];
    uint32_t route_destination_identity[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT];
    int pixel_decode_allowed;
    int level_object_semantics_allowed;
    int render_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02CaptureCampaignReceipt;

/* Requires three independently hashed, complete bundle admissions in fixed
 * start/Soul Room/dungeon order. It compares provenance and opaque transfer
 * identities only; it cannot promote any payload meaning or visual output. */
int theron_v1_track02_capture_campaign_verify(
    const Theron_V1Track02CaptureCampaignRouteInput *inputs,
    size_t input_count,
    Theron_V1Track02CaptureCampaignReceipt *out);

#endif
