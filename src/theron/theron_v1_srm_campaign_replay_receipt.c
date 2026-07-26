#include "theron_v1_srm_campaign_replay_receipt.h"

#include <stdio.h>
#include <string.h>

static uint32_t fnv(const char *text, size_t size) {
    uint32_t h = 2166136261u;
    while (text && *text) { h ^= (unsigned char)*text++; h *= 16777619u; }
    for (unsigned int i = 0; i < sizeof(size); ++i) { h ^= (unsigned char)(size >> (i * 8u)); h *= 16777619u; }
    return h;
}

int theron_v1_srm_campaign_replay_bind(const Theron_V1SrmOpaqueAdmissionReceipt *save,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02RawMediaIntakeReceipt *refreshed,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    uint32_t epoch, Theron_V1SrmCampaignReplayReceipt *out) {
    Theron_V1SrmCampaignReplayReceipt r = {0};
    if (!out) return 0;
    *out = r;
    if (!save || save->status != THERON_V1_SRM_OPAQUE_READY ||
        !save->source_regular_file_verified || !save->source_md5_verified ||
        !save->source_size_verified || !save->admission_version_verified ||
        !save->source_shape_verified || !save->track02_identity_verified ||
        !save->opaque_save_route_ready || save->save_semantics_decoded ||
        save->synthetic_fallback_used || !save->srm_md5[0] || !save->srm_size ||
        !theron_v1_track02_campaign_media_direct_layout_current(media, refreshed, plan) ||
        !replay || !replay->active || !replay->direct_campaign_layout_consumed ||
        !replay->dynamic_cd_read_records_consumed || !replay->accepted_record_count ||
        replay->campaign_layout_epoch != epoch || !epoch ||
        replay->track02_variant != save->track02_variant ||
        strcmp(replay->track02_md5, save->track02_md5) ||
        media->track02_variant != save->track02_variant ||
        strcmp(media->track02_md5, save->track02_md5) ||
        !replay->last_track02_record || !replay->last_raw_sector ||
        replay->level_object_semantics_allowed || replay->pixel_decode_allowed ||
        replay->dungeon_draw_allowed || replay->fallback_visuals_allowed) return 0;
    r.valid = r.opaque_save_consumed = r.direct_campaign_consumed = r.replay_consumed = 1;
    snprintf(r.srm_md5, sizeof(r.srm_md5), "%s", save->srm_md5); r.srm_size = save->srm_size;
    r.srm_identity_fnv1a = fnv(save->srm_md5, save->srm_size);
    r.track02_variant = save->track02_variant; snprintf(r.track02_md5, sizeof(r.track02_md5), "%s", save->track02_md5);
    r.campaign_layout_epoch = epoch; r.replay_final_record = replay->last_track02_record;
    r.replay_final_raw_sector = replay->last_raw_sector; *out = r; return 1;
}
