#ifndef THERON_V1_TRACK02_LATER_ROUTE_CANDIDATE_PLAN_CORPUS_ADMISSION_H
#define THERON_V1_TRACK02_LATER_ROUTE_CANDIDATE_PLAN_CORPUS_ADMISSION_H

#include "theron_v1_track02_handoff_artifact_corpus.h"
#include "theron_v1_track02_later_route_candidate_campaign_index.h"
#include "theron_v1_track02_launch_trace_identity.h"

/* A source-backed later-record family joined to the current capture plan.
 * The retained coordinates are opaque; no record grammar, object, bitmap, or
 * route meaning is admitted. */
typedef struct {
    int valid;
    int candidate_family_consumed;
    int artifact_corpus_consumed;
    int capture_plan_consumed;
    int replay_tail_consumed;
    int capture_required_only;
    int no_draw_only;
    uint32_t campaign_layout_epoch;
    uint32_t campaign_media_scan_epoch;
    uint32_t capture_target_plan_identity;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    char source_trace_md5[33];
    uint16_t loader_pc;
    uint32_t record;
    uint32_t raw_sector;
    uint32_t destination_identity;
    int level_object_semantics_allowed;
    int bitmap_palette_admission_allowed;
    int pixel_decode_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02LaterRouteCandidatePlanCorpusAdmissionReceipt;

int theron_v1_track02_later_route_candidate_plan_corpus_admit(
    const Theron_V1Track02LaterRouteCandidateCampaignIndex *candidates,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02HandoffArtifactCorpusReceipt *artifact_corpus,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    const Theron_V1Track02LaunchTraceIdentityReceipt *trace_identity,
    uint32_t campaign_layout_epoch,
    uint32_t campaign_media_scan_epoch,
    Theron_V1Track02LaterRouteCandidatePlanCorpusAdmissionReceipt *out);

#endif
