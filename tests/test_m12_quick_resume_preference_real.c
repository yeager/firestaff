#include "menu_startup_m12.h"
#include "config_m12.h"
#include "fs_portable_compat.h"
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

static int select_quick_resume_row(M12_StartupMenuState* menu)
{
    int tab;
    for (tab = 0; tab < M12_SETTINGS_TAB_COUNT; ++tab) {
        int count = 0, index;
        const int* rows = M12_StartupMenu_GetSettingsRowsForTab(tab, &count);
        for (index = 0; rows && index < count; ++index) {
            const char* label = M12_StartupMenu_GetSettingsLabel(menu, rows[index]);
            if (label && strcmp(label, "QUICK RESUME") == 0) {
                menu->view = M12_MENU_VIEW_SETTINGS;
                menu->settingsTabIndex = tab;
                menu->settingsTabRowIndex = index;
                menu->settingsSelectedIndex = rows[index];
                return 1;
            }
        }
    }
    return 0;
}

int main(void)
{
    const char* archive = getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    const char* scratch = getenv("TMPDIR");
    const char* prior = getenv("FIRESTAFF_CONFIG_PATH");
    char* saved = NULL;
    char base[FSP_PATH_MAX], dataDir[FSP_PATH_MAX], leaf[80];
    char configPath[M12_CONFIG_PATH_CAPACITY], tmpPath[M12_CONFIG_PATH_CAPACITY + 4];
    char savePath[M12_CONFIG_LAST_SAVE_PATH_CAPACITY];
    M12_Config config;
    M12_StartupMenuState* menu = NULL;
    M12_StartupMenuInitOptions options;
    int n, claimed = 0, dataCreated = 0;
    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM1_AMIGA_V20_ARCHIVE is required");
        return 77;
    }
    if (!FSP_FileExists(archive)) {
        fprintf(stderr, "FAIL: supplied authentic Amiga archive is missing\n");
        return 1;
    }
    n = snprintf(savePath, sizeof(savePath),
        "%s::Dungeon Master v2.0 (1988)(FTL)[save disk].zip::"
        "Dungeon Master v2.0 (1988)(FTL)[save disk].adf::DMGAMEG.DAT", archive);
    if (n < 0 || (size_t)n >= sizeof(savePath)) {
        fprintf(stderr, "FAIL: authentic virtual save path does not fit\n");
        return 1;
    }
    if (prior) {
        saved = (char*)malloc(strlen(prior) + 1u);
        if (!saved) return 1;
        strcpy(saved, prior);
    }
    if (!scratch || !scratch[0]) scratch = "build";
    CHECK(FSP_CreateDirectoryRecursive(scratch) &&
        FSP_ResolvePhysicalPath(base, sizeof(base), scratch), "resolve task-local scratch");
    if (failures) goto cleanup;
    snprintf(leaf, sizeof(leaf), "quick-resume-%llu.cfg", (unsigned long long)SDL_GetTicksNS());
    CHECK(FSP_JoinPath(configPath, sizeof(configPath), base, leaf), "isolated config path fits");
    if (failures) goto cleanup;
    snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", configPath);
    snprintf(leaf, sizeof(leaf), "quick-resume-empty-%llu", (unsigned long long)SDL_GetTicksNS());
    CHECK(FSP_JoinPath(dataDir, sizeof(dataDir), base, leaf), "empty data path fits");
    if (failures) goto cleanup;
    CHECK(!FSP_PathExists(configPath) && !FSP_PathExists(tmpPath) &&
        !FSP_PathExists(dataDir), "claim only unused scratch paths");
    if (failures) goto cleanup;
    CHECK(FSP_SetEnv("FIRESTAFF_CONFIG_PATH", configPath, 1) == 0, "isolate configuration");
    if (failures) goto cleanup;
    claimed = 1;
    CHECK(FSP_CreateDirectory(dataDir), "create genuinely empty data directory");
    if (failures) goto cleanup;
    dataCreated = 1;
    M12_Config_SetDefaults(&config);
    config.quickResumeEnabled = 0;
    config.languageIndex = 0;
    config.languageExplicit = 1;
    snprintf(config.lastSavePath, sizeof(config.lastSavePath), "%s", savePath);
    CHECK(M12_Config_Save(&config), "persist OFF preference with original nested save path");
    if (failures) goto cleanup;
    menu = (M12_StartupMenuState*)calloc(1, sizeof(*menu));
    CHECK(menu != NULL, "allocate real menu owner");
    if (!menu) goto cleanup;
    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(menu, dataDir, NULL, &options);
    CHECK(!menu->settings.quickResumeEnabled && !menu->quickResumeAvailable,
        "persisted OFF preference suppresses quick resume");
    menu->settings.audioMasterVolume = 73;
    M12_StartupMenu_SaveConfig(menu);
    CHECK(M12_Config_Load(&config, dataDir) && strcmp(config.lastSavePath, savePath) == 0,
        "saving an unrelated setting while OFF retains the original save location");
    if (!select_quick_resume_row(menu)) {
        CHECK(0, "find real Quick Resume row through public label mapping");
        goto cleanup;
    }
    M12_StartupMenu_HandleInput(menu, M12_MENU_INPUT_VALUE_RIGHT);
    CHECK(menu->settings.quickResumeEnabled && menu->quickResumeAvailable &&
        strcmp(menu->quickResumeGameId, "dm1") == 0 &&
        strcmp(menu->quickResumeSavePath, savePath) == 0,
        "enabling via settings recovers the validated original Amiga save");
    M12_StartupMenu_HandleInput(menu, M12_MENU_INPUT_VALUE_RIGHT);
    CHECK(!menu->settings.quickResumeEnabled && !menu->quickResumeAvailable,
        "OFF disables availability on the same menu");
    CHECK(M12_Config_Load(&config, dataDir) && strcmp(config.lastSavePath, savePath) == 0,
        "OFF still retains the last original save path");
    M12_StartupMenu_HandleInput(menu, M12_MENU_INPUT_VALUE_RIGHT);
    CHECK(menu->settings.quickResumeEnabled && menu->quickResumeAvailable &&
        strcmp(menu->quickResumeSavePath, savePath) == 0,
        "ON OFF ON restores availability without replacing the menu");
cleanup:
    if (menu) { M12_StartupMenu_Destroy(menu); free(menu); }
    if (claimed) { remove(configPath); remove(tmpPath); }
    if (dataCreated) CHECK(TEST_RMDIR(dataDir) == 0, "remove owned empty data directory");
    if (saved) FSP_SetEnv("FIRESTAFF_CONFIG_PATH", saved, 1);
    else FSP_UnsetEnv("FIRESTAFF_CONFIG_PATH");
    free(saved);
    if (!failures) puts("PASS: original Amiga quick-resume preference lifetime");
    return failures ? 1 : 0;
}
