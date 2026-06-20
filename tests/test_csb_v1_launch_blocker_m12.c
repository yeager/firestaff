#include "menu_hit_m12.h"
#include "menu_startup_m12.h"
#include "menu_startup_render_modern_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void force_csb_available(M12_StartupMenuState* state) {
    state->entries[1].title = "CHAOS STRIKES BACK";
    state->entries[1].gameId = "csb";
    state->entries[1].kind = M12_MENU_ENTRY_GAME;
    state->entries[1].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[1].available = 1;
    /* Reset to MAIN view: M12_StartupMenu_InitWithDataDir triggers a
     * "no game data" popup that overlays the main view, which would
     * intercept clicks as MESSAGE_DISMISS instead of MAIN_CARD. */
    state->view = M12_MENU_VIEW_MAIN;
    state->selectedIndex = 0;
    state->assetStatus.csbAvailable = 1;
    state->assetStatus.versions[1][0].gameId = "csb";
    state->assetStatus.versions[1][0].versionId = "atari-st-v20";
    state->assetStatus.versions[1][0].label = "Atari ST 2.0";
    state->assetStatus.versions[1][0].shortLabel = "ST 2.0";
    state->assetStatus.versions[1][0].matched = 1;
    state->assetStatus.requiredFileCounts[1] = 2;
    state->assetStatus.requiredFiles[1][0].gameId = "csb";
    state->assetStatus.requiredFiles[1][0].roleId = "graphics";
    state->assetStatus.requiredFiles[1][0].label = "GRAPHICS.DAT";
    state->assetStatus.requiredFiles[1][0].required = 1;
    state->assetStatus.requiredFiles[1][0].matched = 1;
    state->assetStatus.requiredFiles[1][1].gameId = "csb";
    state->assetStatus.requiredFiles[1][1].roleId = "dungeon";
    state->assetStatus.requiredFiles[1][1].label = "DUNGEON.DAT";
    state->assetStatus.requiredFiles[1][1].required = 1;
    state->assetStatus.requiredFiles[1][1].matched = 1;
    state->gameOptions[1].versionIndex = 0;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
}

static void force_csb_version_only_missing_required(M12_StartupMenuState* state) {
    force_csb_available(state);
    state->assetStatus.csbAvailable = 0;
    state->assetStatus.requiredFiles[1][0].matched = 0;
    state->assetStatus.requiredFiles[1][1].matched = 0;
    state->settings.graphicsIndex = M12_PRESENTATION_V21_UPSCALED;
    state->gameOptions[1].presentationModeIndex = M12_PRESENTATION_V21_UPSCALED;
}

static int expect(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 0;
    }
    return 1;
}

static int render_smoke_nonblank(const M12_StartupMenuState* state, const char* label) {
    const int w = M12_ModernMenu_NativeWidth();
    const int h = M12_ModernMenu_NativeHeight();
    const size_t bytes = (size_t)w * (size_t)h * 4u;
    unsigned char* rgba = (unsigned char*)malloc(bytes);
    int distinct;
    if (!rgba) {
        fprintf(stderr, "FAIL: %s render buffer allocation\n", label);
        return 0;
    }
    memset(rgba, 0, bytes);
    M12_ModernMenu_Render(state, rgba, w, h);
    distinct = M12_ModernMenu_CountDistinctColors(rgba, w, h, 128);
    free(rgba);
    if (distinct < 3) {
        fprintf(stderr, "FAIL: %s render should produce nonblank startup/menu pixels\n", label);
        return 0;
    }
    return 1;
}

int main(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    int changed;
    const int gridLeft = 42 + 390 + 44;
    const int cardW = (1920 - gridLeft - 48 - 22 * 2) / 3;
    const int cardH = ((1080 - 130) - 40 - 22) / 2;
    const int csbCardCenterX = gridLeft + 1 * (cardW + 22) + cardW / 2;
    const int cardCenterY = 40 + cardH / 2;
    const int launchCenterX = 960;
    const int launchCenterY = 190 + 780 - 54 - 24 + 27;

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&state);

    changed = M12_ModernMenu_HandlePointer(&state, csbCardCenterX, cardCenterY, 1, NULL);
    if (!expect(changed == 1, "CSB card direct click should change menu state")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS, "CSB card direct click should enter game options")) return 1;
    if (!expect(state.activatedIndex == 1, "CSB direct click should activate CSB")) return 1;
    if (!render_smoke_nonblank(&state, "CSB options")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, launchCenterX, launchCenterY, 1, NULL);
    if (!expect(changed == 1, "CSB Launch direct click should be handled")) return 1;
    if (!expect(state.launchRequested == 1, "CSB hash-matched assets should request runtime launch")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_MESSAGE, "CSB launch blocker should show message")) return 1;
    if (!expect(state.messageLine1 && strcmp(state.messageLine1, "RUNTIME NOT READY") != 0,
                "CSB launch transition should not report the old runtime blocker")) return 1;
    if (!render_smoke_nonblank(&state, "CSB ready message")) return 1;

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1, "CSB launch intent is valid when version is matched")) return 1;
    if (!expect(intent.gameId && strcmp(intent.gameId, "csb") == 0, "CSB intent should still identify CSB for diagnostics")) return 1;

    puts("ok: CSB V1 hash-matched launcher path renders options/ready views and exposes a launch intent");
    puts("sourceEvidence=ReDMCSB ENTRANCE.C F0806 launch state and LOADSAVE.C F0435 new-game load boundary");

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_version_only_missing_required(&state);

    changed = M12_ModernMenu_HandlePointer(&state, csbCardCenterX, cardCenterY, 1, NULL);
    if (!expect(changed == 1, "CSB version-only fixture should still open options for regression coverage")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS, "CSB version-only fixture enters game options")) return 1;
    if (!expect(state.gameOptions[1].presentationModeIndex == M12_PRESENTATION_V21_UPSCALED,
                "CSB version-only fixture keeps the V2.1 presentation selection")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, launchCenterX, launchCenterY, 1, NULL);
    if (!expect(changed == 1, "CSB V2.1 launch click should be handled")) return 1;
    if (!expect(state.launchRequested == 0,
                "CSB V2.1 version match must not bypass missing required-file gating")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_MESSAGE,
                "CSB V2.1 missing required data shows a message instead of launching")) return 1;
    if (!expect(state.messageLine2 && strstr(state.messageLine2, "GRAPHICS.DAT") &&
                strstr(state.messageLine2, "DUNGEON.DAT"),
                "CSB V2.1 missing-data popup names both required V1 runtime files")) return 1;

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 0,
                "CSB V2.1 version-only launch intent is invalid without required files")) return 1;

    puts("ok: CSB V2 presentation selection does not bypass CSB required-file launch gating");
    puts("sourceEvidence=ReDMCSB ENTRANCE.C F0806 selects CSB media before LOADSAVE.C F0435 dungeon load");
    return 0;
}
