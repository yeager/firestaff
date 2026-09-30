#include "artpack_admission_m12.h"
#include "config_m12.h"
#include "fs_portable_compat.h"
#include "menu_startup_m12.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;
static void expect_true(int ok, const char* label) {
    if (!ok) { fprintf(stderr, "FAIL: %s\n", label); ++g_failures; }
}
static char* copy_env(const char* name) {
    const char* value = getenv(name);
    char* copy = value ? (char*)malloc(strlen(value) + 1u) : NULL;
    if (copy) strcpy(copy, value);
    return copy;
}
static void restore_env(const char* name, char* value) {
    if (value) FSP_SetEnv(name, value, 1);
    else FSP_UnsetEnv(name);
    free(value);
}

typedef void* (*BeginDialog)(M12_StartupMenuState*);
typedef void (*CompleteDialog)(void*, const char*);
typedef struct Completion {
    CompleteDialog complete;
    void* token;
    const char* path;
} Completion;
static int SDLCALL complete_worker(void* userdata) {
    Completion* completion = (Completion*)userdata;
    completion->complete(completion->token, completion->path);
    return 0;
}
static int deliver(CompleteDialog complete, void* token, const char* path) {
    Completion completion = {complete, token, path};
    SDL_Thread* worker = SDL_CreateThread(complete_worker, "font-artpack-result", &completion);
    int status = -1;
    if (!worker) { complete(token, NULL); return 0; }
    SDL_WaitThread(worker, &status);
    return status == 0;
}
static char* selected_path(M12_StartupMenuState* state, int font) {
    return font ? state->settings.unicodeFontPath : state->settings.artpackPath;
}
static void* pending_job(M12_StartupMenuState* state, int font) {
    return font ? state->unicodeFontDialogJob : state->artpackDialogJob;
}
static void check_dialog(int font, const char* validPath) {
    BeginDialog begin = font ? M12_StartupMenu_BeginUnicodeFontDialog : M12_StartupMenu_BeginArtpackDialog;
    CompleteDialog complete = font ? M12_StartupMenu_CompleteUnicodeFontDialog : M12_StartupMenu_CompleteArtpackDialog;
    M12_StartupMenuState* state = (M12_StartupMenuState*)SDL_calloc(1, sizeof(*state));
    char overlong[M12_CONFIG_DATA_DIR_CAPACITY + 64];
    void* token;
    int scenario;
    expect_true(state != NULL, "allocate dialog owner");
    if (!state) return;
    strcpy(selected_path(state, font), "previous-selection");
    memset(overlong, 'x', sizeof(overlong) - 1u);
    overlong[sizeof(overlong) - 1u] = '\0';
    for (scenario = 0; scenario < 2; ++scenario) {
        token = begin(state);
        expect_true(token && state->dataDirPickerActive && pending_job(state, font),
                    "begin dialog creates independent pending token");
        if (!token) continue;
        expect_true(begin(state) == NULL, "pending dialog cannot be replaced");
        expect_true(deliver(complete, token, scenario ? overlong : NULL),
                    "worker delivers cancellation or overflow");
        expect_true(state->dataDirPickerActive &&
                    strcmp(selected_path(state, font), "previous-selection") == 0,
                    "callback leaves owner state unchanged until Update");
        expect_true(M12_StartupMenu_Update(state) && !state->dataDirPickerActive &&
                    !pending_job(state, font), "Update consumes completed token");
        expect_true(strcmp(selected_path(state, font), "previous-selection") == 0,
                    "cancellation and overflow preserve prior selection");
    }
    if (validPath && validPath[0]) {
        M12_Config before;
        M12_Config after;
        int laneFailures = g_failures;
        (void)M12_Config_Load(&before, NULL);
        token = begin(state);
        expect_true(token != NULL, "begin positive installed-file selection");
        if (token) {
            expect_true(deliver(complete, token, validPath), "worker delivers existing file");
            expect_true(strcmp(selected_path(state, font), "previous-selection") == 0,
                        "positive worker completion remains deferred");
            expect_true(M12_Config_Load(&after, NULL) &&
                strcmp(font ? after.unicodeFontPath : after.artpackPath,
                       font ? before.unicodeFontPath : before.artpackPath) == 0,
                "worker completion does not persist selection before Update");
            expect_true(M12_StartupMenu_Update(state) && !pending_job(state, font) &&
                        strcmp(selected_path(state, font), validPath) == 0,
                        "main-thread Update applies selected file");
            expect_true(M12_Config_Load(&after, NULL) &&
                strcmp(after.path, getenv("FIRESTAFF_CONFIG_PATH")) == 0 &&
                strcmp(font ? after.unicodeFontPath : after.artpackPath, validPath) == 0,
                "Update persists selection in the isolated config file");
            if (font && laneFailures == g_failures)
                puts("PASS lane: actual installed Unicode font selected and persisted");
        }
    } else puts("SKIP lane: installed Unicode font unavailable; lifecycle checks still run");
    token = begin(state);
    expect_true(token != NULL, "begin dialog before owner destruction");
    M12_StartupMenu_Destroy(state);
    SDL_free(state);
    if (token) expect_true(deliver(complete, token, validPath),
                          "late callback completes after Destroy and owner free");
}

static void check_repeated_file_replacements(const char* settingsJsonPath,
                                             const char* manifestPath) {
    M12_Config config;
    M12_Config loaded;

    M12_Config_SetDefaults(&config);
    expect_true(M12_Config_Load(&config, NULL), "load isolated config before replacement check");
    config.languageExplicit = 1;
    config.languageIndex = 1;
    expect_true(M12_Config_Save(&config) && FSP_PathExists(config.path),
                "first config save creates destination");
    config.languageIndex = 4;
    expect_true(M12_Config_Save(&config), "second config save replaces existing destination");
    M12_Config_SetDefaults(&loaded);
    expect_true(M12_Config_Load(&loaded, NULL) && loaded.languageIndex == 4,
                "config reload observes second save contents");
    expect_true(!FSP_ReplaceFile(settingsJsonPath, config.path),
                "replacement rejects a missing temporary file");
    expect_true(M12_Config_Load(&loaded, NULL) && loaded.languageIndex == 4,
                "failed replacement preserves existing config contents");

    config.languageIndex = 2;
    expect_true(M12_Config_ExportJSON(&config, settingsJsonPath) &&
                FSP_PathExists(settingsJsonPath),
                "first settings export creates destination");
    config.languageIndex = 5;
    expect_true(M12_Config_ExportJSON(&config, settingsJsonPath),
                "second settings export replaces existing destination");
    M12_Config_SetDefaults(&loaded);
    expect_true(M12_Config_ImportJSON(&loaded, settingsJsonPath) &&
                loaded.languageIndex == 5,
                "settings import observes second export contents");

    /* Import must not accept a truncated root object or leak fields parsed
     * before the missing closing brace into the caller's live config. */
    {
        static const char truncated_json[] =
            "{\"language_index\":5,\"data_dir\":\"partial-write\",";
        int before_language;
        char before_data_dir[M12_CONFIG_DATA_DIR_CAPACITY];
        FILE* truncated = fopen(settingsJsonPath, "wb");
        M12_Config_SetDefaults(&loaded);
        loaded.languageIndex = 2;
        snprintf(loaded.dataDir, sizeof(loaded.dataDir), "%s", "keep-this-dir");
        before_language = loaded.languageIndex;
        snprintf(before_data_dir, sizeof(before_data_dir), "%s", loaded.dataDir);
        expect_true(truncated != NULL, "open truncated settings fixture");
        if (truncated) {
            expect_true(fwrite(truncated_json, 1u, sizeof(truncated_json) - 1u,
                               truncated) == sizeof(truncated_json) - 1u,
                        "write truncated settings object");
            expect_true(fclose(truncated) == 0, "close truncated settings fixture");
            expect_true(!M12_Config_ImportJSON(&loaded, settingsJsonPath),
                        "settings import rejects a missing root-object close");
            expect_true(loaded.languageIndex == before_language &&
                        strcmp(loaded.dataDir, before_data_dir) == 0,
                        "truncated settings import leaves the live config unchanged");
        }
    }
    {
        static const char trailing_json[] =
            "{\"language_index\":5} trailing";
        FILE* trailing = fopen(settingsJsonPath, "wb");
        M12_Config_SetDefaults(&loaded);
        loaded.languageIndex = 2;
        expect_true(trailing != NULL, "open trailing-token settings fixture");
        if (trailing) {
            expect_true(fwrite(trailing_json, 1u, sizeof(trailing_json) - 1u,
                               trailing) == sizeof(trailing_json) - 1u,
                        "write settings object followed by an invalid token");
            expect_true(fclose(trailing) == 0,
                        "close trailing-token settings fixture");
            expect_true(!M12_Config_ImportJSON(&loaded, settingsJsonPath),
                        "settings import rejects data after the root object");
            expect_true(loaded.languageIndex == 2,
                        "trailing-token settings import leaves config unchanged");
        }
    }

    config.quickResumeEnabled = 1;
    snprintf(config.lastSavePath, sizeof(config.lastSavePath), "%s",
             "first-save-slot.dat");
    expect_true(M12_Config_ExportSaveManifestJSON(&config, manifestPath) &&
                FSP_PathExists(manifestPath),
                "first save manifest export creates destination");
    snprintf(config.lastSavePath, sizeof(config.lastSavePath), "%s",
             "second-save-slot.dat");
    expect_true(M12_Config_ExportSaveManifestJSON(&config, manifestPath),
                "second save manifest export replaces existing destination");
    M12_Config_SetDefaults(&loaded);
    expect_true(M12_Config_ImportSaveManifestJSON(&loaded, manifestPath) &&
                loaded.quickResumeEnabled &&
                strcmp(loaded.lastSavePath, "second-save-slot.dat") == 0,
                "manifest import observes second export contents");
}

int main(void) {
    const char* scratch = getenv("TMPDIR");
    char base[FSP_PATH_MAX], configPath[M12_CONFIG_PATH_CAPACITY], artpackPath[FSP_PATH_MAX];
    char tmpPath[M12_CONFIG_PATH_CAPACITY + 4], leaf[100];
    char settingsJsonPath[FSP_PATH_MAX] = {0}, manifestPath[FSP_PATH_MAX] = {0};
    char settingsJsonTmpPath[FSP_PATH_MAX + 5] = {0};
    char manifestTmpPath[FSP_PATH_MAX + 5] = {0};
    char fontPath[M12_CONFIG_DATA_DIR_CAPACITY] = {0};
    char* savedConfig = copy_env("FIRESTAFF_CONFIG_PATH");
    char* savedFont = copy_env("FIRESTAFF_UI_FONT");
    M12_Config config;
    M12_ArtpackAdmissionReceipt admission;
    FILE* file;
    int claimed = 0;
    int initialized = 0;
    if ((getenv("FIRESTAFF_CONFIG_PATH") && !savedConfig) ||
        (getenv("FIRESTAFF_UI_FONT") && !savedFont)) {
        free(savedConfig);
        free(savedFont);
        return 1;
    }
    if (!scratch || !scratch[0]) scratch = "build";
    expect_true(FSP_CreateDirectoryRecursive(scratch) &&
                FSP_ResolvePhysicalPath(base, sizeof(base), scratch), "resolve task-local scratch");
    if (g_failures) goto cleanup;
    snprintf(leaf, sizeof(leaf), "font-artpack-%llu.cfg", (unsigned long long)SDL_GetTicksNS());
    expect_true(FSP_JoinPath(configPath, sizeof(configPath), base, leaf), "form isolated config path");
    snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", configPath);
    snprintf(leaf, sizeof(leaf), "font-artpack-%llu.fsart", (unsigned long long)SDL_GetTicksNS());
    expect_true(FSP_JoinPath(artpackPath, sizeof(artpackPath), base, leaf), "form admission fixture path");
    snprintf(leaf, sizeof(leaf), "font-artpack-%llu-settings.json", (unsigned long long)SDL_GetTicksNS());
    expect_true(FSP_JoinPath(settingsJsonPath, sizeof(settingsJsonPath), base, leaf), "form settings export path");
    snprintf(leaf, sizeof(leaf), "font-artpack-%llu-manifest.json", (unsigned long long)SDL_GetTicksNS());
    expect_true(FSP_JoinPath(manifestPath, sizeof(manifestPath), base, leaf), "form save manifest path");
    snprintf(settingsJsonTmpPath, sizeof(settingsJsonTmpPath), "%s.tmp", settingsJsonPath);
    snprintf(manifestTmpPath, sizeof(manifestTmpPath), "%s.tmp", manifestPath);
    if (g_failures) goto cleanup;
    expect_true(!FSP_PathExists(configPath) && !FSP_PathExists(tmpPath) &&
                !FSP_PathExists(artpackPath) && !FSP_PathExists(settingsJsonPath) &&
                !FSP_PathExists(manifestPath), "scratch files are unclaimed");
    if (g_failures) goto cleanup;
    claimed = 1;
    expect_true(FSP_SetEnv("FIRESTAFF_CONFIG_PATH", configPath, 1) == 0, "isolate config persistence");
    if (g_failures) goto cleanup;
    FSP_UnsetEnv("FIRESTAFF_UI_FONT");
    if (M12_Config_FindDefaultUnicodeFontPath(fontPath, sizeof(fontPath)) && FSP_FileExists(fontPath)) {
        expect_true(FSP_SetEnv("FIRESTAFF_UI_FONT", fontPath, 1) == 0, "select actual installed font");
        M12_Config_SetDefaults(&config);
        expect_true(strcmp(config.unicodeFontPath, fontPath) == 0, "defaults retain installed font override");
    } else fontPath[0] = '\0';
    /* V2.2 admission metadata only: this header proves no artwork/content parity. */
    file = fopen(artpackPath, "wb");
    expect_true(file != NULL, "create owned FSART admission header");
    if (!file) goto cleanup;
    expect_true(fwrite("FSART001", 1, 8, file) == 8, "write FSART admission header");
    expect_true(fclose(file) == 0, "close admission header");
    expect_true(M12_ArtpackAdmission_Check(artpackPath, &admission) && admission.admitted &&
                !admission.fallbackVisualsPermitted, "header admission does not authorize fallback artwork");
    initialized = SDL_Init(0);
    expect_true(initialized, "initialize SDL worker primitives");
    if (initialized) {
        check_dialog(1, fontPath);
        check_dialog(0, artpackPath);
        check_repeated_file_replacements(settingsJsonPath, manifestPath);
    }
cleanup:
    if (initialized) SDL_Quit();
    if (claimed) {
        remove(configPath);
        remove(tmpPath);
        remove(artpackPath);
        remove(settingsJsonPath);
        remove(manifestPath);
        remove(settingsJsonTmpPath);
        remove(manifestTmpPath);
    }
    restore_env("FIRESTAFF_CONFIG_PATH", savedConfig);
    restore_env("FIRESTAFF_UI_FONT", savedFont);
    if (g_failures) { fprintf(stderr, "%d failures\n", g_failures); return 1; }
    puts("m12 startup font/artpack settings: ok");
    return 0;
}
