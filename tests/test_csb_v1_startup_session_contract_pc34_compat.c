#include "csb_v1_startup_session_contract_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

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

static void make_terminal_session(CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    static unsigned char c017_pixel;
    static unsigned char c040_pixel;
    static unsigned char c001_pixels[320 * 153];
    static unsigned char presents_pixels[320 * 16];
    static unsigned char chaos_pixels[320 * 80];
    static unsigned char strikes_pixels[320 * 57];
    static unsigned char c002_pixels[105 * 161];
    static unsigned char c003_pixels[128 * 161];
    static unsigned char c004_pixels[320 * 200];
    CSB_V1_StartupRuntimeSurface_PC34 *c017;
    CSB_V1_StartupRuntimeSurface_PC34 *c040;
    CSB_V1_StartupRuntimeSurface_PC34 *title;
    CSB_V1_StartupRuntimeSurface_PC34 *presents;
    CSB_V1_StartupRuntimeSurface_PC34 *chaos;
    CSB_V1_StartupRuntimeSurface_PC34 *strikes;
    CSB_V1_StartupRuntimeSurface_PC34 *left;
    CSB_V1_StartupRuntimeSurface_PC34 *right;
    CSB_V1_StartupRuntimeSurface_PC34 *entrance;

    memset(session, 0, sizeof(*session));
    session->valid = session->real_asset_matched = 1;
    session->title_presents_ready = session->title_chaos_ready = 1;
    session->title_strikes_back_ready = session->entrance_assets_ready = 1;
    session->door_assets_ready = 1;
    session->rejects_legacy_wrappers = session->hud_assets_bound = 1;
    session->generation = 9u;
    session->source_tick = 413u;
    session->playback.no_fallback_routes = 1;
    session->playback.title_phase_mask = 0x0f;
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34;
    session->playback.entrance_complete = 1;
    session->surfaces.valid = session->surfaces.real_asset_matched = 1;
    session->surfaces.title_regions_ready = 1;
    session->surfaces.opening_frame_ready = 1;
    session->surfaces.entrance_screen_ready = 1;
    session->surfaces.hud_surfaces_ready = 1;
    c017 = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
    c040 = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34];
    c017->valid = 1; c017->pixels = &c017_pixel; c017->source_asset_id = 17;
    c017->width = 224; c017->height = 136;
    c017->transparent_color = -1;
    c040->valid = 1; c040->pixels = &c040_pixel; c040->source_asset_id = 40;
    c040->width = 144; c040->height = 73;
    c040->transparent_color = 6;
    title = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34];
    presents = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34];
    chaos = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34];
    strikes = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34];
    left = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34];
    right = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34];
    entrance = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34];
    title->valid = 1; title->pixels = c001_pixels; title->source_asset_id = 1;
    title->width = 320; title->height = 153; title->transparent_color = -1;
    presents->valid = 1; presents->pixels = presents_pixels; presents->source_asset_id = 1;
    presents->source_x = 0; presents->source_y = 137;
    presents->width = 320; presents->height = 16; presents->transparent_color = -1;
    chaos->valid = 1; chaos->pixels = chaos_pixels; chaos->source_asset_id = 1;
    chaos->source_x = 0; chaos->source_y = 0;
    chaos->width = 320; chaos->height = 80; chaos->transparent_color = -1;
    strikes->valid = 1; strikes->pixels = strikes_pixels; strikes->source_asset_id = 1;
    strikes->source_x = 0; strikes->source_y = 80;
    strikes->width = 320; strikes->height = 57; strikes->transparent_color = 0;
    left->valid = 1; left->pixels = c002_pixels; left->source_asset_id = 2;
    left->width = 105; left->height = 161; left->transparent_color = -1;
    right->valid = 1; right->pixels = c003_pixels; right->source_asset_id = 3;
    right->width = 128; right->height = 161; right->transparent_color = -1;
    entrance->valid = 1; entrance->pixels = c004_pixels; entrance->source_asset_id = 4;
    entrance->width = 320; entrance->height = 200; entrance->transparent_color = -1;
}

static void make_title_host(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupStage_PC34 stage,
    int special_palette,
    uint32_t host_hash,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host)
{
    memset(host, 0, sizeof(*host));
    host->valid = host->real_asset_matched = 1;
    host->no_legacy_wrappers = host->no_synthetic_surface = 1;
    host->host_surface = CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34;
    host->special_palette = special_palette;
    host->title_special_palette = special_palette;
    host->host_surface_hash = host_hash;
    host->frame.session_generation = session->generation;
    host->frame.special_palette = special_palette;
    host->frame.title_special_palette = special_palette;
    host->frame.stage = stage;
    host->frame.title_surface =
        &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34];
    host->frame.title_phase_tick_count = csb_v1_startup_title_total_ticks_pc34();
    host->frame.frame_route_hash = host_hash ^ 0x51000000u;
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34) {
        host->frame.title_phase_tick = 1;
        host->frame.title_phase_mask = 0x01;
    } else if (stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34) {
        host->frame.title_phase_tick = 21;
        host->frame.title_phase_mask = 0x02;
    } else if (stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34) {
        host->frame.title_phase_tick = 22;
        host->frame.title_phase_mask = 0x08;
    }
    host->raster.valid = host->raster.real_asset_matched = 1;
    host->raster.title_composited = 1;
    host->raster.source_surface_count = 1;
    host->raster.pixel_hash = host_hash ^ 0x71000000u;
    host->raster.route_hash = host_hash ^ 0x72000000u;
}

static void make_opening_host(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host)
{
    memset(host, 0, sizeof(*host));
    host->valid = host->real_asset_matched = 1;
    host->no_legacy_wrappers = host->no_synthetic_surface = 1;
    host->host_surface = CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34;
    host->door_opening_decision = 1;
    host->special_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
    host->title_special_palette = -1;
    host->host_surface_hash = 0xd004u;
    host->frame.session_generation = session->generation;
    host->raster.valid = host->raster.real_asset_matched = 1;
    host->raster.entrance_composited = 1;
    host->raster.door_composited = 1;
    host->raster.source_surface_count = 3;
}

int main(void)
{
    CSB_V1_StartupRenderState_PC34 state;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupSessionTerminalReceipt_PC34 receipt;
    CSB_V1_StartupSessionTerminalReceipt_PC34 terminal;
    CSB_V1_StartupSessionTerminalPackageReceipt_PC34 terminal_package;
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 package_receipt;
    CSB_V1_StartupSessionLiveHudReceipt_PC34 live_hud;
    CSB_V1_StartupSessionDoorHudTickReceipt_PC34 door_tick;
    CSB_V1_StartupSessionInputReceipt_PC34 input;
    CSB_V1_StartupSessionActionReceipt_PC34 action;
    CSB_V1_StartupSessionActionReceipt_PC34 cast_action;
    CSB_V1_StartupSessionSelectionReceipt_PC34 selection;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 presents_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 chaos_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 strikes_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 opening_host;
    CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34 title_opening;
    unsigned int tick;
    unsigned int generation;

    memset(&state, 0, sizeof(state));
    state.entrance_active = state.title_active = 1;
    check(csb_v1_startup_source_render_plan_from_state_pc34(&state, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              plan.source_asset_id == 1 && plan.title_source_y == 137 &&
              plan.title_source_w == 320 && plan.title_source_h == 16 &&
              plan.title_dest_x == 0 && plan.title_dest_y == 90 &&
              plan.title_dest_w == 320 && plan.title_dest_h == 16 &&
              plan.special_palette == VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS,
          "C001 PRESENTS retains source-owned geometry and palette");
    state.title_frame = csb_v1_startup_title_presents_ticks_pc34();
    check(csb_v1_startup_source_render_plan_from_state_pc34(&state, &plan) &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              plan.title_dest_w == 16 && plan.title_dest_h == 4 &&
              plan.special_palette == VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS,
          "C001 CHAOS retains source-owned zoom geometry and palette");

    make_terminal_session(&session);
    memset(&package_receipt, 0, sizeof(package_receipt));
    package_receipt.valid = package_receipt.real_package_matched = 1;
    package_receipt.c001_title_consumed = 1;
    package_receipt.c001_presents_consumed = 1;
    package_receipt.c001_chaos_consumed = 1;
    package_receipt.c001_chaos_zoom_consumed = 1;
    package_receipt.c001_chaos_hold_consumed = 1;
    package_receipt.c001_strikes_back_consumed = 1;
    package_receipt.c017_hud_consumed = package_receipt.c040_hud_consumed = 1;
    package_receipt.title_to_hud_same_session = 1;
    package_receipt.no_legacy_wrappers = package_receipt.no_fallback_routes = 1;
    package_receipt.source_tick = session.source_tick;
    package_receipt.session_generation = session.generation;
    package_receipt.real_asset_receipt_hash = 0x12345678u;
    package_receipt.consumed_surface_hash = 0x87654321u;
    check(csb_v1_startup_session_terminal_receipt_pc34(&session, &receipt) &&
              receipt.valid && receipt.c001_complete &&
              receipt.terminal_f0807_complete && receipt.c017_ready &&
              receipt.c040_ready,
          "terminal F0807 authorizes C017/C040 only in the terminal session");
    check(csb_v1_startup_session_terminal_package_receipt_pc34(
              &session, &package_receipt, &terminal_package) &&
              terminal_package.valid && terminal_package.real_package_matched &&
              terminal_package.c001_title_consumed &&
              terminal_package.c017_hud_consumed &&
              terminal_package.c040_hud_consumed &&
              terminal_package.terminal_f0807_complete &&
              terminal_package.source_tick == session.source_tick &&
              terminal_package.session_generation == session.generation &&
              terminal_package.real_asset_receipt_hash ==
                  package_receipt.real_asset_receipt_hash &&
              terminal_package.consumed_surface_hash ==
                  package_receipt.consumed_surface_hash,
          "terminal F0807 retains the hash-verified C001/C017/C040 package");
    session.playback.title_phase_mask = 0x0b;
    check(!csb_v1_startup_session_terminal_package_receipt_pc34(
              &session, &package_receipt, &terminal_package),
          "missing TITLE.C full-CHAOS hold cannot authorize the terminal HUD session");
    session.playback.title_phase_mask = 0x0f;
    package_receipt.c001_chaos_hold_consumed = 0;
    check(!csb_v1_startup_session_terminal_package_receipt_pc34(
              &session, &package_receipt, &terminal_package),
          "missing package-owned full-CHAOS hold cannot authorize the terminal HUD session");
    package_receipt.c001_chaos_hold_consumed = 1;
    ++package_receipt.source_tick;
    check(!csb_v1_startup_session_terminal_package_receipt_pc34(
               &session, &package_receipt, &terminal_package),
          "stale package tick cannot authorize the terminal HUD session");
    --package_receipt.source_tick;
    package_receipt.consumed_surface_hash = 0u;
    check(!csb_v1_startup_session_terminal_package_receipt_pc34(
               &session, &package_receipt, &terminal_package),
          "missing package surface hash cannot authorize the terminal HUD session");
    package_receipt.consumed_surface_hash = 0x87654321u;
    make_title_host(&session, CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34,
                    VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS, 0xc001u,
                    &presents_host);
    make_title_host(&session, CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34,
                    VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS, 0xc002u,
                    &chaos_host);
    make_title_host(&session, CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34,
                    VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES, 0xc003u,
                    &strikes_host);
    make_opening_host(&session, &opening_host);
    check(csb_v1_startup_session_title_opening_consumption_receipt_pc34(
              &session, &package_receipt, &presents_host, &chaos_host,
              &strikes_host, &opening_host, &title_opening) &&
              title_opening.valid && title_opening.presents_consumed &&
              title_opening.chaos_consumed &&
              title_opening.strikes_back_consumed &&
              title_opening.c004_c002_c003_consumed &&
              title_opening.opening_host_surface_hash ==
                  opening_host.host_surface_hash,
          "C001 title consumption reaches only a real C004/C002/C003 opening raster");
    chaos_host.frame.title_phase_tick = 1;
    check(!csb_v1_startup_session_title_opening_consumption_receipt_pc34(
              &session, &package_receipt, &presents_host, &chaos_host,
              &strikes_host, &opening_host, &title_opening),
          "PRESENTS source step cannot be relabeled as a CHAOS title capture");
    chaos_host.frame.title_phase_tick = 21;
    strikes_host.raster.pixel_hash = presents_host.raster.pixel_hash;
    check(!csb_v1_startup_session_title_opening_consumption_receipt_pc34(
              &session, &package_receipt, &presents_host, &chaos_host,
              &strikes_host, &opening_host, &title_opening),
          "duplicated title pixels cannot satisfy PRESENTS/CHAOS/STRIKES consumption");
    strikes_host.raster.pixel_hash = 0xc003u ^ 0x71000000u;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].width = 1;
    check(!csb_v1_startup_session_title_opening_consumption_receipt_pc34(
              &session, &package_receipt, &presents_host, &chaos_host,
              &strikes_host, &opening_host, &title_opening),
          "forged one-byte C003 cannot satisfy title/opening consumption");
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].width = 128;
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].height = 1;
    check(!csb_v1_startup_session_terminal_receipt_pc34(&session, &receipt),
          "forged one-line C001 cannot authorize terminal C017/C040");
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].height = 153;
    check(csb_v1_startup_session_terminal_receipt_pc34(&session, &receipt) &&
              receipt.valid,
          "restored real-shaped C001 re-authorizes terminal C017/C040");
    opening_host.raster.door_composited = 0;
    check(!csb_v1_startup_session_title_opening_consumption_receipt_pc34(
              &session, &package_receipt, &presents_host, &chaos_host,
              &strikes_host, &opening_host, &title_opening),
          "opening host without door strips cannot satisfy title/opening consumption");
    opening_host.raster.door_composited = 1;
    opening_host.raster.source_surface_count = 2;
    check(!csb_v1_startup_session_title_opening_consumption_receipt_pc34(
              &session, &package_receipt, &presents_host, &chaos_host,
              &strikes_host, &opening_host, &title_opening),
          "partial opening source count cannot satisfy C004/C002/C003 consumption");
    opening_host.raster.source_surface_count = 3;
    tick = receipt.source_tick;
    generation = receipt.session_generation;
    terminal = receipt;
    check(csb_v1_startup_session_live_hud_receipt_pc34(
              &session, &terminal, 1u, tick, generation, &live_hud) &&
              live_hud.valid && live_hud.c040_cleared_once &&
              live_hud.c017_live_base_only &&
              live_hud.c017_source_asset_id == 17 &&
              live_hud.c017_width == 224 && live_hud.c017_height == 136 &&
              live_hud.special_palette == -1,
          "one C040 clear returns to neutral first-live C017 in the terminal session");
    ++session.source_tick;
    check(csb_v1_startup_session_first_action_receipt_pc34(
              &session, &live_hud, CSB_V1_STARTUP_SESSION_ACTION_LEFT_HAND_PC34,
              session.source_tick, session.generation, &action) && action.valid &&
              action.first_post_live_hud_action &&
              action.command == CSB_V1_STARTUP_SESSION_ACTION_LEFT_HAND_PC34 &&
              action.c017_source_asset_id == 17,
          "first post-live-HUD action uses the same C017 session tick");
    check(!csb_v1_startup_session_first_action_receipt_pc34(
               &session, &live_hud, CSB_V1_STARTUP_SESSION_ACTION_NONE_PC34,
               session.source_tick, session.generation, &action),
          "empty post-live-HUD action cannot cross the C040 handoff");
    check(!csb_v1_startup_session_first_action_receipt_pc34(
               &session, &live_hud, CSB_V1_STARTUP_SESSION_ACTION_CAST_PC34,
               live_hud.source_tick, session.generation, &action),
          "stale post-live-HUD action tick cannot cross the C040 handoff");
    ++session.generation;
    check(!csb_v1_startup_session_first_action_receipt_pc34(
               &session, &live_hud, CSB_V1_STARTUP_SESSION_ACTION_CAST_PC34,
               session.source_tick, session.generation, &action),
          "stale post-live-HUD action generation cannot cross the C040 handoff");
    --session.generation;
    check(csb_v1_startup_session_first_action_receipt_pc34(
              &session, &live_hud, CSB_V1_STARTUP_SESSION_ACTION_CAST_PC34,
              session.source_tick, session.generation, &cast_action),
          "first post-live-HUD CAST action is source-session bound");
    ++session.source_tick;
    check(csb_v1_startup_session_selection_receipt_pc34(
              &session, &live_hud, &cast_action,
              CSB_V1_STARTUP_SESSION_SELECTION_SPELL_RUNE_PC34, 1,
              session.source_tick, session.generation, &selection) &&
              selection.valid &&
              selection.kind == CSB_V1_STARTUP_SESSION_SELECTION_SPELL_RUNE_PC34 &&
              selection.selection_index == 1,
          "first post-live-HUD spell selection follows CAST on the next source tick");
    check(!csb_v1_startup_session_selection_receipt_pc34(
               &session, &live_hud, &cast_action,
               CSB_V1_STARTUP_SESSION_SELECTION_ACTION_SLOT_PC34, 0,
               session.source_tick, session.generation, &selection),
          "CAST cannot select a hand action slot");
    check(!csb_v1_startup_session_selection_receipt_pc34(
               &session, &live_hud, &cast_action,
               CSB_V1_STARTUP_SESSION_SELECTION_SPELL_RUNE_PC34, 7,
               session.source_tick, session.generation, &selection),
          "out-of-range spell rune cannot cross the live HUD gate");
    check(!csb_v1_startup_session_selection_receipt_pc34(
               &session, &live_hud, &cast_action,
               CSB_V1_STARTUP_SESSION_SELECTION_SPELL_RUNE_PC34, 1,
               cast_action.source_tick, session.generation, &selection),
          "stale spell-selection tick cannot cross the live HUD gate");
    ++session.generation;
    check(!csb_v1_startup_session_selection_receipt_pc34(
               &session, &live_hud, &cast_action,
               CSB_V1_STARTUP_SESSION_SELECTION_SPELL_RUNE_PC34, 1,
               session.source_tick, session.generation, &selection),
          "stale spell-selection generation cannot cross the live HUD gate");
    --session.generation;
    session.source_tick = live_hud.source_tick + 1u;
    check(csb_v1_startup_session_first_input_receipt_pc34(
              &session, &live_hud, CSB_V1_STARTUP_SESSION_MOVEMENT_FORWARD_PC34,
              session.source_tick, session.generation, &input) && input.valid &&
              input.first_post_c040_input &&
              input.command == CSB_V1_STARTUP_SESSION_MOVEMENT_FORWARD_PC34,
          "first post-C040 input enters movement through the next live HUD tick");
    check(!csb_v1_startup_session_first_input_receipt_pc34(
               &session, &live_hud, CSB_V1_STARTUP_SESSION_MOVEMENT_NONE_PC34,
               session.source_tick, session.generation, &input),
          "nonmovement input cannot enter the first post-C040 command route");
    check(!csb_v1_startup_session_first_input_receipt_pc34(
               &session, &live_hud, CSB_V1_STARTUP_SESSION_MOVEMENT_FORWARD_PC34,
               live_hud.source_tick, session.generation, &input),
          "stale post-C040 input tick cannot enter movement");
    ++session.generation;
    check(!csb_v1_startup_session_first_input_receipt_pc34(
               &session, &live_hud, CSB_V1_STARTUP_SESSION_MOVEMENT_FORWARD_PC34,
               session.source_tick, session.generation, &input),
          "stale post-C040 input generation cannot enter movement");
    --session.generation;
    check(csb_v1_startup_session_first_door_hud_tick_receipt_pc34(
              &session, &live_hud, 0u, 1u, session.source_tick,
              session.generation, &door_tick) && door_tick.valid &&
              door_tick.first_live_door_tick && door_tick.previous_door_step == 0u &&
              door_tick.door_step == 1u,
          "first post-C040 HUD tick starts the runtime door at monotonic step one");
    ++session.source_tick;
    check(csb_v1_startup_session_first_door_hud_tick_receipt_pc34(
              &session, &live_hud, 1u, 2u, session.source_tick,
              session.generation, &door_tick) && door_tick.valid &&
              !door_tick.first_live_door_tick,
          "subsequent live HUD door tick remains monotonic in the same session");
    check(!csb_v1_startup_session_first_door_hud_tick_receipt_pc34(
               &session, &live_hud, 1u, 3u, session.source_tick,
               session.generation, &door_tick),
          "door-step jump cannot reach the live HUD route");
    check(!csb_v1_startup_session_first_door_hud_tick_receipt_pc34(
               &session, &live_hud, 2u, 2u, session.source_tick,
               session.generation, &door_tick),
          "replayed door step cannot reach the live HUD route");
    session.source_tick = live_hud.source_tick;
    check(!csb_v1_startup_session_first_door_hud_tick_receipt_pc34(
               &session, &live_hud, 0u, 1u, session.source_tick,
               session.generation, &door_tick),
          "stale post-C040 HUD tick cannot start the runtime door");
    ++session.source_tick;
    ++session.generation;
    check(!csb_v1_startup_session_first_door_hud_tick_receipt_pc34(
               &session, &live_hud, 0u, 1u, session.source_tick,
               session.generation, &door_tick),
          "stale post-C040 session generation cannot start the runtime door");
    --session.generation;
    check(!csb_v1_startup_session_live_hud_receipt_pc34(
               &session, &terminal, 0u, tick, generation, &live_hud),
          "missing C040 clear cannot enter the first live HUD");
    check(!csb_v1_startup_session_live_hud_receipt_pc34(
               &session, &terminal, 2u, tick, generation, &live_hud),
          "multiple C040 clears cannot enter the first live HUD");
    check(!csb_v1_startup_session_live_hud_receipt_pc34(
               &session, &terminal, 1u, tick + 1u, generation, &live_hud),
          "stale C040-to-C017 tick cannot enter the first live HUD");
    ++session.source_tick;
    check(csb_v1_startup_session_terminal_receipt_pc34(&session, &receipt) &&
              receipt.source_tick != tick && receipt.session_generation == generation,
          "later source tick cannot match an earlier terminal authorization");
    check(!csb_v1_startup_session_live_hud_receipt_pc34(
               &session, &terminal, 1u, tick, generation, &live_hud),
          "session tick drift rejects C040-to-C017 live handoff");
    ++session.generation;
    check(csb_v1_startup_session_terminal_receipt_pc34(&session, &receipt) &&
              receipt.session_generation != generation,
          "later session generation cannot match an earlier terminal authorization");
    check(!csb_v1_startup_session_live_hud_receipt_pc34(
               &session, &terminal, 1u, tick, generation, &live_hud),
          "session generation drift rejects C040-to-C017 live handoff");
    session.playback.entrance_complete = 0;
    check(!csb_v1_startup_session_terminal_receipt_pc34(&session, &receipt) &&
              !receipt.valid,
          "nonterminal F0807 state fails closed before C017 authorization");
    printf("CSB startup session contract: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
