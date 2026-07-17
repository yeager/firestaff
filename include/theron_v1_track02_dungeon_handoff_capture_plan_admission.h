#ifndef THERON_V1_TRACK02_DUNGEON_HANDOFF_CAPTURE_PLAN_ADMISSION_H
#define THERON_V1_TRACK02_DUNGEON_HANDOFF_CAPTURE_PLAN_ADMISSION_H

#include "theron_v1_track02_descriptor_bitmap_palette_capture_intake.h"

typedef enum {
    THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_UNAVAILABLE = 0,
    THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_REJECTED,
    THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_CAPTURE_REQUIRED,
    THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_RESUME_READY
} Theron_V1Track02DungeonCapturePlanStatus;

typedef struct {
    Theron_V1Track02DungeonCapturePlanStatus status;
    int direct_cue_bin_consumed;
    int system_card_required;
    int replay_tail_consumed;
    int capture_plan_consumed;
    int opaque_artifact_required;
    int resume_route_ready;
    int presentation_no_draw;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    char source_trace_md5[33];
    uint32_t campaign_layout_epoch;
    uint32_t campaign_media_scan_epoch;
    uint32_t replay_final_record;
    uint32_t capture_target_plan_identity;
} Theron_V1Track02DungeonCapturePlanAdmissionReceipt;

/* Reads only the operator-local V1 plan grammar. A valid plan without the
 * already admitted observed artifact is CAPTURE_REQUIRED, never ready. */
int theron_v1_track02_dungeon_capture_plan_admit(
    const char *plan_path,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    const Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeReceipt *artifact,
    uint32_t campaign_layout_epoch,
    uint32_t campaign_media_scan_epoch,
    Theron_V1Track02DungeonCapturePlanAdmissionReceipt *out);

#endif
