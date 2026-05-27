#include "menu_hit_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>

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

int main(void) {
    M12_StartupMenuState state;
    M12_MouseHit hit;
    int changed;
    const int gridLeft = 42 + 390 + 44;
    const int cardW = (1920 - gridLeft - 48 - 22 * 2) / 3;
    const int cardH = ((1080 - 130) - 152 - 22) / 2;
    const int dm1CardCenterX = gridLeft + cardW / 2;
    const int cardCenterY = 152 + cardH / 2;
    const int launchCenterX = 960;
    const int launchCenterY = 270 + 560 - 54 - 24 + 27;

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&state);

    changed = M12_ModernMenu_HandlePointer(&state, dm1CardCenterX, cardCenterY, 0, NULL);
    if (!expect(changed == 0 || state.selectedIndex == 0, "DM1 hover should keep/select DM1 card")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, dm1CardCenterX, cardCenterY, 1, NULL);
    if (!expect(changed == 1, "DM1 card direct click should change menu state")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS, "DM1 card direct click should enter game options")) return 1;
    if (!expect(state.activatedIndex == 0, "DM1 direct click should activate DM1")) return 1;

    hit = M12_ModernMenu_HitTest(&state, launchCenterX, launchCenterY);
    if (!expect(hit.kind == M12_HIT_GAMEOPT_LAUNCH, "visible centered V1 Launch button should hit launch action")) return 1;

    changed = M12_ModernMenu_HandlePointer(&state, launchCenterX, launchCenterY, 1, NULL);
    if (!expect(changed == 1, "Launch direct click should be applied")) return 1;
    if (!expect(state.launchRequested == 1, "Launch direct click should request launch")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_MESSAGE, "Launch direct click should show ready-to-launch message")) return 1;

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    {
        const int settingsCenterX = gridLeft + 2 * (cardW + 22) + cardW / 2;
        const int settingsCenterY = 152 + cardH + 22 + cardH / 2;
        changed = M12_ModernMenu_HandlePointer(&state, settingsCenterX, settingsCenterY, 0, NULL);
        if (!expect(changed == 1 && state.selectedIndex == 6, "Firestaff hover should navigate to global settings card")) return 1;
        changed = M12_ModernMenu_HandlePointer(&state, settingsCenterX, settingsCenterY, 1, NULL);
        if (!expect(changed == 1 && state.view == M12_MENU_VIEW_SETTINGS, "Firestaff click should open settings view")) return 1;
    }

    puts("ok: mouse hover navigates main cards; clicks open DM1, Firestaff settings and launch DM1");
    return 0;
}
