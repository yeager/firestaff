#ifndef THERON_V1_TRACK02_CAMPAIGN_BUNDLE_EMITTER_H
#define THERON_V1_TRACK02_CAMPAIGN_BUNDLE_EMITTER_H

#include "theron_v1_track02_capture_target_plan.h"

typedef enum {
    THERON_V1_TRACK02_CAMPAIGN_EMIT_UNAVAILABLE = 0,
    THERON_V1_TRACK02_CAMPAIGN_EMIT_REJECTED,
    THERON_V1_TRACK02_CAMPAIGN_EMIT_DRY_RUN_READY,
    THERON_V1_TRACK02_CAMPAIGN_EMIT_WRITTEN
} Theron_V1Track02CampaignEmitStatus;

typedef struct {
    const char *media_path;
    const char *expected_track02_md5;
    const char *mednafen_trace_path;
    const char *expected_mednafen_trace_md5;
    const char *bundle_path[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT];
    int dry_run;
} Theron_V1Track02CampaignBundleEmitRequest;

typedef struct {
    Theron_V1Track02CampaignEmitStatus status;
    int raw_media_verified;
    int mednafen_trace_verified;
    int route_selection_verified;
    int emitted_without_launch;
    int media_copied;
    int synthetic_capture_row_created;
    int decoder_invoked;
    char track02_md5[33];
    char mednafen_trace_md5[33];
    char bundle_md5[THERON_V1_TRACK02_CAPTURE_TARGET_COUNT][33];
} Theron_V1Track02CampaignBundleEmitReceipt;

/* Attests three already-authenticated plan routes into separate manifests.
 * It never launches Mednafen and writes nothing in dry-run mode. */
int theron_v1_track02_campaign_bundle_emit(
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02CampaignBundleEmitRequest *request,
    Theron_V1Track02CampaignBundleEmitReceipt *out);

#endif
