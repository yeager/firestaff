#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

static void fixture(M12_StartupMenuState *state, M12_LaunchIntent *intent)
{
    Theron_V1Track02CampaignMediaDiscoveryReceipt *media;
    size_t i;
    memset(state, 0, sizeof(*state));
    memset(intent, 0, sizeof(*intent));
    media = &state->assetStatus.theronCampaignMedia;
    media->status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
    media->source = THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CUE;
    media->candidate_count = 1;
    media->exact_layout_bound = 1;
    media->launchable_direct_media = 1;
    media->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(media->track02_md5, sizeof(media->track02_md5),
             "f23601102138f87c33025877767ebf76");
    snprintf(media->candidate_path, sizeof(media->candidate_path),
             "/original/TQUS-Raw.cue");
    snprintf(media->direct_media.payload_path, sizeof(media->direct_media.payload_path),
             "/original/TQUS02.bin");
    snprintf(media->direct_media.track02_md5, sizeof(media->direct_media.track02_md5),
             "%s", media->track02_md5);
    media->direct_media.status = THERON_V1_TRACK02_MEDIA_INTAKE_READY;
    media->direct_media.cue_consumed = 1;
    media->direct_media.mode1_2352 = 1;
    media->direct_media.raw_trace_preparation_allowed = 1;
    media->direct_media.cue_index01_sector = 150u;
    media->direct_media.payload_bytes = 2352u * 4u;
    media->direct_media.sector_count = 4u;
    media->direct_media.first_user_data_offset = 16u;
    media->direct_media.logical_user_data_window_bytes = 2048u * 4u;
    state->assetStatus.theronCampaignMediaPlan.valid = 1;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        state->assetStatus.theronCampaignMediaPlan.targets[i].route =
            (Theron_V1Track02CaptureTargetRoute)i;
        state->assetStatus.theronCampaignMediaPlan.targets[i].track02_variant =
            media->track02_variant;
        snprintf(state->assetStatus.theronCampaignMediaPlan.targets[i].track02_md5,
                 sizeof(state->assetStatus.theronCampaignMediaPlan.targets[i].track02_md5),
                 "%s", media->track02_md5);
    }
    state->assetStatus.theronCampaignMediaLaunchReady = 1;
    intent->gameId = "theron";
    intent->valid = 1;
    intent->theronCampaignMedia = *media;
    intent->theronCampaignMediaPlan = state->assetStatus.theronCampaignMediaPlan;
    intent->theronCampaignMediaBound = 1;
    state->theronCampaignMediaScanEpoch = 1u;
    intent->theronCampaignMediaScanEpoch = 1u;
    state->theronSrmCampaignReplay.valid = 1;
    state->theronSrmCampaignReplay.opaque_save_consumed = 1;
    state->theronSrmCampaignReplay.direct_campaign_consumed = 1;
    state->theronSrmCampaignReplay.replay_consumed = 1;
    snprintf(state->theronSrmCampaignReplay.srm_md5,
             sizeof(state->theronSrmCampaignReplay.srm_md5),
             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    state->theronSrmCampaignReplay.srm_size = 128u;
    state->theronSrmCampaignReplay.srm_identity_fnv1a = 0x12345678u;
    state->theronSrmCampaignReplay.track02_variant = media->track02_variant;
    snprintf(state->theronSrmCampaignReplay.track02_md5,
             sizeof(state->theronSrmCampaignReplay.track02_md5), "%s", media->track02_md5);
    state->theronSrmCampaignReplay.campaign_layout_epoch = 7u;
    state->theronSrmCampaignReplay.replay_final_record = 0x4e1u;
    state->theronSrmCampaignReplay.replay_final_raw_sector = 0x4e1u;
    state->theronSrmCampaignReplayBound = 1;
    intent->theronSrmCampaignReplay = state->theronSrmCampaignReplay;
    intent->theronSrmCampaignReplayBound = 1;
}

int main(void)
{
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt descriptor_intake;

    fixture(&state, &intent);
    if (!M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 1;

    state.assetStatus.theronCampaignMedia.direct_media.payload_path[0] = '\0';
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 2;

    fixture(&state, &intent);
    state.assetStatus.theronCampaignMedia.virtual_container = 1;
    state.assetStatus.theronCampaignMedia.no_media_extracted = 1;
    state.assetStatus.theronCampaignMedia.launchable_direct_media = 0;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 3;

    fixture(&state, &intent);
    snprintf(state.assetStatus.theronCampaignMedia.track02_md5,
             sizeof(state.assetStatus.theronCampaignMedia.track02_md5), "deadbeef");
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 4;

    fixture(&state, &intent);
    intent.theronCampaignMediaBound = 0;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 5;

    fixture(&state, &intent);
    state.assetStatus.theronCampaignMedia.direct_media.cue_index01_sector++;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 8;

    fixture(&state, &intent);
    state.theronSrmCampaignReplay.srm_identity_fnv1a ^= 1u;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 6;

    fixture(&state, &intent);
    state.theronSrmCampaignReplay.srm_size++;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 7;

    fixture(&state, &intent);
    state.theronSrmLaunchDiscovery.status = THERON_V1_SRM_LAUNCH_DISCOVERY_READY;
    state.theronSrmLaunchDiscovery.direct_candidate_count = 1u;
    state.theronSrmLaunchDiscovery.direct_regular_file_verified = 1;
    state.theronSrmLaunchDiscovery.source_md5_verified = 1;
    state.theronSrmLaunchDiscovery.track02_identity_verified = 1;
    state.theronSrmLaunchDiscovery.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(state.theronSrmLaunchDiscovery.track02_md5,
             sizeof(state.theronSrmLaunchDiscovery.track02_md5),
             "f23601102138f87c33025877767ebf76");
    state.theronSrmLaunchDiscovery.admission.status = THERON_V1_SRM_OPAQUE_READY;
    state.theronSrmLaunchDiscovery.admission.srm_size = 128u;
    snprintf(state.theronSrmLaunchDiscovery.admission.srm_md5,
             sizeof(state.theronSrmLaunchDiscovery.admission.srm_md5),
             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    if (!M12_StartupMenu_BindTheronSrmLaunchDiscovery(&state,
                                                       &state.theronSrmLaunchDiscovery)) return 11;
    intent.theronSrmLaunchDiscovery = state.theronSrmLaunchDiscovery;
    intent.theronSrmLaunchDiscoveryBound = state.theronSrmLaunchDiscoveryBound;
    if (!M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 12;
    state.theronSrmLaunchDiscovery.virtual_candidate_count = 1u;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 13;

    fixture(&state, &intent);
    state.theronCampaignMediaScanEpoch++;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 19;

    fixture(&state, &intent);
    state.theronLaunchTraceIdentityBound = 1;
    state.theronLaunchTraceIdentity.valid = 1;
    state.theronLaunchTraceIdentity.direct_campaign_consumed = 1;
    state.theronLaunchTraceIdentity.loader_trace_consumed = 1;
    state.theronLaunchTraceIdentity.event_log_consumed = 1;
    state.theronLaunchTraceIdentity.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    state.theronLaunchTraceIdentity.campaign_layout_epoch = 7u;
    snprintf(state.theronLaunchTraceIdentity.track02_md5, sizeof(state.theronLaunchTraceIdentity.track02_md5), "f23601102138f87c33025877767ebf76");
    snprintf(state.theronLaunchTraceIdentity.source_trace_md5, sizeof(state.theronLaunchTraceIdentity.source_trace_md5), "11111111111111111111111111111111");
    snprintf(state.theronLaunchTraceIdentity.event_log_md5, sizeof(state.theronLaunchTraceIdentity.event_log_md5), "22222222222222222222222222222222");
    state.theronLaunchTraceIdentity.final_track02_record = 0x510u;
    state.theronLaunchTraceIdentity.final_raw_sector = 0x510u;
    state.theronTraceBundle.status = THERON_V1_TRACK02_TRACE_BUNDLE_READY;
    state.theronTraceBundle.direct_candidate_count = 1u;
    state.theronTraceBundle.opaque_only = 1;
    state.theronTraceBundle.campaign_layout_epoch = 7u;
    state.theronTraceBundle.capture_target_plan_identity =
        theron_v1_track02_capture_target_plan_identity(
            &state.assetStatus.theronCampaignMediaPlan);
    snprintf(state.theronTraceBundle.trace.source_trace_md5, sizeof(state.theronTraceBundle.trace.source_trace_md5), "%s", state.theronLaunchTraceIdentity.source_trace_md5);
    snprintf(state.theronTraceBundle.trace.event_log_md5, sizeof(state.theronTraceBundle.trace.event_log_md5), "%s", state.theronLaunchTraceIdentity.event_log_md5);
    if (!M12_StartupMenu_BindTheronTraceBundle(&state, &state.theronTraceBundle)) return 14;
    intent.theronLaunchTraceIdentity = state.theronLaunchTraceIdentity;
    intent.theronLaunchTraceIdentityBound = state.theronLaunchTraceIdentityBound;
    intent.theronTraceBundle = state.theronTraceBundle;
    intent.theronTraceBundleBound = state.theronTraceBundleBound;
    if (!M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 16;
    state.theronSectorRecordCorpus.status = THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_READY;
    state.theronSectorRecordCorpus.direct_candidate_count = 1u;
    state.theronSectorRecordCorpus.direct_regular_files_verified = 1;
    state.theronSectorRecordCorpus.track02_md5_verified = 1;
    state.theronSectorRecordCorpus.trace_md5_verified = 1;
    snprintf(state.theronSectorRecordCorpus.coalesced_trace_md5,
             sizeof(state.theronSectorRecordCorpus.coalesced_trace_md5), "%s",
             state.theronLaunchTraceIdentity.source_trace_md5);
    state.theronSectorRecordCorpus.media = state.assetStatus.theronCampaignMedia.direct_media;
    state.theronSectorRecordCorpus.sector_record.status = THERON_V1_TRACK02_SECTOR_RECORD_READY;
    state.theronSectorRecordCorpus.sector_record.raw_cue_bin_identity_consumed = 1;
    state.theronSectorRecordCorpus.sector_record.stage3_directory_consumed = 1;
    state.theronSectorRecordCorpus.sector_record.observed_later_loader_consumed = 1;
    state.theronSectorRecordCorpus.sector_record.nonstartup_record_consumed = 1;
    state.theronSectorRecordCorpus.sector_record.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(state.theronSectorRecordCorpus.sector_record.track02_md5,
             sizeof(state.theronSectorRecordCorpus.sector_record.track02_md5), "%s",
             state.assetStatus.theronCampaignMedia.track02_md5);
    state.theronSectorRecordCorpus.sector_record.resolved_track02_record = 0x510u;
    state.theronSectorRecordCorpus.sector_record.record_user_data_hash = 0x11223344u;
    state.theronSectorRecordCorpus.sector_record.observed_raw_sector_checksum = 0x55667788u;
    memset(&descriptor_intake, 0, sizeof(descriptor_intake));
    descriptor_intake.status = THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_READY;
    descriptor_intake.direct_cue_bin_consumed = 1;
    descriptor_intake.coalesced_loader_trace_consumed = 1;
    descriptor_intake.replay_tail_consumed = 1;
    descriptor_intake.opaque_descriptor_only = 1;
    descriptor_intake.campaign_layout_epoch = 7u;
    descriptor_intake.campaign_media_scan_epoch = state.theronCampaignMediaScanEpoch;
    snprintf(descriptor_intake.coalesced_trace_md5,
             sizeof(descriptor_intake.coalesced_trace_md5), "%s",
             state.theronLaunchTraceIdentity.source_trace_md5);
    descriptor_intake.corpus = state.theronSectorRecordCorpus;
    if (
        !M12_StartupMenu_BindTheronLevelObjectDescriptorCaptureIntake(
            &state, &descriptor_intake)) return 20;
    intent.theronSectorRecordCorpus = state.theronSectorRecordCorpus;
    intent.theronSectorRecordCorpusBound = state.theronSectorRecordCorpusBound;
    if (!M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 21;
    state.theronSectorRecordCorpus.coalesced_trace_md5[0] = '3';
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 22;
    state.theronSectorRecordCorpus = intent.theronSectorRecordCorpus;
    state.theronSectorRecordCorpus.sector_record.track02_md5[0] = '0';
    if (M12_StartupMenu_BindTheronSectorRecordCorpusDiscovery(
            &state, &state.theronSectorRecordCorpus) ||
        state.theronSectorRecordCorpusBound) return 23;
    state.assetStatus.theronCampaignMediaPlan.targets[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF].destination_identity++;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 17;
    state.theronTraceBundle.virtual_candidate_count = 1u;
    if (M12_StartupMenu_BindTheronTraceBundle(&state, &state.theronTraceBundle) ||
        state.theronTraceBundleBound) return 15;

    fixture(&state, &intent);
    state.theronLaunchTraceIdentityBound = 1;
    state.theronLaunchTraceIdentity.valid = 1;
    state.theronTraceBundleBound = 1;
    state.theronTraceBundle.status = THERON_V1_TRACK02_TRACE_BUNDLE_READY;
    if (M12_StartupMenu_ScanTheronCampaignMedia(
            &state, "/definitely/not/a/theron-track02.cue",
            THERON_TRACK02_MD5_US_BIN,
            &state.assetStatus.theronCampaignMediaPlan) ||
        state.theronLaunchTraceIdentityBound || state.theronTraceBundleBound ||
        state.theronSectorRecordCorpusBound ||
        state.theronLaunchTraceIdentity.valid ||
        state.theronTraceBundle.status != THERON_V1_TRACK02_TRACE_BUNDLE_UNAVAILABLE)
        return 18;

    puts("test_theron_v1_campaign_launch_intent: PASS");
    return 0;
}
