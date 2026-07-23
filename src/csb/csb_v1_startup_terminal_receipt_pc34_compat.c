#include "csb_v1_boot.h"
#include "vga_palette_pc34_compat.h"

#include <string.h>

int csb_v1_boot_startup_complete_timeline_receipt_from_session_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupCompleteTimelineReceipt_PC34 *out_receipt)
{
    const CSB_V1_StartupRuntimeSurface_PC34 *c017;
    const CSB_V1_StartupRuntimeSurface_PC34 *c040;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB TITLE.C F0437, ENTRANCE.C F0807 and PANEL.C F0347: a live
     * C017/C040 handoff is valid only for the terminal source session. */
    if (!session || !out_receipt || !session->valid ||
        !session->real_asset_matched || !session->rejects_legacy_wrappers ||
        !session->playback.no_fallback_routes ||
        /* TITLE.C F0437's frame-80 STRIKES boundary publishes the completed
         * four-bit C001 phase mask. F0807 may not expose C017/C040 until all
         * of PRESENTS, CHAOS, hold, and STRIKES have been consumed. */
        session->playback.title_phase_mask != 0x0f ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34 ||
        !session->playback.entrance_complete ||
        (session->playback.credits_scene_presented &&
         (!session->playback.credits_return_presented ||
          session->playback.credits_source_tick == 0u ||
          session->playback.credits_frame_route_hash == 0u ||
          session->playback.credits_raster_hash == 0u ||
          session->playback.credits_return_source_tick <=
              session->playback.credits_source_tick ||
          session->playback.credits_return_frame_route_hash == 0u ||
          session->playback.credits_return_raster_hash == 0u)) ||
        !session->playback.entrance_scene_presented ||
        !session->playback.door_frame_presented ||
        session->playback.last_door_opening_step != 31 ||
        session->playback.next_door_opening_step != 32 ||
        session->playback.entrance_special_palette !=
            VGA_PALETTE_PC34_SPECIAL_ENTRANCE ||
        !session->hud_assets_bound || session->generation == 0u) return 0;
    c017 = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
    c040 = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34];
    if (!c017->valid || !c017->pixels || c017->source_asset_id != 17 ||
        c017->width != CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34 ||
        c017->height != CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34 ||
        c017->transparent_color != -1 || !c040->valid || !c040->pixels ||
        c040->source_asset_id != 40 ||
        c040->width != CSB_V1_STARTUP_HUD_RESURRECT_WIDTH_PC34 ||
        c040->height != CSB_V1_STARTUP_HUD_RESURRECT_HEIGHT_PC34 ||
        c040->transparent_color !=
            CSB_V1_STARTUP_HUD_RESURRECT_TRANSPARENT_COLOR_PC34) return 0;
    out_receipt->valid = 1;
    out_receipt->c001_complete = 1;
    out_receipt->terminal_f0807_complete = 1;
    out_receipt->hud_session_ready = 1;
    out_receipt->c017_ready = 1;
    out_receipt->c040_ready = 1;
    out_receipt->no_legacy_wrapper = 1;
    out_receipt->no_fallback_route = 1;
    out_receipt->source_tick = session->source_tick;
    out_receipt->session_generation = session->generation;
    return 1;
}

int csb_v1_boot_startup_authorize_live_hud_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupCompleteTimelineReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupAudioAction_PC34 audio_action;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!session || !out_receipt || !session->valid) return 0;
    /* ReDMCSB TITLE.C F0437 reaches ENTRANCE.C only after C001's final
     * source tick. ENTRANCE.C F0807 may then publish C017/C040 to PANEL.C. */
    if (session->playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_TITLE_PC34 &&
        !csb_v1_boot_startup_playback_title_frame_pc34(
            session, csb_v1_startup_title_total_ticks_pc34(), &plan,
            &audio_action)) return 0;
    if (!csb_v1_boot_startup_playback_complete_entrance_pc34(session) ||
        !csb_v1_boot_startup_playback_enter_hud_pc34(session)) return 0;
    return csb_v1_boot_startup_complete_timeline_receipt_from_session_pc34(
        session, out_receipt);
}

int csb_v1_boot_startup_live_hud_terminal_receipt_current_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    uint32_t terminal_source_tick,
    uint32_t terminal_generation)
{
    CSB_V1_StartupCompleteTimelineReceipt_PC34 receipt;

    if (terminal_source_tick == 0u || terminal_generation == 0u ||
        !csb_v1_boot_startup_complete_timeline_receipt_from_session_pc34(
            session, &receipt)) {
        return 0;
    }
    return receipt.source_tick == terminal_source_tick &&
        receipt.session_generation == terminal_generation;
}

int csb_v1_boot_terminal_ui_state_host_raster_pc34(
    const CSB_V1_TerminalUiStateReceipt_PC34 *terminal_ui,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRuntimeAssetFrame_PC34 *frame,
    int include_resurrection_panel,
    CSB_V1_StartupRuntimeRaster_PC34 *out_raster)
{
    if (out_raster) memset(out_raster, 0, sizeof(*out_raster));
    if (!terminal_ui || !session || !frame || !out_raster ||
        !terminal_ui->valid ||
        !terminal_ui->c040_cleared_once ||
        !terminal_ui->post_c101_presented ||
        !terminal_ui->live_c017_only_panel_base ||
        !terminal_ui->palette_neutral ||
        !terminal_ui->no_legacy_wrapper ||
        !terminal_ui->no_cast_or_combat ||
        terminal_ui->c017_source_asset_id != 17 ||
        terminal_ui->c017_width != CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34 ||
        terminal_ui->c017_height != CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34 ||
        terminal_ui->c017_special_palette != -1 ||
        !csb_v1_boot_startup_live_hud_terminal_receipt_current_pc34(
            session, terminal_ui->source_tick,
            terminal_ui->session_generation) ||
        frame->source_tick != terminal_ui->source_tick ||
        frame->session_generation != terminal_ui->session_generation) {
        return 0;
    }
    return csb_v1_boot_startup_runtime_hud_frame_rasterize_pc34(
        frame, include_resurrection_panel, out_raster);
}
