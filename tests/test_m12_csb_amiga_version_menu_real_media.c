#include "asset_status_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        failures++; \
        goto cleanup; \
    } \
} while (0)

static M12_StartupMenuState state;
static int failures;

static void send_input(M12_MenuInput input) {
    M12_StartupMenu_HandleInput(&state, input);
}

int main(int argc, char** argv) {
    M12_StartupMenuInitOptions initOptions;
    M12_LaunchIntent intent;
    const M12_MenuEntry* entry;
    const M12_AssetVersionStatus* version;
    int platforms[M12_ARCH_COUNT];
    int entryIndex = -1;
    int amigaCard = -1;
    int versionIndex;
    int versionCount;
    int tries;
    int launchRowSteps;
    int i;

    if (argc != 2 || !argv[1] || argv[1][0] == '\0') {
        fprintf(stderr, "usage: %s <authentic-csb-data-directory>\n", argv[0]);
        return 2;
    }

    memset(&initOptions, 0, sizeof(initOptions));
    initOptions.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&state, argv[1], "csb", &initOptions);

    versionIndex = M12_AssetStatus_FindVersionIndex("csb", "amiga35-en");
    version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&state.assetStatus, "csb",
                                     (size_t)versionIndex)
        : NULL;
    if (!version || !version->matched) {
        puts("SKIP: authentic CSB Amiga 3.5 English media is unavailable");
        M12_StartupMenu_Destroy(&state);
        return 77;
    }

    CHECK(M12_StartupMenu_GetPlatformCardArchitectures("csb", platforms) > 0,
          "CSB platform cards are available");
    for (i = 0; i < M12_StartupMenu_GetPlatformCardArchitectures("csb", NULL);
         ++i) {
        if (platforms[i] == M12_ARCH_AMIGA) {
            amigaCard = i;
            break;
        }
    }
    CHECK(amigaCard >= 0, "CSB Amiga platform card is available");

    for (i = 0; i < M12_StartupMenu_GetEntryCount(); ++i) {
        entry = M12_StartupMenu_GetEntry(&state, i);
        if (entry && entry->kind == M12_MENU_ENTRY_GAME && entry->gameId &&
            strcmp(entry->gameId, "csb") == 0) {
            entryIndex = i;
            break;
        }
    }
    CHECK(entryIndex >= 0, "CSB game card is present");
    CHECK(state.view == M12_MENU_VIEW_MAIN,
          "authentic CSB media opens at the main menu");

    state.selectedIndex = entryIndex;
    send_input(M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_GAME_OPTIONS &&
              state.gameCardFlowStage == 0,
          "CSB selection opens the platform cards");
    for (i = 0; i < amigaCard; ++i) {
        send_input(M12_MENU_INPUT_DOWN);
    }
    send_input(M12_MENU_INPUT_ACCEPT);
    CHECK(state.gameCardFlowStage == 1,
          "selecting Amiga opens the presentation cards");

    send_input(M12_MENU_INPUT_DOWN);
    send_input(M12_MENU_INPUT_DOWN);
    send_input(M12_MENU_INPUT_ACCEPT);
    CHECK(state.gameCardFlowStage == 2 &&
              state.gameOptSelectedRow == M12_GAME_OPT_ROW_ARCHITECTURE,
          "Game Options opens on the architecture row");

    send_input(M12_MENU_INPUT_DOWN);
    versionCount = (int)M12_AssetStatus_GetVersionCount("csb");
    for (tries = 0; tries < versionCount &&
                    state.gameOptions[1].versionIndex != versionIndex;
         ++tries) {
        send_input(M12_MENU_INPUT_RIGHT);
    }
    CHECK(state.gameOptions[1].versionIndex == versionIndex &&
              state.gameOptions[1].architectureIndex == M12_ARCH_AMIGA,
          "version row selects authentic CSB Amiga 3.5 English");

    for (launchRowSteps = 0;
         state.gameOptSelectedRow < M12_GAME_OPT_ROW_COUNT &&
         launchRowSteps < M12_GAME_OPT_ROW_COUNT + 2;
         ++launchRowSteps) {
        send_input(M12_MENU_INPUT_DOWN);
    }
    CHECK(state.gameOptSelectedRow >= M12_GAME_OPT_ROW_COUNT,
          "game options navigation reaches Launch");
    send_input(M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(state.launchRequested && intent.valid && intent.gameId &&
              strcmp(intent.gameId, "csb") == 0 && intent.versionId &&
              strcmp(intent.versionId, "amiga35-en") == 0 &&
              intent.options.architectureIndex == M12_ARCH_AMIGA,
          "launch intent retains the selected authentic CSB Amiga 3.5 edition");

    printf("PASS: authentic CSB Amiga 3.5 menu selection reaches launch intent (%s)\n",
           version->matchedPath);

cleanup:
    M12_StartupMenu_Destroy(&state);
    return failures ? 1 : 0;
}
