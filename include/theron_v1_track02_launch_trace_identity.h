#ifndef THERON_V1_TRACK02_LAUNCH_TRACE_IDENTITY_H
#define THERON_V1_TRACK02_LAUNCH_TRACE_IDENTITY_H

#include "theron_v1_track02_live_loader_route_admission.h"
#include "theron_v1_track02_loader_trace_replay_consistency.h"

typedef struct {
    int valid;
    int direct_campaign_consumed;
    int loader_trace_consumed;
    int event_log_consumed;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint32_t campaign_layout_epoch;
    char source_trace_md5[33];
    char event_log_md5[33];
    uint32_t final_track02_record;
    size_t final_raw_sector;
    int level_object_semantics_allowed;
    int bitmap_palette_admission_allowed;
    int pixel_decode_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02LaunchTraceIdentityReceipt;

int theron_v1_track02_launch_trace_identity_bind(
    const Theron_V1Track02LiveLoaderRouteAdmissionReceipt *live,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    uint32_t campaign_layout_epoch,
    Theron_V1Track02LaunchTraceIdentityReceipt *out);

#endif
