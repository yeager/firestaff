#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "menu_startup_m12.h"
#include "fs_portable_compat.h"

#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_timer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#define MKDIR(path) _mkdir(path)
static int test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value) == 0;
}
static char* test_mkdtemp(char* templ) {
    char* marker = strstr(templ, "XXXXXX");
    int i;
    if (!marker) return NULL;
    for (i = 0; i < 1000; ++i) {
        snprintf(marker, 7, "%06ld", ((long)_getpid() + i) % 1000000L);
        if (_mkdir(templ) == 0) return templ;
    }
    return NULL;
}
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0700)
static int test_setenv(const char* name, const char* value) {
    return setenv(name, value, 1) == 0;
}
static char* test_mkdtemp(char* templ) {
    return mkdtemp(templ);
}
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

static void reset_dialog_stub(void) {
    dialogCalls = 0;
    dialogHoldOpen = 0;
    dialogPendingCallback = NULL;
    dialogPendingUserdata = NULL;
    dialogDefaultLocation[0] = '\0';
    dialogSelectedPath[0] = '\0';
}

static int make_child_dir(const char* parent,
                          const char* leaf,
                          char out[M12_ASSET_DATA_DIR_CAPACITY]) {
    snprintf(out, M12_ASSET_DATA_DIR_CAPACITY, "%s/%s", parent, leaf);
    return MKDIR(out) == 0;
}

static int isolate_home_and_data_root(char dataRoot[M12_ASSET_DATA_DIR_CAPACITY]) {
#ifdef _WIN32
    char home[256];
    snprintf(home, sizeof(home), ".\\firestaff_m12_data_dir_cancel_home_%lu",
             (unsigned long)rand());
    if (MKDIR(home) != 0) {
        return 0;
    }
    if (!test_setenv("HOME", home) || !test_setenv("USERPROFILE", home)) {
        return 0;
    }
    return make_child_dir(home, "empty-data-root", dataRoot);
#else
    char homeTemplate[] = "/tmp/firestaff_m12_data_dir_cancel_home_XXXXXX";
    char* home = test_mkdtemp(homeTemplate);
    if (!home) {
        return 0;
    }
    if (!test_setenv("HOME", home)) {
        return 0;
    }
    return make_child_dir(home, "empty-data-root", dataRoot);
#endif
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
    CHECK(isolate_home_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
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

    CHECK(dialogCalls == 1);
    CHECK(strcmp(dialogDefaultLocation, beforeDataDir) == 0);
    CHECK(state.dataDirPickerActive == 0);
    CHECK(state.dataDirScanActive == 0);
    CHECK(state.dataDirScanCancelRequested == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
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
    CHECK(isolate_home_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    dialogHoldOpen = 1;
    dialogPendingCallback = NULL;
    dialogPendingUserdata = NULL;

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }

    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = TEST_SETTINGS_ROW_DATA_DIR;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);

    CHECK(dialogCalls == 1);
    CHECK(dialogPendingCallback != NULL);
    CHECK(dialogPendingUserdata == &state);
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

static void check_active_scan_message_requests_cancel(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];

    reset_dialog_stub();
    CHECK(isolate_home_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
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
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    int i;

    reset_dialog_stub();
    CHECK(isolate_home_and_data_root(dataRoot));
    if (failures) {
        return;
    }
    snprintf(dialogSelectedPath, sizeof(dialogSelectedPath), "%s", dataRoot);

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }

    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = TEST_SETTINGS_ROW_DATA_DIR;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);

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
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), dataRoot) == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);
}

static void check_default_data_dir_scans_asynchronously(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char defaultRoot[M12_ASSET_DATA_DIR_CAPACITY];
    int i;

    reset_dialog_stub();
    CHECK(isolate_home_and_data_root(dataRoot));
    CHECK(FSP_GetDefaultOriginalsDir(defaultRoot, sizeof(defaultRoot)));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
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
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), defaultRoot) == 0);
}

int main(void) {
    CHECK(test_setenv("SDL_VIDEODRIVER", "dummy"));
    check_cancel_preserves_no_data_state();
    check_active_picker_blocks_message_reentry();
    check_active_scan_message_requests_cancel();
    check_selected_folder_scans_asynchronously();
    check_default_data_dir_scans_asynchronously();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: M12 data-directory cancel/re-entry preserves no-data state and suppresses duplicate picker popups");
    return 0;
}
