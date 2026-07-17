#include "theron_v1_track02_level_object_descriptor_capture_intake.h"

#include <string.h>

int theron_v1_track02_level_object_descriptor_capture_intake_admit(
    const Theron_V1Track02SectorRecordCorpusDiscoveryReceipt *corpus,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    const Theron_V1Track02LaunchTraceIdentityReceipt *trace_identity,
    uint32_t campaign_layout_epoch,
    uint32_t campaign_media_scan_epoch,
    Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt *out)
{
    Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt receipt = {0};

    if (!out) return 0;
    if (!corpus) {
        receipt.status = THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (!media || !plan || !replay || !trace_identity ||
        corpus->status != THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_READY ||
        corpus->direct_candidate_count != 1u ||
        !corpus->direct_regular_files_verified || !corpus->track02_md5_verified ||
        !corpus->trace_md5_verified || !corpus->coalesced_trace_md5[0] ||
        corpus->media.status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        !corpus->media.cue_consumed || !corpus->media.mode1_2352 ||
        !corpus->media.raw_trace_preparation_allowed ||
        corpus->sector_record.status != THERON_V1_TRACK02_SECTOR_RECORD_READY ||
        !corpus->sector_record.raw_cue_bin_identity_consumed ||
        !corpus->sector_record.stage3_directory_consumed ||
        !corpus->sector_record.observed_later_loader_consumed ||
        !corpus->sector_record.nonstartup_record_consumed ||
        corpus->sector_record.level_object_semantics_allowed ||
        corpus->sector_record.bitmap_palette_admission_allowed ||
        corpus->sector_record.pixel_decode_allowed ||
        corpus->sector_record.dungeon_draw_allowed ||
        corpus->sector_record.fallback_visuals_allowed ||
        !campaign_layout_epoch || !campaign_media_scan_epoch ||
        !theron_v1_track02_campaign_media_direct_layout_current(
            media, &corpus->media, plan) ||
        !replay->active || !replay->direct_campaign_layout_consumed ||
        !replay->dynamic_cd_read_records_consumed || !replay->accepted_record_count ||
        replay->campaign_layout_epoch != campaign_layout_epoch ||
        replay->track02_variant != corpus->sector_record.track02_variant ||
        strcmp(replay->track02_md5, corpus->sector_record.track02_md5) ||
        replay->last_track02_record != corpus->sector_record.resolved_track02_record ||
        replay->last_raw_sector != corpus->sector_record.resolved_track02_record ||
        replay->level_object_semantics_allowed ||
        replay->bitmap_palette_admission_allowed || replay->pixel_decode_allowed ||
        replay->dungeon_draw_allowed || replay->fallback_visuals_allowed ||
        !trace_identity->valid || !trace_identity->direct_campaign_consumed ||
        !trace_identity->loader_trace_consumed || !trace_identity->event_log_consumed ||
        trace_identity->campaign_layout_epoch != campaign_layout_epoch ||
        trace_identity->track02_variant != corpus->sector_record.track02_variant ||
        strcmp(trace_identity->track02_md5, corpus->sector_record.track02_md5) ||
        strcmp(trace_identity->source_trace_md5, corpus->coalesced_trace_md5) ||
        trace_identity->final_track02_record != replay->last_track02_record ||
        trace_identity->final_raw_sector != replay->last_raw_sector ||
        trace_identity->level_object_semantics_allowed ||
        trace_identity->bitmap_palette_admission_allowed ||
        trace_identity->pixel_decode_allowed || trace_identity->dungeon_draw_allowed ||
        trace_identity->fallback_visuals_allowed) {
        receipt.status = THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_REJECTED;
        *out = receipt;
        return 1;
    }
    receipt.status = THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_READY;
    receipt.direct_cue_bin_consumed = 1;
    receipt.coalesced_loader_trace_consumed = 1;
    receipt.replay_tail_consumed = 1;
    receipt.opaque_descriptor_only = 1;
    receipt.campaign_layout_epoch = campaign_layout_epoch;
    receipt.campaign_media_scan_epoch = campaign_media_scan_epoch;
    memcpy(receipt.coalesced_trace_md5, corpus->coalesced_trace_md5,
           sizeof(receipt.coalesced_trace_md5));
    receipt.corpus = *corpus;
    *out = receipt;
    return 1;
}
