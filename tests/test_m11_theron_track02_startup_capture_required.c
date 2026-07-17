#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

static void fixture(Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
                    Theron_V1Track02CaptureTargetPlan *plan,
                    Theron_Track02Variant variant,
                    const char *md5,
                    int cue_consumed)
{
    size_t i;

    memset(media, 0, sizeof(*media));
    memset(plan, 0, sizeof(*plan));
    media->status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
    media->source = cue_consumed ? THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CUE
                                 : THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_LOOSE;
    media->candidate_count = 1;
    media->exact_layout_bound = media->launchable_direct_media = 1;
    media->track02_variant = variant;
    snprintf(media->track02_md5, sizeof(media->track02_md5), "%s", md5);
    media->direct_media.status = THERON_V1_TRACK02_MEDIA_INTAKE_READY;
    media->direct_media.cue_consumed = cue_consumed;
    media->direct_media.mode1_2352 =
        variant == THERON_TRACK02_VARIANT_US_BIN ||
        variant == THERON_TRACK02_VARIANT_JP_BIN;
    media->direct_media.mode1_2048 = !media->direct_media.mode1_2352;
    media->direct_media.variant = variant;
    snprintf(media->direct_media.track02_md5,
             sizeof(media->direct_media.track02_md5), "%s", md5);
    snprintf(media->direct_media.payload_path,
             sizeof(media->direct_media.payload_path), "/original/track02.media");
    media->direct_media.payload_bytes = media->direct_media.mode1_2352
        ? 2352u * 226u : 2048u;
    media->direct_media.sector_count = media->direct_media.mode1_2352 ? 226u : 1u;
    media->direct_media.logical_user_data_window_bytes = 2048u;
    plan->valid = 1;
    plan->cue_track_consumed = plan->cd_read_chain_consumed = 1;
    plan->loader_output_consumed = plan->palette_output_consumed = 1;
    plan->bitmap_transfer_consumed = plan->destination_record_consumed = 1;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        Theron_V1Track02CaptureTarget *target = &plan->targets[i];
        target->route = (Theron_V1Track02CaptureTargetRoute)i;
        target->track02_variant = variant;
        snprintf(target->track02_md5, sizeof(target->track02_md5), "%s", md5);
        target->cd_read_record = 0x500u + (uint32_t)i;
        target->loader_output_raw_offset = 0x1000u + i * 0x100u;
        target->loader_output_bytes = 0x20u;
        target->loader_output_checksum = 0x11000000u + (uint32_t)i;
        target->palette_output_identity = 0x22000000u + (uint32_t)i;
        target->bitmap_transfer_capture_required = 1;
        target->bitmap_raw_offset = 0x3000u + i * 0x100u;
        target->bitmap_bytes = 0x80u;
        target->bitmap_identity = 0x33000000u + (uint32_t)i;
        target->destination_record = 0x600u + (uint32_t)i;
        target->destination_offset = 0x4000u + i * 0x100u;
        target->destination_bytes = 0x80u;
        target->destination_identity = 0x44000000u + (uint32_t)i;
    }
}

static int check_layout(Theron_Track02Variant variant, const char *md5,
                        int cue_consumed)
{
    M11_GameViewState state;
    Theron_V1Track02CampaignMediaDiscoveryReceipt media;
    Theron_V1Track02CaptureTargetPlan plan;
    uint32_t identity;

    fixture(&media, &plan, variant, md5, cue_consumed);
    identity = theron_v1_track02_capture_target_plan_identity(&plan);
    M11_GameView_Init(&state);
    if (!M11_GameView_TheronBindTrack02StartupCaptureRequired(
            &state, &media, &plan, 7u) || !state.active ||
        state.sourceKind != M11_GAME_SOURCE_THERON_TRACK02 ||
        !state.theronState.dungeon_capture_required ||
        state.theronState.dungeon_capture_resume_ready ||
        state.theronState.dungeon_capture_plan_identity != identity ||
        state.theronState.campaign_media_scan_epoch != 7u ||
        !state.theronState.startup_media_ready ||
        state.theronState.startup_media_track02_variant != (int)variant ||
        strcmp(state.theronState.startup_media_track02_md5, md5) ||
        strcmp(state.lastOutcome, "TRACK02 CAPTURE REQUIRED")) {
        M11_GameView_Shutdown(&state);
        return 0;
    }
    snprintf(media.direct_media.media_path,
             sizeof(media.direct_media.media_path), "/original/TQUS02.iso");
    if (!M11_GameView_TheronBindTrack02StartupCaptureRequired(
            &state, &media, &plan, 8u) ||
        state.theronState.campaign_media_scan_epoch != 8u ||
        !state.theronState.startup_capture_media_bound) {
        M11_GameView_Shutdown(&state);
        return 0;
    }
    plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_START].bitmap_identity++;
    if (M11_GameView_TheronBindTrack02StartupCaptureRequired(
            &state, &media, &plan, 8u)) {
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (state.theronState.dungeon_capture_required ||
        state.theronState.startup_capture_media_bound) {
        M11_GameView_Shutdown(&state);
        return 0;
    }
    M11_GameView_Shutdown(&state);
    return 1;
}

int main(void)
{
    if (!check_layout(THERON_TRACK02_VARIANT_US_BIN,
                      THERON_TRACK02_MD5_US_BIN, 0)) return 1;
    if (!check_layout(THERON_TRACK02_VARIANT_US_BIN,
                      THERON_TRACK02_MD5_US_BIN, 1)) return 2;
    if (!check_layout(THERON_TRACK02_VARIANT_US_ISO,
                      THERON_TRACK02_MD5_US_ISO, 0)) return 3;
    if (!check_layout(THERON_TRACK02_VARIANT_JP_REV1_ISO,
                      THERON_TRACK02_MD5_JP_REV1_ISO, 1)) return 4;
    puts("test_m11_theron_track02_startup_capture_required: PASS");
    return 0;
}
