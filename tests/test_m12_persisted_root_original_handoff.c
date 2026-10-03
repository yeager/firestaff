/* Original-media M12 picker persistence through DM1, CSB, and DM2 launch.
 * This is opt-in because it needs the operator's unmodified game archives. */
#include "menu_startup_m12.h"
#include "m11_game_view.h"
#include "config_m12.h"
#include "fs_portable_compat.h"
#include "csb_v1_boot.h"
#include "dm2_v1_boot.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int failures;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); ++failures; } } while (0)

typedef struct GameLeafScanReceipt {
    char searchRoot[FSP_PATH_MAX];
} GameLeafScanReceipt;

static int record_game_leaf_search_root(const M12_AssetScanProgress* progress,
                                        void* userData)
{
    GameLeafScanReceipt* receipt = (GameLeafScanReceipt*)userData;
    if (progress && receipt &&
        strcmp(progress->currentTask, "search root") == 0) {
        snprintf(receipt->searchRoot, sizeof(receipt->searchRoot), "%s",
                 progress->currentPath);
    }
    return 1;
}

int main(void)
{
    static const char* const games[] = {"dm1", "csb", "dm2"};
    static const char* const versions[] = {"pc34-en", "fmtowns-en", "pc-en"};
    static const char* const autoVersions[] = {"fmtowns-en", "fmtowns-en", "fmtowns-ja"};
    static const char* const staleVersions[] = {"pc34-en", "fmtowns-ja", "pc-en"};
    static const M11_GameSourceKind kinds[] = {
        M11_GAME_SOURCE_BUILTIN_CATALOG, M11_GAME_SOURCE_CSB_BOOT,
        M11_GAME_SOURCE_DM2_BOOT
    };
    const char* root = getenv("FIRESTAFF_M12_ORIGINAL_MENU_ROOT");
    const char* scratch = getenv("TMPDIR");
    char physical[FSP_PATH_MAX], configPath[FSP_PATH_MAX];
    M12_StartupMenuState* menu;
    M12_StartupMenuInitOptions options;
    M12_Config config;
    int game;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_M12_ORIGINAL_MENU_ROOT is required");
        return 77;
    }
    if (!scratch || !scratch[0] || !FSP_DirExists(root) ||
        !FSP_ResolvePhysicalPath(physical, sizeof(physical), root) ||
        !FSP_JoinPath(configPath, sizeof(configPath), scratch, "menu.toml") ||
        !FSP_CreateDirectoryRecursive(scratch)) {
        fputs("FAIL: original root or isolated scratch is unavailable\n", stderr);
        return 1;
    }
    CHECK(SDL_setenv_unsafe("FIRESTAFF_CONFIG_PATH", configPath, 1) == 0,
          "select isolated configuration");
    CHECK(SDL_Init(0), "initialize SDL");
    menu = (M12_StartupMenuState*)SDL_calloc(1, sizeof(*menu));
    CHECK(menu != NULL, "allocate menu");
    if (failures) goto cleanup;

    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    options.scanAllGames = 1;
    M12_StartupMenu_InitWithOptions(menu, scratch, NULL, &options);
    CHECK(M12_StartupMenu_SetDataDirectory(menu, root),
          "select original collection through the menu");
    CHECK(strcmp(M12_StartupMenu_GetVisibleDataDir(menu), physical) == 0,
          "menu displays physical selected root");
    (void)M12_Config_Load(&config, NULL);
    CHECK(strcmp(config.dataDir, physical) == 0,
          "selected root is persisted");
    M12_StartupMenu_Destroy(menu);
    memset(menu, 0, sizeof(*menu));
    M12_StartupMenu_InitWithOptions(menu, NULL, NULL, &options);
    CHECK(strcmp(M12_StartupMenu_GetVisibleDataDir(menu), physical) == 0 &&
          strcmp(M12_AssetStatus_GetDataDir(&menu->assetStatus), physical) == 0,
          "reopened menu scans the persisted root");

    /* Direct --game scanning narrows a collection to that game's leaf, while
     * the visible menu above continues to own the entire persisted root. */
    for (game = 0; game < 3 && !failures; ++game) {
        M12_AssetStatus* scoped = (M12_AssetStatus*)SDL_calloc(1, sizeof(*scoped));
        M12_AssetStatusScanOptions scanOptions = {0};
        GameLeafScanReceipt receipt = {{0}};
        char expectedLeaf[FSP_PATH_MAX];
        int versionIndex = M12_AssetStatus_FindVersionIndex(
            games[game], autoVersions[game]);
        const M12_AssetVersionStatus* selectedVersion = NULL;
        CHECK(scoped != NULL &&
              FSP_JoinPath(expectedLeaf, sizeof(expectedLeaf), physical,
                           games[game]),
              "allocate original-media game-leaf scan");
        if (failures) {
            SDL_free(scoped);
            break;
        }
        scanOptions.preferGameLeaf = 1;
        scanOptions.progressFn = record_game_leaf_search_root;
        scanOptions.progressUserData = &receipt;
        M12_AssetStatus_ScanGameWithOptions(scoped, physical, games[game],
                                            &scanOptions);
        CHECK(strcmp(receipt.searchRoot, expectedLeaf) == 0,
              "direct game scan visits only its collection leaf");
        CHECK(strcmp(M12_AssetStatus_GetDataDir(scoped), physical) == 0,
              "direct game scan retains the persisted collection root");
        if (versionIndex >= 0) {
            selectedVersion = M12_AssetStatus_GetVersion(
                scoped, games[game], (size_t)versionIndex);
        }
        CHECK(selectedVersion && selectedVersion->matched,
              "direct game leaf retains the authenticated AUTO edition");
        SDL_free(scoped);
    }

    /* Preserve the explicit PC/FM Towns/DOS handoff coverage. */
    for (game = 0; game < 3 && !failures; ++game) {
        M12_LaunchIntent intent;
        M11_GameViewState* view;
        const M12_AssetVersionStatus* version;
        int versionIndex = M12_AssetStatus_FindVersionIndex(games[game], versions[game]);
        version = versionIndex < 0 ? NULL : M12_AssetStatus_GetVersion(
            &menu->assetStatus, games[game], (size_t)versionIndex);
        if (!version || !version->matched) {
            fprintf(stderr, "FAIL: selected root lacks authenticated %s %s media\n",
                    games[game], versions[game]);
            ++failures;
            break;
        }
        menu->selectedIndex = game;
        menu->activatedIndex = game;
        menu->launchRequested = 1;
        menu->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
        menu->gameOptions[game].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
        menu->gameOptions[game].versionIndex = versionIndex;
        if (game == 1) menu->gameOptions[game].architectureIndex = M12_ARCH_FM_TOWNS;
        if (game == 2) menu->gameOptions[game].architectureIndex = M12_ARCH_PC;
        intent = M12_StartupMenu_GetLaunchIntent(menu);
        CHECK(intent.valid && intent.gameId &&
              strcmp(intent.gameId, games[game]) == 0,
              "selected original game produces a valid launch intent");
        if (failures) break;
        view = (M11_GameViewState*)SDL_calloc(1, sizeof(*view));
        CHECK(view != NULL, "allocate game view");
        if (!view) break;
        M11_GameView_Init(view);
        CHECK(M11_GameView_OpenSelectedMenuEntry(view, menu) == 1,
              "selected menu entry opens original runtime");
        CHECK(view->active && view->startedFromLauncher &&
              view->sourceKind == kinds[game] &&
              strcmp(view->sourceId, games[game]) == 0,
              "launch reaches the game's source-owned M11 state");
        if (game == 1) CHECK(view->csbBootProfile != NULL,
                             "CSB owns a boot profile");
        if (game == 2) CHECK(view->dm2BootProfile != NULL,
                             "DM2 owns a boot profile");
        M11_GameView_Shutdown(view);
        SDL_free(view);
    }

    /* AUTO must override a stale matched row after reopening the collection.
     * The CSB disc supplies both language editions, so JPN is a real stale
     * row rather than a fallback to the expected English edition. */
    for (game = 0; game < 3 && !failures; ++game) {
        M12_LaunchIntent intent;
        M11_GameViewState* view;
        const M12_AssetVersionStatus* version;
        int versionIndex = M12_AssetStatus_FindVersionIndex(games[game], autoVersions[game]);
        int staleVersionIndex = M12_AssetStatus_FindVersionIndex(
            games[game], staleVersions[game]);
        version = versionIndex < 0 ? NULL : M12_AssetStatus_GetVersion(
            &menu->assetStatus, games[game], (size_t)versionIndex);
        if (!version || !version->matched) {
            fprintf(stderr, "FAIL: selected root lacks authenticated %s %s media\n",
                    games[game], autoVersions[game]);
            ++failures;
            break;
        }
        if (staleVersionIndex < 0 ||
            !M12_AssetStatus_GetVersion(&menu->assetStatus, games[game],
                                        (size_t)staleVersionIndex)->matched) {
            fprintf(stderr, "FAIL: selected root lacks stale %s %s media\n",
                    games[game], staleVersions[game]);
            ++failures;
            break;
        }
        menu->selectedIndex = game;
        menu->activatedIndex = game;
        menu->launchRequested = 1;
        menu->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
        menu->gameOptions[game].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
        menu->gameOptions[game].versionIndex = staleVersionIndex;
        menu->gameOptions[game].architectureIndex = M12_ARCH_AUTO;
        intent = M12_StartupMenu_GetLaunchIntent(menu);
        CHECK(intent.valid && intent.gameId &&
              strcmp(intent.gameId, games[game]) == 0,
              "selected original game produces a valid launch intent");
        CHECK(intent.versionId &&
              strcmp(intent.versionId, autoVersions[game]) == 0 &&
              intent.options.versionIndex == versionIndex &&
              intent.options.architectureIndex == M12_ARCH_AUTO,
              "AUTO resolves the authenticated FM Towns edition despite a stale version row");
        if (failures) break;
        view = (M11_GameViewState*)SDL_calloc(1, sizeof(*view));
        CHECK(view != NULL, "allocate game view");
        if (!view) break;
        M11_GameView_Init(view);
        CHECK(M11_GameView_OpenSelectedMenuEntry(view, menu) == 1,
              "selected menu entry opens original runtime");
        CHECK(view->active && view->startedFromLauncher &&
              view->sourceKind == kinds[game] &&
              strcmp(view->sourceId, games[game]) == 0,
              "launch reaches the game's source-owned M11 state");
        if (game == 0) CHECK(view->dm1FmtownsStartupReceiptValid,
                             "DM1 M11 handoff owns FM Towns startup");
        if (game == 1) {
            const CSB_V1_BootProfile* profile =
                (const CSB_V1_BootProfile*)view->csbBootProfile;
            CHECK(profile && profile->variant_id == CSB_V1_VARIANT_FMTOWNS_EN &&
                  profile->fmtowns_executable_size > 0u &&
                  profile->fmtowns_graphics_size > 0u,
                  "CSB M11 handoff owns the verified FM Towns program and graphics");
        }
        if (game == 2) {
            const DM2_V1_BootProfile* profile =
                (const DM2_V1_BootProfile*)view->dm2BootProfile;
            CHECK(profile && profile->platform == DM2_PLATFORM_FMTOWNS_JA &&
                  profile->fmtowns_disc_image_size > 0u,
                  "DM2 M11 handoff owns the authenticated FM Towns disc");
        }
        M11_GameView_Shutdown(view);
        SDL_free(view);
    }
cleanup:
    if (menu) { M12_StartupMenu_Destroy(menu); SDL_free(menu); }
    (void)remove(configPath);
    SDL_Quit();
    if (!failures) puts("PASS: persisted original root launches explicit and AUTO DM1, CSB, and DM2 editions");
    return failures ? 1 : 0;
}
