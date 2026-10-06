#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "menu_startup_m12.h"
#include "menu_startup_state_access_m12.h"
#include "menu_startup_render_modern_m12.h"
#include "asset_status_m12.h"
#include "fs_portable_compat.h"

#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#define MKDIR(path) _mkdir(path)
#define TEST_GETCWD(buffer, size) _getcwd((buffer), (int)(size))
#define TEST_CHDIR(path) _chdir(path)
static int test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value) == 0;
}
static int test_unsetenv(const char* name) {
    return _putenv_s(name, "") == 0;
}
static unsigned long test_process_id(void) { return (unsigned long)_getpid(); }
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0700)
#define TEST_GETCWD(buffer, size) getcwd((buffer), (size))
#define TEST_CHDIR(path) chdir(path)
static int test_setenv(const char* name, const char* value) {
    return setenv(name, value, 1) == 0;
}
static int test_unsetenv(const char* name) {
    return unsetenv(name) == 0;
}
static unsigned long test_process_id(void) { return (unsigned long)getpid(); }
#endif

enum {
    TEST_SETTINGS_ROW_DATA_DIR = 15
};

static int failures = 0;
static int dialogCalls = 0;
static int dialogHoldOpen = 0;
static char dialogDefaultLocation[M12_ASSET_DATA_DIR_CAPACITY];
static char dialogSelectedPath[M12_ASSET_DATA_DIR_CAPACITY];
static SDL_DialogFileCallback dialogPendingCallback = NULL;
static void* dialogPendingUserdata = NULL;
static unsigned int testScratchOrdinal = 0U;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

void SDLCALL SDL_ShowOpenFolderDialog(SDL_DialogFileCallback callback,
                                      void* userdata,
                                      SDL_Window* window,
                                      const char* default_location,
                                      bool allow_many) {
    const char* canceledSelection[] = { NULL };
    (void)window;
    (void)allow_many;
    dialogCalls++;
    snprintf(dialogDefaultLocation, sizeof(dialogDefaultLocation),
             "%s", default_location ? default_location : "");
    if (dialogHoldOpen) {
        dialogPendingCallback = callback;
        dialogPendingUserdata = userdata;
        return;
    }
    if (callback) {
        if (dialogSelectedPath[0] != '\0') {
            const char* selectedSelection[] = { dialogSelectedPath, NULL };
            callback(userdata, selectedSelection, 0);
            return;
        }
        callback(userdata, canceledSelection, -1);
    }
}

static void complete_pending_dialog_cancel(void) {
    const char* canceledSelection[] = { NULL };
    SDL_DialogFileCallback callback = dialogPendingCallback;
    void* userdata = dialogPendingUserdata;
    dialogPendingCallback = NULL;
    dialogPendingUserdata = NULL;
    dialogHoldOpen = 0;
    if (callback) {
        callback(userdata, canceledSelection, -1);
    }
}

typedef struct DataDirDialogCompletion {
    void* callbackToken;
    const char* selectedPath;
} DataDirDialogCompletion;

static int SDLCALL complete_data_dir_dialog_on_worker(void* userdata) {
    DataDirDialogCompletion* completion =
        (DataDirDialogCompletion*)userdata;
    M12_StartupMenu_CompleteDataDirDialog(completion->callbackToken,
                                          completion->selectedPath);
    return 0;
}

static int complete_data_dir_dialog_on_worker_and_join(
    void* callbackToken,
    const char* selectedPath) {
    DataDirDialogCompletion completion;
    SDL_Thread* thread;
    int threadStatus = 0;
    completion.callbackToken = callbackToken;
    completion.selectedPath = selectedPath;
    thread = SDL_CreateThread(complete_data_dir_dialog_on_worker,
                              "data-dir-dialog-test",
                              &completion);
    if (!thread) {
        M12_StartupMenu_CompleteDataDirDialog(callbackToken, selectedPath);
        return 0;
    }
    SDL_WaitThread(thread, &threadStatus);
    return threadStatus == 0;
}

static void reset_dialog_stub(void) {
    dialogCalls = 0;
    dialogHoldOpen = 0;
    dialogPendingCallback = NULL;
    dialogPendingUserdata = NULL;
    dialogDefaultLocation[0] = '\0';
    dialogSelectedPath[0] = '\0';
}

static void use_english_for_text_assertions(M12_StartupMenuState* state) {
    if (!state) {
        return;
    }
    /* Init follows the host locale when config is isolated; these assertions
     * intentionally pin the launcher copy to English. */
    state->settings.languageIndex = 0;
    state->languageExplicit = 1;
}

static int create_build_scratch_and_data_root(char dataRoot[M12_ASSET_DATA_DIR_CAPACITY]) {
    char scratchBase[FSP_PATH_MAX];
    char scratchRoot[FSP_PATH_MAX];
    char configPath[FSP_PATH_MAX];
    char originalsRoot[FSP_PATH_MAX];
    char resolvedOriginals[FSP_PATH_MAX];
    char tooSmall[2];
    const char* configuredTmpDir = getenv("TMPDIR");
    char leaf[128];
    unsigned int attempt;
    if (!configuredTmpDir || !configuredTmpDir[0]) {
        configuredTmpDir = "build";
    }
    if (!FSP_CreateDirectoryRecursive(configuredTmpDir) ||
        !FSP_ResolvePhysicalPath(scratchBase, sizeof(scratchBase),
                                 configuredTmpDir)) {
        return 0;
    }
    for (attempt = 0U; attempt < 100U; ++attempt) {
        snprintf(leaf, sizeof(leaf), "m12-data-dir-cancel-%lu-%u",
                 test_process_id(), testScratchOrdinal++);
        if (!FSP_JoinPath(scratchRoot, sizeof(scratchRoot), scratchBase, leaf)) {
            return 0;
        }
        if (!FSP_PathExists(scratchRoot) && FSP_CreateDirectory(scratchRoot)) {
            break;
        }
    }
    if (attempt == 100U ||
        !FSP_JoinPath(dataRoot, M12_ASSET_DATA_DIR_CAPACITY,
                      scratchRoot, "empty-data-root") ||
        !FSP_CreateDirectoryRecursive(dataRoot) ||
        !FSP_JoinPath(originalsRoot, sizeof(originalsRoot),
                      scratchRoot, "default-originals") ||
        !FSP_CreateDirectoryRecursive(originalsRoot) ||
        !FSP_JoinPath(configPath, sizeof(configPath),
                      scratchRoot, "startup-menu.toml") ||
        !test_setenv("FIRESTAFF_CONFIG_PATH", configPath) ||
        !test_setenv("FIRESTAFF_ORIGINALS_DIR", originalsRoot)) {
        return 0;
    }
    if (!FSP_GetDefaultOriginalsDir(resolvedOriginals,
                                    sizeof(resolvedOriginals)) ||
        strcmp(resolvedOriginals, originalsRoot) != 0) {
        return 0;
    }
    tooSmall[0] = 'x';
    tooSmall[1] = '\0';
    if (FSP_GetDefaultOriginalsDir(tooSmall, sizeof(tooSmall)) ||
        tooSmall[0] != '\0') {
        return 0;
    }
    return 1;
}

static int write_text_file(const char* path, const char* text) {
    FILE* fp;
    size_t len;
    if (!path) {
        return 0;
    }
    fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    if (!text) {
        text = "";
    }
    len = strlen(text);
    if (len > 0U && fwrite(text, 1U, len, fp) != len) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int config_persisted_dot_data_dir(const char* path) {
    FILE* fp;
    char line[1024];
    if (!path) {
        return 0;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strcmp(line, "data_dir = \".\"\n") == 0 ||
            strcmp(line, "data_dir = \"./\"\n") == 0 ||
            strcmp(line, "data_dir = \".\\\\\"\n") == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static int seed_dm1_under_data_root(const char* dataRoot,
                                    char graphicsMd5[M12_ASSET_MD5_CAPACITY],
                                    char dungeonMd5[M12_ASSET_MD5_CAPACITY]) {
    char dm1Dir[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsPath[M12_ASSET_DATA_DIR_CAPACITY];
    char dungeonPath[M12_ASSET_DATA_DIR_CAPACITY];
    static const char graphicsPayload[] =
        "Firestaff M12 synthetic DM1 GRAPHICS.DAT fixture\n";
    static const char dungeonPayload[] =
        "Firestaff M12 synthetic DM1 DUNGEON.DAT fixture\n";
    if (!FSP_JoinPath(dm1Dir, sizeof(dm1Dir), dataRoot, "dm1") ||
        !FSP_CreateDirectoryRecursive(dm1Dir) ||
        !FSP_JoinPath(graphicsPath, sizeof(graphicsPath), dm1Dir, "renamed-gfx.payload") ||
        !FSP_JoinPath(dungeonPath, sizeof(dungeonPath), dm1Dir, "renamed-dungeon.payload") ||
        !write_text_file(graphicsPath, graphicsPayload) ||
        !write_text_file(dungeonPath, dungeonPayload) ||
        !m12_file_md5_hex(graphicsPath, graphicsMd5) ||
        !m12_file_md5_hex(dungeonPath, dungeonMd5)) {
        return 0;
    }
    return 1;
}

static const M12_AssetVersionStatus* first_dm1_version(const M12_StartupMenuState* state) {
    return M12_AssetStatus_GetVersion(&state->assetStatus, "dm1", 0U);
}

static void seed_previous_dm1_selection(M12_StartupMenuState* state) {
    state->selectedIndex = 0;
    state->activatedIndex = 0;
    state->gameOptions[0].versionIndex = 0;
    state->gameOptions[0].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
}

static void check_cancel_preserves_no_data_state(void) {
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    const M12_MenuEntry* dm1Entry;
    const M12_AssetVersionStatus* version;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char beforeDataDir[M12_ASSET_DATA_DIR_CAPACITY];
    const char* beforeVersionId;

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    use_english_for_text_assertions(&state);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), dataRoot) == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);

    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    seed_previous_dm1_selection(&state);
    version = first_dm1_version(&state);
    beforeVersionId = version ? version->versionId : NULL;
    snprintf(beforeDataDir, sizeof(beforeDataDir), "%s",
             M12_AssetStatus_GetDataDir(&state.assetStatus));

    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = TEST_SETTINGS_ROW_DATA_DIR;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    (void)M12_StartupMenu_Update(&state);

    CHECK(dialogCalls == 1);
    CHECK(strcmp(dialogDefaultLocation, beforeDataDir) == 0);
    CHECK(state.dataDirPickerActive == 0);
    CHECK(state.dataDirScanActive == 0);
    CHECK(state.dataDirScanCancelRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    if (!state.messageLine1 ||
        strcmp(state.messageLine1, "DATA DIRECTORY UNCHANGED") != 0) {
        fprintf(stderr, "data-dir cancel message was: %s\n",
                state.messageLine1 ? state.messageLine1 : "<null>");
    }
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "DATA DIRECTORY UNCHANGED") == 0);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), beforeDataDir) == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "csb") == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm2") == 0);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.selectedIndex == 0);
    CHECK(state.activatedIndex == 0);
    CHECK(state.gameOptions[0].versionIndex == 0);
    CHECK(state.gameOptions[0].presentationModeIndex == M12_PRESENTATION_V1_ORIGINAL);

    dm1Entry = M12_StartupMenu_GetEntry(&state, 0);
    CHECK(dm1Entry && dm1Entry->gameId && strcmp(dm1Entry->gameId, "dm1") == 0);
    CHECK(dm1Entry && dm1Entry->available == 0);
    version = first_dm1_version(&state);
    CHECK((beforeVersionId == NULL && version == NULL) ||
          (beforeVersionId && version && version->versionId &&
           strcmp(version->versionId, beforeVersionId) == 0));

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    CHECK(intent.valid == 0);
    CHECK(intent.gameId && strcmp(intent.gameId, "dm1") == 0);
    CHECK(intent.versionId && beforeVersionId &&
          strcmp(intent.versionId, beforeVersionId) == 0);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_MAIN);
    CHECK(state.launchRequested == 0);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), beforeDataDir) == 0);
}

static void check_active_picker_blocks_message_reentry(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    int callsBefore;

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    dialogHoldOpen = 1;
    dialogPendingCallback = NULL;
    dialogPendingUserdata = NULL;

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    use_english_for_text_assertions(&state);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }

    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = TEST_SETTINGS_ROW_DATA_DIR;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);

    CHECK(dialogCalls == 1);
    CHECK(dialogPendingCallback != NULL);
    CHECK(dialogPendingUserdata != NULL);
    CHECK(dialogPendingUserdata != &state);
    CHECK(state.dataDirPickerActive == 1);
    CHECK(state.dataDirScanActive == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "CHOOSE GAME DATA FOLDER") == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);

    callsBefore = dialogCalls;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(dialogCalls == callsBefore);
    CHECK(state.dataDirPickerActive == 1);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "CHOOSE GAME DATA FOLDER") == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    CHECK(dialogCalls == callsBefore);
    CHECK(state.dataDirPickerActive == 1);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "CHOOSE GAME DATA FOLDER") == 0);

    complete_pending_dialog_cancel();
    CHECK(state.dataDirPickerActive == 1);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "CHOOSE GAME DATA FOLDER") == 0);
    (void)M12_StartupMenu_Update(&state);
    CHECK(state.dataDirPickerActive == 0);
    CHECK(state.dataDirScanActive == 0);
    CHECK(state.dataDirScanCancelRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "DATA DIRECTORY UNCHANGED") == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_MAIN);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
}

static void check_data_dir_callback_isolated_from_state_lifetime(void) {
    M12_StartupMenuState state;
    M12_StartupMenuState* destroyedState;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char beforeDataDir[M12_ASSET_DATA_DIR_CAPACITY];
    void* callbackToken;

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    if (failures) {
        return;
    }
    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    use_english_for_text_assertions(&state);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    snprintf(beforeDataDir, sizeof(beforeDataDir), "%s",
             M12_AssetStatus_GetDataDir(&state.assetStatus));
    callbackToken = M12_StartupMenu_BeginDataDirDialog(&state);
    CHECK(callbackToken != NULL && state.dataDirPickerActive);
    if (!callbackToken) {
        M12_StartupMenu_Destroy(&state);
        return;
    }
    CHECK(complete_data_dir_dialog_on_worker_and_join(callbackToken, NULL));
    CHECK(state.dataDirPickerActive == 1);
    CHECK(state.dataDirDialogJob != NULL);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus),
                 beforeDataDir) == 0);
    (void)M12_StartupMenu_Update(&state);
    CHECK(state.dataDirPickerActive == 0);
    CHECK(state.dataDirDialogJob == NULL);
    CHECK(state.messageLine1 &&
          strcmp(state.messageLine1, "DATA DIRECTORY UNCHANGED") == 0);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus),
                 beforeDataDir) == 0);
    M12_StartupMenu_Destroy(&state);

    destroyedState = (M12_StartupMenuState*)SDL_calloc(1U,
                                                       sizeof(*destroyedState));
    CHECK(destroyedState != NULL);
    if (!destroyedState) {
        return;
    }
    callbackToken = M12_StartupMenu_BeginDataDirDialog(destroyedState);
    CHECK(callbackToken != NULL);
    if (!callbackToken) {
        M12_StartupMenu_Destroy(destroyedState);
        SDL_free(destroyedState);
        return;
    }
    M12_StartupMenu_Destroy(destroyedState);
    CHECK(destroyedState->dataDirDialogJob == NULL);
    SDL_free(destroyedState);
    CHECK(complete_data_dir_dialog_on_worker_and_join(callbackToken, NULL));
}

static void check_active_scan_message_requests_cancel(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    use_english_for_text_assertions(&state);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }

    state.view = M12_MENU_VIEW_MESSAGE;
    state.messageReturnView = M12_MENU_VIEW_SETTINGS;
    state.dataDirScanActive = 1;
    state.dataDirScanCancelRequested = 0;
    state.dataDirScanCancelled = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);

    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.dataDirScanActive == 1);
    CHECK(state.dataDirScanCancelRequested == 1);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "CANCELLING DATA SCAN") == 0);
}

static void check_selected_folder_scans_asynchronously(void) {
    M12_StartupMenuState state;
    M12_StartupMenuState reloadedState;
    M12_Config config;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char selectedPhysical[M12_ASSET_DATA_DIR_CAPACITY];
    int i;

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    CHECK(FSP_ResolvePhysicalPath(selectedPhysical, sizeof(selectedPhysical),
                                  dataRoot));
    if (failures) {
        return;
    }
    snprintf(dialogSelectedPath, sizeof(dialogSelectedPath), "%s", dataRoot);

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    use_english_for_text_assertions(&state);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }

    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = TEST_SETTINGS_ROW_DATA_DIR;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    (void)M12_StartupMenu_Update(&state);

    CHECK(dialogCalls == 1);
    CHECK(state.dataDirPickerActive == 0);
    CHECK(state.dataDirScanActive == 1);
    CHECK(state.dataDirScanJob != NULL);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "SCANNING GAME DATA") == 0);

    for (i = 0; i < 200 && state.dataDirScanJob != NULL; ++i) {
        (void)M12_StartupMenu_Update(&state);
        SDL_Delay(1);
    }

    CHECK(state.dataDirScanActive == 0);
    CHECK(state.dataDirScanJob == NULL);
    CHECK(state.dataDirScanCancelled == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "DATA DIRECTORY UPDATED") == 0);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus),
                 selectedPhysical) == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);

    /* The physical folder selected in the dialog must survive the config
     * write and become the root used by a fresh launcher process. */
    M12_Config_Load(&config, NULL);
    CHECK(strcmp(config.dataDir, selectedPhysical) == 0);
    M12_StartupMenu_Init(&reloadedState);
    use_english_for_text_assertions(&reloadedState);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&reloadedState.assetStatus),
                 selectedPhysical) == 0);
    M12_StartupMenu_Destroy(&reloadedState);
    M12_StartupMenu_Destroy(&state);
}

static void check_dot_dialog_result_preserves_data_directory(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char beforeDataDir[M12_ASSET_DATA_DIR_CAPACITY];
    static const char* const placeholders[] = { ".", "./", ".\\", "./.", ".//" };
    size_t i;

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    if (failures) {
        return;
    }
    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    use_english_for_text_assertions(&state);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    snprintf(beforeDataDir, sizeof(beforeDataDir), "%s",
             M12_AssetStatus_GetDataDir(&state.assetStatus));

    for (i = 0U; i < sizeof(placeholders) / sizeof(placeholders[0]); ++i) {
        snprintf(dialogSelectedPath, sizeof(dialogSelectedPath), "%s", placeholders[i]);
        state.view = M12_MENU_VIEW_SETTINGS;
        state.settingsSelectedIndex = TEST_SETTINGS_ROW_DATA_DIR;
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
        (void)M12_StartupMenu_Update(&state);

        CHECK(dialogCalls == (int)i + 1);
        CHECK(state.dataDirPickerActive == 0);
        CHECK(state.dataDirScanActive == 0);
        CHECK(state.dataDirScanJob == NULL);
        CHECK(state.messageLine1 &&
              strcmp(state.messageLine1, "DATA DIRECTORY UNCHANGED") == 0);
        CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus),
                     beforeDataDir) == 0);
        CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), ".") != 0);

        state.view = M12_MENU_VIEW_SETTINGS;
        CHECK(M12_StartupMenu_SetDataDirectory(&state, placeholders[i]) == 0);
        CHECK(state.messageLine1 &&
              strcmp(state.messageLine1, "DATA DIRECTORY UNCHANGED") == 0);
        CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus),
                     beforeDataDir) == 0);
    }

    M12_StartupMenu_Destroy(&state);
}

static void check_parent_dialog_result_is_not_a_placeholder(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char expectedParent[M12_ASSET_DATA_DIR_CAPACITY];
    char parentPath[M12_ASSET_DATA_DIR_CAPACITY];
    char originalCwd[FSP_PATH_MAX];
    int i;

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    CHECK(TEST_GETCWD(originalCwd, sizeof(originalCwd)) != NULL);
    snprintf(parentPath, sizeof(parentPath), "%s/..", dataRoot);
    CHECK(FSP_ResolvePhysicalPath(expectedParent, sizeof(expectedParent),
                                  parentPath));
    if (failures) {
        return;
    }
    CHECK(TEST_CHDIR(dataRoot) == 0);
    snprintf(dialogSelectedPath, sizeof(dialogSelectedPath), "..");

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    use_english_for_text_assertions(&state);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = TEST_SETTINGS_ROW_DATA_DIR;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    (void)M12_StartupMenu_Update(&state);

    CHECK(dialogCalls == 1);
    CHECK(state.dataDirScanActive == 1);
    CHECK(state.messageLine1 &&
          strcmp(state.messageLine1, "SCANNING GAME DATA") == 0);
    for (i = 0; i < 200 && state.dataDirScanJob != NULL; ++i) {
        (void)M12_StartupMenu_Update(&state);
        SDL_Delay(1);
    }
    CHECK(state.dataDirScanActive == 0);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus),
                 expectedParent) == 0);
    M12_StartupMenu_Destroy(&state);
    CHECK(TEST_CHDIR(originalCwd) == 0);
}

static void check_default_data_dir_scans_asynchronously(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char defaultRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char defaultPhysical[M12_ASSET_DATA_DIR_CAPACITY];
    int i;

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    CHECK(FSP_GetDefaultOriginalsDir(defaultRoot, sizeof(defaultRoot)));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    use_english_for_text_assertions(&state);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }

    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = TEST_SETTINGS_ROW_DATA_DIR;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_VALUE_LEFT);

    CHECK(dialogCalls == 0);
    CHECK(state.dataDirPickerActive == 0);
    CHECK(state.dataDirScanActive == 1);
    CHECK(state.dataDirScanJob != NULL);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "SCANNING GAME DATA") == 0);

    for (i = 0; i < 200 && state.dataDirScanJob != NULL; ++i) {
        (void)M12_StartupMenu_Update(&state);
        SDL_Delay(1);
    }

    CHECK(state.dataDirScanActive == 0);
    CHECK(state.dataDirScanJob == NULL);
    CHECK(state.dataDirScanCancelled == 0);
    CHECK(FSP_ResolvePhysicalPath(defaultPhysical, sizeof(defaultPhysical),
                                  defaultRoot));
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus),
                 defaultPhysical) == 0);
}

static void check_start_menu_keeps_saved_game_leaf_scoped(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char nexusLeaf[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    CHECK(FSP_JoinPath(nexusLeaf, sizeof(nexusLeaf), dataRoot, "nexus"));
    CHECK(FSP_CreateDirectoryRecursive(nexusLeaf));
    CHECK(seed_dm1_under_data_root(dataRoot, graphicsMd5, dungeonMd5));
    if (failures) {
        return;
    }

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_StartupMenu_InitWithDataDir(&state, nexusLeaf, NULL);
    use_english_for_text_assertions(&state);

    /* An explicit saved game leaf is a scoped launch selection.  The
     * startup menu must not promote it to the parent and scan unrelated
     * editions before opening the selected game. */
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), nexusLeaf) == 0);
    /* The parent contains DM1 fixtures, but a selected Nexus leaf must not
     * expose sibling-game availability through the scoped scan. */
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);
    CHECK(M12_AssetStatus_GetRuntimeDataDir(&state.assetStatus, "dm1")[0] != '\0');
    CHECK(strcmp(M12_AssetStatus_GetRuntimeDataDir(&state.assetStatus, "dm1"),
                 nexusLeaf) == 0);

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(NULL, NULL);
}

typedef struct RealGameRescanCoverage {
    unsigned int gameMask;
} RealGameRescanCoverage;

static int record_real_game_rescan_progress(
    const M12_AssetScanProgress* progress, void* userData) {
    RealGameRescanCoverage* coverage = (RealGameRescanCoverage*)userData;
    static const char* const ids[] = {"dm1", "csb", "dm2", "nexus", "theron"};
    size_t i;
    if (!coverage || !progress) return 1;
    for (i = 0U; i < sizeof(ids) / sizeof(ids[0]); ++i) {
        if (strcmp(progress->currentGameId, ids[i]) == 0) {
            coverage->gameMask |= 1U << i;
            break;
        }
    }
    return 1;
}

static void check_return_rescan_uses_real_five_game_corpus(void) {
    static const char* const ids[] = {"dm1", "csb", "dm2", "nexus", "theron"};
    M12_StartupMenuState state;
    RealGameRescanCoverage coverage = {0U};
    M12_StartupMenuInitOptions options;
    char scratchRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char dm1Leaf[M12_ASSET_DATA_DIR_CAPACITY];
    const char* corpus = getenv("FIRESTAFF_TEST_REAL_DATA_ROOT");
    size_t i;

    /* This opt-in integration check deliberately uses the installed original
     * corpus. It does not construct or substitute game-data fixtures. */
    if (!corpus || !corpus[0] || !FSP_DirExists(corpus)) {
        puts("  SKIP: FIRESTAFF_TEST_REAL_DATA_ROOT is not an installed corpus");
        return;
    }
    CHECK(create_build_scratch_and_data_root(scratchRoot));
    if (failures) return;
    CHECK(FSP_JoinPath(dm1Leaf, sizeof(dm1Leaf), corpus, "dm1"));
    CHECK(FSP_DirExists(dm1Leaf));
    if (failures) return;

    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&state, dm1Leaf, "dm1", &options);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 1);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "csb") == 0);

    /* Returning from a game must promote the selected game leaf back to its
     * collection root and refresh all five game statuses. */
    M12_StartupMenu_RescanAllGames(&state, record_real_game_rescan_progress,
                                   &coverage);
    CHECK((coverage.gameMask & 0x1fU) == 0x1fU);
    for (i = 0U; i < sizeof(ids) / sizeof(ids[0]); ++i) {
        CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, ids[i]) == 1);
    }
    M12_StartupMenu_Destroy(&state);
}

static void check_dot_config_migrates_to_default_data_directory(void) {
    M12_Config config;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char expected[M12_ASSET_DATA_DIR_CAPACITY];
    static const char* const placeholders[] = { ".", "./", ".\\", "./.", ".//" };
    size_t i;

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    CHECK(FSP_GetDefaultOriginalsDir(expected, sizeof(expected)));
    if (failures) {
        return;
    }
    for (i = 0U; i < sizeof(placeholders) / sizeof(placeholders[0]); ++i) {
        M12_Config_SetDefaults(&config);
        snprintf(config.dataDir, sizeof(config.dataDir), "%s", placeholders[i]);
        CHECK(M12_Config_Save(&config) == 1);
        CHECK(!config_persisted_dot_data_dir(M12_Config_GetPath(&config)));
        M12_Config_Load(&config, NULL);
        CHECK(strcmp(config.dataDir, expected) == 0);
        CHECK(strcmp(config.dataDir, ".") != 0);
    }
}

static void check_dot_environment_never_persists_as_data_directory(void) {
    M12_Config config;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char expected[M12_ASSET_DATA_DIR_CAPACITY];

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    CHECK(FSP_GetDefaultOriginalsDir(expected, sizeof(expected)));
    CHECK(test_setenv("FIRESTAFF_DATA", "."));
    if (failures) {
        (void)test_unsetenv("FIRESTAFF_DATA");
        return;
    }
    M12_Config_SetDefaults(&config);
    CHECK(strcmp(config.dataDir, ".") == 0);
    CHECK(M12_Config_Save(&config) == 1);
    M12_Config_Load(&config, NULL);
    CHECK(strcmp(config.dataDir, expected) == 0);
    CHECK(!config_persisted_dot_data_dir(M12_Config_GetPath(&config)));
    CHECK(test_unsetenv("FIRESTAFF_DATA"));
}

static void check_fresh_config_repairs_dot_in_memory(void) {
    M12_Config config;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char expected[M12_ASSET_DATA_DIR_CAPACITY];

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    CHECK(FSP_GetDefaultOriginalsDir(expected, sizeof(expected)));
    CHECK(test_setenv("FIRESTAFF_DATA", "."));
    if (failures) {
        (void)test_unsetenv("FIRESTAFF_DATA");
        return;
    }

    /* First-run is the important case: config save previously repaired the
     * file but left this live config object at '.', so Settings rendered it. */
    CHECK(M12_Config_Load(&config, NULL) == 0);
    CHECK(strcmp(config.dataDir, expected) == 0);
    CHECK(strcmp(config.dataDir, ".") != 0);
    CHECK(!config_persisted_dot_data_dir(M12_Config_GetPath(&config)));
    CHECK(test_unsetenv("FIRESTAFF_DATA"));
}

static void check_dot_asset_status_does_not_replace_saved_directory(void) {
    M12_StartupMenuState state;
    M12_Config config;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char expected[M12_ASSET_DATA_DIR_CAPACITY];

    reset_dialog_stub();
    CHECK(create_build_scratch_and_data_root(dataRoot));
    if (failures) {
        return;
    }
    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    use_english_for_text_assertions(&state);
    CHECK(M12_StartupMenu_SetDataDirectory(&state, dataRoot) == 1);
    M12_Config_Load(&config, NULL);
    snprintf(expected, sizeof(expected), "%s", config.dataDir);
    CHECK(strcmp(expected, ".") != 0);
    /* Reproduce a platform/backend status token arriving while an unrelated
     * settings change saves the launcher configuration. */
    snprintf(state.assetStatus.dataDir, sizeof(state.assetStatus.dataDir), ".");
    CHECK(strcmp(M12_StartupMenu_GetVisibleDataDir(&state), expected) == 0);
    CHECK(strcmp(M12_StartupMenu_AssetDataDir(&state), expected) == 0);
    M12_StartupMenu_SaveConfig(&state);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), expected) == 0);
    M12_Config_Load(&config, NULL);
    CHECK(strcmp(config.dataDir, expected) == 0);
    CHECK(strcmp(config.dataDir, ".") != 0);
    M12_StartupMenu_Destroy(&state);
}

static void check_active_scan_renders_progress_bar(void) {
    M12_StartupMenuState state;
    const int width = M12_ModernMenu_NativeWidth();
    const int height = M12_ModernMenu_NativeHeight();
    const size_t bytes = (size_t)width * (size_t)height * 4U;
    unsigned char* rgba = (unsigned char*)calloc(1U, bytes);
    size_t filledPixel;
    size_t emptyPixel;

    CHECK(rgba != NULL);
    if (!rgba) {
        return;
    }
    M12_StartupMenu_Init(&state);
    use_english_for_text_assertions(&state);
    state.view = M12_MENU_VIEW_MESSAGE;
    state.messageLine1 = "SCANNING GAME DATA";
    state.messageLine2 = "csb 50%  checking files";
    state.messageLine3 = "/Games/CSB";
    state.dataDirScanActive = 1;
    state.dataDirScanProgress.totalSteps = 100U;
    state.dataDirScanProgress.completedSteps = 50U;
    M12_ModernMenu_Render(&state, rgba, width, height);

    /* The bar begins at x=640, y=558 in the native 1920x1080 message panel.
     * x=700 lies in the 50% fill; x=1200 lies in its unfilled track. */
    filledPixel = ((size_t)567U * (size_t)width + 700U) * 4U;
    emptyPixel = ((size_t)567U * (size_t)width + 1200U) * 4U;
    CHECK(rgba[filledPixel + 0U] > rgba[emptyPixel + 0U]);
    CHECK(rgba[filledPixel + 1U] > rgba[emptyPixel + 1U]);
    free(rgba);
}

static void check_modern_scan_progress_standalone_render(void) {
    const int width = M12_ModernMenu_NativeWidth();
    const int height = M12_ModernMenu_NativeHeight();
    const size_t bytes = (size_t)width * (size_t)height * 4U;
    unsigned char* rgba = (unsigned char*)calloc(1U, bytes);
    M12_AssetScanProgress progress;
    size_t filledPixel;
    size_t emptyPixel;

    CHECK(rgba != NULL);
    if (!rgba) return;
    memset(&progress, 0, sizeof(progress));
    progress.active = 1;
    progress.totalSteps = 100U;
    progress.completedSteps = 50U;
    snprintf(progress.currentGameId, sizeof(progress.currentGameId), "dm1");
    snprintf(progress.currentTask, sizeof(progress.currentTask),
             "matching game versions");
    M12_ModernMenu_RenderScanProgressLocalized(&progress, 0, rgba,
                                                width, height);

    /* The lower-middle scan panel sits at y=712 on the native 1080p canvas;
     * compare the first and second halves of its 50% progress bar. */
    filledPixel = ((size_t)796U * (size_t)width + 760U) * 4U;
    emptyPixel = ((size_t)796U * (size_t)width + 1100U) * 4U;
    CHECK(rgba[filledPixel + 0U] > rgba[emptyPixel + 0U]);
    CHECK(rgba[filledPixel + 1U] > rgba[emptyPixel + 1U]);
    CHECK(strcmp(M12_StartupMenu_ScanTaskDisplayForLocale(
                     0, "matching game versions"),
                 "MATCHING GAME VERSIONS") == 0);
    free(rgba);
}

static void check_scan_progress_uses_display_names(void) {
    int languageIndex;
    const char* dm1 = M12_StartupMenu_GameDisplayTitleForLocale(0, "dm1");
    const char* csb = M12_StartupMenu_GameDisplayTitleForLocale(0, "csb");
    const char* dm2 = M12_StartupMenu_GameDisplayTitleForLocale(0, "dm2");
    const char* nexus = M12_StartupMenu_GameDisplayTitleForLocale(0, "nexus");
    const char* theron = M12_StartupMenu_GameDisplayTitleForLocale(0, "theron");
    const char* scanning = M12_StartupMenu_TranslateForLocale(
        0, "SCANNING GAME DATA");

    CHECK(dm1 && strcmp(dm1, "Dungeon Master") == 0);
    CHECK(csb && strcmp(csb, "Chaos Strikes Back") == 0);
    CHECK(dm2 && strcmp(dm2,
                        "Dungeon Master II: The Legend of Skullkeep") == 0);
    CHECK(nexus && strcmp(nexus, "Dungeon Master Nexus") == 0);
    CHECK(theron && strcmp(theron, "Theron's Quest") == 0);
    CHECK(scanning && strcmp(scanning, "SCANNING GAME DATA") == 0);
    CHECK(strcmp(dm1, "dm1") != 0);
    CHECK(strcmp(csb, "csb") != 0);
    CHECK(strcmp(dm2, "dm2") != 0);
    CHECK(strcmp(nexus, "nexus") != 0);
    CHECK(strcmp(theron, "theron") != 0);

    /* Scan names are retail names, never localized or reduced to ids. */
    for (languageIndex = 0; languageIndex < 19; ++languageIndex) {
        const char* localizedNexus =
            M12_StartupMenu_GameDisplayTitleForLocale(languageIndex, "nexus");
        const char* localizedTheron =
            M12_StartupMenu_GameDisplayTitleForLocale(languageIndex, "theron");
        CHECK(localizedNexus &&
              strcmp(localizedNexus, "Dungeon Master Nexus") == 0);
        CHECK(localizedTheron &&
              strcmp(localizedTheron, "Theron's Quest") == 0);
    }
}

int main(void) {
    char previousConfigPath[FSP_PATH_MAX] = {0};
    char previousOriginalsDir[FSP_PATH_MAX] = {0};
    const char* envValue;
    int hadConfigPath;
    int hadOriginalsDir;
    envValue = getenv("FIRESTAFF_CONFIG_PATH");
    hadConfigPath = envValue && envValue[0] != '\0';
    if (hadConfigPath) {
        snprintf(previousConfigPath, sizeof(previousConfigPath), "%s", envValue);
    }
    envValue = getenv("FIRESTAFF_ORIGINALS_DIR");
    hadOriginalsDir = envValue && envValue[0] != '\0';
    if (hadOriginalsDir) {
        snprintf(previousOriginalsDir, sizeof(previousOriginalsDir), "%s", envValue);
    }
    CHECK(test_setenv("SDL_VIDEODRIVER", "dummy"));
    CHECK(SDL_Init(0));
    check_cancel_preserves_no_data_state();
    check_active_picker_blocks_message_reentry();
    check_data_dir_callback_isolated_from_state_lifetime();
    check_active_scan_message_requests_cancel();
    check_selected_folder_scans_asynchronously();
    check_dot_dialog_result_preserves_data_directory();
    check_parent_dialog_result_is_not_a_placeholder();
    check_default_data_dir_scans_asynchronously();
    check_start_menu_keeps_saved_game_leaf_scoped();
    check_return_rescan_uses_real_five_game_corpus();
    check_dot_config_migrates_to_default_data_directory();
    check_dot_environment_never_persists_as_data_directory();
    check_fresh_config_repairs_dot_in_memory();
    check_dot_asset_status_does_not_replace_saved_directory();
    check_active_scan_renders_progress_bar();
    check_modern_scan_progress_standalone_render();
    check_scan_progress_uses_display_names();
    SDL_Quit();

    if (hadConfigPath) {
        CHECK(test_setenv("FIRESTAFF_CONFIG_PATH", previousConfigPath));
    } else {
        CHECK(test_unsetenv("FIRESTAFF_CONFIG_PATH"));
    }
    if (hadOriginalsDir) {
        CHECK(test_setenv("FIRESTAFF_ORIGINALS_DIR", previousOriginalsDir));
    } else {
        CHECK(test_unsetenv("FIRESTAFF_ORIGINALS_DIR"));
    }

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: M12 data-directory cancel/re-entry preserves no-data state and suppresses duplicate picker popups");
    return 0;
}
