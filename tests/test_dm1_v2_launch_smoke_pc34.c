#include "dm1_v2_movement_command_adapter_pc34.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static void force_dm1_available(M12_StartupMenuState* state) {
    state->entries[0].title = "DUNGEON MASTER";
    state->entries[0].gameId = "dm1";
    state->entries[0].kind = M12_MENU_ENTRY_GAME;
    state->entries[0].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[0].available = 1;
    state->assetStatus.dm1Available = 1;
    state->assetStatus.versions[0][0].gameId = "dm1";
    state->assetStatus.versions[0][0].versionId = "pc34-en";
    state->assetStatus.versions[0][0].label = "PC 3.4 English";
    state->assetStatus.versions[0][0].shortLabel = "PC 3.4 EN";
    state->assetStatus.versions[0][0].matched = 1;
    state->gameOptions[0].versionIndex = 0;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->activatedIndex = 0;
    state->launchRequested = 1;
}

int main(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    DM1_V2_RuntimeState runtime;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandRoute sourceRoute;
    DM1_V2_MovementCommandResult move;

    M12_StartupMenu_InitWithDataDir(&menu, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&menu);

    /* Phase 1 launch-smoke source anchors:
     * ReDMCSB GAMELOOP.C:164-219 drives one queued command per input wait,
     * COMMAND.C:2045-2155 dispatches source command ids, and DEFS.H:238-243
     * owns those ids. This smoke only opts into V2 presentation after the
     * M12 launch intent; the V1/source route remains inspectable and unchanged. */
    menu.settings.graphicsIndex = M12_PRESENTATION_V21_UPSCALED;
    menu.gameOptions[0].presentationModeIndex = M12_PRESENTATION_V21_UPSCALED;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 1);
    CHECK(intent.gameId && strcmp(intent.gameId, "dm1") == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, "pc34-en") == 0);
    CHECK(intent.presentationMode == M12_PRESENTATION_V21_UPSCALED);
    CHECK(intent.rendererBackendAvailable == 1);
    CHECK(intent.options.presentationModeIndex == M12_PRESENTATION_V21_UPSCALED);
    CHECK(intent.savePath == NULL);

    sourceRoute = dm1_v2_movement_command_route_for_presentation(0, DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD);
    CHECK(sourceRoute.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
    CHECK(sourceRoute.sourceCommand == 3);
    CHECK(sourceRoute.runtimeCommand == 3);

    dm1_v2_runtime_init(&runtime);
    dm1_v2_camera_init(&camera, &runtime.player);
    dm1_v2_runtime_start(&runtime, 1000U);
    CHECK(dm1_v2_runtime_is_running(&runtime));
    CHECK(dm1_v2_vp_is_dirty(&runtime.viewport));

    dm1_v2_runtime_tick(&runtime, 1016U);
    CHECK(runtime.tickCount == 1U);
    CHECK(runtime.viewport.frameCount == 1);
    CHECK(runtime.viewport.lastRenderMs == 1016);
    CHECK(!dm1_v2_vp_is_dirty(&runtime.viewport));

    move = dm1_v2_movement_command_apply(&runtime, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 1032U, 64);
    CHECK(move.accepted == 1);
    CHECK(move.sourceCommand == 3);
    CHECK(move.runtimeCommand == 1);
    CHECK(runtime.lastCommand == 1);
    CHECK(dm1_v2_has_moved(&runtime.player));
    CHECK(dm1_v2_camera_is_active(&camera));

    dm1_v2_runtime_stop(&runtime);
    CHECK(!dm1_v2_runtime_is_running(&runtime));

    menu.settings.graphicsIndex = M12_PRESENTATION_V22_MODERN;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 0);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_launch_smoke_pc34: ok");
    return 0;
}
