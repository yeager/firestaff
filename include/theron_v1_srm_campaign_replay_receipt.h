#ifndef THERON_V1_SRM_CAMPAIGN_REPLAY_RECEIPT_H
#define THERON_V1_SRM_CAMPAIGN_REPLAY_RECEIPT_H

#include "theron_v1_srm_opaque_admission.h"
#include "theron_v1_track02_campaign_media_discovery.h"
#include "theron_v1_track02_loader_trace_replay_consistency.h"

typedef struct {
    int valid;
    int opaque_save_consumed;
    int direct_campaign_consumed;
    int replay_consumed;
    char srm_md5[33];
    size_t srm_size;
    uint32_t srm_identity_fnv1a;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint32_t campaign_layout_epoch;
    uint32_t replay_final_record;
    size_t replay_final_raw_sector;
    int save_semantics_decoded;
    int synthetic_fallback_used;
} Theron_V1SrmCampaignReplayReceipt;

int theron_v1_srm_campaign_replay_bind(
    const Theron_V1SrmOpaqueAdmissionReceipt *save,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02RawMediaIntakeReceipt *refreshed,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    uint32_t campaign_layout_epoch,
    Theron_V1SrmCampaignReplayReceipt *out);

#endif
