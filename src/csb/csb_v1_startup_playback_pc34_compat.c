#include "csb_v1_boot.h"

#include <string.h>

static void csb_v1_startup_playback_clear_action_pc34(
    CSB_V1_StartupAudioAction_PC34 *out_audio_action)
{
    if (out_audio_action) {
        *out_audio_action = CSB_V1_STARTUP_AUDIO_ACTION_NONE_PC34;
    }
}

static int csb_v1_startup_playback_title_phase_mask_pc34(
    CSB_V1_StartupStage_PC34 stage)
{
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34) return 0x01;
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34) return 0x02;
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34) return 0x08;
    return 0;
}

static int csb_v1_startup_playback_session_owned_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    return session && session->valid && session->real_asset_matched &&
        session->title_assets_ready && session->entrance_assets_ready &&
        session->hud_assets_bound && session->surfaces.valid &&
        session->surfaces.title_regions_ready &&
        session->surfaces.opening_frame_ready;
}

int csb_v1_boot_startup_playback_begin_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupAudioAction_PC34 *out_audio_action)
{
    csb_v1_startup_playback_clear_action_pc34(out_audio_action);
    if (!csb_v1_startup_playback_session_owned_pc34(session) ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_NONE_PC34) {
        return 0;
    }
    /* ReDMCSB SWSH.C F0909 starts the owned swoosh before TITLE.C F0437. */
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_FTL_SWOOSH_PC34;
    session->playback.swoosh_active = 1;
    session->playback.no_fallback_routes = 1;
    if (out_audio_action) {
        *out_audio_action = CSB_V1_STARTUP_AUDIO_ACTION_PLAY_FTL_SWOOSH_PC34;
    }
    return 1;
}

int csb_v1_boot_startup_playback_complete_swoosh_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupAudioAction_PC34 *out_audio_action)
{
    csb_v1_startup_playback_clear_action_pc34(out_audio_action);
    if (!csb_v1_startup_playback_session_owned_pc34(session) ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_FTL_SWOOSH_PC34 ||
        !session->playback.swoosh_active) {
        return 0;
    }
    /* ReDMCSB SWSH.C F0910 releases the swoosh before TITLE.C consumes C001. */
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_TITLE_PC34;
    session->playback.title_stage = CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34;
    session->playback.title_frame = 0;
    session->playback.swoosh_active = 0;
    if (out_audio_action) {
        *out_audio_action = CSB_V1_STARTUP_AUDIO_ACTION_RELEASE_FTL_SWOOSH_PC34;
    }
    return 1;
}

int csb_v1_boot_startup_playback_title_frame_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    int title_frame,
    CSB_V1_StartupRenderPlan_PC34 *out_plan,
    CSB_V1_StartupAudioAction_PC34 *out_audio_action)
{
    CSB_V1_StartupStage_PC34 title_stage;

    csb_v1_startup_playback_clear_action_pc34(out_audio_action);
    if (out_plan) {
        memset(out_plan, 0, sizeof(*out_plan));
    }
    if (!csb_v1_startup_playback_session_owned_pc34(session) || !out_plan ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_TITLE_PC34 ||
        title_frame < 0) {
        return 0;
    }
    if (title_frame >= csb_v1_startup_title_total_ticks_pc34()) {
        /* ReDMCSB TITLE.C F0437 must have presented all C001 phase routes
         * before ENTRANCE.C F0806 owns the next screen/session. */
        if (session->playback.title_phase_mask != 0x0b) {
            return 0;
        }
        /* ReDMCSB ENTRANCE.C F0806:733 starts entrance music as its owned
         * screen/door/HUD session becomes active. */
        session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34;
        session->playback.entrance_music_active = 1;
        if (out_audio_action) {
            *out_audio_action = CSB_V1_STARTUP_AUDIO_ACTION_PLAY_ENTRANCE_MUSIC_PC34;
        }
        return 1;
    }
    title_stage = (CSB_V1_StartupStage_PC34)
        csb_v1_startup_title_stage_for_frame_pc34(title_frame);
    out_plan->surface = CSB_V1_STARTUP_RENDER_TITLE_PC34;
    out_plan->title_stage = title_stage;
    out_plan->title_source_step = (int)
        csb_v1_startup_title_source_step_for_frame_pc34(title_frame);
    out_plan->asset_command_count = 1;
    out_plan->asset_commands[0].visible = 1;
    out_plan->asset_commands[0].asset_id = 1;
    out_plan->asset_commands[0].kind =
        title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34
            ? CSB_V1_STARTUP_ASSET_TITLE_SCALED_REGION_PC34
            : CSB_V1_STARTUP_ASSET_TITLE_REGION_PC34;
    session->playback.title_frame = title_frame;
    session->playback.title_stage = title_stage;
    session->playback.title_phase_mask |=
        csb_v1_startup_playback_title_phase_mask_pc34(title_stage);
    return 1;
}

int csb_v1_boot_startup_playback_enter_hud_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    if (!csb_v1_startup_playback_session_owned_pc34(session) ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34 ||
        !session->playback.entrance_music_active ||
        !session->playback.entrance_complete) {
        return 0;
    }
    /* ENTRANCE.C F0806 owns the closed-door menu until entering the dungeon;
     * the same verified HUD bindings then become the runtime owner. */
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34;
    return 1;
}

int csb_v1_boot_startup_playback_complete_entrance_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    if (!csb_v1_startup_playback_session_owned_pc34(session) ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34 ||
        !session->playback.entrance_music_active) {
        return 0;
    }
    /* ReDMCSB ENTRANCE.C F0806:857-889 / CSBWin CSBCode.cpp:9515-9535:
     * only the completed door action returns control to the game/HUD. */
    session->playback.entrance_complete = 1;
    return 1;
}
