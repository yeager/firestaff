#include "menu_startup_m12.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#define test_pid _getpid
#define test_setenv(name, value) _putenv_s((name), (value))
#else
#include <unistd.h>
#define test_pid getpid
#define test_setenv(name, value) setenv((name), (value), 1)
#endif

static int failures;

static void check(int condition, const char *name)
{
    if (condition) {
        printf("PASS %s\n", name);
    } else {
        printf("FAIL %s\n", name);
        ++failures;
    }
}

static int enter_game_select(M12_StartupMenuState *state)
{
    state->mainMenuSelected = M12_MAIN_MENU_PLAY;
    m12_redesigned_handle_input(state, 0, 0, 0, 0, 1, 0);
    return m12_get_nav_level() == M12_NAV_GAME_SELECT &&
           state->view == M12_MENU_VIEW_MAIN;
}

int main(void)
{
    M12_StartupMenuState state;
    char root[512];
    char data_dir[640];
    char screenshots_dir[640];
    const char *game_ids[] = {"dm1", "csb", "dm2"};
    int index;

#ifdef _WIN32
    snprintf(root, sizeof(root), "%s\\firestaff-m12-flow-%ld",
             getenv("TEMP") ? getenv("TEMP") : ".", (long)test_pid());
#else
    snprintf(root, sizeof(root), "/tmp/firestaff-m12-flow-%ld",
             (long)test_pid());
#endif
    snprintf(data_dir, sizeof(data_dir), "%s/data", root);
    snprintf(screenshots_dir, sizeof(screenshots_dir), "%s/screenshots", root);
    if (!FSP_CreateDirectoryRecursive(root) ||
        !FSP_CreateDirectoryRecursive(data_dir)) {
        fprintf(stderr, "FAIL could not create isolated, empty launcher data root\n");
        return 2;
    }
    if (test_setenv("HOME", root) != 0 ||
        test_setenv("APPDATA", root) != 0 ||
        test_setenv("XDG_CONFIG_HOME", root) != 0 ||
        test_setenv("FIRESTAFF_SCREENSHOTS_DIR", screenshots_dir) != 0) {
        fprintf(stderr, "FAIL could not isolate launcher test environment\n");
        return 2;
    }

    M12_StartupMenu_InitWithDataDir(&state, data_dir, NULL);
    if (state.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    }
    check(state.view == M12_MENU_VIEW_MAIN &&
              m12_get_nav_level() == M12_NAV_MAIN &&
              state.launchRequested == 0,
          "empty original-data directory opens safely at the main menu");
    check(enter_game_select(&state),
          "Play opens the hierarchical game selector");

    for (index = 0; index < 3; ++index) {
        state.gameSelectSelected = (M12_GameSelectItem)index;
        m12_redesigned_handle_input(&state, 0, 0, 0, 0, 1, 0);
        check(state.view == M12_MENU_VIEW_MESSAGE &&
                  state.messageIsMissingGameData == 1 &&
                  strcmp(state.messageGameId, game_ids[index]) == 0 &&
                  state.launchRequested == 0 &&
                  state.messageLine1 && state.messageLine1[0] != '\0' &&
                  state.messageLine2 && state.messageLine2[0] != '\0',
              game_ids[index]);
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
        check(state.view == M12_MENU_VIEW_MAIN &&
                  m12_get_nav_level() == M12_NAV_GAME_SELECT &&
                  state.gameSelectSelected == (M12_GameSelectItem)index,
              "missing-data popup returns to the selected game card");
    }

    /* The product's active launcher is the card -> platform -> presentation
     * flow in M12_StartupMenu_HandleInput.  Exercise its real missing-media
     * boundary separately from the older hierarchical navigation helper
     * above, so an available-looking card can never launch on partial or
     * absent originals. */
    for (index = 0; index < 3; ++index) {
        state.view = M12_MENU_VIEW_MAIN;
        state.selectedIndex = index;
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
        check(state.view == M12_MENU_VIEW_GAME_OPTIONS &&
                  state.activatedIndex == index &&
                  state.gameCardFlowStage == 0 &&
                  state.launchRequested == 0,
              "active game card opens platform selection without launching");
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
        check(state.view == M12_MENU_VIEW_MESSAGE &&
                  state.messageIsMissingGameData == 1 &&
                  strcmp(state.messageGameId, game_ids[index]) == 0 &&
                  state.launchRequested == 0,
              "active platform selection blocks launch without original media");
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
        check(state.view == M12_MENU_VIEW_GAME_OPTIONS &&
                  state.activatedIndex == index,
              "active missing-media popup returns to the selected game's platform card");
        M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
        check(state.view == M12_MENU_VIEW_MAIN &&
                  state.selectedIndex == index,
              "active platform card returns to its selected game entry");
    }

    m12_redesigned_handle_input(&state, 0, 0, 0, 0, 0, 1);
    check(state.view == M12_MENU_VIEW_MAIN &&
              m12_get_nav_level() == M12_NAV_MAIN,
          "Escape from game selection returns to the main menu");

    state.mainMenuSelected = M12_MAIN_MENU_SETTINGS;
    m12_redesigned_handle_input(&state, 0, 0, 0, 0, 1, 0);
    check(state.view == M12_MENU_VIEW_SETTINGS &&
              m12_get_nav_level() == M12_NAV_SETTINGS,
          "Settings opens from the main menu");
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    check(state.view == M12_MENU_VIEW_MAIN &&
              m12_get_nav_level() == M12_NAV_MAIN,
          "Settings Escape returns to the main menu");

    state.mainMenuSelected = M12_MAIN_MENU_EXTRAS;
    m12_redesigned_handle_input(&state, 0, 0, 0, 0, 1, 0);
    state.extrasSelected = M12_EXTRAS_SPELLS;
    m12_redesigned_handle_input(&state, 0, 0, 0, 0, 1, 0);
    check(state.view == M12_MENU_VIEW_MESSAGE &&
              state.messageLine1 && state.messageLine1[0] != '\0' &&
              state.messageLine2 && state.messageLine2[0] != '\0' &&
              state.launchRequested == 0,
          "unavailable Extras opens an explanatory popup without launching");
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    check(state.view == M12_MENU_VIEW_MAIN,
          "unavailable Extras popup can be dismissed");

    state.mainMenuSelected = M12_MAIN_MENU_QUIT;
    m12_redesigned_handle_input(&state, 0, 0, 0, 0, 1, 0);
    check(state.shouldExit == 1,
          "Quit requests exit from the main menu");

    M12_StartupMenu_Destroy(&state);
    printf("# summary: %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
