#include "csb_v1_boot.h"
#include "csb_v1_startup_session_contract_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

#include <string.h>

static void csb_v1_startup_playback_clear_action_pc34(
    CSB_V1_StartupAudioAction_PC34 *out_audio_action)
{
    if (out_audio_action) {
        *out_audio_action = CSB_V1_STARTUP_AUDIO_ACTION_NONE_PC34;
    }
}

static int csb_v1_startup_playback_title_phase_mask_pc34(
    CSB_V1_StartupStage_PC34 stage, int title_frame)
{
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34) return 0x01;
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34) {
        /* TITLE.C F0437 retains the CHAOS render stage through its full-size
         * wait. Source step 21 begins that distinct post-zoom hold. */
        return csb_v1_startup_title_source_step_for_frame_pc34(title_frame) ==
                   21u
            ? 0x04
            : 0x02;
    }
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34) {
        /* The first C426 frame follows TITLE.C's Delay(20) full-size CHAOS
         * hold.
         * Record both source-visible phases even when playback is sampled
         * directly at the transition boundary. */
        return title_frame == csb_v1_startup_title_presents_ticks_pc34() +
                                  csb_v1_startup_title_chaos_zoom_ticks_pc34() +
                                  csb_v1_startup_title_chaos_hold_ticks_pc34()
            ? 0x0c
            : 0x08;
    }
    return 0;
}

static int csb_v1_startup_playback_session_owned_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    return session && session->valid && session->real_asset_matched &&
        session->title_assets_ready && session->entrance_assets_ready &&
        session->surfaces.valid && session->surfaces.title_regions_ready &&
        session->surfaces.opening_frame_ready;
}

int csb_v1_boot_startup_title_capture_plan_admit_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    int title_frame)
{
    if (!plan || plan->surface != CSB_V1_STARTUP_RENDER_TITLE_PC34 ||
        plan->source_asset_id != 1 || title_frame < 0 ||
        plan->title_source_step != (int)
            csb_v1_startup_title_source_step_for_frame_pc34(
                title_frame) ||
        plan->title_stage != (int)csb_v1_startup_title_stage_for_frame_pc34(
            title_frame)) {
        return 0;
    }
    switch (plan->title_stage) {
    case CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34:
        return plan->title_source_step == 1 &&
            plan->title_source_y == 137 && plan->title_source_w == 320 &&
            plan->title_source_h == 16 && plan->title_dest_y == 90 &&
            plan->title_blit_kind == CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34;
    case CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34:
        return plan->title_source_step >= 2 && plan->title_source_step <= 21 &&
            plan->title_source_y == 0 && plan->title_source_w == 320 &&
            plan->title_source_h == 80 && plan->title_dest_w > 0 &&
            plan->title_dest_h > 0 &&
            plan->title_blit_kind ==
                CSB_V1_STARTUP_TITLE_BLIT_SCALED_REGION_PC34;
    case CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34:
        return plan->title_source_step >= 21 &&
            plan->title_source_y == 80 && plan->title_source_w == 320 &&
            plan->title_source_h == 57 && plan->title_dest_y == 118 &&
            plan->title_blit_kind == CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34 &&
            plan->title_transparent_color == 0;
    default:
        return 0;
    }
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
    session->playback.title_phase_mask = 0;
    session->playback.swoosh_active = 0;
    if (out_audio_action) {
        *out_audio_action = CSB_V1_STARTUP_AUDIO_ACTION_RELEASE_FTL_SWOOSH_PC34;
    }
    return 1;
}

int csb_v1_boot_startup_playback_accepts_title_plan_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    int title_frame)
{
    return csb_v1_startup_playback_session_owned_pc34(session) &&
        session->playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_TITLE_PC34 &&
        csb_v1_boot_startup_title_capture_plan_admit_pc34(plan, title_frame);
}

int csb_v1_boot_startup_playback_title_frame_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    int title_frame,
    CSB_V1_StartupRenderPlan_PC34 *out_plan,
    CSB_V1_StartupAudioAction_PC34 *out_audio_action)
{
    CSB_V1_StartupStage_PC34 title_stage;
    CSB_V1_StartupRenderState_PC34 render_state;

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
        if (session->playback.title_phase_mask != 0x0f) {
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
    /* TITLE.C F0437's source rectangles, palette phase, and scaled CHAOS
     * geometry are owned by the startup state-to-plan adapter. Playback must
     * consume that exact plan rather than reconstructing a partial C001 blit. */
    memset(&render_state, 0, sizeof(render_state));
    render_state.entrance_active = 1;
    render_state.title_active = 1;
    render_state.title_frame = title_frame;
    if (!csb_v1_startup_source_render_plan_from_state_pc34(&render_state,
                                                            out_plan) ||
        out_plan->surface != CSB_V1_STARTUP_RENDER_TITLE_PC34 ||
        out_plan->title_stage != (int)title_stage ||
        !csb_v1_boot_startup_title_capture_plan_admit_pc34(out_plan,
                                                            title_frame)) {
        memset(out_plan, 0, sizeof(*out_plan));
        return 0;
    }
    session->playback.title_frame = title_frame;
    session->playback.title_stage = title_stage;
    session->playback.title_phase_mask |=
        csb_v1_startup_playback_title_phase_mask_pc34(title_stage, title_frame);
    return 1;
}

int csb_v1_boot_startup_playback_complete_entrance_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    if (!csb_v1_startup_playback_session_owned_pc34(session) ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34 ||
        !csb_v1_startup_session_title_handoff_complete_pc34(session) ||
        !session->playback.entrance_music_active ||
        (session->playback.credits_scene_presented &&
         (!session->playback.credits_return_presented ||
          session->playback.credits_source_tick == 0u ||
          session->playback.credits_frame_route_hash == 0u ||
          session->playback.credits_raster_hash == 0u ||
          session->playback.credits_return_source_tick <=
              session->playback.credits_source_tick ||
          session->playback.credits_return_frame_route_hash == 0u ||
          session->playback.credits_return_raster_hash == 0u)) ||
        !session->entrance_assets_ready || !session->door_assets_ready ||
        !session->hud_assets_bound) {
        return 0;
    }
    /* ENTRANCE.C F0442 may present C005 from the F0806 command loop. When
     * that optional route was used, consume its real frame receipt and the
     * later C004/C002/C003 return before F0807 authorizes the dungeon
     * handoff. The ordinary no-credits path remains source-valid. */
    session->playback.entrance_complete = 1;
    session->playback.no_fallback_routes = 1;
    return 1;
}

int csb_v1_boot_startup_playback_enter_hud_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    if (!csb_v1_startup_playback_session_owned_pc34(session) ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34 ||
        !session->playback.entrance_music_active) {
        return 0;
    }
    if (!session->playback.entrance_complete &&
        !csb_v1_boot_startup_playback_complete_entrance_pc34(session)) {
        return 0;
    }
    /* ENTRANCE.C F0806 owns the closed-door menu until entering the dungeon;
     * the same verified HUD bindings then become the runtime owner. */
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34;
    return 1;
}
