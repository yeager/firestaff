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
    CSB_V1_StartupRuntimeSurface_PC34 *c017;
    CSB_V1_StartupRuntimeSurface_PC34 *c040;

    memset(session, 0, sizeof(*session));
    session->valid = session->real_asset_matched = 1;
    session->title_presents_ready = session->title_chaos_ready = 1;
    session->title_strikes_back_ready = session->entrance_assets_ready = 1;
    session->door_assets_ready = 1;
    session->rejects_legacy_wrappers = session->hud_assets_bound = 1;
    session->generation = 9u;
    session->source_tick = 413u;
    session->playback.no_fallback_routes = 1;
    session->playback.title_phase_mask = 0x0b;
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34;
    session->playback.entrance_complete = 1;
    c017 = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
    c040 = &session->surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34];
    c017->valid = 1; c017->pixels = &c017_pixel; c017->source_asset_id = 17;
    c017->width = 224; c017->height = 136;
    c017->transparent_color = -1;
    c040->valid = 1; c040->pixels = &c040_pixel; c040->source_asset_id = 40;
    c040->width = 144; c040->height = 73;
    c040->transparent_color = 6;
}

int main(void)
{
    CSB_V1_StartupRenderState_PC34 state;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupSessionTerminalReceipt_PC34 receipt;
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
    check(csb_v1_startup_session_terminal_receipt_pc34(&session, &receipt) &&
              receipt.valid && receipt.c001_complete &&
              receipt.terminal_f0807_complete && receipt.c017_ready &&
              receipt.c040_ready,
          "terminal F0807 authorizes C017/C040 only in the terminal session");
    tick = receipt.source_tick;
    generation = receipt.session_generation;
    ++session.source_tick;
    check(csb_v1_startup_session_terminal_receipt_pc34(&session, &receipt) &&
              receipt.source_tick != tick && receipt.session_generation == generation,
          "later source tick cannot match an earlier terminal authorization");
    ++session.generation;
    check(csb_v1_startup_session_terminal_receipt_pc34(&session, &receipt) &&
              receipt.session_generation != generation,
          "later session generation cannot match an earlier terminal authorization");
    session.playback.entrance_complete = 0;
    check(!csb_v1_startup_session_terminal_receipt_pc34(&session, &receipt) &&
              !receipt.valid,
          "nonterminal F0807 state fails closed before C017 authorization");
    printf("CSB startup session contract: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
