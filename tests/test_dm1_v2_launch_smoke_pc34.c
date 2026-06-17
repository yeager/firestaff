#include "dm1_v2_movement_command_adapter_pc34.h"
#include "config_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static int m12_test_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value);
#else
    return setenv(name, value, 1);
#endif
}

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
    state->view = M12_MENU_VIEW_MAIN;
}

static int write_fake_quicksave(const char* path) {
    static const unsigned char hdr[16] = {
        'F','S','M','1','1','Q','S','1',
        4,0,0,0,
        0,0,0,0
    };
    static const unsigned char blob[4] = { 0, 0, 0, 0 };
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    if (fwrite(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr) ||
        fwrite(blob, 1U, sizeof(blob), fp) != sizeof(blob) ||
        fclose(fp) != 0) {
        return 0;
    }
    return 1;
}

int main(void) {
    char tmpTemplate[] = "/tmp/firestaff-dm1-v2-launch-XXXXXX";
    char savePath[512];
    M12_StartupMenuState menu;
    M12_StartupMenuState reloaded;
    M12_LaunchIntent intent;
    DM1_V2_RuntimeState runtime;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandRoute sourceRoute;
    DM1_V2_MovementCommandResult move;

    if (!mkdtemp(tmpTemplate)) {
        perror("mkdtemp");
        return 1;
    }
    m12_test_setenv("HOME", tmpTemplate);
    M12_Config_SetLastSavePath("");

    M12_StartupMenu_InitWithDataDir(&menu, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&menu);

    /* Phase 1 launch-smoke source anchors:
     * ReDMCSB GAMELOOP.C:164-219 drives one queued command per input wait,
     * COMMAND.C:2045-2155 dispatches source command ids, and DEFS.H:238-243
     * owns those ids. This smoke only opts into V2 presentation after the
     * M12 launch intent; the V1/source route remains inspectable and unchanged. */
    menu.settings.graphicsIndex = M12_PRESENTATION_V21_UPSCALED;
    menu.gameOptions[0].presentationModeIndex = M12_PRESENTATION_V21_UPSCALED;
    menu.gameOptions[0].resolution = M12_RES_3840x2160;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 1);
    CHECK(intent.gameId && strcmp(intent.gameId, "dm1") == 0);
    CHECK(intent.versionId && strcmp(intent.versionId, "pc34-en") == 0);
    CHECK(intent.presentationMode == M12_PRESENTATION_V21_UPSCALED);
    CHECK(intent.options.resolution == M12_RES_3840x2160);
    CHECK(intent.resolutionWidth == 3840);
    CHECK(intent.resolutionHeight == 2160);
    CHECK(intent.rendererBackendAvailable == 1);
    CHECK(intent.options.presentationModeIndex == M12_PRESENTATION_V21_UPSCALED);
    CHECK(intent.savePath == NULL);

    M12_StartupMenu_SaveConfig(&menu);
    M12_StartupMenu_InitWithDataDir(&reloaded, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&reloaded);
    CHECK(reloaded.settings.graphicsIndex == M12_PRESENTATION_V21_UPSCALED);
    CHECK(reloaded.gameOptions[0].resolution == M12_RES_3840x2160);
    intent = M12_StartupMenu_GetLaunchIntent(&reloaded);
    CHECK(intent.valid == 1);
    CHECK(intent.savePath == NULL);
    CHECK(intent.options.resolution == M12_RES_3840x2160);
    CHECK(intent.resolutionWidth == 3840);
    CHECK(intent.resolutionHeight == 2160);

    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-quicksave.sav", tmpTemplate);
    CHECK(write_fake_quicksave(savePath));
    M12_Config_SetLastSavePath(savePath);
    M12_StartupMenu_InitWithDataDir(&reloaded, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&reloaded);
    CHECK(reloaded.quickResumeAvailable == 1);
    CHECK(strcmp(reloaded.quickResumeSavePath, savePath) == 0);
    reloaded.launchRequested = 0;
    reloaded.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&reloaded, M12_MENU_INPUT_ACCEPT);
    CHECK(reloaded.quickResumeLaunchRequested == 1);
    CHECK(reloaded.activatedIndex == 0);
    intent = M12_StartupMenu_GetLaunchIntent(&reloaded);
    CHECK(intent.valid == 1);
    CHECK(intent.savePath && strcmp(intent.savePath, savePath) == 0);
    CHECK(intent.options.resolution == M12_RES_3840x2160);
    CHECK(intent.resolutionWidth == 3840);
    CHECK(intent.resolutionHeight == 2160);

    menu.settings.graphicsIndex = M12_PRESENTATION_V20_FILTERED;
    menu.gameOptions[0].presentationModeIndex = M12_PRESENTATION_V20_FILTERED;
    menu.gameOptions[0].resolution = M12_RES_3840x2160;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 1);
    /* V20 allows the user-chosen 3840x2160 (above the 640x400
     * launch floor); the floor only promotes stored 320x200
     * resolutions, never clamps higher resolutions down. */
    CHECK(intent.options.resolution == M12_RES_3840x2160);
    CHECK(intent.resolutionWidth == 3840);
    CHECK(intent.resolutionHeight == 2160);

    M12_StartupMenu_SaveConfig(&menu);
    M12_StartupMenu_InitWithDataDir(&reloaded, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&reloaded);
    intent = M12_StartupMenu_GetLaunchIntent(&reloaded);
    CHECK(intent.valid == 1);
    CHECK(intent.options.resolution == M12_RES_3840x2160);
    CHECK(intent.resolutionWidth == 3840);
    CHECK(intent.resolutionHeight == 2160);

    /* V20 stored at 320x200: floor promotes to 640x400 (4d228162 contract). */
    menu.gameOptions[0].resolution = M12_RES_320x200;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 1);
    CHECK(intent.options.resolution == M12_RES_640x400);
    CHECK(intent.resolutionWidth == 640);
    CHECK(intent.resolutionHeight == 400);

    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    menu.gameOptions[0].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    menu.gameOptions[0].resolution = M12_RES_3840x2160;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 1);
    CHECK(intent.options.resolution == M12_RES_320x200);
    CHECK(intent.resolutionWidth == 320);
    CHECK(intent.resolutionHeight == 200);

    M12_StartupMenu_SaveConfig(&menu);
    M12_StartupMenu_InitWithDataDir(&reloaded, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&reloaded);
    intent = M12_StartupMenu_GetLaunchIntent(&reloaded);
    CHECK(intent.valid == 1);
    CHECK(intent.options.resolution == M12_RES_320x200);
    CHECK(intent.resolutionWidth == 320);
    CHECK(intent.resolutionHeight == 200);

    menu.settings.graphicsIndex = M12_PRESENTATION_V21_UPSCALED;
    menu.gameOptions[0].presentationModeIndex = M12_PRESENTATION_V21_UPSCALED;
    menu.gameOptions[0].resolution = M12_RES_320x200;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 1);
    /* Launch-res floor (4d228162 contract): V2.1 stored at 320x200
     * promotes to 640x400 at launch, since the actual upscaled render
     * path requires >=640x400.  The row cycle in
     * m12_enforce_mode_constraints stays full (INV_M12_18
     * 320->640->1280->...); the floor only lives in GetLaunchIntent. */
    CHECK(intent.options.resolution == M12_RES_640x400);
    CHECK(intent.resolutionWidth == 640);
    CHECK(intent.resolutionHeight == 400);

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
    menu.gameOptions[0].presentationModeIndex = M12_PRESENTATION_V22_MODERN;
    menu.gameOptions[0].resolution = M12_RES_2560x1440;
    M12_StartupMenu_SaveConfig(&menu);
    M12_StartupMenu_InitWithDataDir(&reloaded, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&reloaded);
    CHECK(reloaded.settings.graphicsIndex == M12_PRESENTATION_V22_MODERN);
    CHECK(reloaded.gameOptions[0].resolution == M12_RES_2560x1440);
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid == 0);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_launch_smoke_pc34: ok");
    return 0;
}
