#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

static void fixture(Theron_V1Track02CaptureCampaignReceipt *campaign,
                    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *window)
{
    memset(campaign, 0, sizeof(*campaign));
    memset(window, 0, sizeof(*window));
    campaign->valid = 1;
    campaign->independent_bundles_verified = 1;
    campaign->shared_track02_provenance_verified = 1;
    campaign->shared_loader_provenance_verified = 1;
    campaign->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(campaign->track02_md5, sizeof(campaign->track02_md5),
             "f23601102138f87c33025877767ebf76");
    snprintf(campaign->mednafen_trace_md5,
             sizeof(campaign->mednafen_trace_md5),
             "11111111111111111111111111111111");
    snprintf(campaign->bundle_md5[THERON_V1_TRACK02_CAPTURE_TARGET_START], 33,
             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    snprintf(campaign->bundle_md5[THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM], 33,
             "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
    snprintf(campaign->bundle_md5[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF], 33,
             "cccccccccccccccccccccccccccccccc");
    campaign->route_destination_identity[
        THERON_V1_TRACK02_CAPTURE_TARGET_START] = 0x44000000u;
    campaign->route_destination_identity[
        THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM] = 0x44000001u;
    campaign->route_destination_identity[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] = 0x7182u;
    window->valid = 1;
    window->opaque_route_ready = 1;
    window->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(window->track02_md5, sizeof(window->track02_md5), "%s",
             campaign->track02_md5);
    window->dungeon_record_window_checksum = 0x7182u;
}

static void direct_media_fixture(
    Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    Theron_V1Track02RawMediaIntakeReceipt *refreshed,
    Theron_V1Track02CaptureTargetPlan *plan)
{
    size_t i;
    memset(media, 0, sizeof(*media));
    memset(plan, 0, sizeof(*plan));
    media->status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
    media->source = THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CUE;
    media->candidate_count = 1;
    media->exact_layout_bound = media->launchable_direct_media = 1;
    media->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(media->track02_md5, sizeof(media->track02_md5),
             "f23601102138f87c33025877767ebf76");
    media->direct_media.status = THERON_V1_TRACK02_MEDIA_INTAKE_READY;
    media->direct_media.cue_consumed = media->direct_media.mode1_2352 = 1;
    media->direct_media.raw_trace_preparation_allowed = 1;
    media->direct_media.variant = media->track02_variant;
    snprintf(media->direct_media.track02_md5,
             sizeof(media->direct_media.track02_md5), "%s", media->track02_md5);
    snprintf(media->direct_media.media_path, sizeof(media->direct_media.media_path),
             "/original/TQUS-Raw.cue");
    snprintf(media->direct_media.payload_path, sizeof(media->direct_media.payload_path),
             "/original/TQUS02.bin");
    media->direct_media.cue_index01_sector = 225u;
    media->direct_media.payload_bytes = 2352u * 0x600u;
    media->direct_media.sector_count = 0x600u;
    media->direct_media.first_user_data_offset = 225u * 2352u + 16u;
    media->direct_media.logical_user_data_window_bytes = 2048u;
    *refreshed = media->direct_media;
    plan->valid = 1;
    plan->cue_track_consumed = plan->cd_read_chain_consumed = 1;
    plan->loader_output_consumed = plan->palette_output_consumed = 1;
    plan->bitmap_transfer_consumed = plan->destination_record_consumed = 1;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        plan->targets[i].route = (Theron_V1Track02CaptureTargetRoute)i;
        plan->targets[i].track02_variant = media->track02_variant;
        snprintf(plan->targets[i].track02_md5,
                 sizeof(plan->targets[i].track02_md5), "%s", media->track02_md5);
        plan->targets[i].cd_read_record = 0x500u + (uint32_t)i;
        plan->targets[i].loader_output_raw_offset = 0x2000u + i * 0x100u;
        plan->targets[i].loader_output_bytes = 0x20u;
        plan->targets[i].loader_output_checksum = 0x11000000u + (uint32_t)i;
        plan->targets[i].palette_output_identity = 0x22000000u + (uint32_t)i;
        plan->targets[i].bitmap_identity = 0x33000000u + (uint32_t)i;
        plan->targets[i].destination_record = 0x600u + (uint32_t)i;
        plan->targets[i].destination_offset = 0x3000u + i * 0x100u;
        plan->targets[i].destination_bytes = 0x80u;
        plan->targets[i].destination_identity = 0x44000000u + (uint32_t)i;
    }
    plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .destination_identity = 0x7182u;
    plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .destination_record = 0x510u;
    plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .destination_offset = 0x1c00u;
    plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .destination_bytes = 0x80u;
    plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .cd_read_record = 0x510u;
    plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .loader_output_raw_offset = 0x2340u;
    plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .loader_output_bytes = 0x20u;
    plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .loader_output_checksum = 0x0d1e2f30u;
    plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .palette_output_identity = 0x1a2b3c4du;
    plan->targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .bitmap_identity = 0x5e6f7081u;
}

static void replay_fixture(Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay)
{
    memset(replay, 0, sizeof(*replay));
    replay->active = replay->direct_campaign_layout_consumed = 1;
    replay->dynamic_cd_read_records_consumed = 1;
    replay->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(replay->track02_md5, sizeof(replay->track02_md5),
             "f23601102138f87c33025877767ebf76");
    replay->campaign_layout_epoch = 7u;
    replay->accepted_record_count = 2u;
    replay->first_track02_record = 0x4e0u;
    replay->last_track02_record = 0x4e1u;
    replay->last_raw_sector = 0x4e1u;
    replay->ordered_record_checksum = 0x12345678u;
}

static void save_fixture(Theron_V1SrmCampaignReplayReceipt *save)
{
    memset(save, 0, sizeof(*save));
    save->valid = save->opaque_save_consumed = save->direct_campaign_consumed = 1;
    save->replay_consumed = 1;
    snprintf(save->srm_md5, sizeof(save->srm_md5),
             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    save->srm_size = 128u;
    save->srm_identity_fnv1a = 0x12345678u;
    save->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(save->track02_md5, sizeof(save->track02_md5),
             "f23601102138f87c33025877767ebf76");
    save->campaign_layout_epoch = 7u;
    save->replay_final_record = 0x4e1u;
    save->replay_final_raw_sector = 0x4e1u;
}

int main(void)
{
    M11_GameViewState state;
    Theron_V1Track02CaptureCampaignReceipt campaign;
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt window;
    Theron_V1Track02SectorRecordAdmissionReceipt sector = {0};
    Theron_V1Track02SectorRecordCorpusDiscoveryReceipt sector_discovery = {0};
    Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt descriptor_intake = {0};
    Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeReceipt presentation_intake = {0};
    Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt artifact = {0};
    Theron_V1Track02DungeonCapturePlanAdmissionReceipt capture_plan = {0};
    Theron_V1Track02LiveLoaderRouteAdmissionReceipt live = {0};
    Theron_V1Track02CampaignMediaDiscoveryReceipt direct_media;
    Theron_V1Track02RawMediaIntakeReceipt refreshed_media;
    Theron_V1Track02CaptureTargetPlan direct_plan;
    Theron_V1Track02LoaderTraceReplayConsistencyReceipt replay;
    Theron_V1SrmCampaignReplayReceipt save;
    Theron_V1SrmLaunchDiscoveryReceipt discovery;

    memset(&state, 0, sizeof(state));
    state.sourceKind = M11_GAME_SOURCE_THERON_TRACK02;
    state.theronState.campaign_media_scan_epoch = 3u;
    state.theronState.startup_media_ready = 1;
    state.theronState.startup_media_track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(state.theronState.startup_media_track02_md5,
             sizeof(state.theronState.startup_media_track02_md5),
             "f23601102138f87c33025877767ebf76");
    fixture(&campaign, &window);
    direct_media_fixture(&direct_media, &refreshed_media, &direct_plan);
    replay_fixture(&replay);
    save_fixture(&save);
    if (!M11_GameView_TheronBindSrmCampaignReplayReceipt(&state, &save)) return 14;
    memset(&discovery, 0, sizeof(discovery));
    discovery.status = THERON_V1_SRM_LAUNCH_DISCOVERY_READY;
    discovery.direct_candidate_count = 1u;
    discovery.direct_regular_file_verified = discovery.source_md5_verified = 1;
    discovery.track02_identity_verified = 1;
    discovery.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    discovery.admission.status = THERON_V1_SRM_OPAQUE_READY;
    discovery.admission.srm_size = 128u;
    snprintf(discovery.admission.srm_md5, sizeof(discovery.admission.srm_md5),
             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    snprintf(discovery.track02_md5, sizeof(discovery.track02_md5),
             "f23601102138f87c33025877767ebf76");
    if (!M11_GameView_TheronBindSrmLaunchDiscoveryReceipt(&state, &discovery)) return 24;
    discovery.virtual_candidate_count = 1u;
    if (M11_GameView_TheronBindSrmLaunchDiscoveryReceipt(&state, &discovery) ||
        state.theronState.srm_launch_discovery_bound) return 25;

    live.valid = live.dynamic_cd_read_ownership_consumed = 1;
    live.huc6280_event_log_consumed = live.manifest_bound = 1;
    live.opaque_runtime_route_ready = 1;
    live.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(live.track02_md5, sizeof(live.track02_md5), "%s",
             state.theronState.startup_media_track02_md5);
    snprintf(live.source_trace_md5, sizeof(live.source_trace_md5),
             "11111111111111111111111111111111");
    snprintf(live.huc6280_event_log_md5, sizeof(live.huc6280_event_log_md5),
             "22222222222222222222222222222222");
    state.theronState.launch_trace_identity_bound = 1;
    state.theronState.launch_trace_identity.valid = 1;
    state.theronState.launch_trace_identity.direct_campaign_consumed = 1;
    state.theronState.launch_trace_identity.loader_trace_consumed = 1;
    state.theronState.launch_trace_identity.event_log_consumed = 1;
    state.theronState.launch_trace_identity.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(state.theronState.launch_trace_identity.track02_md5,
             sizeof(state.theronState.launch_trace_identity.track02_md5), "%s", live.track02_md5);
    state.theronState.launch_trace_identity.campaign_layout_epoch = 7u;
    snprintf(state.theronState.launch_trace_identity.source_trace_md5,
             sizeof(state.theronState.launch_trace_identity.source_trace_md5), "%s", live.source_trace_md5);
    snprintf(state.theronState.launch_trace_identity.event_log_md5,
             sizeof(state.theronState.launch_trace_identity.event_log_md5), "%s", live.huc6280_event_log_md5);
    state.theronState.launch_trace_identity.final_track02_record = replay.last_track02_record;
    state.theronState.launch_trace_identity.final_raw_sector = replay.last_raw_sector;
    state.theronState.trace_bundle_bound = 1;
    state.theronState.trace_bundle.status = THERON_V1_TRACK02_TRACE_BUNDLE_READY;
    state.theronState.trace_bundle.direct_candidate_count = 1u;
    state.theronState.trace_bundle.opaque_only = 1;
    state.theronState.trace_bundle.campaign_layout_epoch = 7u;
    state.theronState.trace_bundle.capture_target_plan_identity =
        theron_v1_track02_capture_target_plan_identity(&direct_plan);
    snprintf(state.theronState.trace_bundle.trace.source_trace_md5,
             sizeof(state.theronState.trace_bundle.trace.source_trace_md5), "%s", live.source_trace_md5);
    snprintf(state.theronState.trace_bundle.trace.event_log_md5,
             sizeof(state.theronState.trace_bundle.trace.event_log_md5), "%s", live.huc6280_event_log_md5);
    if (!M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u) ||
        !state.theronState.live_loader_soul_room_ready ||
        M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_DUNGEON_HANDOFF, 3u) ||
        state.theronState.live_loader_route_admission_valid) return 7;
    if (!M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u) ||
        !M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_DUNGEON_HANDOFF, 2u) ||
        !state.theronState.live_loader_dungeon_ready) return 8;
    if (M11_GameView_TheronBindTrack02FirstDungeonWorldAdmission(
            &state, &sector, &direct_media, &refreshed_media, &direct_plan,
            &replay, 7u, 3u) ||
        state.theronState.first_dungeon_record_world_admission_valid ||
        state.theronState.first_dungeon_level_object_opaque_ready) return 44;

    if (!M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u)) return 27;
    state.theronState.campaign_media_scan_epoch = 4u;
    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_DUNGEON_HANDOFF, 2u) ||
        state.theronState.live_loader_route_admission_valid) return 29;
    state.theronState.campaign_media_scan_epoch = 3u;
    if (!M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u)) return 30;
    direct_plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .destination_identity++;
    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_DUNGEON_HANDOFF, 2u) ||
        state.theronState.live_loader_route_admission_valid ||
        state.theronState.live_loader_soul_room_ready ||
        state.theronState.live_loader_dungeon_ready) return 28;
    direct_plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]
        .destination_identity--;

    state.theronState.trace_bundle.campaign_layout_epoch = 8u;
    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u)) return 26;
    state.theronState.trace_bundle.campaign_layout_epoch = 7u;

    if (!M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u)) return 10;
    snprintf(live.huc6280_event_log_md5, sizeof(live.huc6280_event_log_md5),
             "33333333333333333333333333333333");
    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_DUNGEON_HANDOFF, 2u) ||
        state.theronState.live_loader_route_admission_valid) return 11;
    snprintf(live.huc6280_event_log_md5, sizeof(live.huc6280_event_log_md5),
             "22222222222222222222222222222222");

    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 8u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u) ||
        state.theronState.live_loader_route_admission_valid) return 12;
    replay.active = 0;
    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u) ||
        state.theronState.live_loader_route_admission_valid) return 13;
    replay.active = 1;

    save_fixture(&save);
    save.campaign_layout_epoch++;
    if (!M11_GameView_TheronBindSrmCampaignReplayReceipt(&state, &save) ||
        M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u) ||
        state.theronState.live_loader_route_admission_valid) return 15;
    save_fixture(&save);
    snprintf(save.track02_md5, sizeof(save.track02_md5),
             "00000000000000000000000000000000");
    if (!M11_GameView_TheronBindSrmCampaignReplayReceipt(&state, &save) ||
        M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u)) return 16;
    save_fixture(&save);
    if (!M11_GameView_TheronBindSrmCampaignReplayReceipt(&state, &save)) return 17;
    replay.last_track02_record++;
    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u)) return 18;
    replay.last_track02_record--;
    if (!M11_GameView_TheronBindSrmCampaignReplayReceipt(&state, &save)) return 19;
    state.theronState.srm_campaign_replay.srm_size++;
    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u)) return 20;
    save_fixture(&save);
    if (!M11_GameView_TheronBindSrmCampaignReplayReceipt(&state, &save)) return 21;
    state.theronState.srm_campaign_replay.srm_identity_fnv1a ^= 1u;
    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u)) return 22;
    save_fixture(&save);
    if (!M11_GameView_TheronBindSrmCampaignReplayReceipt(&state, &save)) return 23;

    refreshed_media.sector_count++;
    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u) ||
        state.theronState.live_loader_route_admission_valid ||
        state.theronState.live_loader_soul_room_ready ||
        state.theronState.live_loader_dungeon_ready) return 9;
    refreshed_media = direct_media.direct_media;

    sector.status = THERON_V1_TRACK02_SECTOR_RECORD_READY;
    sector.raw_cue_bin_identity_consumed = 1;
    sector.stage3_directory_consumed = 1;
    sector.observed_later_loader_consumed = 1;
    sector.nonstartup_record_consumed = 1;
    sector.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(sector.track02_md5, sizeof(sector.track02_md5), "%s",
             state.theronState.startup_media_track02_md5);
    sector.stage3_track02_record = 0x4e0u;
    sector.resolved_track02_record = 0x510u;
    sector.descriptor_ordinal = 1u;
    sector.descriptor_selector = 0x30u;
    sector.descriptor_source_hash = 0x44556677u;
    sector.record_user_data_bytes = 2048u;
    sector.record_user_data_hash = 0x11223344u;
    sector.loader_caller_pc = 0x4010u;
    sector.loader_return_pc = 0x4013u;
    sector.loader_caller_opcode = 0x20u;
    sector.loader_caller_target = 0xe009u;
    sector.loader_post_return_pc = 0x4013u;
    sector.loader_post_return_next_pc = 0x4015u;
    sector.loader_record_cl = 0x10u;
    sector.loader_record_dl = 0x05u;
    sector.loader_record_ch = 0x00u;
    sector.loader_sector_count = 1u;
    sector.loader_observed_raw_sector_lba = 0x980u;
    sector.loader_callsite_context_verified = 1;
    sector.loader_destination = 0x3800u;
    sector.loader_destination_span_bytes = 0x20u;
    sector.loader_destination_span_checksum = 0x0d1e2f30u;
    sector.loader_destination_payload_bytes = 0x800u;
    sector.loader_destination_payload_checksum = 0x11223344u;
    sector.observed_raw_sector_checksum = 0x55667788u;

    sector_discovery.status = THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_READY;
    sector_discovery.direct_candidate_count = 1u;
    sector_discovery.direct_regular_files_verified = 1;
    sector_discovery.track02_md5_verified = 1;
    sector_discovery.trace_md5_verified = 1;
    snprintf(sector_discovery.coalesced_trace_md5,
             sizeof(sector_discovery.coalesced_trace_md5),
             "11111111111111111111111111111111");
    sector_discovery.media = direct_media.direct_media;
    sector_discovery.sector_record = sector;
    if (!M11_GameView_TheronBindTrack02SectorRecordCorpusDiscovery(
            &state, &sector_discovery) ||
        !state.theronState.sector_record_admission_valid ||
        !state.theronState.sector_record_dungeon_ready ||
        state.theronState.sector_record_track02_record != 0x510u) return 5;
    replay.last_track02_record = 0x510u;
    replay.last_raw_sector = 0x510u;
    state.theronState.launch_trace_identity.final_track02_record = 0x510u;
    state.theronState.launch_trace_identity.final_raw_sector = 0x510u;
    if (!M11_GameView_TheronTrack02SectorRecordAdmissionCurrentForDirectMedia(
            &state, &sector, &direct_media, &refreshed_media, &direct_plan,
            &replay, 7u, 3u)) return 40;
    replay.last_raw_sector++;
    if (M11_GameView_TheronTrack02SectorRecordAdmissionCurrentForDirectMedia(
            &state, &sector, &direct_media, &refreshed_media, &direct_plan,
            &replay, 7u, 3u) ||
        state.theronState.sector_record_admission_valid ||
        state.theronState.sector_record_dungeon_ready) return 41;
    replay.last_track02_record = 0x510u;
    replay.last_raw_sector = 0x510u;
    state.theronState.launch_trace_identity.final_track02_record = 0x510u;
    state.theronState.launch_trace_identity.final_raw_sector = 0x510u;
    save_fixture(&save);
    save.replay_final_record = 0x510u;
    save.replay_final_raw_sector = 0x510u;
    if (!M11_GameView_TheronBindSrmCampaignReplayReceipt(&state, &save) ||
        !M11_GameView_TheronBindTrack02SectorRecordCorpusDiscovery(
            &state, &sector_discovery) ||
        !M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_SOUL_ROOM, 1u) ||
        !M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_DUNGEON_HANDOFF, 2u) ||
        !M11_GameView_TheronBindTrack02FirstDungeonWorldAdmission(
            &state, &sector, &direct_media, &refreshed_media, &direct_plan,
            &replay, 7u, 3u) ||
        !state.theronState.first_dungeon_record_world_admission_valid ||
        !state.theronState.first_dungeon_level_object_opaque_ready ||
        state.theronState.first_dungeon_resolved_record != 0x510u ||
        state.theronState.first_dungeon_descriptor_selector != 0x30u) return 42;
    if (!theron_v1_track02_level_object_descriptor_capture_intake_admit(
            &sector_discovery, &direct_media, &direct_plan, &replay,
            &state.theronState.launch_trace_identity, 7u, 3u,
            &descriptor_intake) ||
        descriptor_intake.status !=
            THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_READY ||
        !M11_GameView_TheronBindTrack02LevelObjectDescriptorCaptureIntake(
            &state, &descriptor_intake)) return 45;
    artifact.status = THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY;
    artifact.bundle_md5_verified = artifact.mednafen_trace_md5_verified = 1;
    artifact.complete_route_set_consumed = artifact.opaque_envelope_verified =
        artifact.opaque_runtime_ready = 1;
    artifact.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    artifact.campaign_route = THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF;
    artifact.descriptor_selector = 0x30u;
    artifact.descriptor_ordinal = 1u;
    artifact.descriptor_source_hash = 0x44556677u;
    snprintf(artifact.track02_md5, sizeof(artifact.track02_md5), "%s",
             state.theronState.startup_media_track02_md5);
    snprintf(artifact.mednafen_trace_md5, sizeof(artifact.mednafen_trace_md5), "%s",
             state.theronState.launch_trace_identity.source_trace_md5);
    artifact.capture_target_plan_identity =
        theron_v1_track02_capture_target_plan_identity(&direct_plan);
    snprintf(artifact.bundle_md5, sizeof(artifact.bundle_md5),
             "cccccccccccccccccccccccccccccccc");
    for (size_t i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        artifact.cd_read_record[i] = direct_plan.targets[i].cd_read_record ?
            direct_plan.targets[i].cd_read_record : 0x500u + (uint32_t)i;
        artifact.loader_output_raw_offset[i] =
            direct_plan.targets[i].loader_output_raw_offset;
        artifact.loader_output_bytes[i] = direct_plan.targets[i].loader_output_bytes ?
            direct_plan.targets[i].loader_output_bytes : 0x20u;
        artifact.loader_output_identity[i] =
            direct_plan.targets[i].loader_output_checksum ?
            direct_plan.targets[i].loader_output_checksum : 0x10101010u + (uint32_t)i;
        artifact.palette_output_identity[i] =
            direct_plan.targets[i].palette_output_identity ?
            direct_plan.targets[i].palette_output_identity : 0x20202020u + (uint32_t)i;
        artifact.bitmap_transfer_identity[i] = direct_plan.targets[i].bitmap_identity ?
            direct_plan.targets[i].bitmap_identity : 0x30303030u + (uint32_t)i;
        artifact.destination_record[i] = direct_plan.targets[i].destination_record ?
            direct_plan.targets[i].destination_record : 0x600u + (uint32_t)i;
        artifact.destination_offset[i] = direct_plan.targets[i].destination_offset;
        artifact.destination_bytes[i] = direct_plan.targets[i].destination_bytes ?
            direct_plan.targets[i].destination_bytes : 0x40u;
        artifact.destination_identity[i] = direct_plan.targets[i].destination_identity ?
            direct_plan.targets[i].destination_identity : 0x40404040u + (uint32_t)i;
    }
    if (!theron_v1_track02_descriptor_bitmap_palette_capture_intake_admit(
            &descriptor_intake, &direct_media, &direct_plan, &replay,
            &state.theronState.launch_trace_identity, &artifact, 7u, 3u,
            &presentation_intake) ||
        presentation_intake.status !=
            THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_READY ||
        !M11_GameView_TheronBindTrack02DescriptorBitmapPaletteCaptureIntake(
            &state, &presentation_intake) ||
        !M11_GameView_TheronTrack02DescriptorBitmapPalettePresentationNoDrawCurrent(
            &state) ||
        !state.theronState.first_dungeon_bitmap_palette_presentation_no_draw) return 46;
    capture_plan.status = THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_RESUME_READY;
    capture_plan.direct_cue_bin_consumed = capture_plan.system_card_required = 1;
    capture_plan.replay_tail_consumed = capture_plan.capture_plan_consumed = 1;
    capture_plan.opaque_artifact_required = capture_plan.resume_route_ready = 1;
    capture_plan.presentation_no_draw = 1;
    capture_plan.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(capture_plan.track02_md5, sizeof(capture_plan.track02_md5), "%s",
             state.theronState.startup_media_track02_md5);
    snprintf(capture_plan.source_trace_md5, sizeof(capture_plan.source_trace_md5), "%s",
             state.theronState.launch_trace_identity.source_trace_md5);
    capture_plan.campaign_layout_epoch = 7u;
    capture_plan.campaign_media_scan_epoch = 3u;
    capture_plan.replay_final_record = 0x510u;
    capture_plan.capture_target_plan_identity =
        presentation_intake.capture_target_plan_identity;
    state.theronState.handoff_artifact_corpus_bound = 1;
    state.theronState.handoff_artifact_corpus_scan_epoch =
        state.theronState.campaign_media_scan_epoch;
    state.theronState.handoff_artifact_corpus.status =
        THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_READY;
    state.theronState.handoff_artifact_corpus.supplied_candidate_count = 1u;
    state.theronState.handoff_artifact_corpus.direct_candidate_count = 1u;
    state.theronState.handoff_artifact_corpus.direct_cue_bin_consumed = 1;
    state.theronState.handoff_artifact_corpus.source_trace_md5_verified = 1;
    state.theronState.handoff_artifact_corpus.capture_target_plan_consumed = 1;
    state.theronState.handoff_artifact_corpus.opaque_artifact_consumed = 1;
    state.theronState.handoff_artifact_corpus.capture_required_only = 1;
    state.theronState.handoff_artifact_corpus.no_draw_only = 1;
    state.theronState.handoff_artifact_corpus.capture_target_plan_identity =
        capture_plan.capture_target_plan_identity;
    snprintf(state.theronState.handoff_artifact_corpus.track02_md5,
             sizeof(state.theronState.handoff_artifact_corpus.track02_md5), "%s",
             capture_plan.track02_md5);
    snprintf(state.theronState.handoff_artifact_corpus.source_trace_md5,
             sizeof(state.theronState.handoff_artifact_corpus.source_trace_md5), "%s",
             state.theronState.launch_trace_identity.source_trace_md5);
    state.theronState.handoff_artifact_corpus.artifact = artifact;
    if (!M11_GameView_TheronBindTrack02HandoffLoaderCapture(
            &state, &state.theronState.handoff_artifact_corpus,
            &sector_discovery, &direct_plan,
            &state.theronState.launch_trace_identity, 3u) ||
        !state.theronState.handoff_loader_capture_bound ||
        state.theronState.handoff_loader_capture_record != 0x510u ||
        state.theronState.handoff_loader_capture_post_return_pc != 0x4013u ||
        state.theronState.handoff_loader_capture_record_dl != 0x05u ||
        state.theronState.handoff_loader_capture_post_return_next_pc != 0x4015u ||
        state.theronState.handoff_loader_capture_span_bytes != 0x20u ||
        state.theronState.handoff_loader_capture_span_checksum != 0x0d1e2f30u)
        return 61;
    sector_discovery.sector_record.loader_destination_span_checksum++;
    if (M11_GameView_TheronBindTrack02HandoffLoaderCapture(
            &state, &state.theronState.handoff_artifact_corpus,
            &sector_discovery, &direct_plan,
            &state.theronState.launch_trace_identity, 3u) ||
        state.theronState.handoff_loader_capture_bound) return 62;
    sector_discovery.sector_record.loader_destination_span_checksum--;
    if (!M11_GameView_TheronBindTrack02HandoffLoaderCapture(
            &state, &state.theronState.handoff_artifact_corpus,
            &sector_discovery, &direct_plan,
            &state.theronState.launch_trace_identity, 3u)) return 63;
    sector_discovery.sector_record.loader_post_return_pc++;
    if (M11_GameView_TheronBindTrack02HandoffLoaderCapture(
            &state, &state.theronState.handoff_artifact_corpus,
            &sector_discovery, &direct_plan,
            &state.theronState.launch_trace_identity, 3u) ||
        state.theronState.handoff_loader_capture_bound) return 64;
    sector_discovery.sector_record.loader_post_return_pc--;
    if (!M11_GameView_TheronBindTrack02HandoffLoaderCapture(
            &state, &state.theronState.handoff_artifact_corpus,
            &sector_discovery, &direct_plan,
            &state.theronState.launch_trace_identity, 3u)) return 65;
    sector_discovery.sector_record.loader_record_dl++;
    if (M11_GameView_TheronBindTrack02HandoffLoaderCapture(
            &state, &state.theronState.handoff_artifact_corpus,
            &sector_discovery, &direct_plan,
            &state.theronState.launch_trace_identity, 3u) ||
        state.theronState.handoff_loader_capture_bound) return 66;
    sector_discovery.sector_record.loader_record_dl--;
    if (!M11_GameView_TheronBindTrack02HandoffLoaderCapture(
            &state, &state.theronState.handoff_artifact_corpus,
            &sector_discovery, &direct_plan,
            &state.theronState.launch_trace_identity, 3u)) return 67;
    sector_discovery.sector_record.loader_post_return_next_pc++;
    if (M11_GameView_TheronBindTrack02HandoffLoaderCapture(
            &state, &state.theronState.handoff_artifact_corpus,
            &sector_discovery, &direct_plan,
            &state.theronState.launch_trace_identity, 3u) ||
        state.theronState.handoff_loader_capture_bound) return 68;
    sector_discovery.sector_record.loader_post_return_next_pc--;
    if (!M11_GameView_TheronBindTrack02HandoffLoaderCapture(
            &state, &state.theronState.handoff_artifact_corpus,
            &sector_discovery, &direct_plan,
            &state.theronState.launch_trace_identity, 3u)) return 69;
    if (!M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        !state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 48;
    state.theronState.handoff_artifact_corpus_scan_epoch++;
    if (M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 50;
    state.theronState.handoff_artifact_corpus_scan_epoch =
        state.theronState.campaign_media_scan_epoch;
    if (!M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        !state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 51;
    state.theronState.handoff_artifact_corpus.artifact.campaign_route =
        THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM;
    if (M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 57;
    state.theronState.handoff_artifact_corpus.artifact.campaign_route =
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF;
    state.theronState.handoff_artifact_corpus.artifact.descriptor_selector++;
    if (M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 58;
    state.theronState.handoff_artifact_corpus.artifact.descriptor_selector--;
    state.theronState.handoff_artifact_corpus.artifact.descriptor_ordinal++;
    if (M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 59;
    state.theronState.handoff_artifact_corpus.artifact.descriptor_ordinal--;
    state.theronState.handoff_artifact_corpus.artifact.descriptor_source_hash++;
    if (M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 60;
    state.theronState.handoff_artifact_corpus.artifact.descriptor_source_hash--;
    state.theronState.handoff_artifact_corpus.artifact.cd_read_record[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]++;
    if (M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 54;
    state.theronState.handoff_artifact_corpus.artifact.cd_read_record[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]--;
    state.theronState.handoff_artifact_corpus.artifact.loader_output_identity[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]++;
    if (M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 53;
    state.theronState.handoff_artifact_corpus.artifact.loader_output_identity[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]--;
    state.theronState.handoff_artifact_corpus.artifact.loader_output_bytes[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]++;
    if (M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 55;
    state.theronState.handoff_artifact_corpus.artifact.loader_output_bytes[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]--;
    state.theronState.handoff_artifact_corpus.artifact.destination_bytes[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]++;
    if (M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 56;
    state.theronState.handoff_artifact_corpus.artifact.destination_bytes[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]--;
    state.theronState.handoff_artifact_corpus.artifact.bitmap_transfer_identity[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]++;
    if (M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_required) return 52;
    state.theronState.handoff_artifact_corpus.artifact.bitmap_transfer_identity[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]--;
    capture_plan.status = THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_CAPTURE_REQUIRED;
    capture_plan.resume_route_ready = 0;
    if (!M11_GameView_TheronBindTrack02DungeonCapturePlan(&state, &capture_plan) ||
        !state.theronState.dungeon_capture_required ||
        state.theronState.dungeon_capture_resume_ready) return 49;
    artifact.bitmap_transfer_identity[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]++;
    if (!theron_v1_track02_descriptor_bitmap_palette_capture_intake_admit(
            &descriptor_intake, &direct_media, &direct_plan, &replay,
            &state.theronState.launch_trace_identity, &artifact, 7u, 3u,
            &presentation_intake) ||
        presentation_intake.status !=
            THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_REJECTED ||
        M11_GameView_TheronBindTrack02DescriptorBitmapPaletteCaptureIntake(
            &state, &presentation_intake) ||
        M11_GameView_TheronTrack02DescriptorBitmapPalettePresentationNoDrawCurrent(
            &state)) return 47;
    state.theronState.sector_record_corpus.coalesced_trace_md5[0] = '3';
    if (M11_GameView_TheronBindTrack02LiveLoaderRouteAdmission(
            &state, &live, &direct_media, &refreshed_media, &direct_plan, &replay, 7u, 3u,
            THERON_V1_TRACK02_LIVE_ROUTE_DUNGEON_HANDOFF, 2u) ||
        state.theronState.live_loader_route_admission_valid ||
        state.theronState.first_dungeon_record_world_admission_valid ||
        state.theronState.first_dungeon_level_object_opaque_ready) return 43;
    replay.last_track02_record = 0x4e1u;
    replay.last_raw_sector = 0x4e1u;
    sector.dungeon_draw_allowed = 1;
    if (M11_GameView_TheronBindTrack02SectorRecordAdmission(&state, &sector) ||
        state.theronState.sector_record_admission_valid ||
        state.theronState.sector_record_dungeon_ready) return 6;
    sector.dungeon_draw_allowed = 0;

    if (!M11_GameView_TheronBindTrack02CaptureCampaignAdmission(
            &state, &campaign, &window) ||
        !state.theronState.capture_campaign_admission_valid ||
        !state.theronState.capture_campaign_startup_ready ||
        !state.theronState.capture_campaign_soul_room_ready ||
        !state.theronState.capture_campaign_dungeon_ready ||
        strcmp(state.theronState.capture_campaign_track02_md5,
               campaign.track02_md5) ||
        !M11_GameView_TheronTrack02CaptureCampaignAdmissionCurrent(
            &state, &campaign, &window) ||
        !M11_GameView_TheronTrack02CaptureCampaignAdmissionCurrentForDirectMedia(
            &state, &campaign, &window, &direct_media, &refreshed_media,
            &direct_plan, 3u)) return 1;

    direct_plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM]
        .destination_identity++;
    if (M11_GameView_TheronTrack02CaptureCampaignAdmissionCurrentForDirectMedia(
            &state, &campaign, &window, &direct_media, &refreshed_media,
            &direct_plan, 3u) ||
        state.theronState.capture_campaign_admission_valid ||
        state.theronState.capture_campaign_soul_room_ready) return 36;
    direct_plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM]
        .destination_identity--;
    if (!M11_GameView_TheronBindTrack02CaptureCampaignAdmission(
            &state, &campaign, &window)) return 37;
    if (M11_GameView_TheronTrack02CaptureCampaignAdmissionCurrentForDirectMedia(
            &state, &campaign, &window, &direct_media, &refreshed_media,
            &direct_plan, 4u) ||
        state.theronState.capture_campaign_admission_valid ||
        state.theronState.capture_campaign_dungeon_ready) return 38;
    if (!M11_GameView_TheronBindTrack02CaptureCampaignAdmission(
            &state, &campaign, &window)) return 39;

    snprintf(campaign.bundle_md5[THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM], 33,
             "dddddddddddddddddddddddddddddddd");
    if (M11_GameView_TheronTrack02CaptureCampaignAdmissionCurrent(
            &state, &campaign, &window) ||
        state.theronState.capture_campaign_admission_valid ||
        state.theronState.capture_campaign_soul_room_ready) return 31;
    fixture(&campaign, &window);
    if (!M11_GameView_TheronBindTrack02CaptureCampaignAdmission(
            &state, &campaign, &window)) return 32;
    snprintf(campaign.mednafen_trace_md5,
             sizeof(campaign.mednafen_trace_md5),
             "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");
    if (M11_GameView_TheronTrack02CaptureCampaignAdmissionCurrent(
            &state, &campaign, &window) ||
        state.theronState.capture_campaign_admission_valid ||
        state.theronState.capture_campaign_startup_ready) return 34;
    fixture(&campaign, &window);
    if (!M11_GameView_TheronBindTrack02CaptureCampaignAdmission(
            &state, &campaign, &window)) return 35;
    window.dungeon_record_window_checksum++;
    if (M11_GameView_TheronTrack02CaptureCampaignAdmissionCurrent(
            &state, &campaign, &window) ||
        state.theronState.capture_campaign_admission_valid ||
        state.theronState.capture_campaign_dungeon_ready) return 33;
    fixture(&campaign, &window);

    snprintf(state.theronState.startup_media_track02_md5,
             sizeof(state.theronState.startup_media_track02_md5), "deadbeef");
    if (M11_GameView_TheronBindTrack02CaptureCampaignAdmission(
            &state, &campaign, &window) ||
        state.theronState.capture_campaign_admission_valid ||
        state.theronState.capture_campaign_startup_ready ||
        state.theronState.capture_campaign_soul_room_ready ||
        state.theronState.capture_campaign_dungeon_ready ||
        state.theronState.capture_campaign_track02_md5[0]) return 2;

    snprintf(state.theronState.startup_media_track02_md5,
             sizeof(state.theronState.startup_media_track02_md5), "%s",
             campaign.track02_md5);
    campaign.route_destination_identity[
        THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] ^= 1u;
    if (M11_GameView_TheronBindTrack02CaptureCampaignAdmission(
            &state, &campaign, &window) ||
        state.theronState.capture_campaign_admission_valid) return 3;

    fixture(&campaign, &window);
    state.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    if (M11_GameView_TheronBindTrack02CaptureCampaignAdmission(
            &state, &campaign, &window) ||
        state.theronState.capture_campaign_admission_valid) return 4;

    puts("test_m11_theron_track02_capture_campaign_host: PASS");
    return 0;
}
