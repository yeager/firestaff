/*
 * test_m12_no_data_popup_once_gate.c
 *
 * M12 launcher regression gate proving the no-data / missing-required-files
 * popup appears exactly once per relevant scan/selection state, not
 * repeatedly within the same state.
 *
 * Each scenario seeds the launcher with an empty data root (so no game is
 * available) and asserts:
 *
 *   1. M12_StartupMenu_InitWithDataDir surfaces the initial "NO GAME DATA
 *      FOUND" popup exactly once (single MESSAGE view, single non-empty
 *      messageLine1/2/3, no leftover data from a prior state).
 *   2. ACK on the MESSAGE view cleanly returns to MAIN and clears the
 *      message lines so the next interaction starts from a clean slate.
 *   3. ACK on an unavailable game card surfaces the missing-game-data
 *      popup once with the game prefix ("DM1 GAME DATA NOT FOUND") and
 *      a DATA DIR footer.
 *   4. Dismissing and re-selecting the same unavailable game card
 *      re-shows the popup exactly once per selection event (no leaked
 *      state, no doubled rendering).
 *   5. Opening the data-dir picker (without selecting a folder) and
 *      dismissing via the existing "DATA DIRECTORY UNCHANGED" path does
 *      NOT push a duplicate no-data popup on top.
 *   6. A no-op rescan (M12_StartupMenu_SetDataDirectory with the same
 *      empty directory) shows the result popup once, with the empty-data
 *      line2 ("NO VERIFIED DATA"), and the next ACK cleanly
 *      returns to MAIN with cleared message lines.
 *   7. When quick resume is armed for an unavailable game, selecting it
 *      via the quick-resume slot surfaces the missing-game-data popup
 *      once instead of looping or stacking.
 *
 * The test runs under SDL_VIDEODRIVER=dummy and stubs out the native
 * folder-picker dialog so it can drive the picker flow synchronously.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#define FIRESTAFF_ASSET_STATUS_TESTING 1
#include "asset_status_m12.h"
#include "menu_startup_m12.h"

#include <SDL3/SDL_dialog.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#include <sys/stat.h>
#define MKDIR(path) _mkdir(path)
static int test_dir_exists(const char* path) {
    struct _stat st;
    return path && _stat(path, &st) == 0 && (st.st_mode & _S_IFDIR) != 0;
}
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
static int test_dir_exists(const char* path) {
    struct stat st;
    return path && stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}
static int test_setenv(const char* name, const char* value) {
    return setenv(name, value, 1) == 0;
}
static char* test_mkdtemp(char* templ) {
    return mkdtemp(templ);
}
#endif

/* ------------------------------------------------------------------------- */
/* Folder-picker dialog stub: SDL_ShowOpenFolderDialog is intercepted so the
 * picker flow can be exercised in a headless dummy-driver run. The stub
 * records how many times the picker was opened, the default location passed
 * in, and can either immediately cancel (cancel-on-next-call) or hold the
 * callback for an explicit cancel later. */

static int dialogCalls = 0;
static int dialogHoldOpen = 0;
static int dialogCancelOnNext = 0;
static char dialogDefaultLocation[M12_ASSET_DATA_DIR_CAPACITY];
static SDL_DialogFileCallback dialogPendingCallback = NULL;
static void* dialogPendingUserdata = NULL;

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
    if (dialogCancelOnNext) {
        dialogCancelOnNext = 0;
        if (callback) {
            callback(userdata, canceledSelection, -1);
        }
        return;
    }
    /* Default behaviour: do not invoke the callback — the test will call
     * complete_pending_dialog_cancel() explicitly when it needs the
     * launcher's "DATA DIRECTORY UNCHANGED" path. */
    dialogPendingCallback = callback;
    dialogPendingUserdata = userdata;
}

static void complete_pending_dialog_cancel(void) {
    const char* canceledSelection[] = { NULL };
    SDL_DialogFileCallback callback = dialogPendingCallback;
    void* dialogUserdata = dialogPendingUserdata;
    dialogPendingCallback = NULL;
    dialogPendingUserdata = NULL;
    dialogHoldOpen = 0;
    if (callback) {
        callback(dialogUserdata, canceledSelection, -1);
    }
}

static void reset_dialog_stub(void) {
    dialogCalls = 0;
    dialogHoldOpen = 0;
    dialogCancelOnNext = 0;
    dialogPendingCallback = NULL;
    dialogPendingUserdata = NULL;
    dialogDefaultLocation[0] = '\0';
}

/* ------------------------------------------------------------------------- */
/* Test scaffolding. */

enum { SETTINGS_ROW_DATA_DIR = 15 };

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static int make_child_dir(const char* parent,
                          const char* leaf,
                          char out[M12_ASSET_DATA_DIR_CAPACITY]) {
    snprintf(out, M12_ASSET_DATA_DIR_CAPACITY, "%s/%s", parent, leaf);
    return MKDIR(out) == 0;
}

static int ensure_nested_dir(const char* root, const char* rel) {
    char partial[M12_ASSET_DATA_DIR_CAPACITY];
    const char* cursor;

    if (!root || !rel) return 0;
    snprintf(partial, sizeof(partial), "%s", root);
    cursor = rel;
    while (*cursor) {
        const char* slash = strchr(cursor, '/');
        size_t segmentLen = slash ? (size_t)(slash - cursor) : strlen(cursor);
        if (segmentLen > 0U) {
            size_t len = strlen(partial);
            if (len + 1U + segmentLen >= sizeof(partial)) {
                return 0;
            }
            partial[len++] = '/';
            memcpy(partial + len, cursor, segmentLen);
            partial[len + segmentLen] = '\0';
            if (MKDIR(partial) != 0 && !test_dir_exists(partial)) {
                return 0;
            }
        }
        if (!slash) break;
        cursor = slash + 1;
    }
    return test_dir_exists(partial);
}

static int write_payload_with_md5(const char* root,
                                  const char* rel,
                                  const char* payload,
                                  char outMd5[M12_ASSET_MD5_CAPACITY]) {
    char path[M12_ASSET_DATA_DIR_CAPACITY];
    const char* slash;
    FILE* fp;
    size_t len;

    if (!root || !rel || !payload || !outMd5) return 0;
    slash = strrchr(rel, '/');
    if (slash) {
        char dirRel[M12_ASSET_DATA_DIR_CAPACITY];
        size_t dirLen = (size_t)(slash - rel);
        if (dirLen >= sizeof(dirRel)) return 0;
        memcpy(dirRel, rel, dirLen);
        dirRel[dirLen] = '\0';
        if (!ensure_nested_dir(root, dirRel)) return 0;
    }
    snprintf(path, sizeof(path), "%s/%s", root, rel);
    fp = fopen(path, "wb");
    if (!fp) return 0;
    len = strlen(payload);
    if (fwrite(payload, 1U, len, fp) != len) {
        fclose(fp);
        return 0;
    }
    if (fclose(fp) != 0) return 0;
    return m12_file_md5_hex(path, outMd5) != 0;
}

static int isolate_home_and_data_root(char dataRoot[M12_ASSET_DATA_DIR_CAPACITY]) {
#ifdef _WIN32
    char home[256];
    snprintf(home, sizeof(home), ".\\firestaff_m12_no_data_popup_home_%lu",
             (unsigned long)rand());
    if (MKDIR(home) != 0) {
        return 0;
    }
    if (!test_setenv("HOME", home) || !test_setenv("USERPROFILE", home)) {
        return 0;
    }
    return make_child_dir(home, "empty-data-root", dataRoot);
#else
    char homeTemplate[] = "/tmp/firestaff_m12_no_data_popup_home_XXXXXX";
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

static int has_text_prefix(const char* haystack, const char* prefix) {
    if (!haystack || !prefix) {
        return 0;
    }
    while (*prefix) {
        if (*haystack != *prefix) {
            return 0;
        }
        ++haystack;
        ++prefix;
    }
    return 1;
}

static int popup_lines_are_cleared(const M12_StartupMenuState* state) {
    if (!state) {
        return 0;
    }
    /* The renderer reads through the messageLineN pointers (which are
     * either the storage buffers or a static string), so the popup is
     * only truly "cleared" once every active pointer is the empty
     * string. The storage buffers are internal scratch and may retain
     * the previous text; that does not affect the displayed popup. */
    if (state->messageLine1 && state->messageLine1[0] != '\0') return 0;
    if (state->messageLine2 && state->messageLine2[0] != '\0') return 0;
    if (state->messageLine3 && state->messageLine3[0] != '\0') return 0;
    return 1;
}

static int launcher_has_clean_main_view(const M12_StartupMenuState* state) {
    if (!state) return 0;
    return state->view == M12_MENU_VIEW_MAIN &&
           state->launchRequested == 0 &&
           state->quickResumeLaunchRequested == 0 &&
           state->dataDirPickerActive == 0 &&
           popup_lines_are_cleared(state);
}

static int launcher_has_clean_settings_view(const M12_StartupMenuState* state) {
    if (!state) return 0;
    return state->view == M12_MENU_VIEW_SETTINGS &&
           state->launchRequested == 0 &&
           state->quickResumeLaunchRequested == 0 &&
           state->dataDirPickerActive == 0 &&
           popup_lines_are_cleared(state);
}

/* Helper: dismiss whatever popup is currently visible via the same BACK /
 * ACCEPT / ACTION keys the real launcher uses. The destination is whatever
 * messageReturnView the launcher captured when the popup was opened. */
static void dismiss_message(M12_StartupMenuState* state) {
    if (!state || state->view != M12_MENU_VIEW_MESSAGE) return;
    if (state->dataDirPickerActive) return;
    M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
}

/* ------------------------------------------------------------------------- */
/* Scenario 1 + 2: initial no-data popup appears once and dismissal returns
 * the launcher to a clean MAIN view. */

static void check_initial_no_data_popup_appears_once(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    const char* line1;

    reset_dialog_stub();
    CHECK(isolate_home_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);

    /* Initial no-data popup must be visible exactly once. */
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(state.dataDirPickerActive == 0);

    line1 = state.messageLine1 ? state.messageLine1 : "";
    CHECK(has_text_prefix(line1, "NO GAME DATA FOUND"));

    /* Repeated calls into the launcher must NOT stack or re-show the
     * popup while the dismiss path is unused — the popup state was set
     * during init and must not be refreshed by later internal calls. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    line1 = state.messageLine1 ? state.messageLine1 : "";
    CHECK(has_text_prefix(line1, "NO GAME DATA FOUND"));

    /* Dismiss once — view must return to MAIN with cleared message lines. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_MAIN);
    CHECK(state.launchRequested == 0);
    CHECK(launcher_has_clean_main_view(&state));

    /* A no-op BACK from MAIN must not re-surface the initial popup. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    CHECK(state.shouldExit == 1);
}

/* ------------------------------------------------------------------------- */
/* Scenario 3 + 4: ACK on an unavailable game card surfaces the missing
 * popup once per selection, and re-selecting after dismiss re-shows it
 * once with a fresh message — never stacked or duplicated. */

static void check_unavailable_game_popup_appears_once_per_selection(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    const char* line1;
    int dm1CardIndex;

    reset_dialog_stub();
    CHECK(isolate_home_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    CHECK(launcher_has_clean_main_view(&state));

    /* DM1 is the first game card by catalog order. */
    dm1CardIndex = 0;
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);
    {
        const M12_MenuEntry* entry = M12_StartupMenu_GetEntry(&state, dm1CardIndex);
        CHECK(entry && entry->gameId && strcmp(entry->gameId, "dm1") == 0);
        CHECK(entry && entry->available == 0);
    }

    /* First selection event. */
    state.selectedIndex = dm1CardIndex;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.launchRequested == 0);
    line1 = state.messageLine1 ? state.messageLine1 : "";
    CHECK(has_text_prefix(line1, "DM1"));
    CHECK(strstr(line1, "GAME DATA NOT FOUND") != NULL);
    /* The missing-files line lists verified-but-missing roles. */
    CHECK(state.messageLine2 && state.messageLine2[0] != '\0');
    /* Footer shows the active data directory so the user can fix it. */
    CHECK(state.messageLine3 && has_text_prefix(state.messageLine3, "DATA DIR:"));

    /* No-op DOWN/UP must not re-render or duplicate the popup content. */
    {
        const char* line1Before = state.messageLine1;
        const char* line2Before = state.messageLine2;
        const char* line3Before = state.messageLine3;
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
        CHECK(state.view == M12_MENU_VIEW_MESSAGE);
        CHECK(state.messageLine1 == line1Before);
        CHECK(state.messageLine2 == line2Before);
        CHECK(state.messageLine3 == line3Before);
    }

    /* Dismiss. */
    dismiss_message(&state);
    CHECK(launcher_has_clean_main_view(&state));

    /* Second selection event (the user navigated back to DM1). The popup
     * must appear once again — fresh, not stacked or duplicated. */
    state.selectedIndex = dm1CardIndex;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    line1 = state.messageLine1 ? state.messageLine1 : "";
    CHECK(has_text_prefix(line1, "DM1"));
    CHECK(strstr(line1, "GAME DATA NOT FOUND") != NULL);

    /* Dismiss via BACK this time to also cover the BACK-from-message path. */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    CHECK(launcher_has_clean_main_view(&state));
}

/* ------------------------------------------------------------------------- */
/* Scenario 5: opening the data-dir picker and cancelling via the existing
 * "DATA DIRECTORY UNCHANGED" path must NOT push a duplicate no-data popup
 * on top of the result popup. The picker path already gates via the
 * dataDirPickerActive flag, but this gate proves the contract. */

static void check_data_dir_picker_cancel_does_not_re_show_no_data(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    const char* line1;
    int dialogCallsBefore;

    reset_dialog_stub();
    dialogHoldOpen = 1; /* The picker callback is held until we cancel. */
    CHECK(isolate_home_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    CHECK(launcher_has_clean_main_view(&state));

    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsSelectedIndex = SETTINGS_ROW_DATA_DIR;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);

    /* Picker was opened exactly once and the launcher is parked on the
     * "CHOOSE GAME DATA FOLDER" message while the picker is open. */
    CHECK(dialogCalls == 1);
    CHECK(state.dataDirPickerActive == 1);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    line1 = state.messageLine1 ? state.messageLine1 : "";
    CHECK(strcmp(line1, "CHOOSE GAME DATA FOLDER") == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);

    /* The picker-pending state must NOT allow ACKing into the no-data
     * popup. Repeated ACKs while the picker is open must not advance
     * the launcher to a different popup. */
    dialogCallsBefore = dialogCalls;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(dialogCalls == dialogCallsBefore);
    CHECK(state.dataDirPickerActive == 1);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    line1 = state.messageLine1 ? state.messageLine1 : "";
    CHECK(strcmp(line1, "CHOOSE GAME DATA FOLDER") == 0);

    /* Cancel the picker. The launcher must surface the
     * "DATA DIRECTORY UNCHANGED" popup once — not the original no-data
     * popup — and the picker must be marked inactive. */
    complete_pending_dialog_cancel();
    CHECK(state.dataDirPickerActive == 0);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    line1 = state.messageLine1 ? state.messageLine1 : "";
    CHECK(strcmp(line1, "DATA DIRECTORY UNCHANGED") == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);

    /* Dismiss the result popup. The original no-data popup must NOT leak
     * through, and because the picker was opened from Settings the result
     * returns to Settings rather than flattening the user back to MAIN. */
    dismiss_message(&state);
    CHECK(launcher_has_clean_settings_view(&state));
}

/* ------------------------------------------------------------------------- */
/* Scenario 6: a no-op rescan via M12_StartupMenu_SetDataDirectory must
 * surface the "NO VERIFIED DATA" result popup exactly once,
 * and dismissal must return cleanly to MAIN. */

static void check_rescan_with_empty_dir_shows_no_data_result_once(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    const char* line1;
    const char* line2;

    reset_dialog_stub();
    CHECK(isolate_home_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    CHECK(launcher_has_clean_main_view(&state));

    /* Rescan the same empty root directly via the public API. */
    CHECK(M12_StartupMenu_SetDataDirectory(&state, dataRoot) == 1);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.dataDirPickerActive == 0);
    line1 = state.messageLine1 ? state.messageLine1 : "";
    line2 = state.messageLine2 ? state.messageLine2 : "";
    CHECK(strcmp(line1, "DATA DIRECTORY UPDATED") == 0);
    CHECK(strcmp(line2, "NO VERIFIED DATA") == 0);
    CHECK(state.messageLine3 && has_text_prefix(state.messageLine3, "DATA DIR:"));

    /* Repeated ACKs while the result popup is open must not stack a
     * second no-data popup on top. */
    {
        const char* line1Before = state.messageLine1;
        const char* line2Before = state.messageLine2;
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
        CHECK(state.view == M12_MENU_VIEW_MESSAGE);
        CHECK(state.messageLine1 == line1Before);
        CHECK(state.messageLine2 == line2Before);
    }

    /* Dismiss cleanly. */
    dismiss_message(&state);
    CHECK(launcher_has_clean_main_view(&state));
}

/* ------------------------------------------------------------------------- */
/* Scenario 7: quick resume armed for an unavailable game must surface
 * the missing-game-data popup once when the user ACKs the resume slot. */

static void check_quick_resume_unavailable_shows_missing_popup_once(void) {
    M12_StartupMenuState state;
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    const char* line1;

    reset_dialog_stub();
    CHECK(isolate_home_and_data_root(dataRoot));
    if (failures) {
        return;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataRoot, NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    }
    CHECK(launcher_has_clean_main_view(&state));

    /* Arm quick resume for an unavailable game. The data root is empty so
     * the DM1 game card is unavailable; force the slot and gameId. */
    state.quickResumeAvailable = 1;
    snprintf(state.quickResumeGameId, sizeof(state.quickResumeGameId), "dm1");
    state.selectedIndex = -1;

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    line1 = state.messageLine1 ? state.messageLine1 : "";
    CHECK(has_text_prefix(line1, "DM1"));
    CHECK(strstr(line1, "GAME DATA NOT FOUND") != NULL);

    /* Dismiss — must return to MAIN with cleared message lines and no
     * lingering quick-resume launch request. */
    dismiss_message(&state);
    CHECK(state.view == M12_MENU_VIEW_MAIN);
    CHECK(state.launchRequested == 0);
    CHECK(state.quickResumeLaunchRequested == 0);
    CHECK(popup_lines_are_cleared(&state));
}

/* ------------------------------------------------------------------------- */
/* Scenario 8: changing the configured data root from a complete synthetic
 * recursive layout to a partial layout must clear stale availability for
 * multiple games and route each game card through the centralized required
 * file popup. This keeps the folder-picker/config path tied to
 * M12_AssetStatus_Scan rather than filename or prior-state shortcuts. */

static void check_data_root_switch_partial_required_pairs_for_dm1_dm2(void) {
    M12_StartupMenuState state;
    char homeRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char fullRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char partialRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char dm1GraphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dm1DungeonMd5[M12_ASSET_MD5_CAPACITY];
    char dm2GraphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dm2DungeonMd5[M12_ASSET_MD5_CAPACITY];
    const M12_MenuEntry* dm1Entry;
    const M12_MenuEntry* dm2Entry;

    reset_dialog_stub();
    CHECK(isolate_home_and_data_root(homeRoot));
    if (failures) return;
    CHECK(make_child_dir(homeRoot, "complete-recursive-root", fullRoot));
    CHECK(make_child_dir(homeRoot, "partial-recursive-root", partialRoot));
    if (failures) return;

    CHECK(write_payload_with_md5(fullRoot,
                                 "loose/deep/dm1/odd-name-gfx.bin",
                                 "m12 popup gate DM1 graphics complete\n",
                                 dm1GraphicsMd5));
    CHECK(write_payload_with_md5(fullRoot,
                                 "elsewhere/dm1/odd-name-dungeon.bin",
                                 "m12 popup gate DM1 dungeon complete\n",
                                 dm1DungeonMd5));
    CHECK(write_payload_with_md5(fullRoot,
                                 "dm2/deep/not-graphics.dat",
                                 "m12 popup gate DM2 graphics complete\n",
                                 dm2GraphicsMd5));
    CHECK(write_payload_with_md5(fullRoot,
                                 "dm2/other/not-dungeon.dat",
                                 "m12 popup gate DM2 dungeon complete\n",
                                 dm2DungeonMd5));
    if (failures) return;

    /* Partial root deliberately crosses the missing role per game:
     * DM1 has graphics only, DM2 has dungeon only. The files keep
     * non-canonical names and nested locations so recursive hash
     * scanning remains the only way to find them. */
    CHECK(write_payload_with_md5(partialRoot,
                                 "partial/dm1/graphics-only.bin",
                                 "m12 popup gate DM1 graphics complete\n",
                                 dm1GraphicsMd5));
    CHECK(write_payload_with_md5(partialRoot,
                                 "partial/dm2/dungeon-only.bin",
                                 "m12 popup gate DM2 dungeon complete\n",
                                 dm2DungeonMd5));
    if (failures) return;

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(dm1GraphicsMd5,
                                                           dm1DungeonMd5);
    M12_AssetStatus_TestSetDm2SyntheticHashes(dm2GraphicsMd5, dm2DungeonMd5);

    M12_StartupMenu_InitWithDataDir(&state, fullRoot, NULL);
    CHECK(state.view == M12_MENU_VIEW_MAIN);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), fullRoot) == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 1);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm2") == 1);
    dm1Entry = M12_StartupMenu_GetEntry(&state, 0);
    dm2Entry = M12_StartupMenu_GetEntry(&state, 2);
    CHECK(dm1Entry && dm1Entry->available == 1);
    CHECK(dm2Entry && dm2Entry->available == 1);

    CHECK(M12_StartupMenu_SetDataDirectory(&state, partialRoot) == 1);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.messageLine1 && strcmp(state.messageLine1, "DATA DIRECTORY UPDATED") == 0);
    CHECK(state.messageLine2 && strcmp(state.messageLine2, "FILES FOUND") == 0);
    CHECK(strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), partialRoot) == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm1") == 0);
    CHECK(M12_AssetStatus_GameAvailable(&state.assetStatus, "dm2") == 0);
    dm1Entry = M12_StartupMenu_GetEntry(&state, 0);
    dm2Entry = M12_StartupMenu_GetEntry(&state, 2);
    CHECK(dm1Entry && dm1Entry->available == 0);
    CHECK(dm2Entry && dm2Entry->available == 0);
    dismiss_message(&state);
    CHECK(launcher_has_clean_main_view(&state));

    state.selectedIndex = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.launchRequested == 0);
    CHECK(state.messageLine1 && has_text_prefix(state.messageLine1, "DM1"));
    CHECK(state.messageLine2 && strstr(state.messageLine2, "DUNGEON.DAT") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "GRAPHICS.DAT") == NULL);
    dismiss_message(&state);
    CHECK(launcher_has_clean_main_view(&state));

    state.selectedIndex = 2;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.view == M12_MENU_VIEW_MESSAGE);
    CHECK(state.launchRequested == 0);
    CHECK(state.messageLine1 && has_text_prefix(state.messageLine1, "DM2"));
    CHECK(state.messageLine2 && strstr(state.messageLine2, "GRAPHICS.DAT") != NULL);
    CHECK(state.messageLine2 && strstr(state.messageLine2, "DUNGEON.DAT") == NULL);
    dismiss_message(&state);
    CHECK(launcher_has_clean_main_view(&state));

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
}

int main(void) {
    CHECK(test_setenv("SDL_VIDEODRIVER", "dummy"));

    check_initial_no_data_popup_appears_once();
    check_unavailable_game_popup_appears_once_per_selection();
    check_data_dir_picker_cancel_does_not_re_show_no_data();
    check_rescan_with_empty_dir_shows_no_data_result_once();
    check_quick_resume_unavailable_shows_missing_popup_once();
    check_data_root_switch_partial_required_pairs_for_dm1_dm2();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: M12 no-data / missing-required-files popup appears once per scan/selection state");
    return 0;
}
