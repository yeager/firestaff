/* Native callback -> main-thread scan -> authenticated PC34 admission. */
#include "menu_startup_m12.h"
#include "config_m12.h"
#include "fs_portable_compat.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define REMOVE_DIR _rmdir
#else
#include <unistd.h>
#define REMOVE_DIR rmdir
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

int main(void)
{
    const char* mediaDir = getenv("FIRESTAFF_DM1_PICKER_TEST_DATA_DIR");
    const char* temp = getenv("TMPDIR");
    const char* priorConfig = getenv("FIRESTAFF_CONFIG_PATH");
    char* savedConfig = priorConfig ? SDL_strdup(priorConfig) : NULL;
    char scratch[FSP_PATH_MAX] = {0}, empty[FSP_PATH_MAX], configPath[FSP_PATH_MAX];
    char expected[FSP_PATH_MAX], leaf[96], configTemp[FSP_PATH_MAX];
    M12_StartupMenuState* menu = NULL;
    M12_StartupMenuInitOptions options;
    M12_Config config;
    Completion completion;
    SDL_Thread* worker;
    Uint64 deadline;
    int created = 0, configured = 0, index;
    const M12_AssetVersionStatus* version;
    if (!mediaDir || !mediaDir[0]) {
        SDL_free(savedConfig);
        puts("SKIP: FIRESTAFF_DM1_PICKER_TEST_DATA_DIR is required");
        return 77;
    }
    if (priorConfig && !savedConfig) return 1;
    CHECK(FSP_DirExists(mediaDir) &&
          FSP_ResolvePhysicalPath(expected, sizeof(expected), mediaDir),
          "selected original-media folder exists");
    if (failures) goto cleanup;
    if (!temp || !temp[0]) temp = "build";
    CHECK(FSP_CreateDirectoryRecursive(temp), "create task-local scratch parent");
    snprintf(leaf, sizeof(leaf), "data-dialog-%llu",
             (unsigned long long)SDL_GetTicksNS());
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
    M12_StartupMenu_InitWithOptions(menu, empty, "dm1", &options);
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
    index = M12_AssetStatus_FindVersionIndex("dm1", "pc34-en");
    version = index >= 0
        ? M12_AssetStatus_GetVersion(&menu->assetStatus, "dm1", (size_t)index) : NULL;
    CHECK(version && version->matched, "scan authenticates original PC34 media");
    CHECK(strcmp(M12_StartupMenu_GetVisibleDataDir(menu), expected) == 0,
          "selected physical folder remains visible");
    (void)M12_Config_Load(&config, NULL);
    CHECK(strcmp(config.path, configPath) == 0 &&
          strcmp(config.dataDir, expected) == 0,
          "main-thread completion persists only to isolated configuration");
    /* Reopen the ordinary multi-game menu without a CLI data-dir override.
     * Its persisted original-media root must remain selected even when the
     * machine's default root contains other, unrelated games. */
    M12_StartupMenu_Destroy(menu);
    memset(menu, 0, sizeof(*menu));
    options.scanAllGames = 1;
    M12_StartupMenu_InitWithOptions(menu, NULL, NULL, &options);
    CHECK(strcmp(M12_StartupMenu_GetVisibleDataDir(menu), expected) == 0 &&
          strcmp(M12_AssetStatus_GetDataDir(&menu->assetStatus), expected) == 0,
          "multi-game menu keeps the persisted original-media root");
    (void)M12_Config_Load(&config, NULL);
    CHECK(strcmp(config.dataDir, expected) == 0,
          "menu reopen does not replace persisted media with default root");
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
    if (!failures) puts("PASS: original media selected off-thread and admitted on main");
    return failures ? 1 : 0;
}
