#include "csb_v1_startup_presentation_receipt_pc34_compat.h"

#include <string.h>

static uint32_t hash_step(uint32_t hash, uint64_t value)
{
    unsigned int byte_index;
    for (byte_index = 0u; byte_index < 8u; ++byte_index) {
        hash ^= (uint32_t)((value >> (byte_index * 8u)) & 0xffu);
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int swoosh_matches(const CSB_V1_SwshSoundInitReceipt_PC34 *init,
                          const CSB_V1_SwshSoundPlayReceipt_PC34 *play,
                          const CSB_V1_SwshSoundReleaseReceipt_PC34 *release)
{
    return init && play && release && init->valid && play->valid && release->valid &&
        init->source_sample_byte_count == CSB_V1_SWSH_F0908_SOUND_BYTE_COUNT_PC34 &&
        init->source_sample_period == CSB_V1_SWSH_F0908_SOUND_PERIOD_PC34 &&
        init->source_sample_hash != 0u && init->stereo_channels_bound &&
        init->owned_sample_buffer_bound && init->no_synthetic_sound_data &&
        init->no_legacy_swoosh_wrapper && play->init_consumed &&
        play->left_channel_started && play->right_channel_started &&
        play->control_start_command_sent && play->title_not_started_yet &&
        play->no_synthetic_sound_data && play->no_legacy_swoosh_wrapper &&
        release->play_consumed && release->finish_before_stop &&
        release->channels_waited && release->owned_sample_buffer_released &&
        release->title_may_consume_after_release &&
        release->no_synthetic_sound_data && release->no_legacy_swoosh_wrapper;
}

static int cadence_matches(const CSB_V1_StartupPresentationCadenceFacts_PC34 *cadence)
{
    return cadence && cadence->valid &&
        cadence->presents_source_step == CSB_V1_STARTUP_PRESENTATION_PRESENTS_STEP_PC34 &&
        cadence->chaos_first_source_step == CSB_V1_STARTUP_PRESENTATION_CHAOS_FIRST_STEP_PC34 &&
        cadence->chaos_last_source_step == CSB_V1_STARTUP_PRESENTATION_CHAOS_LAST_STEP_PC34 &&
        cadence->chaos_hold_vblanks == CSB_V1_STARTUP_PRESENTATION_CHAOS_HOLD_VBLANKS_PC34 &&
        cadence->strikes_source_step == CSB_V1_STARTUP_PRESENTATION_STRIKES_STEP_PC34 &&
        cadence->title_total_ticks == CSB_V1_STARTUP_PRESENTATION_TITLE_TOTAL_TICKS_PC34 &&
        cadence->entrance_door_steps == CSB_V1_STARTUP_PRESENTATION_ENTRANCE_DOOR_STEPS_PC34 &&
        cadence->no_host_timing_padding && cadence->no_synthetic_palette_program;
}

void csb_v1_startup_package_presentation_receipt_init_pc34(
    CSB_V1_StartupPackagePresentationReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int csb_v1_startup_package_presentation_receipt_from_source_pc34(
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34 *title_opening,
    const CSB_V1_StartupSessionHudDoorInputPackageReceipt_PC34 *hud_runtime,
    const CSB_V1_StartupPresentationCadenceFacts_PC34 *cadence,
    const CSB_V1_SwshSoundInitReceipt_PC34 *swoosh_init,
    const CSB_V1_SwshSoundPlayReceipt_PC34 *swoosh_play,
    const CSB_V1_SwshSoundReleaseReceipt_PC34 *swoosh_release,
    const CSB_V1_StartupEntranceAudioFacts_PC34 *entrance_audio,
    CSB_V1_StartupPackagePresentationReceipt_PC34 *out_receipt)
{
    uint32_t hash = 2166136261u;

    csb_v1_startup_package_presentation_receipt_init_pc34(out_receipt);
    if (!package_receipt || !title_opening || !hud_runtime || !cadence ||
        !entrance_audio || !out_receipt || !package_receipt->valid ||
        !package_receipt->real_package_matched ||
        !package_receipt->c001_title_consumed ||
        !package_receipt->c001_presents_consumed ||
        !package_receipt->c001_chaos_consumed ||
        !package_receipt->c001_chaos_zoom_consumed ||
        !package_receipt->c001_chaos_hold_consumed ||
        !package_receipt->c001_strikes_back_consumed ||
        !package_receipt->c002_left_door_consumed ||
        !package_receipt->c003_right_door_consumed ||
        !package_receipt->c004_entrance_consumed ||
        !package_receipt->c005_credits_consumed ||
        !package_receipt->c017_hud_consumed || !package_receipt->c040_hud_consumed ||
        !package_receipt->title_to_entrance_same_session ||
        !package_receipt->title_to_hud_same_session ||
        !package_receipt->no_legacy_wrappers || !package_receipt->no_fallback_routes ||
        package_receipt->source_tick == 0u || package_receipt->session_generation == 0u ||
        package_receipt->real_asset_receipt_hash == 0u ||
        package_receipt->consumed_surface_hash == 0u || !title_opening->valid ||
        !title_opening->real_package_matched || !title_opening->presents_consumed ||
        !title_opening->chaos_consumed || !title_opening->strikes_back_consumed ||
        !title_opening->c004_c002_c003_consumed ||
        !title_opening->no_legacy_wrappers || !title_opening->no_synthetic_surface ||
        title_opening->session_generation != package_receipt->session_generation ||
        title_opening->real_asset_receipt_hash != package_receipt->real_asset_receipt_hash ||
        title_opening->consumed_surface_hash != package_receipt->consumed_surface_hash ||
        title_opening->presents_host_surface_hash == 0u ||
        title_opening->chaos_host_surface_hash == 0u ||
        title_opening->strikes_host_surface_hash == 0u ||
        title_opening->opening_host_surface_hash == 0u || !hud_runtime->valid ||
        !hud_runtime->real_package_matched || !hud_runtime->c017_hud_consumed ||
        !hud_runtime->c040_hud_consumed || !hud_runtime->first_live_door_frame ||
        !hud_runtime->first_runtime_input || !hud_runtime->no_legacy_wrappers ||
        !hud_runtime->no_synthetic_surface ||
        hud_runtime->session_generation != package_receipt->session_generation ||
        hud_runtime->real_asset_receipt_hash != package_receipt->real_asset_receipt_hash ||
        hud_runtime->consumed_surface_hash != package_receipt->consumed_surface_hash ||
        hud_runtime->hud_source_tick == 0u ||
        hud_runtime->first_runtime_tick != hud_runtime->hud_source_tick + 1u ||
        hud_runtime->hud_host_surface_hash == 0u || !cadence_matches(cadence) ||
        !swoosh_matches(swoosh_init, swoosh_play, swoosh_release) ||
        !entrance_audio->valid || !entrance_audio->entrance_music_started_after_title ||
        !entrance_audio->entrance_music_active_through_door_open ||
        !entrance_audio->source_music_route_bound || !entrance_audio->no_synthetic_audio ||
        !entrance_audio->no_legacy_audio_wrapper || entrance_audio->source_audio_hash == 0u) {
        return 0;
    }

    hash = hash_step(hash, package_receipt->real_asset_receipt_hash);
    hash = hash_step(hash, package_receipt->consumed_surface_hash);
    hash = hash_step(hash, package_receipt->session_generation);
    hash = hash_step(hash, package_receipt->source_tick);
    hash = hash_step(hash, title_opening->presents_host_surface_hash);
    hash = hash_step(hash, title_opening->chaos_host_surface_hash);
    hash = hash_step(hash, title_opening->strikes_host_surface_hash);
    hash = hash_step(hash, title_opening->opening_host_surface_hash);
    hash = hash_step(hash, hud_runtime->hud_host_surface_hash);
    hash = hash_step(hash, swoosh_init->source_sample_hash);
    hash = hash_step(hash, entrance_audio->source_audio_hash);

    out_receipt->valid = out_receipt->real_package_consumed = 1;
    out_receipt->c001_c005_c017_c040_bound = 1;
    out_receipt->source_pixels_and_palettes_bound = 1;
    out_receipt->source_cadence_bound = out_receipt->source_audio_bound = 1;
    out_receipt->title_to_entrance_to_hud_same_session = 1;
    out_receipt->no_legacy_wrappers = out_receipt->no_fallback_routes = 1;
    out_receipt->session_generation = package_receipt->session_generation;
    out_receipt->source_tick = package_receipt->source_tick;
    out_receipt->real_asset_receipt_hash = package_receipt->real_asset_receipt_hash;
    out_receipt->consumed_surface_hash = package_receipt->consumed_surface_hash;
    out_receipt->presentation_receipt_hash = hash;
    out_receipt->source_evidence = csb_v1_startup_package_presentation_receipt_source_evidence_pc34();
    return 1;
}

const char *csb_v1_startup_package_presentation_receipt_source_evidence_pc34(void)
{
    return "ReDMCSB SWSHSND.C F0908-F0910; TITLE.C F0437; "
           "ENTRANCE.C F0438/F0442/F0806/F0807; PANEL.C F0346/F0347; "
           "CSBWin Graphics.cpp OpenPrisonDoors/ExpandGraphic";
}
