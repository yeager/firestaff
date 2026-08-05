#include "nexus_v1_launcher.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void build_title_ready_menu_blocked_state(
    Nexus_V1_Engine *engine,
    Nexus_V1_StartupRuntimeState *state)
{
    static unsigned char title_pixel;
    static unsigned char warning_pixel;

    memset(engine, 0, sizeof(*engine));
    memset(state, 0, sizeof(*state));
    engine->level_loaded = 1;
    engine->ui.surfaces[NEXUS_SURFACE_TITLE].data = &title_pixel;
    engine->ui.surfaces[NEXUS_SURFACE_TITLE].w = 1;
    engine->ui.surfaces[NEXUS_SURFACE_TITLE].h = 1;
    engine->ui.surfaces[NEXUS_SURFACE_TITLE].source =
        "TITLE.CG/4bpp-atlas";
    engine->ui.surfaces[NEXUS_SURFACE_WARNING].data = &warning_pixel;
    engine->ui.surfaces[NEXUS_SURFACE_WARNING].w = 1;
    engine->ui.surfaces[NEXUS_SURFACE_WARNING].h = 1;
    engine->ui_startup_surfaces_expected = 2;
    engine->ui_startup_surfaces_loaded = 2;
    engine->ui_faces_expected = 1;
    engine->ui_faces_loaded = 1;
    engine->sfx_runtime_receipt.status = NEXUS_SFX_RUNTIME_READY_DECODED;
    engine->sfx_runtime_receipt.level_index = 0;
    engine->sfx_runtime_receipt.cd_track = 2;

    state->engine = engine;
    state->title_active = 1;
    state->title_frame = 200;
    state->slot_mask = 1u;
    state->save_selected_row = 0;
}

static void check_asset_handoff_blocks_saturn_route_on_menu_prs3(void)
{
    Nexus_V1_Engine engine;
    Nexus_V1_LauncherRuntimeReceipt runtime;
    Nexus_V1_StartupAssetHandoffReceipt handoff;

    memset(&engine, 0, sizeof(engine));
    memset(&runtime, 0, sizeof(runtime));
    engine.level_loaded = 1;
    engine.menu_bpk_decode_receipt_attempted = 1;
    engine.menu_bpk_decode_receipt_valid = 1;
    engine.menu_bpk_decode_receipt.route =
        NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3;
    engine.menu_bpk_decode_receipt.archive_entries = 163;
    engine.menu_bpk_decode_receipt.surface_entries = 162;
    engine.menu_bpk_decode_receipt.blocked_prs3_surfaces = 162U;

    runtime.engine = &engine;
    runtime.level_loaded = 1;
    runtime.title_loaded = 1;
    runtime.startup_assets.title_route_ready = 1;
    runtime.startup_assets.startup_audio_handoff_ready = 1;
    runtime.startup_assets.real_menu_surface_route_ready = 0;
    runtime.startup_assets.real_menu_surface_route_blocked = 1;
    runtime.startup_assets.startup_menu_asset_route =
        "menu-bpk-prs3-capture-required";
    runtime.startup_assets.real_menu_surface_blocker =
        "menu-bpk-prs3-capture-required";

    expect(nexus_v1_launcher_startup_asset_handoff_from_runtime_receipt(
               &runtime,
               &handoff) == 1,
           "asset handoff receipt is produced for PRS3-blocked MENU.BPK");
    expect(handoff.route == NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED,
           "PRS3-blocked MENU.BPK keeps asset handoff menu-blocked");
    expect(handoff.menu_bpk_renderer_handoff_valid == 1,
           "asset handoff consumes renderer handoff evidence");
    expect(handoff.menu_bpk_prs3_blocks_real_menu_route == 1,
           "asset handoff records PRS3 as the real-menu blocker");
    expect(!handoff.real_menu_asset_handoff_ready,
           "PRS3-blocked MENU.BPK cannot ready real-menu assets");
    expect(!handoff.main_menu_route_ready,
           "PRS3-blocked MENU.BPK cannot ready the main menu route");
    expect(!handoff.saturn_asset_handoff_ready,
           "PRS3-blocked MENU.BPK cannot ready the Saturn asset handoff");
    expect(!handoff.real_asset_route_ready,
           "PRS3-blocked MENU.BPK cannot ready the real asset route");
    expect(handoff.blocks_main_menu_route,
           "PRS3-blocked MENU.BPK blocks the host main-menu route");
    expect(handoff.status &&
               strcmp(handoff.status, "menu-bpk-prs3-capture-required") == 0,
           "asset handoff reports the PRS3 capture requirement");
}

int main(void)
{
    Nexus_V1_Engine engine;
    Nexus_V1_StartupRuntimeState state;
    Nexus_V1_StartupTitleRouteReceipt input_route;
    Nexus_V1_StartupTitleRouteReceipt pointer_route;
    Nexus_V1_StartupTitleHandoffReceipt handoff;

    build_title_ready_menu_blocked_state(&engine, &state);

    expect(nexus_v1_launcher_startup_title_route_receipt_from_runtime_state(
               &state,
               9,
               &input_route) == 1,
           "title input route receipt is produced");
    expect(input_route.route == NEXUS_V1_STARTUP_TITLE_ROUTE_ASSET_BLOCKED,
           "accepted title input cannot advertise save-select while menu assets are blocked");
    expect(input_route.host_input_result == NEXUS_V1_STARTUP_HOST_INPUT_REDRAW,
           "blocked title input route requests a redraw");
    expect(strcmp(nexus_v1_startup_title_route_name(input_route.route),
                  "asset-blocked") == 0,
           "blocked title route has a stable name");
    expect(input_route.status_scope &&
               strcmp(input_route.status_scope, "ASSETS") == 0,
           "blocked title input route reports asset scope");
    expect(input_route.status &&
               strcmp(input_route.status, "blocked-title-vdp-capture") == 0,
           "raw TITLE.CG route reports the Saturn capture blocker");
    expect(!input_route.set_save_select_active &&
               !input_route.set_champion_select_active,
           "blocked title input route does not open startup submenus");

    expect(nexus_v1_launcher_startup_title_pointer_route_receipt_from_runtime_state(
               &state,
               &pointer_route) == 1,
           "title pointer route receipt is produced");
    expect(pointer_route.route == NEXUS_V1_STARTUP_TITLE_ROUTE_ASSET_BLOCKED,
           "title pointer cannot advertise save-select while menu assets are blocked");
    expect(pointer_route.status &&
               strcmp(pointer_route.status, "blocked-title-vdp-capture") == 0,
           "blocked title pointer route reports the Saturn capture blocker");

    expect(nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
               &state,
               9,
               &handoff) == 1,
           "title handoff receipt is still produced");
    expect(handoff.route_blocked &&
               handoff.title_route.route ==
                   NEXUS_V1_STARTUP_TITLE_ROUTE_ASSET_BLOCKED &&
               handoff.status &&
               strcmp(handoff.status, "blocked-title-vdp-capture") == 0,
           "handoff consumes the raw-title capture blocker");

    check_asset_handoff_blocks_saturn_route_on_menu_prs3();

    if (failures) {
        fprintf(stderr,
                "Nexus startup title route asset gate: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus startup title route asset gate passed");
    return 0;
}
