#include "csb_v1_startup_presentation_receipt_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int failures;

static void check(int condition, const char *message)
{
    ++checks;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", message);
    }
}

static void make_package(CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *r)
{
    memset(r, 0, sizeof(*r));
    r->valid = r->real_package_matched = 1;
    r->c001_title_consumed = r->c001_presents_consumed = 1;
    r->c001_chaos_consumed = r->c001_chaos_zoom_consumed = 1;
    r->c001_chaos_hold_consumed = r->c001_strikes_back_consumed = 1;
    r->c002_left_door_consumed = r->c003_right_door_consumed = 1;
    r->c004_entrance_consumed = r->c005_credits_consumed = 1;
    r->c017_hud_consumed = r->c040_hud_consumed = 1;
    r->title_to_entrance_same_session = r->title_to_hud_same_session = 1;
    r->no_legacy_wrappers = r->no_fallback_routes = 1;
    r->source_tick = 413u;
    r->session_generation = 9u;
    r->real_asset_receipt_hash = UINT64_C(0x1122334455667788);
    r->consumed_surface_hash = UINT64_C(0x8877665544332211);
}

static void make_title(CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34 *r)
{
    memset(r, 0, sizeof(*r));
    r->valid = r->real_package_matched = 1;
    r->presents_consumed = r->chaos_consumed = r->strikes_back_consumed = 1;
    r->c004_c002_c003_consumed = 1;
    r->no_legacy_wrappers = r->no_synthetic_surface = 1;
    r->session_generation = 9u;
    r->real_asset_receipt_hash = UINT64_C(0x1122334455667788);
    r->consumed_surface_hash = UINT64_C(0x8877665544332211);
    r->presents_host_surface_hash = 0x1001u;
    r->chaos_host_surface_hash = 0x1002u;
    r->strikes_host_surface_hash = 0x1003u;
    r->opening_host_surface_hash = 0x1004u;
}

static void make_hud(CSB_V1_StartupSessionHudDoorInputPackageReceipt_PC34 *r)
{
    memset(r, 0, sizeof(*r));
    r->valid = r->real_package_matched = 1;
    r->c017_hud_consumed = r->c040_hud_consumed = 1;
    r->first_live_door_frame = r->first_runtime_input = 1;
    r->no_legacy_wrappers = r->no_synthetic_surface = 1;
    r->session_generation = 9u;
    r->hud_source_tick = 413u;
    r->first_runtime_tick = 414u;
    r->hud_host_surface_hash = 0x1005u;
    r->real_asset_receipt_hash = UINT64_C(0x1122334455667788);
    r->consumed_surface_hash = UINT64_C(0x8877665544332211);
}

static void make_cadence(CSB_V1_StartupPresentationCadenceFacts_PC34 *r)
{
    memset(r, 0, sizeof(*r));
    r->valid = 1;
    r->presents_source_step = CSB_V1_STARTUP_PRESENTATION_PRESENTS_STEP_PC34;
    r->chaos_first_source_step = CSB_V1_STARTUP_PRESENTATION_CHAOS_FIRST_STEP_PC34;
    r->chaos_last_source_step = CSB_V1_STARTUP_PRESENTATION_CHAOS_LAST_STEP_PC34;
    r->chaos_hold_vblanks = CSB_V1_STARTUP_PRESENTATION_CHAOS_HOLD_VBLANKS_PC34;
    r->strikes_source_step = CSB_V1_STARTUP_PRESENTATION_STRIKES_STEP_PC34;
    r->title_total_ticks = CSB_V1_STARTUP_PRESENTATION_TITLE_TOTAL_TICKS_PC34;
    r->entrance_door_steps = CSB_V1_STARTUP_PRESENTATION_ENTRANCE_DOOR_STEPS_PC34;
    r->no_host_timing_padding = r->no_synthetic_palette_program = 1;
}

static void make_swoosh(CSB_V1_SwshSoundInitReceipt_PC34 *init,
                        CSB_V1_SwshSoundPlayReceipt_PC34 *play,
                        CSB_V1_SwshSoundReleaseReceipt_PC34 *release)
{
    memset(init, 0, sizeof(*init));
    memset(play, 0, sizeof(*play));
    memset(release, 0, sizeof(*release));
    init->valid = init->stereo_channels_bound = init->owned_sample_buffer_bound = 1;
    init->source_sample_byte_count = CSB_V1_SWSH_F0908_SOUND_BYTE_COUNT_PC34;
    init->source_sample_period = CSB_V1_SWSH_F0908_SOUND_PERIOD_PC34;
    init->source_sample_hash = 0x5101u;
    init->no_synthetic_sound_data = init->no_legacy_swoosh_wrapper = 1;
    play->valid = play->init_consumed = play->left_channel_started = 1;
    play->right_channel_started = play->control_start_command_sent = 1;
    play->title_not_started_yet = play->no_synthetic_sound_data = 1;
    play->no_legacy_swoosh_wrapper = 1;
    release->valid = release->play_consumed = release->finish_before_stop = 1;
    release->channels_waited = release->owned_sample_buffer_released = 1;
    release->title_may_consume_after_release = 1;
    release->no_synthetic_sound_data = release->no_legacy_swoosh_wrapper = 1;
}

static void make_entrance_audio(CSB_V1_StartupEntranceAudioFacts_PC34 *r)
{
    memset(r, 0, sizeof(*r));
    r->valid = r->entrance_music_started_after_title = 1;
    r->entrance_music_active_through_door_open = r->source_music_route_bound = 1;
    r->no_synthetic_audio = r->no_legacy_audio_wrapper = 1;
    r->source_audio_hash = 0x6101u;
}

int main(void)
{
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 package;
    CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34 title;
    CSB_V1_StartupSessionHudDoorInputPackageReceipt_PC34 hud;
    CSB_V1_StartupPresentationCadenceFacts_PC34 cadence;
    CSB_V1_SwshSoundInitReceipt_PC34 init;
    CSB_V1_SwshSoundPlayReceipt_PC34 play;
    CSB_V1_SwshSoundReleaseReceipt_PC34 release;
    CSB_V1_StartupEntranceAudioFacts_PC34 entrance_audio;
    CSB_V1_StartupPackagePresentationReceipt_PC34 receipt;

    check(CSB_V1_STARTUP_PRESENTATION_TITLE_TOTAL_TICKS_PC34 == 102u &&
              CSB_V1_STARTUP_PRESENTATION_CHAOS_HOLD_VBLANKS_PC34 == 20u,
          "presentation cadence preserves TITLE.C's 60 + 20 + Delay(20) + Delay(2) timeline");

    make_package(&package); make_title(&title); make_hud(&hud);
    make_cadence(&cadence); make_swoosh(&init, &play, &release);
    make_entrance_audio(&entrance_audio);
    check(csb_v1_startup_package_presentation_receipt_from_source_pc34(
              &package, &title, &hud, &cadence, &init, &play, &release,
              &entrance_audio, &receipt),
          "full source-owned C001-C005/C017/C040 presentation is admitted");
    check(receipt.valid && receipt.source_pixels_and_palettes_bound &&
              receipt.source_cadence_bound && receipt.source_audio_bound &&
              receipt.presentation_receipt_hash != 0u,
          "admitted presentation retains pixel palette cadence and audio receipt");

    cadence.chaos_hold_vblanks = 1u;
    check(!csb_v1_startup_package_presentation_receipt_from_source_pc34(
              &package, &title, &hud, &cadence, &init, &play, &release,
              &entrance_audio, &receipt) && !receipt.valid,
          "wrong CHAOS hold cannot use a package frame with altered cadence");
    make_cadence(&cadence);
    package.c005_credits_consumed = 0;
    check(!csb_v1_startup_package_presentation_receipt_from_source_pc34(
              &package, &title, &hud, &cadence, &init, &play, &release,
              &entrance_audio, &receipt),
          "missing real C005 credits blocks complete presentation promotion");
    package.c005_credits_consumed = 1;
    init.no_synthetic_sound_data = 0;
    check(!csb_v1_startup_package_presentation_receipt_from_source_pc34(
              &package, &title, &hud, &cadence, &init, &play, &release,
              &entrance_audio, &receipt),
          "synthetic swoosh data is rejected before C001 presentation");
    init.no_synthetic_sound_data = 1;
    hud.first_runtime_tick++;
    check(!csb_v1_startup_package_presentation_receipt_from_source_pc34(
              &package, &title, &hud, &cadence, &init, &play, &release,
              &entrance_audio, &receipt),
          "stale HUD-to-runtime edge cannot reuse a real panel receipt");

    printf("csb startup presentation receipt: %d/%d checks passed\n",
           checks - failures, checks);
    return failures ? 1 : 0;
}
