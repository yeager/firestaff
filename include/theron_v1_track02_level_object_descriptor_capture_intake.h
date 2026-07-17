#ifndef THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_INTAKE_H
#define THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_INTAKE_H

#include "theron_v1_track02_sector_record_corpus_discovery.h"
#include "theron_v1_track02_loader_trace_replay_consistency.h"
#include "theron_v1_track02_launch_trace_identity.h"

typedef enum {
    THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_UNAVAILABLE = 0,
    THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_REJECTED,
    THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_READY
} Theron_V1Track02LevelObjectDescriptorCaptureIntakeStatus;

/* This receipt deliberately preserves only an observed descriptor-selected
 * record and its source identities. It grants no record-payload semantics. */
typedef struct {
    Theron_V1Track02LevelObjectDescriptorCaptureIntakeStatus status;
    int direct_cue_bin_consumed;
    int coalesced_loader_trace_consumed;
    int replay_tail_consumed;
    int opaque_descriptor_only;
    uint32_t campaign_layout_epoch;
    uint32_t campaign_media_scan_epoch;
    char coalesced_trace_md5[33];
    Theron_V1Track02SectorRecordCorpusDiscoveryReceipt corpus;
} Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt;

/* Attests a single existing direct CUE/BIN+coalesced-trace corpus discovery
 * against the current campaign layout, replay tail, and trace identity. The
 * caller owns discovery; missing local corpus is UNAVAILABLE, never a
 * synthesized descriptor. */
int theron_v1_track02_level_object_descriptor_capture_intake_admit(
    const Theron_V1Track02SectorRecordCorpusDiscoveryReceipt *corpus,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    const Theron_V1Track02LaunchTraceIdentityReceipt *trace_identity,
    uint32_t campaign_layout_epoch,
    uint32_t campaign_media_scan_epoch,
    Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt *out);

#endif
