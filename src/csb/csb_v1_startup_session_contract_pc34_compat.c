#include "csb_v1_startup_session_contract_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <string.h>

#define CSB_V1_CONTRACT_C017_WIDTH_PC34 224
#define CSB_V1_CONTRACT_C017_HEIGHT_PC34 136
#define CSB_V1_CONTRACT_C040_WIDTH_PC34 144
#define CSB_V1_CONTRACT_C040_HEIGHT_PC34 73
#define CSB_V1_CONTRACT_C040_TRANSPARENT_PC34 6

int csb_v1_startup_session_hud_surface_contract_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    const CSB_V1_StartupRuntimeSurface_PC34 *c017;
    const CSB_V1_StartupRuntimeSurface_PC34 *c040;

    if (!session || !session->hud_assets_bound) return 0;
    c017 = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
    c040 = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34];
    return c017->valid && c017->pixels && c017->source_asset_id == 17 &&
        c017->width == CSB_V1_CONTRACT_C017_WIDTH_PC34 &&
        c017->height == CSB_V1_CONTRACT_C017_HEIGHT_PC34 &&
        c017->transparent_color == -1 && c040->valid && c040->pixels &&
        c040->source_asset_id == 40 &&
        c040->width == CSB_V1_CONTRACT_C040_WIDTH_PC34 &&
        c040->height == CSB_V1_CONTRACT_C040_HEIGHT_PC34 &&
        c040->transparent_color == CSB_V1_CONTRACT_C040_TRANSPARENT_PC34;
}

int csb_v1_startup_session_terminal_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupSessionTerminalReceipt_PC34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!session || !out_receipt || !session->valid ||
        !session->real_asset_matched || !session->title_presents_ready ||
        !session->title_chaos_ready || !session->title_strikes_back_ready ||
        !session->entrance_assets_ready || !session->door_assets_ready ||
        !session->rejects_legacy_wrappers ||
        !session->playback.no_fallback_routes ||
        session->playback.title_phase_mask != 0x0f ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34 ||
        !session->playback.entrance_complete ||
        !session->hud_assets_bound || session->generation == 0u ||
        !csb_v1_startup_session_hud_surface_contract_pc34(session)) return 0;
    out_receipt->valid = 1;
    out_receipt->c001_complete = 1;
    out_receipt->terminal_f0807_complete = 1;
    out_receipt->c017_ready = 1;
    out_receipt->c040_ready = 1;
    out_receipt->source_tick = session->source_tick;
    out_receipt->session_generation = session->generation;
    return 1;
}

int csb_v1_startup_session_terminal_package_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    CSB_V1_StartupSessionTerminalPackageReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 terminal;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB TITLE.C F0437 lines 424-464 consumes C001 before ENTRANCE.C
     * F0807 lines 85-90 completes the entrance. PANEL.C F0347 line 2376 may
     * expose C017/C040 only from that same hash-verified package, never from
     * a wrapper-owned replacement. */
    if (!session || !package_receipt || !out_receipt ||
        !package_receipt->valid || !package_receipt->real_package_matched ||
        !package_receipt->c001_title_consumed ||
        !package_receipt->c001_presents_consumed ||
        !package_receipt->c001_chaos_consumed ||
        !package_receipt->c001_chaos_zoom_consumed ||
        !package_receipt->c001_chaos_hold_consumed ||
        !package_receipt->c001_strikes_back_consumed ||
        !package_receipt->c017_hud_consumed ||
        !package_receipt->c040_hud_consumed ||
        !package_receipt->title_to_hud_same_session ||
        !package_receipt->no_legacy_wrappers ||
        !package_receipt->no_fallback_routes ||
        package_receipt->real_asset_receipt_hash == 0u ||
        package_receipt->consumed_surface_hash == 0u ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &terminal) ||
        package_receipt->source_tick != terminal.source_tick ||
        package_receipt->session_generation != terminal.session_generation) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->real_package_matched = 1;
    out_receipt->c001_title_consumed = 1;
    out_receipt->c017_hud_consumed = 1;
    out_receipt->c040_hud_consumed = 1;
    out_receipt->terminal_f0807_complete = 1;
    out_receipt->no_legacy_wrappers = 1;
    out_receipt->no_fallback_routes = 1;
    out_receipt->source_tick = terminal.source_tick;
    out_receipt->session_generation = terminal.session_generation;
    out_receipt->real_asset_receipt_hash = package_receipt->real_asset_receipt_hash;
    out_receipt->consumed_surface_hash = package_receipt->consumed_surface_hash;
    return 1;
}

int csb_v1_startup_session_package_title_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    CSB_V1_StartupSessionPackageTitleReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 terminal;
    const CSB_V1_StartupRuntimeSurface_PC34 *title;
    const CSB_V1_StartupRuntimeSurface_PC34 *presents;
    const CSB_V1_StartupRuntimeSurface_PC34 *chaos;
    const CSB_V1_StartupRuntimeSurface_PC34 *strikes;

    /* ReDMCSB TITLE.C F0437 lines 424-463 draws C001 as three distinct
     * PRESENTS/CHAOS/STRIKES BACK regions. ENTRANCE.C F0806 lines 850-903
     * may enter live play only after that title session has reached its
     * terminal HUD handoff. */
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!session || !package_receipt || !out_receipt ||
        !package_receipt->valid || !package_receipt->real_package_matched ||
        !package_receipt->c001_title_consumed ||
        !package_receipt->c001_presents_consumed ||
        !package_receipt->c001_chaos_consumed ||
        !package_receipt->c001_chaos_zoom_consumed ||
        !package_receipt->c001_chaos_hold_consumed ||
        !package_receipt->c001_strikes_back_consumed ||
        !package_receipt->title_to_hud_same_session ||
        !package_receipt->no_legacy_wrappers ||
        !package_receipt->no_fallback_routes ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &terminal) ||
        package_receipt->source_tick != terminal.source_tick ||
        package_receipt->session_generation != terminal.session_generation ||
        package_receipt->real_asset_receipt_hash == 0u ||
        package_receipt->consumed_surface_hash == 0u) return 0;

    title = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34];
    presents = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34];
    chaos = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34];
    strikes = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34];
    if (!title->valid || !title->pixels || title->source_asset_id != 1 ||
        !presents->valid || !presents->pixels || presents->source_asset_id != 1 ||
        presents->source_x != 0 || presents->source_y != 137 ||
        presents->width != 320 || presents->height != 16 ||
        !chaos->valid || !chaos->pixels || chaos->source_asset_id != 1 ||
        chaos->source_x != 0 || chaos->source_y != 0 ||
        chaos->width != 320 || chaos->height != 80 ||
        !strikes->valid || !strikes->pixels || strikes->source_asset_id != 1 ||
        strikes->source_x != 0 || strikes->source_y != 80 ||
        strikes->width != 320 || strikes->height != 57 ||
        strikes->transparent_color != 0) return 0;

    out_receipt->real_package_matched = 1;
    out_receipt->c001_title_ready = 1;
    out_receipt->c001_presents_ready = 1;
    out_receipt->c001_chaos_ready = 1;
    out_receipt->c001_strikes_back_ready = 1;
    out_receipt->title_to_hud_same_session = 1;
    out_receipt->no_legacy_wrappers = 1;
    out_receipt->no_fallback_routes = 1;
    out_receipt->source_tick = terminal.source_tick;
    out_receipt->session_generation = terminal.session_generation;
    out_receipt->real_asset_receipt_hash = package_receipt->real_asset_receipt_hash;
    out_receipt->consumed_surface_hash = package_receipt->consumed_surface_hash;
    out_receipt->valid = 1;
    return 1;
}

int csb_v1_startup_session_opening_door_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_surface,
    CSB_V1_StartupSessionOpeningDoorReceipt_PC34 *out_receipt)
{
    const CSB_V1_StartupRuntimeSurface_PC34 *entrance;
    const CSB_V1_StartupRuntimeSurface_PC34 *left;
    const CSB_V1_StartupRuntimeSurface_PC34 *right;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB ENTRANCE.C F0806 lines 775-826 composes C004, C002, and C003
     * in the resident entrance session. The host must not replace that
     * opening composition after TITLE.C F0437 has authenticated C001. */
    if (!session || !package_receipt || !host_surface || !out_receipt ||
        !session->valid || !package_receipt->valid ||
        !package_receipt->real_package_matched ||
        !package_receipt->title_to_hud_same_session ||
        !package_receipt->no_legacy_wrappers ||
        !package_receipt->no_fallback_routes || !host_surface->valid ||
        !host_surface->real_asset_matched ||
        !host_surface->no_legacy_wrappers ||
        !host_surface->no_synthetic_surface ||
        host_surface->host_surface !=
            CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34 ||
        !host_surface->door_opening_decision ||
        host_surface->frame.session_generation != session->generation ||
        package_receipt->session_generation != session->generation ||
        package_receipt->real_asset_receipt_hash == 0u ||
        package_receipt->consumed_surface_hash == 0u ||
        !host_surface->raster.valid || !host_surface->raster.real_asset_matched ||
        !host_surface->raster.entrance_composited ||
        !host_surface->raster.door_composited ||
        host_surface->raster.source_surface_count != 3 ||
        host_surface->host_surface_hash == 0u) return 0;

    entrance = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34];
    left = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34];
    right = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34];
    if (!entrance->valid || !entrance->pixels || entrance->source_asset_id != 4 ||
        !left->valid || !left->pixels || left->source_asset_id != 2 ||
        !right->valid || !right->pixels || right->source_asset_id != 3 ||
        host_surface->frame.entrance_surface != entrance ||
        host_surface->frame.left_door_surface != left ||
        host_surface->frame.right_door_surface != right) return 0;

    out_receipt->valid = 1;
    out_receipt->real_package_matched = 1;
    out_receipt->c004_entrance_ready = 1;
    out_receipt->c002_left_door_ready = 1;
    out_receipt->c003_right_door_ready = 1;
    out_receipt->opening_to_title_same_session = 1;
    out_receipt->no_legacy_wrappers = 1;
    out_receipt->no_fallback_routes = 1;
    out_receipt->source_tick = host_surface->frame.source_tick;
    out_receipt->session_generation = session->generation;
    out_receipt->real_asset_receipt_hash = package_receipt->real_asset_receipt_hash;
    out_receipt->consumed_surface_hash = package_receipt->consumed_surface_hash;
    return 1;
}

int csb_v1_startup_session_title_opening_consumption_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *presents_host,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *chaos_host,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *strikes_host,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *opening_host,
    CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34 *out_receipt)
{
    const CSB_V1_StartupRuntimeSurface_PC34 *presents;
    const CSB_V1_StartupRuntimeSurface_PC34 *chaos;
    const CSB_V1_StartupRuntimeSurface_PC34 *strikes;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB TITLE.C F0437 lines 424-463 emits three C001 phases before
     * ENTRANCE.C F0806 lines 775-826 composes C004/C002/C003. Retain the
     * owning host receipts so a callback cannot replace any phase. */
    if (!session || !package_receipt || !presents_host || !chaos_host ||
        !strikes_host || !opening_host || !out_receipt || !session->valid ||
        !package_receipt->valid || !package_receipt->real_package_matched ||
        !package_receipt->c001_presents_consumed ||
        !package_receipt->c001_chaos_consumed ||
        !package_receipt->c001_chaos_zoom_consumed ||
        !package_receipt->c001_chaos_hold_consumed ||
        !package_receipt->c001_strikes_back_consumed ||
        !package_receipt->no_legacy_wrappers ||
        !package_receipt->no_fallback_routes ||
        package_receipt->session_generation != session->generation ||
        package_receipt->real_asset_receipt_hash == 0u ||
        package_receipt->consumed_surface_hash == 0u) return 0;

    presents = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34];
    chaos = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34];
    strikes = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34];
    if (!presents->valid || !chaos->valid || !strikes->valid ||
        !presents->pixels || !chaos->pixels || !strikes->pixels ||
        presents->source_asset_id != 1 || chaos->source_asset_id != 1 ||
        strikes->source_asset_id != 1 ||
        !presents_host->valid || !chaos_host->valid || !strikes_host->valid ||
        !opening_host->valid || !presents_host->real_asset_matched ||
        !chaos_host->real_asset_matched || !strikes_host->real_asset_matched ||
        !opening_host->real_asset_matched ||
        !presents_host->no_legacy_wrappers || !chaos_host->no_legacy_wrappers ||
        !strikes_host->no_legacy_wrappers || !opening_host->no_legacy_wrappers ||
        !presents_host->no_synthetic_surface || !chaos_host->no_synthetic_surface ||
        !strikes_host->no_synthetic_surface || !opening_host->no_synthetic_surface ||
        presents_host->host_surface != CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34 ||
        chaos_host->host_surface != CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34 ||
        strikes_host->host_surface != CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34 ||
        opening_host->host_surface !=
            CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34 ||
        !opening_host->door_opening_decision ||
        presents_host->frame.session_generation != session->generation ||
        chaos_host->frame.session_generation != session->generation ||
        strikes_host->frame.session_generation != session->generation ||
        opening_host->frame.session_generation != session->generation ||
        presents_host->frame.stage != CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 ||
        chaos_host->frame.stage != CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 ||
        strikes_host->frame.stage != CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 ||
        presents_host->frame.title_surface != presents ||
        chaos_host->frame.title_surface != chaos ||
        strikes_host->frame.title_surface != strikes ||
        !presents_host->raster.valid || !chaos_host->raster.valid ||
        !strikes_host->raster.valid || !opening_host->raster.valid ||
        !presents_host->raster.title_composited ||
        !chaos_host->raster.title_composited ||
        !strikes_host->raster.title_composited ||
        presents_host->raster.source_surface_count != 1 ||
        chaos_host->raster.source_surface_count != 1 ||
        strikes_host->raster.source_surface_count != 1 ||
        presents_host->host_surface_hash == 0u || chaos_host->host_surface_hash == 0u ||
        strikes_host->host_surface_hash == 0u || opening_host->host_surface_hash == 0u)
        return 0;

    out_receipt->valid = 1;
    out_receipt->real_package_matched = 1;
    out_receipt->presents_consumed = 1;
    out_receipt->chaos_consumed = 1;
    out_receipt->strikes_back_consumed = 1;
    out_receipt->c004_c002_c003_consumed = 1;
    out_receipt->no_legacy_wrappers = 1;
    out_receipt->no_synthetic_surface = 1;
    out_receipt->session_generation = session->generation;
    out_receipt->presents_host_surface_hash = presents_host->host_surface_hash;
    out_receipt->chaos_host_surface_hash = chaos_host->host_surface_hash;
    out_receipt->strikes_host_surface_hash = strikes_host->host_surface_hash;
    out_receipt->opening_host_surface_hash = opening_host->host_surface_hash;
    out_receipt->real_asset_receipt_hash = package_receipt->real_asset_receipt_hash;
    out_receipt->consumed_surface_hash = package_receipt->consumed_surface_hash;
    return 1;
}

int csb_v1_startup_session_hud_door_input_package_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *hud_host,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    const CSB_V1_StartupSessionDoorHudTickReceipt_PC34 *door_receipt,
    const CSB_V1_StartupSessionInputReceipt_PC34 *input_receipt,
    CSB_V1_StartupSessionHudDoorInputPackageReceipt_PC34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB PANEL.C F0346/F0347 restores C017 after C040, then
     * DUNGEON.C advances the first live door tick and COMMAND.C consumes the
     * first input. The package receipt is carried throughout, never copied
     * into a wrapper-owned framebuffer. CSBWin's OpenPrisonDoors reaches the
     * same live panel-before-command boundary. */
    if (!session || !package_receipt || !hud_host || !live_hud_receipt ||
        !door_receipt || !input_receipt || !out_receipt || !session->valid ||
        !package_receipt->valid || !package_receipt->real_package_matched ||
        !package_receipt->c017_hud_consumed ||
        !package_receipt->c040_hud_consumed ||
        !package_receipt->no_legacy_wrappers ||
        !package_receipt->no_fallback_routes ||
        package_receipt->session_generation != session->generation ||
        package_receipt->real_asset_receipt_hash == 0u ||
        package_receipt->consumed_surface_hash == 0u || !hud_host->valid ||
        !hud_host->real_asset_matched || !hud_host->no_legacy_wrappers ||
        !hud_host->no_synthetic_surface ||
        hud_host->host_surface != CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_HUD_PC34 ||
        !hud_host->runtime_hud_decision || !hud_host->uses_c017_inventory ||
        !hud_host->uses_c040_resurrect ||
        hud_host->frame.session_generation != session->generation ||
        hud_host->host_surface_hash == 0u || !live_hud_receipt->valid ||
        !live_hud_receipt->c040_cleared_once ||
        !live_hud_receipt->c017_live_base_only ||
        live_hud_receipt->c017_source_asset_id != 17 ||
        live_hud_receipt->session_generation != session->generation ||
        live_hud_receipt->source_tick != hud_host->frame.source_tick ||
        !door_receipt->valid || !door_receipt->first_live_door_tick ||
        door_receipt->previous_door_step != 0u || door_receipt->door_step != 1u ||
        door_receipt->session_generation != session->generation ||
        door_receipt->source_tick != live_hud_receipt->source_tick + 1u ||
        !input_receipt->valid || !input_receipt->first_post_c040_input ||
        input_receipt->command == CSB_V1_STARTUP_SESSION_MOVEMENT_NONE_PC34 ||
        input_receipt->session_generation != session->generation ||
        input_receipt->source_tick != live_hud_receipt->source_tick + 1u ||
        input_receipt->source_tick != door_receipt->source_tick) return 0;

    out_receipt->valid = 1;
    out_receipt->real_package_matched = 1;
    out_receipt->c017_hud_consumed = 1;
    out_receipt->c040_hud_consumed = 1;
    out_receipt->first_live_door_frame = 1;
    out_receipt->first_runtime_input = 1;
    out_receipt->no_legacy_wrappers = 1;
    out_receipt->no_synthetic_surface = 1;
    out_receipt->session_generation = session->generation;
    out_receipt->hud_source_tick = live_hud_receipt->source_tick;
    out_receipt->first_runtime_tick = input_receipt->source_tick;
    out_receipt->hud_host_surface_hash = hud_host->host_surface_hash;
    out_receipt->real_asset_receipt_hash = package_receipt->real_asset_receipt_hash;
    out_receipt->consumed_surface_hash = package_receipt->consumed_surface_hash;
    return 1;
}

int csb_v1_startup_session_live_hud_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionTerminalReceipt_PC34 *terminal_receipt,
    unsigned int c040_clear_count,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionLiveHudReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 current;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB PANEL.C F0346 clears the candidate C040 surface once; F0347
     * then restores the normal C017 panel. The return cannot borrow a later
     * session or tick, even if its source assets still happen to match. */
    if (!session || !terminal_receipt || !out_receipt ||
        c040_clear_count != 1u || !terminal_receipt->valid ||
        !terminal_receipt->c017_ready || !terminal_receipt->c040_ready ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &current) ||
        source_tick != terminal_receipt->source_tick ||
        session_generation != terminal_receipt->session_generation ||
        current.source_tick != terminal_receipt->source_tick ||
        current.session_generation != terminal_receipt->session_generation) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->c040_cleared_once = 1;
    out_receipt->c017_live_base_only = 1;
    out_receipt->c017_source_asset_id = 17;
    out_receipt->c017_width = CSB_V1_CONTRACT_C017_WIDTH_PC34;
    out_receipt->c017_height = CSB_V1_CONTRACT_C017_HEIGHT_PC34;
    out_receipt->special_palette = -1;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session_generation;
    return 1;
}

int csb_v1_startup_session_first_door_hud_tick_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    unsigned int previous_door_step,
    unsigned int door_step,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionDoorHudTickReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 current;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB DUNGEON.C advances the live door state by one tick. The first
     * action after C040's clear begins a new runtime door at step zero; it
     * cannot replay F0807 or consume a newer C017 session. */
    if (!session || !live_hud_receipt || !out_receipt ||
        !live_hud_receipt->valid || !live_hud_receipt->c040_cleared_once ||
        !live_hud_receipt->c017_live_base_only ||
        previous_door_step >= 31u || door_step != previous_door_step + 1u ||
        source_tick <= live_hud_receipt->source_tick ||
        door_step != source_tick - live_hud_receipt->source_tick ||
        session_generation != live_hud_receipt->session_generation ||
        session->source_tick != source_tick ||
        session->generation != session_generation ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &current) ||
        current.session_generation != session_generation) return 0;
    out_receipt->valid = 1;
    out_receipt->first_live_door_tick = previous_door_step == 0u ? 1 : 0;
    out_receipt->previous_door_step = previous_door_step;
    out_receipt->door_step = door_step;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session_generation;
    return 1;
}

int csb_v1_startup_session_first_input_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    CSB_V1_StartupSessionMovementCommand_PC34 command,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionInputReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 current;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB COMMAND.C consumes the first movement only after PANEL.C
     * restores C017. Keep this action in the immediate next source tick;
     * a delayed command must re-enter through a later runtime receipt. */
    if (!session || !live_hud_receipt || !out_receipt ||
        !live_hud_receipt->valid || !live_hud_receipt->c040_cleared_once ||
        !live_hud_receipt->c017_live_base_only ||
        command <= CSB_V1_STARTUP_SESSION_MOVEMENT_NONE_PC34 ||
        command > CSB_V1_STARTUP_SESSION_MOVEMENT_TURN_RIGHT_PC34 ||
        source_tick != live_hud_receipt->source_tick + 1u ||
        session_generation != live_hud_receipt->session_generation ||
        session->source_tick != source_tick ||
        session->generation != session_generation ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &current) ||
        current.session_generation != session_generation) return 0;
    out_receipt->valid = 1;
    out_receipt->first_post_c040_input = 1;
    out_receipt->command = command;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session_generation;
    return 1;
}

int csb_v1_startup_session_first_action_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    CSB_V1_StartupSessionActionCommand_PC34 command,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionActionReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 current;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB COMMAND.C follows PANEL.C's C040 clear with C017-owned hand
     * or cast dispatch. This first action is bound to exactly the next tick,
     * preventing a stale action surface from crossing a session boundary. */
    if (!session || !live_hud_receipt || !out_receipt ||
        !live_hud_receipt->valid || !live_hud_receipt->c040_cleared_once ||
        !live_hud_receipt->c017_live_base_only ||
        command <= CSB_V1_STARTUP_SESSION_ACTION_NONE_PC34 ||
        command > CSB_V1_STARTUP_SESSION_ACTION_CAST_PC34 ||
        source_tick != live_hud_receipt->source_tick + 1u ||
        session_generation != live_hud_receipt->session_generation ||
        session->source_tick != source_tick ||
        session->generation != session_generation ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &current) ||
        current.session_generation != session_generation) return 0;
    out_receipt->valid = 1;
    out_receipt->first_post_live_hud_action = 1;
    out_receipt->command = command;
    out_receipt->c017_source_asset_id = 17;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session_generation;
    return 1;
}

int csb_v1_startup_session_selection_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    const CSB_V1_StartupSessionActionReceipt_PC34 *action_receipt,
    CSB_V1_StartupSessionSelectionKind_PC34 kind,
    int selection_index,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionSelectionReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 current;
    int action_slot;
    int spell_rune;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    action_slot = kind == CSB_V1_STARTUP_SESSION_SELECTION_ACTION_SLOT_PC34 &&
        selection_index >= 0 && selection_index < 4 &&
        (action_receipt &&
         (action_receipt->command == CSB_V1_STARTUP_SESSION_ACTION_LEFT_HAND_PC34 ||
          action_receipt->command == CSB_V1_STARTUP_SESSION_ACTION_RIGHT_HAND_PC34));
    spell_rune = kind == CSB_V1_STARTUP_SESSION_SELECTION_SPELL_RUNE_PC34 &&
        selection_index >= 1 && selection_index <= 6 && action_receipt &&
        action_receipt->command == CSB_V1_STARTUP_SESSION_ACTION_CAST_PC34;
    /* ReDMCSB COMMAND.C resolves spell/action selection after the action
     * command. A selection cannot borrow an earlier action or a later C017
     * session merely because its visual panel remains present. */
    if (!session || !live_hud_receipt || !action_receipt || !out_receipt ||
        !live_hud_receipt->valid || !action_receipt->valid ||
        (!action_slot && !spell_rune) ||
        action_receipt->source_tick != live_hud_receipt->source_tick + 1u ||
        action_receipt->session_generation != live_hud_receipt->session_generation ||
        source_tick != action_receipt->source_tick + 1u ||
        session_generation != action_receipt->session_generation ||
        session->source_tick != source_tick ||
        session->generation != session_generation ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &current) ||
        current.session_generation != session_generation) return 0;
    out_receipt->valid = 1;
    out_receipt->kind = kind;
    out_receipt->selection_index = selection_index;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session_generation;
    return 1;
}
