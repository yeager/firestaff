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
    state->assetStatus.versions[1][0].gameId = "csb";
    state->assetStatus.versions[1][0].versionId = "atari-st-v20";
    state->assetStatus.versions[1][0].label = "Atari ST 2.0";
    state->assetStatus.versions[1][0].shortLabel = "ST 2.0";
    state->assetStatus.versions[1][0].matched = 1;
    state->gameOptions[1].versionIndex = 0;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
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
    const int entryCount = M12_StartupMenu_GetEntryCount();
    const int cardCount = entryCount + 1; /* brand card + entries */
    const int cardW = (1920 - 2 * 48 - 22 * (cardCount - 1)) / cardCount;
    const int csbCardCenterX = 48 + 2 * (cardW + 22) + cardW / 2;
    const int cardCenterY = (170 + (1080 - 130)) / 2;
    const int launchCenterX = 960;
    const int launchCenterY = 669;

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&state);

    changed = M12_ModernMenu_HandlePointer(&state, csbCardCenterX, cardCenterY, 1, NULL);
    if (!expect(changed == 1, "CSB card direct click should change menu state")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS, "CSB card direct click should enter game options")) return 1;
    if (!expect(state.activatedIndex == 1, "CSB direct click should activate CSB")) return 1;
    if (!render_smoke_nonblank(&state, "CSB options")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, launchCenterX, launchCenterY, 1, NULL);
    if (!expect(changed == 1, "CSB Launch direct click should be handled")) return 1;
    if (!expect(state.launchRequested == 0, "CSB hash-matched assets must not request runtime launch yet")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_MESSAGE, "CSB launch blocker should show message")) return 1;
    if (!expect(state.messageLine1 && strcmp(state.messageLine1, "RUNTIME NOT READY") == 0,
                "CSB launch blocker should explain runtime is not ready")) return 1;
    if (!render_smoke_nonblank(&state, "CSB blocker message")) return 1;

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 0, "CSB launch intent remains invalid even when version is matched")) return 1;
    if (!expect(intent.gameId && strcmp(intent.gameId, "csb") == 0, "CSB intent should still identify CSB for diagnostics")) return 1;

    puts("ok: CSB V1 hash-matched launcher path renders options/blocker views and stays blocked");
    puts("sourceEvidence=ReDMCSB CSB startup/utility payload strings require dungeon/graphics plus CSB save-game handling; menu_startup_m12.c DM1-only handoff guard");
    return 0;
}
