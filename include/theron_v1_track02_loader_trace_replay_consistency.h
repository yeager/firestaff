#ifndef THERON_V1_TRACK02_LOADER_TRACE_REPLAY_CONSISTENCY_H
#define THERON_V1_TRACK02_LOADER_TRACE_REPLAY_CONSISTENCY_H

#include "theron_v1_track02_campaign_media_discovery.h"
#include "theron_v1_track02_dynamic_cd_read_ownership.h"

/* Stateful opaque replay receipt for observed dynamic CD_READ ownership.
 * Records are never decoded: this retains only sequence and source layout
 * identity so an out-of-order or stale trace cannot reach a later route. */
typedef struct {
    int active;
    int direct_campaign_layout_consumed;
    int dynamic_cd_read_records_consumed;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint32_t campaign_layout_epoch;
    uint32_t accepted_record_count;
    uint32_t first_track02_record;
    uint32_t last_track02_record;
    size_t last_raw_sector;
    uint32_t ordered_record_checksum;
    int level_object_semantics_allowed;
    int bitmap_palette_admission_allowed;
    int pixel_decode_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02LoaderTraceReplayConsistencyReceipt;

/* Begins one direct-media layout epoch. The supplied campaign receipt, fresh
 * raw intake and plan must already agree exactly. */
int theron_v1_track02_loader_trace_replay_consistency_begin(
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02RawMediaIntakeReceipt *refreshed,
    const Theron_V1Track02CaptureTargetPlan *plan,
    uint32_t campaign_layout_epoch,
    Theron_V1Track02LoaderTraceReplayConsistencyReceipt *out);

/* Accepts one source-owned dynamic CD_READ in strictly increasing record and
 * raw-sector order under the original layout epoch. Any duplicate, reordered,
 * stale, mixed, or semantic receipt clears the replay state. */
int theron_v1_track02_loader_trace_replay_consistency_accept(
    Theron_V1Track02LoaderTraceReplayConsistencyReceipt *state,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02RawMediaIntakeReceipt *refreshed,
    const Theron_V1Track02CaptureTargetPlan *plan,
    uint32_t campaign_layout_epoch,
    const Theron_V1Track02DynamicCdReadOwnershipReceipt *ownership);

#endif
