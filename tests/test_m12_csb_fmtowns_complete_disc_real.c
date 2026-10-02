#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char* message) {
    if (!condition) fprintf(stderr, "FAIL: %s\n", message);
    return condition;
}

int main(int argc, char** argv) {
    M12_StartupMenuState state;
    M12_StartupMenuInitOptions options;
    M12_LaunchIntent intent;
    const M12_AssetVersionStatus* version;
    int index;
    int ok = 1;
    if (argc != 3) {
        fprintf(stderr, "usage: %s MIXED_ROOT ARCHIVE_BASENAME\n", argv[0]);
        return 2;
    }
    memset(&options, 0, sizeof(options));
    options.scanAllGames = 1;
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&state, argv[1], "csb", &options);
    ok &= check(state.selectedIndex == 1 && state.entries[1].available,
                "CSB card is selected and available from original media");
    index = state.gameOptions[1].versionIndex;
    version = index >= 0 ? M12_AssetStatus_GetVersion(&state.assetStatus,
                                                       "csb", (size_t)index) : NULL;
    ok &= check(version && version->matched && version->versionId &&
                strcmp(version->versionId, "fmtowns-en") == 0 &&
                strstr(version->matchedPath, argv[2]) &&
                strstr(version->matchedPath, "::CDATA/GRAPHICS.DAT"),
                "AUTO binds the complete English CD image, not loose CDATA");
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    ok &= check(state.view == M12_MENU_VIEW_GAME_OPTIONS &&
                state.activatedIndex == 1 && state.gameCardFlowStage == 0,
                "CSB card opens platform selection");
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    ok &= check(state.gameCardFlowStage == 1 &&
                state.gameOptions[1].architectureIndex == M12_ARCH_FM_TOWNS,
                "FM Towns platform advances to presentation selection");
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    ok &= check(state.launchRequested && intent.valid && intent.gameId &&
                strcmp(intent.gameId, "csb") == 0 && intent.versionId &&
                strcmp(intent.versionId, "fmtowns-en") == 0,
                "Original card yields valid CSB FM Towns launch intent");
    M12_StartupMenu_Destroy(&state);
    if (ok) puts("PASS: M12 CSB FM Towns menu selects complete original CD");
    return ok ? 0 : 1;
}
