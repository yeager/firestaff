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
               strcmp(input_route.status, "blocked-menu-bpk") == 0,
           "blocked title input route reports the launcher asset blocker");
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
               strcmp(pointer_route.status, "blocked-menu-bpk") == 0,
           "blocked title pointer route reports the launcher asset blocker");

    expect(nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
               &state,
               9,
               &handoff) == 1,
           "title handoff receipt is still produced");
    expect(handoff.route_blocked &&
               handoff.title_route.route ==
                   NEXUS_V1_STARTUP_TITLE_ROUTE_ASSET_BLOCKED &&
               handoff.status &&
               strcmp(handoff.status, "blocked-menu-bpk") == 0,
           "handoff consumes the blocked route receipt");

    if (failures) {
        fprintf(stderr,
                "Nexus startup title route asset gate: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus startup title route asset gate passed");
    return 0;
}
