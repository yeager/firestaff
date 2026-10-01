#include "menu_startup_m12.h"
#include "config_m12.h"
#include "fs_portable_compat.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_startup_menu.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define TEST_RMDIR _rmdir
#else
#include <unistd.h>
#define TEST_RMDIR rmdir
#endif

static int failures;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); ++failures; } } while (0)

static int find_primary_save(const char* root, char* out, size_t capacity)
{
    DM2_SKSaveCorpusReceipt corpus;
    unsigned i;
    if (!dm2_v1_sksave_corpus_scan(root, &corpus)) return 0;
    for (i = 0; i < corpus.candidate_receipt_count; ++i) {
        const char* path = corpus.candidate_receipts[i].path;
        size_t length = strlen(path);
        char candidateRoot[512];
        uint8_t slot;
        int lastSession;
        DM2_SKSaveCorpusReceipt candidate;
        if (length < 4 || SDL_strcasecmp(path + length - 4, ".dat") != 0 || length >= capacity)
            continue;
        if (!dm2_v1_startup_save_path_to_root_slot(path, candidateRoot,
                sizeof(candidateRoot), &slot, &lastSession) ||
            !dm2_v1_sksave_corpus_scan(candidateRoot, &candidate)) continue;
        if (!(lastSession ? candidate.has_last_session :
              (slot < DM2_SLOT_MAX && (candidate.valid_slot_mask & (1u << slot))))) continue;
        strcpy(out, path);
        return 1;
    }
    return 0;
}

int main(void)
{
    const char* dataRoot = getenv("FIRESTAFF_DM2_M12_DATA_ROOT");
    const char* saveRoot = getenv("FIRESTAFF_DM2_SAVE_ROOT");
    const char* scratch = getenv("TMPDIR");
    const char* prior = getenv("FIRESTAFF_CONFIG_PATH");
    char* saved = NULL;
    char base[FSP_PATH_MAX], leaf[96], emptyData[FSP_PATH_MAX];
    char configPath[M12_CONFIG_PATH_CAPACITY], configTmp[M12_CONFIG_PATH_CAPACITY + 4];
    char manifest[FSP_PATH_MAX], manifestTmp[FSP_PATH_MAX + 4];
    char missing[M12_CONFIG_LAST_SAVE_PATH_CAPACITY], expected[M12_CONFIG_LAST_SAVE_PATH_CAPACITY];
    M12_Config config;
    char selectedRoot[M12_ASSET_DATA_DIR_CAPACITY];
    M12_StartupMenuState* menu = NULL;
    M12_StartupMenuInitOptions options;
    int claimed = 0, emptyCreated = 0;
    if (!dataRoot || !dataRoot[0] || !saveRoot || !saveRoot[0]) {
        puts("SKIP: FIRESTAFF_DM2_M12_DATA_ROOT and FIRESTAFF_DM2_SAVE_ROOT required");
        return 77;
    }
    if (!FSP_DirExists(dataRoot) || !FSP_DirExists(saveRoot) ||
        !find_primary_save(saveRoot, expected, sizeof(expected))) {
        fprintf(stderr, "FAIL: supplied real DM2 roots lack an admitted primary save\n");
        return 1;
    }
    if (prior) {
        saved = (char*)malloc(strlen(prior) + 1u);
        if (!saved) return 1;
        strcpy(saved, prior);
    }
    if (!scratch || !scratch[0]) scratch = "build";
    CHECK(FSP_CreateDirectoryRecursive(scratch) &&
        FSP_ResolvePhysicalPath(base, sizeof(base), scratch), "resolve isolated scratch");
    if (failures) goto cleanup;
    snprintf(leaf, sizeof(leaf), "dm2-manifest-%llu.cfg", (unsigned long long)SDL_GetTicksNS());
    CHECK(FSP_JoinPath(configPath, sizeof(configPath), base, leaf), "config path fits");
    snprintf(leaf, sizeof(leaf), "dm2-manifest-%llu.json", (unsigned long long)SDL_GetTicksNS());
    CHECK(FSP_JoinPath(manifest, sizeof(manifest), base, leaf), "manifest path fits");
    snprintf(leaf, sizeof(leaf), "dm2-missing-%llu.dat", (unsigned long long)SDL_GetTicksNS());
    CHECK(FSP_JoinPath(missing, sizeof(missing), base, leaf), "missing metadata path fits");
    if (failures) goto cleanup;
    snprintf(configTmp, sizeof(configTmp), "%s.tmp", configPath);
    snprintf(manifestTmp, sizeof(manifestTmp), "%s.tmp", manifest);
    CHECK(!FSP_PathExists(configPath) && !FSP_PathExists(configTmp) &&
        !FSP_PathExists(manifest) && !FSP_PathExists(manifestTmp) && !FSP_PathExists(missing),
        "own only unused scratch paths");
    if (failures) goto cleanup;
    CHECK(FSP_SetEnv("FIRESTAFF_CONFIG_PATH", configPath, 1) == 0, "isolate profile");
    if (failures) goto cleanup;
    claimed = 1;
    M12_Config_SetDefaults(&config);
    config.quickResumeEnabled = 1;
    config.gameArchitectureIndex[2] = M12_ARCH_PC;
    config.lastSavePath[0] = '\0';
    CHECK(strlen(dataRoot) < sizeof(config.dataDir), "selected data root fits config");
    if (failures) goto cleanup;
    strcpy(config.dataDir, dataRoot);
    CHECK(M12_Config_Save(&config), "persist enabled preference with no selected save");
    if (failures) goto cleanup;
    menu = (M12_StartupMenuState*)calloc(1, sizeof(*menu));
    CHECK(menu != NULL, "allocate real launcher owner");
    if (!menu) goto cleanup;
    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    snprintf(leaf, sizeof(leaf), "empty-selection-%llu", (unsigned long long)SDL_GetTicksNS());
    CHECK(FSP_JoinPath(emptyData, sizeof(emptyData), base, leaf) &&
        !FSP_PathExists(emptyData) && FSP_CreateDirectory(emptyData),
        "create owned empty non-DM2 data selection");
    if (failures) goto cleanup;
    emptyCreated = 1;
    M12_StartupMenu_InitWithOptions(menu, emptyData, NULL, &options);
    CHECK(!menu->quickResumeAvailable,
        "explicit save root alone cannot create resume for empty non-DM2 selection");
    M12_StartupMenu_Destroy(menu);
    memset(menu, 0, sizeof(*menu));
    M12_StartupMenu_InitWithOptions(menu, dataRoot, "dm2", &options);
    CHECK(M12_AssetStatus_GameAvailable(&menu->assetStatus, "dm2"),
        "real DM2 media is admitted without fabricated availability");
    CHECK(menu->quickResumeAvailable && strcmp(menu->quickResumeGameId, "dm2") == 0 &&
        strcmp(menu->quickResumeSavePath, expected) == 0,
        "empty selection discovers original save from the explicit runtime save root");
    CHECK(M12_StartupMenu_DM2ResumeSupportedOnSelectedPlatform(menu),
        "retain authenticated DM2 DOS Resume on the DOS platform");
    snprintf(selectedRoot, sizeof(selectedRoot), "%s",
        M12_AssetStatus_GetDataDir(&menu->assetStatus));
    M12_Config_SetDefaults(&config);
    config.quickResumeEnabled = 1;
    snprintf(config.lastSavePath, sizeof(config.lastSavePath), "%s", missing);
    CHECK(M12_Config_ExportSaveManifestJSON(&config, manifest),
        "export metadata-only manifest with a missing selected path");
    CHECK(M12_StartupMenu_ImportSaveManifestPath(menu, manifest), "import through production menu API");
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&menu->assetStatus), selectedRoot) == 0,
        "manifest import preserves the selected original-media root");
    CHECK(menu->quickResumeAvailable && strcmp(menu->quickResumeGameId, "dm2") == 0 &&
        strcmp(menu->quickResumeSavePath, expected) == 0 &&
        strcmp(menu->quickResumeSavePath, missing) != 0,
        "import keeps discovered authentic save instead of overwriting it with missing metadata");
    CHECK(M12_Config_Load(&config, dataRoot) && strcmp(config.path, configPath) == 0 &&
        strcmp(config.lastSavePath, expected) == 0,
        "import immediately persists the discovered authentic resume identity");
    M12_StartupMenu_SaveConfig(menu);
    CHECK(M12_Config_Load(&config, dataRoot) && strcmp(config.path, configPath) == 0 &&
        strcmp(config.lastSavePath, expected) == 0,
        "subsequent settings persistence agrees with the discovered resume identity");

    /* A valid DOS SKSave is not a Macintosh Resume candidate. Mac header and
     * dungeon-prefix parsing do not yet cover SKProject DM2_GAME_LOAD's full
     * record and possession reconstruction. */
    M12_Config_SetDefaults(&config);
    config.quickResumeEnabled = 1;
    config.gameArchitectureIndex[2] = M12_ARCH_MAC;
    snprintf(config.dataDir, sizeof(config.dataDir), "%s", dataRoot);
    snprintf(config.lastSavePath, sizeof(config.lastSavePath), "%s", expected);
    CHECK(M12_Config_Save(&config),
        "persist authentic DOS save identity with explicit Mac selection");
    M12_StartupMenu_Destroy(menu);
    memset(menu, 0, sizeof(*menu));
    M12_StartupMenu_InitWithOptions(menu, dataRoot, "dm2", &options);
    CHECK(menu->gameOptions[2].architectureIndex == M12_ARCH_MAC,
        "retain explicit Mac architecture for the resume check");
    CHECK(!M12_StartupMenu_DM2ResumeSupportedOnSelectedPlatform(menu),
        "report Mac DM2 Resume as unavailable until full GAME_LOAD is owned");
    CHECK(!menu->quickResumeAvailable,
        "do not offer an authentic DOS SKSave to the incomplete Mac GAME_LOAD path");
cleanup:
    if (menu) { M12_StartupMenu_Destroy(menu); free(menu); }
    if (emptyCreated) CHECK(TEST_RMDIR(emptyData) == 0, "remove owned empty data directory");
    if (claimed) { remove(configPath); remove(configTmp); remove(manifest); remove(manifestTmp); }
    if (saved) FSP_SetEnv("FIRESTAFF_CONFIG_PATH", saved, 1);
    else FSP_UnsetEnv("FIRESTAFF_CONFIG_PATH");
    free(saved);
    if (!failures) puts("PASS: real DM2 explicit-root and manifest fallback");
    return failures ? 1 : 0;
}
