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
        state->assetStatus.theronCampaignMediaPlan.targets[i].cd_read_record =
            0x500u + (uint32_t)i;
        state->assetStatus.theronCampaignMediaPlan.targets[i].loader_output_raw_offset =
            0x1000u + i * 0x100u;
        state->assetStatus.theronCampaignMediaPlan.targets[i].loader_output_bytes = 0x20u;
        state->assetStatus.theronCampaignMediaPlan.targets[i].loader_output_checksum =
            0x10101010u + (uint32_t)i;
        state->assetStatus.theronCampaignMediaPlan.targets[i].palette_output_identity =
            0x20202020u + (uint32_t)i;
        state->assetStatus.theronCampaignMediaPlan.targets[i].bitmap_identity =
            0x30303030u + (uint32_t)i;
        state->assetStatus.theronCampaignMediaPlan.targets[i].destination_record =
            0x600u + (uint32_t)i;
        state->assetStatus.theronCampaignMediaPlan.targets[i].destination_offset =
            0x2000u + i * 0x100u;
        state->assetStatus.theronCampaignMediaPlan.targets[i].destination_bytes = 0x80u;
        state->assetStatus.theronCampaignMediaPlan.targets[i].destination_identity =
            0x40404040u + (uint32_t)i;
    }
    state->assetStatus.theronCampaignMediaPlan.cue_track_consumed = 1;
    state->assetStatus.theronCampaignMediaPlan.cd_read_chain_consumed = 1;
    state->assetStatus.theronCampaignMediaPlan.loader_output_consumed = 1;
    state->assetStatus.theronCampaignMediaPlan.palette_output_consumed = 1;
    state->assetStatus.theronCampaignMediaPlan.bitmap_transfer_consumed = 1;
    state->assetStatus.theronCampaignMediaPlan.destination_record_consumed = 1;
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

static int bind_handoff_artifact_corpus_fixture(M12_StartupMenuState *state)
{
    Theron_V1Track02HandoffArtifactCorpusReceipt receipt = {0};
    uint32_t plan_identity;
    size_t i;

    if (!state || !state->theronLaunchTraceIdentityBound) return 0;
    plan_identity = theron_v1_track02_capture_target_plan_identity(
        &state->assetStatus.theronCampaignMediaPlan);
    if (!plan_identity) return 0;
    receipt.status = THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_READY;
    receipt.supplied_candidate_count = receipt.direct_candidate_count = 1u;
    receipt.direct_cue_bin_consumed = receipt.source_trace_md5_verified = 1;
    receipt.capture_target_plan_consumed = receipt.opaque_artifact_consumed = 1;
    receipt.capture_required_only = receipt.no_draw_only = 1;
    receipt.capture_target_plan_identity = plan_identity;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             state->assetStatus.theronCampaignMedia.track02_md5);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s",
             state->theronLaunchTraceIdentity.source_trace_md5);
    receipt.artifact.status = THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY;
    receipt.artifact.bundle_md5_verified = receipt.artifact.mednafen_trace_md5_verified = 1;
    receipt.artifact.complete_route_set_consumed = receipt.artifact.opaque_envelope_verified = 1;
    receipt.artifact.opaque_runtime_ready = 1;
    receipt.artifact.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    receipt.artifact.campaign_route = THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF;
    receipt.artifact.descriptor_selector = 0x30u;
    receipt.artifact.descriptor_ordinal = 1u;
    receipt.artifact.descriptor_source_hash = 0x44556677u;
    receipt.artifact.capture_target_plan_identity = plan_identity;
    snprintf(receipt.artifact.track02_md5, sizeof(receipt.artifact.track02_md5), "%s",
             receipt.track02_md5);
    snprintf(receipt.artifact.bundle_md5, sizeof(receipt.artifact.bundle_md5),
             "cccccccccccccccccccccccccccccccc");
    snprintf(receipt.artifact.mednafen_trace_md5,
             sizeof(receipt.artifact.mednafen_trace_md5), "%s",
             receipt.source_trace_md5);
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        const Theron_V1Track02CaptureTarget *target =
            &state->assetStatus.theronCampaignMediaPlan.targets[i];
        receipt.artifact.cd_read_record[i] = target->cd_read_record;
        receipt.artifact.loader_output_raw_offset[i] = target->loader_output_raw_offset;
        receipt.artifact.loader_output_bytes[i] = target->loader_output_bytes;
        receipt.artifact.loader_output_identity[i] = target->loader_output_checksum;
        receipt.artifact.palette_output_identity[i] = target->palette_output_identity;
        receipt.artifact.bitmap_transfer_identity[i] = target->bitmap_identity;
        receipt.artifact.destination_record[i] = target->destination_record;
        receipt.artifact.destination_offset[i] = target->destination_offset;
        receipt.artifact.destination_bytes[i] = target->destination_bytes;
        receipt.artifact.destination_identity[i] = target->destination_identity;
    }
    return M12_StartupMenu_BindTheronHandoffArtifactCorpus(state, &receipt);
}

int main(void)
{
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt descriptor_intake;
    Theron_V1Track02ExternalCaptureReceipt handoff = {0};
    Theron_V1Track02HandoffArtifactCorpusCandidate corpus_candidate = {0};
    Theron_V1Track02HandoffArtifactCorpusReceipt corpus_import;

    fixture(&state, &intent);
    state.theronSrmCampaignReplayBound = 0;
    intent.theronSrmCampaignReplayBound = 0;
    if (!M12_StartupMenu_ValidateTheronCampaignCaptureRequiredIntent(
            &state, &intent)) return 35;
    state.theronCampaignMediaScanEpoch++;
    if (M12_StartupMenu_ValidateTheronCampaignCaptureRequiredIntent(
            &state, &intent)) return 38;
    intent.theronCampaignMediaScanEpoch = state.theronCampaignMediaScanEpoch;
    if (!M12_StartupMenu_ValidateTheronCampaignCaptureRequiredIntent(
            &state, &intent)) return 39;
    intent.theronLaunchTraceIdentityBound = 1;
    if (M12_StartupMenu_ValidateTheronCampaignCaptureRequiredIntent(
            &state, &intent)) return 36;
    intent.theronLaunchTraceIdentityBound = 0;
    intent.theronCampaignMedia.direct_media.mode1_2352 = 0;
    intent.theronCampaignMedia.direct_media.mode1_2048 = 0;
    if (M12_StartupMenu_ValidateTheronCampaignCaptureRequiredIntent(
            &state, &intent)) return 37;

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

    handoff.status = THERON_V1_TRACK02_EXTERNAL_CAPTURE_RUNTIME_READY;
    handoff.raw_media_intake_verified = 1;
    handoff.mednafen_trace_source_verified = 1;
    handoff.capture_target_plan_verified = 1;
    handoff.positive_handoff_capture_required = 1;
    handoff.capture_target_plan_identity =
        theron_v1_track02_capture_target_plan_identity(
            &state.assetStatus.theronCampaignMediaPlan);
    snprintf(handoff.track02_md5, sizeof(handoff.track02_md5), "%s",
             state.assetStatus.theronCampaignMedia.track02_md5);
    snprintf(handoff.mednafen_trace_source_md5,
             sizeof(handoff.mednafen_trace_source_md5), "%s",
             state.theronLaunchTraceIdentity.source_trace_md5);
    snprintf(handoff.media_path, sizeof(handoff.media_path), "%s",
             "/tmp/firestaff-no-local-theron-track02.cue");
    snprintf(handoff.mednafen_trace_source_path,
             sizeof(handoff.mednafen_trace_source_path), "%s",
             "/tmp/firestaff-no-local-theron-loader-trace.txt");
    corpus_candidate.bundle_path = "/tmp/firestaff-no-local-theron-handoff.bundle";
    corpus_candidate.expected_bundle_md5 =
        "cccccccccccccccccccccccccccccccc";
    if (M12_StartupMenu_ImportTheronHandoffArtifactCorpus(
            &state, &handoff, &corpus_candidate, 1u, &corpus_import) ||
        corpus_import.status != THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_REJECTED ||
        state.theronHandoffArtifactCorpusBound) return 35;
    handoff.mednafen_trace_source_md5[0] = '3';
    if (M12_StartupMenu_ImportTheronHandoffArtifactCorpus(
            &state, &handoff, &corpus_candidate, 1u, &corpus_import) ||
        corpus_import.status != THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_REJECTED ||
        state.theronHandoffArtifactCorpusBound) return 36;
    snprintf(handoff.mednafen_trace_source_md5,
             sizeof(handoff.mednafen_trace_source_md5), "%s",
             state.theronLaunchTraceIdentity.source_trace_md5);
    if (!bind_handoff_artifact_corpus_fixture(&state)) return 28;
    intent.theronHandoffArtifactCorpus = state.theronHandoffArtifactCorpus;
    intent.theronHandoffArtifactCorpusBound = state.theronHandoffArtifactCorpusBound;
    if (!M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 29;
    state.theronHandoffArtifactCorpus.artifact.palette_output_identity[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]++;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 30;
    state.theronHandoffArtifactCorpus = intent.theronHandoffArtifactCorpus;
    state.theronHandoffArtifactCorpus.artifact.campaign_route =
        THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 31;
    state.theronHandoffArtifactCorpus = intent.theronHandoffArtifactCorpus;
    state.theronHandoffArtifactCorpus.artifact.descriptor_selector++;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 32;
    state.theronHandoffArtifactCorpus = intent.theronHandoffArtifactCorpus;
    state.theronHandoffArtifactCorpus.artifact.descriptor_ordinal++;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 33;
    state.theronHandoffArtifactCorpus = intent.theronHandoffArtifactCorpus;
    state.theronHandoffArtifactCorpus.artifact.descriptor_source_hash++;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 34;
    state.theronHandoffArtifactCorpus = intent.theronHandoffArtifactCorpus;
    state.theronLaterRouteCandidateIndex.valid = 1;
    state.theronLaterRouteCandidateIndex.capture_required_only = 1;
    state.theronLaterRouteCandidateIndex.count = 1u;
    state.theronLaterRouteCandidateIndex.entries[0].status =
        THERON_V1_TRACK02_LATER_ROUTE_CAPTURE_REQUIRED;
    state.theronLaterRouteCandidateIndex.entries[0].observed_trace_row_consumed = 1;
    state.theronLaterRouteCandidateIndex.entries[0].direct_media_consumed = 1;
    state.theronLaterRouteCandidateIndex.entries[0].replay_tail_consumed = 1;
    state.theronLaterRouteCandidateIndex.entries[0].opaque_only = 1;
    state.theronLaterRouteCandidateIndex.entries[0].loader_pc = 0x4010u;
    state.theronLaterRouteCandidateIndex.entries[0].record = 0x500u;
    state.theronLaterRouteCandidateIndex.entries[0].raw_sector = 0x500u;
    state.theronLaterRouteCandidateIndex.entries[0].destination_identity = 1u;
    state.theronLaterRouteCandidateIndex.entries[0].campaign_layout_epoch = 7u;
    snprintf(state.theronLaterRouteCandidateIndex.entries[0].track02_md5,
             sizeof(state.theronLaterRouteCandidateIndex.entries[0].track02_md5), "%s",
             state.assetStatus.theronCampaignMedia.track02_md5);
    snprintf(state.theronLaterRouteCandidateIndex.entries[0].source_trace_md5,
             sizeof(state.theronLaterRouteCandidateIndex.entries[0].source_trace_md5), "%s",
             state.theronLaunchTraceIdentity.source_trace_md5);
    if (!M12_StartupMenu_BindTheronLaterRouteCandidateIndex(
            &state, &state.theronLaterRouteCandidateIndex)) return 24;
    intent.theronLaterRouteCandidateIndex = state.theronLaterRouteCandidateIndex;
    intent.theronLaterRouteCandidateIndexBound =
        state.theronLaterRouteCandidateIndexBound;
    if (!M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 25;
    state.theronLaunchTraceIdentity.campaign_layout_epoch++;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 26;
    state.theronLaunchTraceIdentity.campaign_layout_epoch--;
    state.theronLaterRouteCandidateIndex.entries[0].destination_identity++;
    if (M12_StartupMenu_ValidateTheronCampaignLaunchIntent(&state, &intent)) return 27;
    state.theronLaterRouteCandidateIndex = intent.theronLaterRouteCandidateIndex;
    state.theronLaterRouteCandidateIndexBound =
        intent.theronLaterRouteCandidateIndexBound;
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
