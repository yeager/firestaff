/* Native callback -> main-thread scan -> authenticated game admission. */
#include "menu_startup_m12.h"
#include "config_m12.h"
#include "fs_portable_compat.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#define REMOVE_DIR _rmdir
#define PROCESS_ID _getpid
#else
#include <unistd.h>
#define REMOVE_DIR rmdir
#define PROCESS_ID getpid
#endif

static int failures;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); ++failures; } } while (0)
typedef struct Completion { void* token; const char* path; } Completion;
static int SDLCALL complete_on_worker(void* data)
{
    Completion* completion = (Completion*)data;
    M12_StartupMenu_CompleteDataDirDialog(completion->token, completion->path);
    return 0;
}

int main(int argc, char** argv)
{
    const char* gameId = argc > 1 ? argv[1] : NULL;
    const char* mediaDir = argc > 2 ? argv[2] : NULL;
    const char* temp = getenv("TMPDIR");
    const char* priorConfig = getenv("FIRESTAFF_CONFIG_PATH");
    char* savedConfig = priorConfig ? SDL_strdup(priorConfig) : NULL;
    char scratch[FSP_PATH_MAX] = {0}, empty[FSP_PATH_MAX], configPath[FSP_PATH_MAX];
    char expected[FSP_PATH_MAX], leaf[96], configTemp[FSP_PATH_MAX];
    char selectedVersionId[64] = {0};
    M12_StartupMenuState* menu = NULL;
    M12_StartupMenuInitOptions options;
    M12_Config config;
    Completion completion;
    SDL_Thread* worker;
    Uint64 deadline;
    int created = 0, configured = 0, matchedVersion = 0, gameSlot;
    if (!gameId || !mediaDir ||
        (strcmp(gameId, "dm1") != 0 && strcmp(gameId, "csb") != 0 &&
         strcmp(gameId, "dm2") != 0) || !mediaDir[0]) {
        SDL_free(savedConfig);
        puts("SKIP: provide a DM1, CSB, or DM2 game id and original-media folder");
        return 77;
    }
    gameSlot = strcmp(gameId, "dm1") == 0 ? 0 :
               strcmp(gameId, "csb") == 0 ? 1 : 2;
    if (!FSP_DirExists(mediaDir)) {
        SDL_free(savedConfig);
        printf("SKIP: original-media folder is unavailable for %s: %s\n",
               gameId, mediaDir);
        return 77;
    }
    if (priorConfig && !savedConfig) return 1;
    CHECK(FSP_ResolvePhysicalPath(expected, sizeof(expected), mediaDir),
          "selected original-media folder resolves physically");
    if (failures) goto cleanup;
    if (!temp || !temp[0]) temp = "build";
    CHECK(FSP_CreateDirectoryRecursive(temp), "create task-local scratch parent");
    snprintf(leaf, sizeof(leaf), "data-dialog-%ld-%llu",
             (long)PROCESS_ID(), (unsigned long long)SDL_GetTicksNS());
    CHECK(FSP_JoinPath(scratch, sizeof(scratch), temp, leaf) &&
          !FSP_PathExists(scratch) && FSP_CreateDirectory(scratch),
          "create unique test scratch directory");
    if (failures) goto cleanup;
    created = 1;
    CHECK(FSP_JoinPath(empty, sizeof(empty), scratch, "empty") &&
          FSP_CreateDirectory(empty), "create empty initial data directory");
    CHECK(FSP_JoinPath(configPath, sizeof(configPath), scratch, "menu.toml"),
          "construct isolated configuration path");
    if (failures) goto cleanup;
    CHECK(SDL_setenv_unsafe("FIRESTAFF_CONFIG_PATH", configPath, 1) == 0,
          "select isolated configuration");
    configured = 1;
    CHECK(SDL_Init(0), "initialize callback thread support");
    if (failures) goto cleanup;
    menu = (M12_StartupMenuState*)SDL_calloc(1, sizeof(*menu));
    CHECK(menu != NULL, "allocate menu state");
    if (!menu) goto cleanup;
    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(menu, empty, gameId, &options);
    menu->languageExplicit = 1;
    menu->settings.languageIndex = 0;
    CHECK(M12_StartupMenu_SetDataDirectory(menu, empty),
          "synchronous empty folder scan completes");
    CHECK(menu->messageLine1 && strcmp(menu->messageLine1, "NO GAME DATA FOUND") == 0,
          "synchronous empty folder preserves installation guidance");
    completion.token = M12_StartupMenu_BeginDataDirDialog(menu);
    CHECK(completion.token != NULL, "begin empty folder request");
    if (!completion.token) goto cleanup;
    M12_StartupMenu_CompleteDataDirDialog(completion.token, empty);
    (void)M12_StartupMenu_Update(menu);
    deadline = SDL_GetTicks() + 15000U;
    while (menu->dataDirScanJob && SDL_GetTicks() < deadline) {
        (void)M12_StartupMenu_Update(menu);
        SDL_Delay(2);
    }
    CHECK(!menu->dataDirScanJob && !menu->dataDirScanActive,
          "asynchronous empty folder scan completes");
    CHECK(menu->messageLine1 && strcmp(menu->messageLine1, "NO GAME DATA FOUND") == 0,
          "asynchronous empty folder preserves installation guidance");
    completion.token = M12_StartupMenu_BeginDataDirDialog(menu);
    completion.path = mediaDir;
    CHECK(completion.token != NULL, "begin data folder request");
    if (!completion.token) goto cleanup;
    worker = SDL_CreateThread(complete_on_worker, "data-dialog-original", &completion);
    CHECK(worker != NULL, "deliver real folder selection from worker");
    if (!worker) {
        M12_StartupMenu_CompleteDataDirDialog(completion.token, NULL);
        goto cleanup;
    }
    SDL_WaitThread(worker, NULL);
    CHECK(menu->dataDirPickerActive && !menu->dataDirScanActive &&
          !menu->dataDirScanJob, "callback publishes only; scan has not started");
    CHECK(M12_StartupMenu_Update(menu), "main-thread update consumes selection");
    deadline = SDL_GetTicks() + 15000U;
    while (menu->dataDirScanJob && SDL_GetTicks() < deadline) {
        (void)M12_StartupMenu_Update(menu);
        SDL_Delay(2);
    }
    CHECK(!menu->dataDirScanJob && !menu->dataDirScanActive &&
          !menu->dataDirPickerActive, "original-media scan completes within bound");
    for (size_t i = 0; i < M12_AssetStatus_GetVersionCount(gameId); ++i) {
        const M12_AssetVersionStatus* version =
            M12_AssetStatus_GetVersion(&menu->assetStatus, gameId, i);
        if (version && version->matched) {
            matchedVersion = 1;
            break;
        }
    }
    CHECK(matchedVersion, "scan authenticates original media for the selected game");
    if (menu->gameOptions[gameSlot].versionIndex >= 0 &&
        (size_t)menu->gameOptions[gameSlot].versionIndex <
            M12_AssetStatus_GetVersionCount(gameId)) {
        const M12_AssetVersionStatus* selectedVersion =
            M12_AssetStatus_GetVersion(&menu->assetStatus, gameId,
                (size_t)menu->gameOptions[gameSlot].versionIndex);
        if (selectedVersion && selectedVersion->matched &&
            selectedVersion->versionId) {
            snprintf(selectedVersionId, sizeof(selectedVersionId), "%s",
                     selectedVersion->versionId);
        }
    }
    CHECK(selectedVersionId[0] != '\0',
          "automatic platform selection targets authenticated media");
    CHECK(strcmp(M12_StartupMenu_GetVisibleDataDir(menu), expected) == 0,
          "selected physical folder remains visible");
    (void)M12_Config_Load(&config, NULL);
    CHECK(strcmp(config.path, configPath) == 0 &&
          strcmp(config.dataDir, expected) == 0,
          "main-thread completion persists only to isolated configuration");
    /* Reopen the same game's startup path without a CLI data-dir override.
     * Keep this scan scoped to the selected game so the test never scans
     * games maintained by other agents. */
    M12_StartupMenu_Destroy(menu);
    memset(menu, 0, sizeof(*menu));
    options.scanAllGames = 0;
    M12_StartupMenu_InitWithOptions(menu, NULL, gameId, &options);
    CHECK(strcmp(M12_StartupMenu_GetVisibleDataDir(menu), expected) == 0 &&
          strcmp(M12_AssetStatus_GetDataDir(&menu->assetStatus), expected) == 0,
          "game startup keeps the persisted original-media folder");
    if (menu->gameOptions[gameSlot].versionIndex >= 0 &&
        (size_t)menu->gameOptions[gameSlot].versionIndex <
            M12_AssetStatus_GetVersionCount(gameId)) {
        const M12_AssetVersionStatus* selectedVersion =
            M12_AssetStatus_GetVersion(&menu->assetStatus, gameId,
                (size_t)menu->gameOptions[gameSlot].versionIndex);
        CHECK(selectedVersion && selectedVersion->matched &&
              selectedVersion->versionId && selectedVersionId[0] &&
              strcmp(selectedVersion->versionId, selectedVersionId) == 0,
              "reopening retains the same authenticated edition selection");
    } else {
        CHECK(0, "reopening retains the same authenticated edition selection");
    }
    (void)M12_Config_Load(&config, NULL);
    CHECK(strcmp(config.dataDir, expected) == 0,
          "game startup does not replace persisted media with default root");
cleanup:
    if (menu) { M12_StartupMenu_Destroy(menu); SDL_free(menu); }
    if (created) {
        if (FSP_JoinPath(configPath, sizeof(configPath), scratch, "menu.toml"))
            (void)remove(configPath);
        if (FSP_JoinPath(configTemp, sizeof(configTemp), scratch, "menu.toml.tmp"))
            (void)remove(configTemp);
        if (FSP_JoinPath(empty, sizeof(empty), scratch, "empty"))
            (void)REMOVE_DIR(empty);
        (void)REMOVE_DIR(scratch);
    }
    if (configured) {
        if (savedConfig) SDL_setenv_unsafe("FIRESTAFF_CONFIG_PATH", savedConfig, 1);
        else SDL_unsetenv_unsafe("FIRESTAFF_CONFIG_PATH");
    }
    SDL_free(savedConfig);
    SDL_Quit();
    if (!failures) printf("PASS: %s original media selected off-thread and admitted on main\n", gameId);
    return failures ? 1 : 0;
}
