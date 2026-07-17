#include "theron_v1_track02_descriptor_bitmap_palette_capture_intake.h"

#include <stdio.h>
#include <string.h>

int theron_v1_track02_descriptor_bitmap_palette_capture_intake_admit(
    const Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt *descriptor,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    const Theron_V1Track02LaunchTraceIdentityReceipt *trace_identity,
    const Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *artifact,
    uint32_t campaign_layout_epoch,
    uint32_t campaign_media_scan_epoch,
    Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeReceipt *out)
{
    Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeReceipt receipt = {0};
    const Theron_V1Track02CaptureTarget *target;
    uint32_t plan_identity;

    if (!out) return 0;
    if (!descriptor || !artifact) {
        receipt.status = THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    plan_identity = theron_v1_track02_capture_target_plan_identity(plan);
    target = plan ? &plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] : NULL;
    if (!media || !plan || !target || !replay || !trace_identity ||
        descriptor->status != THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_READY ||
        !descriptor->direct_cue_bin_consumed ||
        !descriptor->coalesced_loader_trace_consumed || !descriptor->replay_tail_consumed ||
        !descriptor->opaque_descriptor_only ||
        descriptor->campaign_layout_epoch != campaign_layout_epoch ||
        descriptor->campaign_media_scan_epoch != campaign_media_scan_epoch ||
        !campaign_layout_epoch || !campaign_media_scan_epoch || !plan_identity ||
        !theron_v1_track02_campaign_media_direct_layout_current(
            media, &descriptor->corpus.media, plan) ||
        !replay->active || !replay->direct_campaign_layout_consumed ||
        !replay->dynamic_cd_read_records_consumed || !replay->accepted_record_count ||
        replay->campaign_layout_epoch != campaign_layout_epoch ||
        replay->track02_variant != descriptor->corpus.sector_record.track02_variant ||
        strcmp(replay->track02_md5, descriptor->corpus.sector_record.track02_md5) ||
        replay->last_track02_record != descriptor->corpus.sector_record.resolved_track02_record ||
        replay->last_raw_sector != descriptor->corpus.sector_record.resolved_track02_record ||
        !trace_identity->valid || !trace_identity->direct_campaign_consumed ||
        !trace_identity->loader_trace_consumed || !trace_identity->event_log_consumed ||
        trace_identity->campaign_layout_epoch != campaign_layout_epoch ||
        trace_identity->track02_variant != descriptor->corpus.sector_record.track02_variant ||
        strcmp(trace_identity->track02_md5, descriptor->corpus.sector_record.track02_md5) ||
        strcmp(trace_identity->source_trace_md5, descriptor->coalesced_trace_md5) ||
        trace_identity->final_track02_record != replay->last_track02_record ||
        trace_identity->final_raw_sector != replay->last_raw_sector ||
        artifact->status != THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY ||
        !artifact->bundle_md5_verified || !artifact->mednafen_trace_md5_verified ||
        !artifact->complete_route_set_consumed || !artifact->opaque_envelope_verified ||
        !artifact->opaque_runtime_ready ||
        artifact->capture_target_plan_identity != plan_identity ||
        artifact->campaign_route != THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF ||
        artifact->descriptor_selector !=
            descriptor->corpus.sector_record.descriptor_selector ||
        artifact->descriptor_ordinal !=
            descriptor->corpus.sector_record.descriptor_ordinal ||
        artifact->descriptor_source_hash !=
            descriptor->corpus.sector_record.descriptor_source_hash ||
        artifact->track02_variant != descriptor->corpus.sector_record.track02_variant ||
        strcmp(artifact->track02_md5, descriptor->corpus.sector_record.track02_md5) ||
        strcmp(artifact->mednafen_trace_md5, descriptor->coalesced_trace_md5) ||
        target->route != THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF ||
        target->track02_variant != descriptor->corpus.sector_record.track02_variant ||
        strcmp(target->track02_md5, descriptor->corpus.sector_record.track02_md5) ||
        !target->cd_read_record || !target->loader_output_checksum ||
        !target->palette_output_identity ||
        !target->bitmap_identity || !target->destination_bytes ||
        target->destination_record != descriptor->corpus.sector_record.resolved_track02_record ||
        artifact->cd_read_record[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] !=
            target->cd_read_record ||
        artifact->loader_output_raw_offset[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] !=
            target->loader_output_raw_offset ||
        artifact->loader_output_bytes[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] !=
            target->loader_output_bytes ||
        artifact->loader_output_identity[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] !=
            target->loader_output_checksum ||
        artifact->palette_output_identity[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] !=
            target->palette_output_identity ||
        artifact->bitmap_transfer_identity[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] !=
            target->bitmap_identity ||
        artifact->destination_record[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] !=
            descriptor->corpus.sector_record.resolved_track02_record ||
        artifact->destination_offset[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] !=
            target->destination_offset ||
        artifact->destination_bytes[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] !=
            target->destination_bytes ||
        artifact->destination_identity[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] !=
            target->destination_identity ||
        artifact->pixel_decode_allowed || artifact->level_object_semantics_allowed ||
        artifact->render_allowed || artifact->fallback_visuals_allowed) {
        receipt.status = THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_REJECTED;
        *out = receipt;
        return 1;
    }
    receipt.status = THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_READY;
    receipt.direct_cue_bin_consumed = 1;
    receipt.coalesced_loader_trace_consumed = 1;
    receipt.replay_tail_consumed = 1;
    receipt.descriptor_chain_consumed = 1;
    receipt.palette_output_consumed = 1;
    receipt.bitmap_transfer_consumed = 1;
    receipt.opaque_presentation_only = 1;
    receipt.track02_variant = target->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", target->track02_md5);
    snprintf(receipt.coalesced_trace_md5, sizeof(receipt.coalesced_trace_md5), "%s",
             descriptor->coalesced_trace_md5);
    receipt.campaign_layout_epoch = campaign_layout_epoch;
    receipt.campaign_media_scan_epoch = campaign_media_scan_epoch;
    receipt.capture_target_plan_identity = plan_identity;
    receipt.cd_read_record = target->cd_read_record;
    receipt.descriptor_selector =
        descriptor->corpus.sector_record.descriptor_selector;
    receipt.descriptor_ordinal =
        descriptor->corpus.sector_record.descriptor_ordinal;
    receipt.descriptor_source_hash =
        descriptor->corpus.sector_record.descriptor_source_hash;
    receipt.descriptor_record = descriptor->corpus.sector_record.resolved_track02_record;
    receipt.loader_output_raw_offset = target->loader_output_raw_offset;
    receipt.loader_output_bytes = target->loader_output_bytes;
    receipt.loader_output_identity = target->loader_output_checksum;
    receipt.palette_output_identity = target->palette_output_identity;
    receipt.bitmap_transfer_identity = target->bitmap_identity;
    receipt.destination_offset = target->destination_offset;
    receipt.destination_bytes = target->destination_bytes;
    receipt.destination_identity = target->destination_identity;
    *out = receipt;
    return 1;
}
